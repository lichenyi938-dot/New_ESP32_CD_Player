/**
 *
 * 枚举设备和实现usb主机基础命令
 * Implement basic commands for USB hosts and identify devices
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_intr_alloc.h"      // for ESP_INTR_FLAG_LEVEL1 (IDF v5)

#include "usbhost_driver.h"

extern SemaphoreHandle_t scsiExeLock;

#define CLASS_DRIVER_ACTION_NEW_DEV  0x01
#define CLASS_DRIVER_ACTION_CLOSE_DEV 0x02

#define DAEMON_TASK_PRIORITY 2
#define CLIENT_TASK_PRIORITY 3

QueueHandle_t queue_client = NULL;
usbhost_driver_t usbhost_driverObj = {0};

static inline void senMsgToClientTask(uint8_t msg)
{
    xQueueSend(queue_client, &msg, 0);
}

/* ------------------------ 打开/关闭设备 & 描述符处理 ------------------------ */

esp_err_t usbhost_openDevice(void)
{
    // 打开设备
    ESP_LOGI("client_task", "Open device");
    printf("Device addr: %d\n", usbhost_driverObj.dev_addr);

    usb_host_device_open(usbhost_driverObj.handle_client,
                         usbhost_driverObj.dev_addr,
                         &usbhost_driverObj.handle_device);

    // 读取设备信息
    ESP_LOGI("client_task", "Get device information");

    usb_device_info_t dev_info;
    usb_host_device_info(usbhost_driverObj.handle_device, &dev_info);

    printf("USB speed: %s speed\n", (dev_info.speed == USB_SPEED_LOW) ? "Low" : "Full");
    printf("bConfigurationValue: %d\n", dev_info.bConfigurationValue);
    printf("string desc manufacturer: ");
    usb_print_string_descriptor(dev_info.str_desc_manufacturer);
    printf("string desc product:      ");
    usb_print_string_descriptor(dev_info.str_desc_product);
    printf("string desc sn:           ");
    usb_print_string_descriptor(dev_info.str_desc_serial_num);

    // 读取设备描述符
    ESP_LOGI("client_task", "Get device descriptor");
    const usb_device_desc_t *dev_desc;
    usb_host_get_device_descriptor(usbhost_driverObj.handle_device, &dev_desc);
    usb_print_device_descriptor(dev_desc);

    // 读取配置/接口/端点描述符
    ESP_LOGI("client_task", "Get config descriptor");
    const usb_config_desc_t *config_desc;
    usb_host_get_active_config_descriptor(usbhost_driverObj.handle_device, &config_desc);
    usb_print_config_descriptor(config_desc, NULL);

    int offset = 0;
    const usb_standard_desc_t *each_desc = (const usb_standard_desc_t *)config_desc;
    while (each_desc != NULL)
    {
        if (each_desc->bDescriptorType == USB_B_DESCRIPTOR_TYPE_INTERFACE) {
            usbhost_driverObj.desc_interface = (usb_intf_desc_t *)each_desc;
        } else if (each_desc->bDescriptorType == USB_B_DESCRIPTOR_TYPE_ENDPOINT) {
            uint8_t epAddr = ((usb_ep_desc_t *)each_desc)->bEndpointAddress;
            uint8_t type   = ((usb_ep_desc_t *)each_desc)->bmAttributes;
            if (epAddr & 0x80) {           // IN
                if ((type & 0x3) == 2)     // BULK
                    usbhost_driverObj.desc_ep_in = (usb_ep_desc_t *)each_desc;
            } else {                       // OUT
                if ((type & 0x3) == 2)
                    usbhost_driverObj.desc_ep_out = (usb_ep_desc_t *)each_desc;
            }
        }

        if (usbhost_driverObj.desc_interface && usbhost_driverObj.desc_ep_out && usbhost_driverObj.desc_ep_in) {
            break;
        }
        each_desc = usb_parse_next_descriptor(each_desc, config_desc->wTotalLength, &offset);
    }

    // 判断设备类型 MSC Bulk-Only
    if (usbhost_driverObj.desc_interface->bInterfaceClass == 0x08 &&             // Mass Storage
        ((usbhost_driverObj.desc_interface->bInterfaceSubClass == 0x05) ||        // Obsolete SFF-8070I
         (usbhost_driverObj.desc_interface->bInterfaceSubClass == 0x06) ||        // SCSI transparent
         (usbhost_driverObj.desc_interface->bInterfaceSubClass == 0x02)) &&       // MMC-5
        usbhost_driverObj.desc_interface->bInterfaceProtocol == 0x50)             // Bulk-Only Transport
    ) {
        printf("USB Mass Storage Class Bulk-Only device\n");
    } else {
        printf("Not BBB device\n");
        return ESP_FAIL;
    }

    // 记录端点信息
    if (usbhost_driverObj.desc_ep_out == NULL || usbhost_driverObj.desc_ep_in == NULL) {
        printf("Endpoint descriptor not found.\n");
        return ESP_FAIL;
    }
    usbhost_driverObj.ep_in_num       = usbhost_driverObj.desc_ep_in->bEndpointAddress;   // 不 &0x0f
    usbhost_driverObj.ep_in_packsize  = usbhost_driverObj.desc_ep_in->wMaxPacketSize;
    usbhost_driverObj.ep_out_num      = usbhost_driverObj.desc_ep_out->bEndpointAddress;  // 不 &0x0f
    usbhost_driverObj.ep_out_packsize = usbhost_driverObj.desc_ep_out->wMaxPacketSize;

    printf("ep in:%d, packsize:%d\n",  usbhost_driverObj.ep_in_num,  usbhost_driverObj.ep_in_packsize);
    printf("ep out:%d, packsize:%d\n", usbhost_driverObj.ep_out_num, usbhost_driverObj.ep_out_packsize);

    // 申请传输对象
    esp_err_t err = usb_host_transfer_alloc(usbhost_driverObj.ep_out_packsize, 0, &usbhost_driverObj.transferObj);
    if (err != ESP_OK) {
        printf("usb_host_transfer_alloc fail\n");
        return ESP_FAIL;
    }

    // 声明接口
    usb_host_interface_claim(
        usbhost_driverObj.handle_client,
        usbhost_driverObj.handle_device,
        usbhost_driverObj.desc_interface->bInterfaceNumber,
        usbhost_driverObj.desc_interface->bAlternateSetting);

    // 给设备一点时间初始化
    vTaskDelay(pdMS_TO_TICKS(1234));
    usbhost_driverObj.deviceIsOpened = 1;

    return ESP_OK;
}

