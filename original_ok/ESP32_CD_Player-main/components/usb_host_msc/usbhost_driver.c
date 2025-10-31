/**
 * usbhost_driver.c  —— 可直接替换版
 *
 * 作用：
 *  - 注册 USB Host 客户端，识别 MSC Bulk-Only 设备
 *  - 打开/关闭设备，读设备/配置/接口/端点描述符
 *  - 控制传输、批量传输
 *  - 任务与回调
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
#include "esp_intr_alloc.h"       // 需要 ESP_INTR_FLAG_LEVEL1

#include "usbhost_driver.h"       // 你的头文件，内含 usbhost_driver_t 等

/* 来自别处的互斥量（SCSI 用） */
extern SemaphoreHandle_t scsiExeLock;

/* 任务优先级与动作 */
#define DAEMON_TASK_PRIORITY 2
#define CLIENT_TASK_PRIORITY 3

#define CLASS_DRIVER_ACTION_NEW_DEV   0x01
#define CLASS_DRIVER_ACTION_CLOSE_DEV 0x02

/* 全局对象与队列 */
QueueHandle_t queue_client = NULL;
usbhost_driver_t usbhost_driverObj = {0};

/* 发送消息给 client 任务 */
static inline void senMsgToClientTask(uint8_t msg) {
    if (queue_client) xQueueSend(queue_client, &msg, 0);
}

/* ---------------------------
 *  打开设备：读取信息与描述符
 * --------------------------- */
