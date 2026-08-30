#include "wifi_manager.h"
#include "Common/globals.h"

#include <WiFi.h>
#include <array>

// #include "Storage/storage_manager.h"

static const char *TAG = "WIFI_MANAGER";

esp_err_t WifiManager::init() 
{
    int attempts = 0;

    usb_serial.println("Initiating Wifi Connection");

    // StorageManager storage_manager;

    std::array<std::string, 2> cred = storage_manager.fetch_wifi_credentials();
    if(cred[0].empty() or cred[1].empty()) {
        errored = true;
        return ESP_FAIL;
    }

    ssid_ = cred[0];
    password_ = cred[1];

    usb_serial.printf("SSID: %s; PASSWORD: %s\n", cred[0].c_str(), cred[1].c_str());

    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);

    WiFi.begin(cred[0].c_str(), cred[1].c_str());
    
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_MAX_ATTEMPTS) {
        vTaskDelay(pdMS_TO_TICKS(500));
        usb_serial.println("Connecting Wifi ......");
        attempts++;
    }

    if(WiFi.status() != WL_CONNECTED)
    {
        errored = true;
        return ESP_FAIL;
    }

    errored = false;
    return ESP_OK;
}

std::string WifiManager::get_ssid()
{
    return ssid_;
}

std::string WifiManager::get_password()
{
    return password_;
}

void WifiManager::update_local_credentials(std::string ssid, std::string password)
{
    ssid_ = ssid;
    password_ = password;
}

esp_err_t WifiManager::deinit() {
    if (!WiFi.disconnect(true))
    {
        errored = true;
        return ESP_FAIL;
    }

    if (!WiFi.mode(WIFI_OFF))
    {
        errored = true;
        return ESP_FAIL;
    }

    vTaskDelay(pdMS_TO_TICKS(150));

    errored = false;
    return ESP_OK;
}