void usbhost_closeDevice(void)
{
    if (usbhost_driverObj.handle_device == NULL) return;

    usb_host_interface_release(
        usbhost_driverObj.handle_client,
        usbhost_driverObj.handle_device,
        usbhost_driverObj.desc_interface->bInterfaceNumber);

    usb_host_device_close(usbhost_driverObj.handle_client, usbhost_driverObj.handle_device);
    usb_host_transfer_free(usbhost_driverObj.transferObj);

    usbhost_driverObj.handle_device   = NULL;
    usbhost_driverObj.desc_interface  = NULL;
    usbhost_driverObj.desc_ep_out     = NULL;
    usbhost_driverObj.desc_ep_in      = NULL;
    usbhost_driverObj.dev_addr        = 0;
    usbhost_driverObj.ep_in_num       = 0;
    usbhost_driverObj.ep_in_packsize  = 0;
    usbhost_driverObj.ep_out_num      = 0;
    usbhost_driverObj.ep_out_packsize = 0;
    usbhost_driverObj.deviceIsOpened  = 0;
}

/* ------------------------ Client/Daemon 任务与回调 ------------------------ */

static void usbhost_cb_client(const usb_host_client_event_msg_t *event_msg, void *arg)
{
    usbhost_driver_t *ctx = (usbhost_driver_t *)arg;

    switch (event_msg->event) {
    case USB_HOST_CLIENT_EVENT_NEW_DEV:
        if (event_msg->new_dev.address != 0 && ctx->handle_device == NULL) {
            ctx->dev_addr = event_msg->new_dev.address;
            senMsgToClientTask(CLASS_DRIVER_ACTION_NEW_DEV);
        }
        break;

    case USB_HOST_CLIENT_EVENT_DEV_GONE:
        if (ctx->handle_device != NULL)
            senMsgToClientTask(CLASS_DRIVER_ACTION_CLOSE_DEV);
        break;

    default:
        abort();
    }
}

