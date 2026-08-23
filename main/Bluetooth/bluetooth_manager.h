#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "esp_err.h"

#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"

enum BLE_STATUS
{
    CONNECTED = 1,
    DISCONNECTED = 2,
    PAIRING = 3,
    PAIRED = 4
};

class BluetoothManager
{
private:
    static constexpr char DEVICE_NAME[] = "NSW";

    static bool is_connected;
    static BLE_STATUS ble_status;

    // Gadgetbridge / Nordic UART Service UUIDs
    static const ble_uuid128_t SERVICE_UUID;
    static const ble_uuid128_t RX_UUID;
    static const ble_uuid128_t TX_UUID;

    static constexpr size_t RX_BUFFER_SIZE = 4096;

    // RX buffer
    static char rx_buffer[RX_BUFFER_SIZE];
    static size_t rx_buffer_length;

    // BLE connection state
    static uint16_t connection_handle;
    static uint16_t tx_value_handle;
    static bool tx_notifications_enabled;
    static uint8_t own_addr_type;

    // Pairing
    static uint32_t passkey;

    // GATT database
    static struct ble_gatt_chr_def gatt_characteristics[];
    static const struct ble_gatt_svc_def gatt_services[];

    // NimBLE callbacks
    static int gatt_access_callback(
        uint16_t conn_handle,
        uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt,
        void *arg);

    static int ble_gap_event(
        struct ble_gap_event *event,
        void *arg);

    static void gatt_register_callback(
        struct ble_gatt_register_ctxt *ctxt,
        void *arg);

    // RX processing
    static void process_rx_bytes(
        const uint8_t *data,
        size_t length);

    static void reset_rx_buffer();

    static void process_complete_line(
        const char *line);

    // NimBLE host
    static void nimble_host_task(void *param);

    static void ble_on_reset(int reason);

    static void ble_on_sync();

    // Advertising
    static esp_err_t init_advertising();

    static bool ble_send_to_phone(
        const std::string &json);

    // Pairing
    static uint32_t generate_passkey();

    // Application integration
    static void ble_handle_gadgetbridge_line(
        const char *line);

public:
    esp_err_t init();

    esp_err_t deinit();

    uint32_t get_passkey();

    // BLE helpers
    static bool is_device_connected();
    static BLE_STATUS get_ble_conn_status();

    esp_err_t handle_events(uint32_t events);
};