esp_err_t usbhost_openDevice(void)
{
    ESP_LOGI("client_task", "Open device");
    printf("Device addr: %d\n", usbhost_driverObj.dev_addr);

    /* 打开设备 */
    ESP_ERROR_CHECK(usb_host_device_open(usbhost_driverObj.handle_client,
                                         usbhost_driverObj.dev_addr,
                                         &usbhost_driverObj.handle_device));

    /* 设备信息 */
    ESP_LOGI("client_task", "Get device information");
    usb_device_info_t dev_info;
    ESP_ERROR_CHECK(usb_host_device_info(usbhost_driverObj.handle_device, &dev_info));
    printf("USB speed: %s speed\n", (dev_info.speed == USB_SPEED_LOW) ? "Low" : "Full");
    printf("bConfigurationValue: %d\n", dev_info.bConfigurationValue);
    printf("string desc manufacturer: "); usb_print_string_descriptor(dev_info.str_desc_manufacturer);
    printf("string desc product:      "); usb_print_string_descriptor(dev_info.str_desc_product);
    printf("string desc sn:           "); usb_print_string_descriptor(dev_info.str_desc_serial_num);

    /* 设备描述符 */
    ESP_LOGI("client_task", "Get device descriptor");
    const usb_device_desc_t *dev_desc = NULL;
    ESP_ERROR_CHECK(usb_host_get_device_descriptor(usbhost_driverObj.handle_device, &dev_desc));
    usb_print_device_descriptor(dev_desc);

    /* 配置/接口/端点描述符 */
    ESP_LOGI("client_task", "Get config descriptor");
    const usb_config_desc_t *config_desc = NULL;
    ESP_ERROR_CHECK(usb_host_get_active_config_descriptor(usbhost_driverObj.handle_device, &config_desc));
    usb_print_config_descriptor(config_desc, NULL);

    /* 遍历找 interface / endpoint */
    int offset = 0;
    const usb_standard_desc_t *each_desc = (const usb_standard_desc_t *)config_desc;
    usbhost_driverObj.desc_interface = NULL;
    usbhost_driverObj.desc_ep_in = NULL;
    usbhost_driverObj.desc_ep_out = NULL;

    while (each_desc != NULL) {
        if (each_desc->bDescriptorType == USB_B_DESCRIPTOR_TYPE_INTERFACE) {
            usbhost_driverObj.desc_interface = (usb_intf_desc_t *)each_desc;
        } else if (each_desc->bDescriptorType == USB_B_DESCRIPTOR_TYPE_ENDPOINT) {
            usb_ep_desc_t *ep = (usb_ep_desc_t *)each_desc;
            const uint8_t epAddr = ep->bEndpointAddress;
            const uint8_t type   = ep->bmAttributes & 0x03;   /* 0: ctrl 1:iso 2:bulk 3:int */
            if (type == 2) { /* BULK */
                if (epAddr & 0x80) usbhost_driverObj.desc_ep_in  = ep; /* IN */
                else               usbhost_driverObj.desc_ep_out = ep; /* OUT */
            }
        }

        if (usbhost_driverObj.desc_interface &&
            usbhost_driverObj.desc_ep_out  &&
            usbhost_driverObj.desc_ep_in) {
            break;
        }

        each_desc = usb_parse_next_descriptor(each_desc, config_desc->wTotalLength, &offset);
    }

    /* 校验端点 */
    if (!usbhost_driverObj.desc_interface || !usbhost_driverObj.desc_ep_in || !usbhost_driverObj.desc_ep_out) {
        printf("Endpoint / interface descriptor not complete.\n");
        return ESP_FAIL;
    }

    /* -------- MSC Bulk-Only 设备判断（安全写法） -------- */
    bool is_msc_bulk_only = false;
    {
        const usb_intf_desc_t *intf = usbhost_driverObj.desc_interface;
        is_msc_bulk_only =
            (intf->bInterfaceClass    == 0x08) &&                     /* Mass Storage */
            ((intf->bInterfaceSubClass == 0x05) ||                    /* Obsolete SFF-8070I */
             (intf->bInterfaceSubClass == 0x06) ||                    /* SCSI transparent */
             (intf->bInterfaceSubClass == 0x02)) &&                   /* MMC-5 */
            (intf->bInterfaceProtocol == 0x50);                       /* Bulk-Only Transport */
    }

    if (is_msc_bulk_only) {
        printf("USB Mass Storage Class Bulk-Only device\n");
    } else {
        printf("Not BBB device\n");
        return ESP_FAIL;
    }
    /* --------------------------------------------------- */

    /* 记录端点号与包大小（注意：不做 &0x0F，以满足上层 API 习惯） */
    usbhost_driverObj.ep_in_num       = usbhost_driverObj.desc_ep_in->bEndpointAddress;
    usbhost_driverObj.ep_in_packsize  = usbhost_driverObj.desc_ep_in->wMaxPacketSize;
    usbhost_driverObj.ep_out_num      = usbhost_driverObj.desc_ep_out->bEndpointAddress;
    usbhost_driverObj.ep_out_packsize = usbhost_driverObj.desc_ep_out->wMaxPacketSize;
    printf("ep in:%d, packsize:%d\n",  usbhost_driverObj.ep_in_num,  usbhost_driverObj.ep_in_packsize);
    printf("ep out:%d, packsize:%d\n", usbhost_driverObj.ep_out_num, usbhost_driverObj.ep_out_packsize);

    /* 分配传输对象 */
    ESP_ERROR_CHECK(usb_host_transfer_alloc(usbhost_driverObj.ep_out_packsize, 0, &usbhost_driverObj.transferObj));

    /* 声明接口 */
    ESP_ERROR_CHECK(usb_host_interface_claim(usbhost_driverObj.handle_client,
                                             usbhost_driverObj.handle_device,
                                             usbhost_driverObj.desc_interface->bInterfaceNumber,
                                             usbhost_driverObj.desc_interface->bAlternateSetting));

    /* 给设备一点启动时间 */
    vTaskDelay(pdMS_TO_TICKS(1234));
    usbhost_driverObj.deviceIsOpened = 1;

    return ESP_OK;
}

