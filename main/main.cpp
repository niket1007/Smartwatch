#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

#include "bsp/esp-bsp.h"
#include "bsp/display.h"

#include "iot_button.h"
#include "button_gpio.h"

// #include "lvgl.h"
// #define LV_USE_PRIVATE_API
// #include "src/indev/lv_indev_private.h"

#include "Common/globals.h"
#include "Screen/screen_manager.h"

static const char *TAG = "APP_MAIN";

void background_task_func(void *pvParameters)
{
    int64_t update_time_event_timer = 0;
    int now;

    while (true)
    {
        uint32_t events = 0;
        now = millis();

        bool notified = xTaskNotifyWait(
            0, UINT32_MAX, &events,
            display_manager.is_sleeping() ? portMAX_DELAY : pdMS_TO_TICKS(2000));

        if (events & BLUETOOTH_INIT)
        {
            bluetooth_manager.init();
        }
        if (display_manager.is_screen_timeout_enabled() and
            (now - display_manager.get_screen_timeout_timer()) >= 15000) // 15sec
        {
            display_manager.reset_screen_timeout_timer(now);
            xTaskNotify(gui_task_handle, SCREEN_OFF_EVENT, eSetBits);
        }
    }

    vTaskDelete(nullptr);
}

void gui_task_func(void *pvParameters)
{
    if (display_manager.lock(1000))
    {
        screen_manager.load_default_screen();
        display_manager.unlock();
    }
    else
    {
        ESP_LOGE(TAG, "Failed to acquire display lock");
    }

    if (background_task_handle != nullptr)
    {
        xTaskNotify(background_task_handle, BLUETOOTH_INIT, eSetBits);
    }

    while (true)
    {
        uint32_t events = 0;

        bool notified = xTaskNotifyWait(
            0, UINT32_MAX, &events, portMAX_DELAY);

        if (events & SCREEN_OFF_EVENT)
        {
            display_manager.sleep();
        }
        else
        {
            if (display_manager.lock(100))
            {
                esp_err_t err = screen_manager.handle_events(events);
                if (err != ESP_OK)
                {
                    ESP_LOGW(TAG, "Screen event handling failed: %s",
                             esp_err_to_name(err));
                }

                display_manager.unlock();
            }
            else
            {
                ESP_LOGW(
                    TAG, "Failed to acquire display lock for event handling");
            }
        }
    }
    vTaskDelete(nullptr);
}

static void boot_button_single_click_event(void *arg, void *data)
{
    if (display_manager.is_sleeping())
    {
        display_manager.wake();
    }
    else
    {
        display_manager.sleep();
    }
    display_manager.reset_screen_timeout_timer();
}

esp_err_t register_boot_button()
{
    const button_config_t btn_cfg = {0};
    const button_gpio_config_t btn_gpio_cfg = {
        .gpio_num = BOOT_BUTTON_PIN,
        .active_level = BUTTON_ACTIVE_LEVEL,
        .enable_power_save = true,
    };
    button_handle_t gpio_btn = NULL;
    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &gpio_btn);
    if (NULL == gpio_btn)
    {
        ESP_LOGE(TAG, "Failed to attach boot button");
        return ESP_ERR_INVALID_ARG;
    }
    else
    {
        ret = iot_button_register_cb(
            gpio_btn, BUTTON_SINGLE_CLICK, NULL, boot_button_single_click_event, NULL);
        return ret;
    }
    return ESP_OK;
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(nvs_manager.init());
    ESP_ERROR_CHECK(display_manager.init());
    ESP_ERROR_CHECK(screen_manager.init());
    ESP_ERROR_CHECK(register_boot_button());

    xTaskCreatePinnedToCore(
        gui_task_func,
        "GUI",
        8192,
        NULL,
        2,
        &gui_task_handle,
        1);

    xTaskCreatePinnedToCore(
        background_task_func,
        "BACKGROUND",
        8192,
        NULL,
        1,
        &background_task_handle,
        0);
}