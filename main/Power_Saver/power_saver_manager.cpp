#include "power_saver_manager.h"
#include "Common/globals.h"

static const char *TAG = "POWER_SAVER";

    esp_err_t
    PowerSaverManager::change_brightness_value(int batt)
{
    uint32_t brightness = current_brightness_percentage_;

    if (batt >= 70)
    {
        brightness = 70;
    }
    else if (batt >= 40 && batt <= 69)
    {
        brightness = 50;
    }
    else if (batt >= 20 && batt <= 39)
    {
        brightness = 30;
    }
    else
    {
        brightness = 20;
    }

    if (current_brightness_percentage_ != brightness)
    {
        current_brightness_percentage_ = brightness;
        ESP_RETURN_ON_ERROR(
            display_driver.set_brightness(current_brightness_percentage_),
            TAG, "Failed to set the display brightness");
        ESP_LOGI(TAG, "Brightness changed");
    }
    return ESP_OK;
}

esp_err_t PowerSaverManager::change_screen_timeout_value(int batt)
{
    uint32_t screen_timeout = current_screen_timeout_;
    if (batt >= 70)
    {
        screen_timeout = 15000; // 15 sec
    }
    else if (batt > 20 && batt <= 69)
    {
        screen_timeout = 10000; // 10 sec
    }
    else
    {
        screen_timeout = 5000; // 5 sec
    }

    if (current_screen_timeout_ != screen_timeout)
    {
        current_screen_timeout_ = screen_timeout;
        ESP_LOGI(TAG, "Screen timeout changed");
    }

    return ESP_OK;
}

esp_err_t PowerSaverManager::touch_toggled(int batt)
{
    bool touch_enabled = is_touch_enabled_;

    if (batt >= 20)
    {
        touch_enabled = true;
    }
    else
    {
        touch_enabled = false;
    }

    if (touch_enabled != is_touch_enabled_)
    {
        if (touch_enabled)
        {
            if (touch_task_handle != nullptr)
            {
                vTaskResume(touch_task_handle);
            }
        }
        else
        {
            if (touch_task_handle != nullptr)
            {
                vTaskSuspend(touch_task_handle);
            }
        }
        is_touch_enabled_ = touch_enabled;
    }

    return ESP_OK;
}

uint32_t PowerSaverManager::get_brightness_percentage()
{
    return current_brightness_percentage_;
}

esp_err_t PowerSaverManager::init()
{
    int batt = battery_manager.get_battery_percentage();

    ESP_RETURN_ON_ERROR(
        change_brightness_value(batt), TAG, "Failed to update brightness");

    ESP_RETURN_ON_ERROR(
        change_screen_timeout_value(batt), TAG, "Failed to update screen timeout");

    ESP_RETURN_ON_ERROR(
        touch_toggled(batt), TAG, "Failed to toggle touch");

    return ESP_OK;
}

uint32_t PowerSaverManager::get_screen_timeout()
{
    return current_screen_timeout_;
}

esp_err_t PowerSaverManager::handle_events(uint32_t events)
{
    if (events & BAT_PERCENT_CHG_EVENT)
    {
        int batt = battery_manager.get_battery_percentage();

        ESP_RETURN_ON_ERROR(
            change_brightness_value(batt), TAG, "Failed to update brightness");

        ESP_RETURN_ON_ERROR(
            change_screen_timeout_value(batt), TAG, "Failed to update screen timeout");

        ESP_RETURN_ON_ERROR(
            touch_toggled(batt), TAG, "Failed to toggle touch");
    }

    return ESP_OK;
}