/* 关闭设备 */
void usbhost_closeDevice(void)
{
    if (usbhost_driverObj.handle_device == NULL) return;

    /* 释放接口/关闭/释放传输对象 */
    usb_host_interface_release(usbhost_driverObj.handle_client,
                               usbhost_driverObj.handle_device,
                               usbhost_driverObj.desc_interface->bInterfaceNumber);
    usb_host_device_close(usbhost_driverObj.handle_client, usbhost_driverObj.handle_device);
    if (usbhost_driverObj.transferObj) usb_host_transfer_free(usbhost_driverObj.transferObj);

    /* 清理记录 */
    usbhost_driverObj.handle_device = NULL;
    usbhost_driverObj.desc_interface = NULL;
    usbhost_driverObj.desc_ep_out = NULL;
    usbhost_driverObj.desc_ep_in = NULL;
    usbhost_driverObj.dev_addr = 0;
    usbhost_driverObj.ep_in_num = 0;
    usbhost_driverObj.ep_in_packsize = 0;
    usbhost_driverObj.ep_out_num = 0;
    usbhost_driverObj.ep_out_packsize = 0;
    usbhost_driverObj.deviceIsOpened = 0;
}

/* client 回调：设备插拔事件 -> 发消息给 client 任务 */
static void usbhost_cb_client(const usb_host_client_event_msg_t *event_msg, void *arg)
{
    usbhost_driver_t *p = (usbhost_driver_t *)arg;
    switch (event_msg->event) {
        case USB_HOST_CLIENT_EVENT_NEW_DEV:
            if (event_msg->new_dev.address != 0 && p->handle_device == NULL) {
                p->dev_addr = event_msg->new_dev.address;
                senMsgToClientTask(CLASS_DRIVER_ACTION_NEW_DEV);
            }
            break;
        case USB_HOST_CLIENT_EVENT_DEV_GONE:
            if (p->handle_device != NULL) {
                senMsgToClientTask(CLASS_DRIVER_ACTION_CLOSE_DEV);
            }
            break;
        default:
            abort();
    }
}

/* client 任务：处理事件、打开/关闭设备 */
void usbhost_task_client(void *arg)
{
    queue_client = xQueueCreate(10, sizeof(uint8_t));

    ESP_LOGI("client_task", "Registering Client");
    const usb_host_client_config_t client_config = {
        .is_synchronous = false,
        .max_num_event_msg = 5,
        .async = {
            .client_event_callback = usbhost_cb_client,
            .callback_arg = (void *)&usbhost_driverObj,
        },
    };
    ESP_ERROR_CHECK(usb_host_client_register(&client_config, &usbhost_driverObj.handle_client));

    uint8_t msg = 0;
    for (;;) {
        /* 阻塞等待事件发生（发生后会先进入回调，再解除阻塞） */
        usb_host_client_handle_events(usbhost_driverObj.handle_client, portMAX_DELAY);

        if (xQueueReceive(queue_client, &msg, 0) == pdTRUE) {
            if (msg == CLASS_DRIVER_ACTION_NEW_DEV) {
                printf("USB device connected.\n");
                if (usbhost_openDevice() != ESP_OK) {
                    usbhost_closeDevice();
                }
            } else if (msg == CLASS_DRIVER_ACTION_CLOSE_DEV) {
                printf("USB device disconnected.\n");
                usbhost_closeDevice();
            }
        }
    }
}

/* 守护任务：处理 USB Host Library 事件 */
void usbhost_task_usblibDaemon(void *arg)
{
    for (;;) {
        uint32_t event_flags = 0;
        ESP_ERROR_CHECK(usb_host_lib_handle_events(portMAX_DELAY, &event_flags));
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            ESP_LOGI("usbhost_task_usblibDaemon", "USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS");
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
            ESP_LOGI("usbhost_task_usblibDaemon", "USB_HOST_LIB_EVENT_FLAGS_ALL_FREE");
        }
    }
}

/* 驱动初始化：安装 Host 库 + 启动两个任务 + 创建同步对象 */
void usbhost_driverInit(void)
{
    ESP_LOGI("usbhost_driverInit", "Installing USB Host Library");

    /* 仅设置常用字段：跳过 PHY、设置中断等级 1（IDF v5 兼容）*/
    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags     = ESP_INTR_FLAG_LEVEL1,
    };
    ESP_ERROR_CHECK(usb_host_install(&host_config));

    BaseType_t ret;

    /* 守护任务 */
    ret = xTaskCreatePinnedToCore(usbhost_task_usblibDaemon,
                                  "usbhost_task_usblibDaemon",
                                  4096, NULL, DAEMON_TASK_PRIORITY, NULL, 1);
    if (ret != pdPASS) {
        ESP_LOGE("usbhost_driverInit", "usbhost_task_usblibDaemon creat fail");
    }

    /* client 任务 */
    ret = xTaskCreatePinnedToCore(usbhost_task_client,
                                  "usbhost_task_client",
                                  4096, NULL, CLIENT_TASK_PRIORITY, NULL, 1);
    if (ret != pdPASS) {
        ESP_LOGE("usbhost_driverInit", "usbhost_task_client creat fail");
    }

    vTaskDelay(pdMS_TO_TICKS(10));   /* 给 client 任务一点启动时间 */

    usbhost_driverObj.transferDone = xSemaphoreCreateBinary();
    scsiExeLock = xSemaphoreCreateMutex();
}

