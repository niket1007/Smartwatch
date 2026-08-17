#include "nvs_manager.h"
#include "nvs_flash.h"
#include "nvs.h"

static constexpr char *TAG = "NVS_MANAGER";

esp_err_t NVSManager::init()
{
    if (is_init_)
        return ESP_OK;
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_RETURN_ON_ERROR(
            nvs_flash_erase(), TAG, "Failed to erase nvs");
        ret = nvs_flash_init();
    }
    ESP_RETURN_ON_ERROR(
        ret, TAG, "Failed to init nvs;");
    is_init_ = true;
    return ESP_OK;
}

esp_err_t NVSManager::deinit()
{
    if (!is_init_)
        return ESP_OK;

    ESP_RETURN_ON_ERROR(
        nvs_flash_deinit(), TAG, "Failed to de-initialize nvs");

    is_init_ = false;
    return ESP_OK;
}

bool NVSManager::is_initialised() const
{
    return is_init_;
}