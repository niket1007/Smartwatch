#pragma once

#include "esp_err.h"

class BluetoothManager
{
private:
    esp_err_t handle_notifications();
    int passkey = 0;

public:
    esp_err_t init();
    esp_err_t deinit();
};