/* ------------------------
 *    传输与回调
 * ------------------------ */
static void usbhost_cb_transfer(usb_transfer_t *transfer)
{
    if (transfer->status != USB_TRANSFER_STATUS_COMPLETED) {
        ESP_LOGE("usbhost_cb_transfer", "Transfer failed Status %d", transfer->status);
    }
    xSemaphoreGive(usbhost_driverObj.transferDone);
}

static usb_transfer_status_t usbhost_waitForTransDone(usb_transfer_t *xfer)
{
    BaseType_t ret = xSemaphoreTake(usbhost_driverObj.transferDone, pdMS_TO_TICKS(xfer->timeout_ms));
    usb_transfer_status_t status = xfer->status;

    if (ret != pdTRUE) {
        ESP_LOGE("usbhost_waitForTransDone", "time out, stop transfer.");
        usb_host_endpoint_halt (xfer->device_handle, xfer->bEndpointAddress);
        usb_host_endpoint_flush(xfer->device_handle, xfer->bEndpointAddress);
        usb_host_endpoint_clear(xfer->device_handle, xfer->bEndpointAddress);
        xSemaphoreTake(usbhost_driverObj.transferDone, portMAX_DELAY);
        status = USB_TRANSFER_STATUS_TIMED_OUT;
    }
    return status;
}

/* 清除端点特性（连带发一个标准请求） */
esp_err_t usbhost_clearFeature(uint8_t endpoint)
{
    esp_err_t ret = usb_host_endpoint_halt(usbhost_driverObj.handle_device, endpoint);
    if (ret != ESP_OK) return ret;

    ret = usb_host_endpoint_flush(usbhost_driverObj.handle_device, endpoint);
    if (ret != ESP_OK) return ret;

    ret = usb_host_endpoint_clear(usbhost_driverObj.handle_device, endpoint);
    if (ret != ESP_OK) return ret;

    usb_setup_packet_t setupPack = {
        .bmRequestType = 0x02, /* to endpoint */
        .bRequest      = 1,    /* CLEAR_FEATURE */
        .wValue        = 0,
        .wIndex        = endpoint,
        .wLength       = 0,
    };
    (void)usbhost_controlTransfer(&setupPack, sizeof(setupPack));
    return ESP_OK;
}

/* 控制传输 */
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

/* 批量传输（IN/OUT） */
esp_err_t usbhost_bulkTransfer(void *data, uint32_t *size, usbhost_transDir_t dir, uint32_t timeoutMs)
{
    usb_transfer_t *xfer = usbhost_driverObj.transferObj;

    /* 缓冲不足则重新分配（IN 向上取整到 MPS） */
    size_t transfer_size = (dir == DEV_TO_HOST)
                            ? usb_round_up_to_mps(*size, usbhost_driverObj.ep_in_packsize)
                            : *size;
    if (xfer->data_buffer_size < transfer_size) {
        usb_host_transfer_free(xfer);
        ESP_ERROR_CHECK(usb_host_transfer_alloc(transfer_size, 0, &usbhost_driverObj.transferObj));
        xfer = usbhost_driverObj.transferObj;
    }

    if (dir == HOST_TO_DEV) {
        memcpy(xfer->data_buffer, data, *size);
        xfer->bEndpointAddress = usbhost_driverObj.ep_out_num;
    } else {
        xfer->bEndpointAddress = usbhost_driverObj.ep_in_num;
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
