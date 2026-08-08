#pragma once

#include <string>
#include "esp_event.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_netif.h"

class WiFiManager
{
private:
    const std::string SSID = "Amit 1st Floor";
    const std::string PASSWORD = "12345689";

    esp_event_handler_instance_t wifi_event_instance_ = nullptr;
    esp_event_handler_instance_t ip_event_instance_ = nullptr;
    esp_netif_t *sta_netif_ = nullptr;

    bool is_system_inited_ = false;
    bool stopped_ = false;

    inline static int retry_num = 0;
    constexpr static int MAXIMUM_RETRY = 10;

    static void event_handler(
        void *arg,
        esp_event_base_t event_base,
        int32_t event_id,
        void *event_data);

public:
    esp_err_t init();
    esp_err_t deinit();
};