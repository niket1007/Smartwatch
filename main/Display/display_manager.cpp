#include "display_manager.h"
#include "Common/globals.h"
// #include "Locks/locks_manager.h"


#include "bsp/esp-bsp.h"
#include "bsp/display.h"

#define LV_USE_PRIVATE_API
#include "src/indev/lv_indev_private.h"

#include "esp_pm.h"

static constexpr const char *TAG = "DISPLAY_MANAGER";

static void touch_event_cb(lv_event_t *e)
{
    display_manager.reset_screen_timeout_timer();
}

void DisplayManager::update_configs()
{
    lv_indev_t *indev = bsp_display_get_input_dev();
    if (indev)
    {
        indev->gesture_limit = 30;       // Minimum pixel distance
        indev->gesture_min_velocity = 3; // Minimum swipe velocity
        lv_indev_add_event_cb(
            indev,
            touch_event_cb,
            LV_EVENT_ALL,
            nullptr);
    }
}

esp_err_t DisplayManager::init()
{
    // LocksManager locks(cpu_freq_lock, no_sleep_lock);

    lv_display_t *display = bsp_display_start();
    if (display == nullptr)
    {
        ESP_LOGE(TAG, "Failed to init the display");
        return ESP_ERR_INVALID_ARG;
    }
    display_pointer = display;

    // 1. Update the gesture limit for smooth gesture catch
    // 2. Register global touch event
    update_configs();

    return ESP_OK;
}

bool DisplayManager::lock(uint32_t timeout_ms)
{
    return bsp_display_lock(timeout_ms);
}

void DisplayManager::unlock()
{
    bsp_display_unlock();
}

bool DisplayManager::is_sleeping()
{
    return is_sleep;
}

esp_err_t DisplayManager::sleep()
{
    // LocksManager locks(cpu_freq_lock, no_sleep_lock);

    ESP_LOGI(TAG, "Sleep func called; is_sleep: %d", is_sleep);

    if (is_sleep)
    {
        return ESP_OK;
    }

    // if (!bsp_display_lock(1000))
    // {
    //     ESP_LOGE(TAG, "Failed to acquire display lock for sleep");
    //     return ESP_ERR_TIMEOUT;
    // }

    esp_err_t ret = ESP_OK;

    ret = set_brightness(0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set brightness to 0%%");
        // bsp_display_unlock();
        return ret;
    }

    // ret = bsp_display_backlight_off();
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to turn off the display");
    //     bsp_display_unlock();
    //     return ret;
    // }

    is_sleep = true;

    // bsp_display_unlock();

    ret = esp_pm_lock_release(no_sleep_lock);
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to release no sleep lock");
    }

    return ESP_OK;
}


esp_err_t DisplayManager::wake()
{
    // LocksManager locks(cpu_freq_lock, no_sleep_lock);
    
    ESP_LOGI(TAG, "Wake func called; is_sleep: %d", is_sleep);

    if (!is_sleep)
    {
        return ESP_OK;
    }

    esp_err_t ret = esp_pm_lock_acquire(no_sleep_lock);
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to acquire no sleep lock");
    }

    // if (!bsp_display_lock(1000))
    // {
    //     ESP_LOGE(TAG, "Failed to acquire display lock for wake");
    //     return ESP_ERR_TIMEOUT;
    // }

    // ret = bsp_display_backlight_on();
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to turn on the display");
    //     bsp_display_unlock();
    //     return ret;
    // }

    uint32_t brightness = power_saver_manager.get_brightness_percentage();

    ret = set_brightness(brightness);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set brightness");
        // bsp_display_unlock();
        return ret;
    }

    is_sleep = false;

    // bsp_display_unlock();

    if (background_task_handle != nullptr)
    {
        xTaskNotify(
            background_task_handle, SCREEN_ON_EVENT, eSetBits);
    }
    else 
    {
        ESP_LOGE(TAG, "Background task handle is null");
    }

    if (gui_task_handle != nullptr)
    {
        xTaskNotify(
            gui_task_handle, UPDATE_TIME_EVENT, eSetBits);
    }

    return ESP_OK;
}

esp_err_t DisplayManager::set_brightness(int brightness_percentage)
{
    ESP_RETURN_ON_ERROR(
        bsp_display_brightness_set(brightness_percentage),
        TAG, "Failed to set the display");
    return ESP_OK;
}

int DisplayManager::get_brightness()
{
    return bsp_display_brightness_get();
}

void DisplayManager::set_screen_timeout_enabled(bool enabled)
{
    screen_timeout_enabled_ = enabled;
}

bool DisplayManager::is_screen_timeout_enabled() const
{
    return screen_timeout_enabled_;
}

esp_err_t DisplayManager::reset_screen_timeout_timer(int64_t timer)
{
    if (timer == 0)
        timer = esp_timer_get_time() / 1000;
    screen_timeout_ = timer;
    return ESP_OK;
}

int64_t DisplayManager::get_screen_timeout_timer()
{
    return screen_timeout_;
}