void usbhost_task_client(void *arg)
{
    queue_client = xQueueCreate(10, sizeof(uint8_t));

    // 注册客户端
    ESP_LOGI("client_task", "Registering Client");
    usb_host_client_config_t client_config = {
        .is_synchronous    = false,   // 同步客户端当前不支持
        .max_num_event_msg = 5,
        .async = {
            .client_event_callback = usbhost_cb_client,
            .callback_arg          = (void *)&usbhost_driverObj,
        },
    };
    ESP_ERROR_CHECK(usb_host_client_register(&client_config, &usbhost_driverObj.handle_client));

    uint8_t msg;
    while (1)
    {
        // 阻塞等待事件并进入回调
        usb_host_client_handle_events(usbhost_driverObj.handle_client, portMAX_DELAY);

        BaseType_t got = xQueueReceive(queue_client, &msg, 0);
        if (got == pdTRUE) {
            switch (msg) {
            case CLASS_DRIVER_ACTION_NEW_DEV: {
                printf("USB device connected.\n");
                esp_err_t ret = usbhost_openDevice();
                if (ret == ESP_FAIL) usbhost_closeDevice();
            } break;
            case CLASS_DRIVER_ACTION_CLOSE_DEV:
                printf("USB device disconnected.\n");
                usbhost_closeDevice();
                break;
            default:
                break;
            }
        }
    }
}

void usbhost_task_usblibDaemon(void *arg)
{
    while (1)
    {
        uint32_t event_flags;
        ESP_ERROR_CHECK(usb_host_lib_handle_events(portMAX_DELAY, &event_flags));

        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            ESP_LOGI("usbhost_task_usblibDaemon", "USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS");
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
            ESP_LOGI("usbhost_task_usblibDaemon", "USB_HOST_LIB_EVENT_FLAGS_ALL_FREE");
        }
    }
}

/* ------------------------ 主机库安装（已修好） ------------------------ */

