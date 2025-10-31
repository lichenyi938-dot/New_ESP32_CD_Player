// patches/ble_example.c  -- BLE GATT control service example (ESP-IDF NimBLE style)
// This is a simplified illustrative snippet. Integrate into your project's BLE init.
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "console/console.h"

// UUIDs (example) 
// Service: 12345678-1234-5678-1234-56789abcdef0
// Char RX (write): 12345678-1234-5678-1234-56789abcdef1
// Char TX (notify):12345678-1234-5678-1234-56789abcdef2

void ble_on_write(uint16_t conn_handle, uint16_t attr_handle, void *data, uint16_t len) {
    // parse control commands from phone and post to playback queue
}

void ble_init(void) {
    // init nimble, register GATT service with RX/TX characteristics
}
