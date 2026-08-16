#include "wifi_manager.h"
#include "Common/globals.h"
#include <cstring>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"

static constexpr char *TAG = "WIFI_MANAGER";

void WiFiManager::event_handler(
    void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    auto *self = static_cast<WiFiManager *>(arg);
    if (self->stopped_)
        return;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (retry_num < MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            retry_num++;
            ESP_LOGI(TAG, "Retry connecting to AP (%d/%d)", retry_num, MAXIMUM_RETRY);
        }
        else
        {
            xTaskNotify(time_sync_handle, WIFI_FAILED_EVENT, eSetBits);
            ESP_LOGI(TAG, "WIFI CONNECTION FAILED");
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        retry_num = 0;

        xTaskNotify(time_sync_handle, WIFI_CONNECTED_EVENT, eSetBits);
    }
}

esp_err_t WiFiManager::init()
{
    stopped_ = false;
    retry_num = 0;

    if (!is_system_inited_)
    {
        if (!nvs_manager.is_initialised())
        {
            ESP_RETURN_ON_ERROR(nvs_manager.init(), TAG, "Failed to init nvs");
        }

        ESP_RETURN_ON_ERROR(esp_netif_init(), TAG, "Failed to init netif");

        esp_err_t err = esp_event_loop_create_default();
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
        {
            ESP_LOGE(TAG, "Failed to create event loop: %s", esp_err_to_name(err));
            return err;
        }
        is_system_inited_ = true;
    }

    // Create netif station interface if not present
    if (sta_netif_ == nullptr)
    {
        sta_netif_ = esp_netif_create_default_wifi_sta();
        ESP_RETURN_ON_FALSE(sta_netif_ != nullptr, ESP_FAIL, TAG, "Failed to create default wifi sta");
    }

    // Initialize Wi-Fi driver stack
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&cfg), TAG, "Failed to init wifi");

    // Register event handlers
    ESP_RETURN_ON_ERROR(
        esp_event_handler_instance_register(
            WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, this, &wifi_event_instance_),
        TAG, "Failed to register wifi event");

    ESP_RETURN_ON_ERROR(
        esp_event_handler_instance_register(
            IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, this, &ip_event_instance_),
        TAG, "Failed to register ip event");

    // Configure & Start Wi-Fi
    wifi_config_t wifi_config = {};
    strncpy(reinterpret_cast<char *>(wifi_config.sta.ssid), SSID.c_str(), sizeof(wifi_config.sta.ssid) - 1);
    strncpy(reinterpret_cast<char *>(wifi_config.sta.password), PASSWORD.c_str(), sizeof(wifi_config.sta.password) - 1);

    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "Failed to set wifi mode");
    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config), TAG, "Failed to set wifi config");
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "Failed to start wifi");

    ESP_LOGI(TAG, "Wi-Fi initialization finished successfully.");
    return ESP_OK;
}
esp_err_t WiFiManager::deinit()
{
    stopped_ = true;

    // Stop Wi-Fi before removing the driver
    esp_err_t err = esp_wifi_stop();
    if (err != ESP_OK && err != ESP_ERR_WIFI_NOT_INIT)
    {
        ESP_LOGE(
            TAG,
            "Failed to stop Wi-Fi: %s",
            esp_err_to_name(err));
        return err;
    }

    // Unregister Wi-Fi event handler
    if (wifi_event_instance_)
    {
        err = esp_event_handler_instance_unregister(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            wifi_event_instance_);

        if (err != ESP_OK)
        {
            ESP_LOGE(
                TAG,
                "Failed to unregister Wi-Fi event handler: %s",
                esp_err_to_name(err));
            return err;
        }

        wifi_event_instance_ = nullptr;
    }

    // Unregister IP event handler
    if (ip_event_instance_)
    {
        err = esp_event_handler_instance_unregister(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            ip_event_instance_);

        if (err != ESP_OK)
        {
            ESP_LOGE(
                TAG,
                "Failed to unregister IP event handler: %s",
                esp_err_to_name(err));
            return err;
        }

        ip_event_instance_ = nullptr;
    }

    // Deinitialize Wi-Fi driver
    err = esp_wifi_deinit();
    if (err != ESP_OK && err != ESP_ERR_WIFI_NOT_INIT)
    {
        ESP_LOGE(
            TAG,
            "Failed to deinit Wi-Fi: %s",
            esp_err_to_name(err));
        return err;
    }

    // Destroy Wi-Fi STA network interface
    if (sta_netif_)
    {
        esp_netif_destroy_default_wifi(sta_netif_);
        sta_netif_ = nullptr;
    }

    // Delete default event loop in init()
    err = esp_event_loop_delete_default();
    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to delete default event loop: %s",
            esp_err_to_name(err));
        return err;
    }

    is_system_inited_ = false;

    ESP_LOGI(TAG, "Wi-Fi deinitialized and powered off.");
    return ESP_OK;
}