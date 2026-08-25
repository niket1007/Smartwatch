#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

#include "bsp/esp-bsp.h"
#include "bsp/display.h"

#include "iot_button.h"
#include "button_gpio.h"

#include "esp_pm.h"

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

        bool notified = xTaskNotifyWait(
            0, UINT32_MAX, &events,
            display_manager.is_sleeping() ? portMAX_DELAY : pdMS_TO_TICKS(2000));

        now = millis();

        if (events & SCREEN_ON_EVENT)
        {
            battery_manager.refresh();
        }

        if (!display_manager.is_sleeping())
        {
            battery_manager.refresh();
        }

        if (events & BLUETOOTH_INIT)
        {
            esp_err_t err = bluetooth_manager.init();
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to intialize bluetooth");
            }
        }

        if (now - update_time_event_timer > UPDATE_TIME_TIMER)
        {
            update_time_event_timer = now;
            xTaskNotify(
                gui_task_handle, UPDATE_TIME_EVENT, eSetBits);
        }

        if (display_manager.is_screen_timeout_enabled() and
            (now - display_manager.get_screen_timeout_timer()) >=
                power_saver_manager.get_screen_timeout())
        {

            display_manager.reset_screen_timeout_timer(now);
            if (gui_task_handle != nullptr)
            {
                xTaskNotify(gui_task_handle, SCREEN_OFF_EVENT, eSetBits);
            }
        }

        esp_err_t ret = bluetooth_manager.handle_events(events);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to handle bluetooth event");
        }

        ret = power_saver_manager.update_bluetooth_state();
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to update the bluetooth status");
        }

        // esp_pm_dump_locks(stdout);
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

            power_saver_manager.handle_events(events);
        }
    }
    vTaskDelete(nullptr);
}

void time_sync_task_func(void *pvParameters)
{
    ESP_LOGI(TAG, "Boot Wi-Fi sync starting...");

    if (wifi_manager.init() == ESP_OK)
    {
        uint32_t events = 0;

        bool notified = xTaskNotifyWait(
            0,
            UINT32_MAX,
            &events,
            pdMS_TO_TICKS(15000));

        if (notified && (events & WIFI_CONNECTED_EVENT))
        {
            ESP_LOGI(TAG, "Wi-Fi Connected! Syncing RTC...");
            rtc_manager.init();
            rtc_manager.deinit();
        }
        else if (!notified)
        {
            ESP_LOGE(TAG, "Wi-Fi sync timed out");
        }
        else if (events & WIFI_FAILED_EVENT)
        {
            ESP_LOGE(TAG, "Wi-Fi connection failed (Max retries reached)");
        }

        wifi_manager.deinit();
    }
    else
    {
        ESP_LOGE(TAG, "Failed to initialize Wi-Fi manager");
    }

    rtc_manager.get_PCF85063_rtc();
    if (gui_task_handle != nullptr)
    {
        xTaskNotify(gui_task_handle, UPDATE_TIME_EVENT, eSetBits);
    }
    if (background_task_handle != nullptr)
    {
        vTaskDelay(pdMS_TO_TICKS(3000));
        xTaskNotify(background_task_handle, BLUETOOTH_INIT, eSetBits);
    }

    ESP_LOGI(TAG, "Boot Wi-Fi sync completed. Deleting task.");

    time_sync_handle = NULL;
    vTaskDelete(NULL);
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

    ESP_ERROR_CHECK(i2c_manager.init());

    ESP_ERROR_CHECK(nvs_manager.init());

    ESP_ERROR_CHECK(battery_manager.init());

    ESP_ERROR_CHECK(display_manager.init());

    ESP_ERROR_CHECK(screen_manager.init());

    ESP_ERROR_CHECK(power_saver_manager.init());

    ESP_ERROR_CHECK(register_boot_button());

    xTaskCreate(
        time_sync_task_func,
        "TIME_SYNC",
        4096,
        NULL,
        1,
        &time_sync_handle);

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

    esp_pm_config_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 40,
        .light_sleep_enable = true,
    };

    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
}