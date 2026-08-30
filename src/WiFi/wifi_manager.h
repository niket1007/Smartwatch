#pragma once

#include "esp_err.h"
#include <string>

class WifiManager
{
    private:
        std::string ssid_ = "";
        std::string password_ = "";
        const int WIFI_MAX_ATTEMPTS = 20;

    public:
        bool errored;
        
        esp_err_t init();
        esp_err_t deinit();

        std::string get_ssid();
        std::string get_password();
        void update_local_credentials(std::string ssid, std::string password);
};