void usbhost_driverInit(void)
{
    ESP_LOGI("usbhost_driverInit", "Installing USB Host Library");

    // IDF v5: host_config 只需要 intr_flags；不要再使用旧的 skip_phy_setup / enum_filter_cb 成员
    usb_host_config_t host_config = {
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    ESP_ERROR_CHECK(usb_host_install(&host_config));   // 只安装一次

    BaseType_t ret;

    // 守护任务
    ret = xTaskCreatePinnedToCore(usbhost_task_usblibDaemon,
                                  "usbhost_task_usblibDaemon",
                                  4096, NULL, DAEMON_TASK_PRIORITY, NULL, 1);
    if (ret != pdPASS) {
        ESP_LOGE("usbhost_driverInit", "usbhost_task_usblibDaemon create fail");
    }

    // 客户端任务
    ret = xTaskCreatePinnedToCore(usbhost_task_client,
                                  "usbhost_task_client",
                                  4096, NULL, CLIENT_TASK_PRIORITY, NULL, 1);
    if (ret != pdPASS) {
        ESP_LOGE("usbhost_driverInit", "usbhost_task_client create fail");
    }

    vTaskDelay(pdMS_TO_TICKS(10));   // 让 client 任务先跑起来

    usbhost_driverObj.transferDone = xSemaphoreCreateBinary();
    scsiExeLock = xSemaphoreCreateMutex();
}

/* ------------------------ 传输回调与收发 ------------------------ */

void usbhost_cb_transfer(usb_transfer_t *transfer)
{
    if (transfer->status != USB_TRANSFER_STATUS_COMPLETED) {
        ESP_LOGE("usbhost_cb_transfer", "Transfer failed Status %d", transfer->status);
    }
    xSemaphoreGive(usbhost_driverObj.transferDone);
}

usb_transfer_status_t usbhost_waitForTransDone(usb_transfer_t *xfer)
{
    BaseType_t ret = xSemaphoreTake(usbhost_driverObj.transferDone,
                                    pdMS_TO_TICKS(xfer->timeout_ms));
    usb_transfer_status_t status = xfer->status;

    if (ret != pdTRUE) {
        // 停止先前提交的传输并清端点
        ESP_LOGE("usbhost_waitForTransDone", "time out, stop transfer.");
        usb_host_endpoint_halt (xfer->device_handle, xfer->bEndpointAddress);
        usb_host_endpoint_flush(xfer->device_handle, xfer->bEndpointAddress);
        usb_host_endpoint_clear(xfer->device_handle, xfer->bEndpointAddress);
        xSemaphoreTake(usbhost_driverObj.transferDone, portMAX_DELAY); // 刷 EP 后会立刻返回
        status = USB_TRANSFER_STATUS_TIMED_OUT;
    }
    return status;
}

esp_err_t usbhost_clearFeature(uint8_t endpoint)
{
    esp_err_t ret = usb_host_endpoint_halt(usbhost_driverObj.handle_device, endpoint);
    if (ret != ESP_OK) return ret;

    ret = usb_host_endpoint_flush(usbhost_driverObj.handle_device, endpoint);
    if (ret != ESP_OK) {
        // 没有 STALL 时 flush 会失败，直接返回即可
        return ret;
    }

    ret = usb_host_endpoint_clear(usbhost_driverObj.handle_device, endpoint);
    if (ret != ESP_OK) return ret;

    usb_setup_packet_t setupPack = {
        .bmRequestType = 0x02, // to endpoint
        .bRequest      = 1,    // clear feature
        .wValue        = 0,
        .wIndex        = endpoint,
        .wLength       = 0,
    };
    usbhost_controlTransfer(&setupPack, 8);
    return ESP_OK;
}

esp_err_t usbhost_controlTransfer(void *data, size_t size)
{
    usb_transfer_t *xfer = usbhost_driverObj.transferObj;

    memcpy(xfer->data_buffer, data, size);
    xfer->bEndpointAddress = 0;
    xfer->num_bytes        = size;
    xfer->callback         = usbhost_cb_transfer;
    xfer->context          = NULL;
    xfer->timeout_ms       = 5000;
    xfer->device_handle    = usbhost_driverObj.handle_device;

    ESP_RETURN_ON_ERROR(usb_host_transfer_submit_control(usbhost_driverObj.handle_client, xfer),
                        "usb_host_transfer_submit_control", "");

    usb_transfer_status_t status = usbhost_waitForTransDone(xfer);
    if (status != USB_TRANSFER_STATUS_COMPLETED) {
        ESP_LOGE("usbhost_controlTransfer", "Transfer fail: %d", status);
        return ESP_FAIL;
    }

    memcpy(data, xfer->data_buffer, size);
    return ESP_OK;
}

esp_err_t usbhost_bulkTransfer(void *data, uint32_t *size, usbhost_transDir_t dir, uint32_t timeoutMs)
{
    usb_transfer_t *xfer = usbhost_driverObj.transferObj;

    // re-alloc if buffer not big enough
    size_t transfer_size = (dir == DEV_TO_HOST)
                           ? usb_round_up_to_mps(*size, usbhost_driverObj.ep_in_packsize)
                           : *size;

    if (xfer->data_buffer_size < transfer_size) {
        usb_host_transfer_free(xfer);
        usb_host_transfer_alloc(transfer_size, 0, &usbhost_driverObj.transferObj);
        xfer = usbhost_driverObj.transferObj;
    }

    if (dir == HOST_TO_DEV) {
        memcpy(xfer->data_buffer, data, *size);
        xfer->bEndpointAddress = usbhost_driverObj.ep_out_num; // OUT
    } else {
        xfer->bEndpointAddress = usbhost_driverObj.ep_in_num;  // IN
    }

    xfer->num_bytes     = transfer_size;
    xfer->device_handle = usbhost_driverObj.handle_device;
    xfer->callback      = usbhost_cb_transfer;
    xfer->timeout_ms    = timeoutMs;
    xfer->context       = NULL;

    ESP_RETURN_ON_ERROR(usb_host_transfer_submit(xfer), "usbhost_bulkTransfer", "");

    usb_transfer_status_t status = usbhost_waitForTransDone(xfer);
    *size = xfer->actual_num_bytes;

    if (status != USB_TRANSFER_STATUS_COMPLETED) {
        ESP_LOGE("usbhost_bulkTransfer", "Transfer fail: %d", status);
        return status;
    }

    if (dir == DEV_TO_HOST) {
        memcpy(data, xfer->data_buffer, xfer->actual_num_bytes);
    }
    return USB_TRANSFER_STATUS_COMPLETED;
}
