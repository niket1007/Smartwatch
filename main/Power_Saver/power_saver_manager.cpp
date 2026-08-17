#include "power_saver_manager.h"
#include "Common/globals.h"

static const char *TAG = "POWER_SAVER";

void PowerSaverManager::clear_user_defined_options()
{
    user_defined_option.is_manual_override = false;
    user_defined_option.battery = 0;
    user_defined_option.brightness_percentage = 0;
}

esp_err_t PowerSaverManager::change_brightness_value(int batt)
{
    if (xSemaphoreTake(brightness_mutex_, portMAX_DELAY) != pdTRUE)
    {
        return ESP_FAIL;
    }

    uint32_t brightness = current_brightness_percentage_;

    if (batt >= 70)
    {
        if (user_defined_option.is_manual_override &&
            user_defined_option.battery >= 70)
        {
            brightness =
                user_defined_option.brightness_percentage;
        }
        else
        {
            clear_user_defined_options();
            brightness = 70;
        }
    }
    else if (batt >= 40 && batt <= 69)
    {
        if (user_defined_option.is_manual_override &&
            user_defined_option.battery >= 40 &&
            user_defined_option.battery <= 69)
        {
            brightness =
                user_defined_option.brightness_percentage;
        }
        else
        {
            clear_user_defined_options();
            brightness = 50;
        }
    }
    else if (batt >= 20 && batt <= 39)
    {
        if (user_defined_option.is_manual_override &&
            user_defined_option.battery >= 20 &&
            user_defined_option.battery <= 39)
        {
            brightness =
                user_defined_option.brightness_percentage;
        }
        else
        {
            clear_user_defined_options();
            brightness = 30;
        }
    }
    else
    {
        clear_user_defined_options();
        brightness = 20;
    }

    if (current_brightness_percentage_ == brightness)
    {
        xSemaphoreGive(brightness_mutex_);
        return ESP_OK;
    }

    current_brightness_percentage_ = brightness;
    esp_err_t ret = display_driver.set_brightness(current_brightness_percentage_);

    xSemaphoreGive(brightness_mutex_);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set the display brightness");
        return ret;
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

uint32_t PowerSaverManager::get_screen_timeout()
{
    return current_screen_timeout_;
}

esp_err_t PowerSaverManager::set_user_defined_options(int brightness_percent)
{

    if (brightness_percent < 10 ||
        brightness_percent > 100)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(brightness_mutex_, portMAX_DELAY) != pdTRUE)
    {
        return ESP_FAIL;
    }

    user_defined_option.battery = battery_manager.get_battery_percentage();
    user_defined_option.brightness_percentage = brightness_percent;
    user_defined_option.is_manual_override = true;

    current_brightness_percentage_ = user_defined_option.brightness_percentage;

    esp_err_t ret = display_driver.set_brightness(current_brightness_percentage_);

    xSemaphoreGive(brightness_mutex_);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set the display brightness");
        return ret;
    }
    return ESP_OK;
}

esp_err_t PowerSaverManager::init()
{
    brightness_mutex_ = xSemaphoreCreateMutex();

    if (brightness_mutex_ == nullptr)
    {
        ESP_LOGE(TAG, "Failed to create brightness mutex");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
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