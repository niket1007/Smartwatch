#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include "esp_err.h"

// BLE configuration
#define BLE_DEVICE_NAME  "NSW"

#define BLE_SERVICE_UUID \
    "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"

#define BLE_CHAR_RX_UUID \
    "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

#define BLE_CHAR_TX_UUID \
    "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

enum BLE_STATUS
{
    NOTHING = 0,
    CONNECTED = 1,
    DISCONNECTED = 2,
    PAIRING = 3,
    PAIRTING_FAILED = 4,
    PAIRED = 5
};


class BluetoothManager
{
private:
    NimBLEServer* pServer = nullptr;
    NimBLECharacteristic* pTxCharacteristic = nullptr;

    bool is_initialised = false;

    esp_err_t init_advertising();

    esp_err_t send_to_phone(const JsonDocument& json);

public:
    static BLE_STATUS ble_status;
    static String rxBuffer;
    static std::string connected_device_name;
    esp_err_t init();
    esp_err_t deinit();
    esp_err_t handle_events();

    std::string get_connected_device_name() const;
    bool is_connected() const;
    bool is_init() const;
    BLE_STATUS get_ble_status();
};