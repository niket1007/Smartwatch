#include "display_manager.h"
#include "Common/globals.h"

#include "bsp/esp-bsp.h"
#include "bsp/display.h"

#define LV_USE_PRIVATE_API
#include "src/indev/lv_indev_private.h"

#include "esp_heap_caps.h"

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
    ESP_LOGI(TAG, "Sleep func called; is_sleep: %d", is_sleep);
    if (is_sleep)
    {
        return ESP_OK;
    }

    ESP_RETURN_ON_ERROR(
        set_brightness(0), TAG, "Failed to set brightness to 0%%");

    ESP_RETURN_ON_ERROR(
        bsp_display_backlight_off(),
        TAG, "Failed to turn off the display");

    vTaskDelay(pdMS_TO_TICKS(120));

    is_sleep = true;
    return ESP_OK;
}

esp_err_t DisplayManager::wake()
{
    ESP_LOGI(TAG, "Wake func called; is_sleep: %d", is_sleep);
    if (!is_sleep)
    {
        return ESP_OK;
    }

    ESP_RETURN_ON_ERROR(
        bsp_display_backlight_on(),
        TAG, "Failed to turn on the display");

    uint32_t brightness = power_saver_manager.get_brightness_percentage();
    ESP_RETURN_ON_ERROR(
        set_brightness(brightness), TAG, "Failed to set brightness");

    is_sleep = false;

    if (background_task_handle != nullptr)
    {
        xTaskNotify(background_task_handle, SCREEN_ON_EVENT, eSetBits);
    }

    if (gui_task_handle != nullptr)
    {
        xTaskNotify(gui_task_handle, UPDATE_TIME_EVENT, eSetBits);
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