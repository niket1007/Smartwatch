#pragma once

#include "esp_err.h"

class WifiManager
{
    private:
        const int WIFI_MAX_ATTEMPTS = 20;

    public:
        esp_err_t init();
        esp_err_t deinit();
};