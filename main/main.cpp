#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "sdkconfig.h"
#include "iot_button.h"
#include "button_gpio.h"
#include "esp_pm.h"
#include "esp_timer.h"
#include "esp_sleep.h"
extern "C"
{
#include "esp_private/esp_clk.h"
}
#include "Common/globals.h"
#include "Screen/screen_manager.h"

#define BOOT_BUTTON_PIN 0
#define BUTTON_ACTIVE_LEVEL 0

static const char *TAG = "APP_MAIN";

static RTC_DATA_ATTR int sleep_enter_count = 0;
static RTC_DATA_ATTR int sleep_exit_count = 0;
static RTC_DATA_ATTR int cpu_40hz_count = 0;

esp_pm_lock_handle_t pm_lock;

int64_t millis(void)
{
    // esp_timer_get_time return microseconds
    return esp_timer_get_time() / 1000;
}

void background_task_func(void *pvParameters)
{
    vTaskDelay(pdMS_TO_TICKS(1000));

    int64_t update_time_event_timer = 0;

    while (1)
    {
        uint32_t events = 0;
        int64_t now = millis();

        xTaskNotifyWait(
            0,
            UINT32_MAX,
            &events,
            display_driver.is_sleep ? portMAX_DELAY : pdMS_TO_TICKS(2000));

        if (events & SCREEN_ON_EVENT)
        {
            battery_manager.refresh();
        }

        if (!display_driver.is_sleep)
        {
            battery_manager.refresh();
        }

        if (
            (now - display_driver.get_screen_timeout_timer()) >
            power_saver_manager.get_screen_timeout())
        {
            display_driver.reset_screen_timeout_timer(now);
            xTaskNotify(
                gui_task_handle,
                SCREEN_OFF_EVENT,
                eSetBits);
        }

        if (now - update_time_event_timer > UPDATE_TIME_TIMER)
        {
            update_time_event_timer = now;
            xTaskNotify(
                gui_task_handle,
                UPDATE_TIME_EVENT,
                eSetBits);
        }

        // esp_pm_dump_locks(stdout);
        // ESP_LOGI(TAG, "CPU freq = %d MHz",
        //          esp_clk_cpu_freq() / 1000000);
        // ESP_LOGI(TAG,
        //          "Wakeup cause: %d",
        //          esp_sleep_get_wakeup_cause());
        // ESP_LOGI(TAG,
        //     "Sleep Enter Count: %d; Sleep Exit Count: %d;",sleep_enter_count, sleep_exit_count);
        // ESP_LOGI(TAG, "CPU 40Hz Count: %d", cpu_40hz_count);
    }
    vTaskDelete(nullptr);
}

void touch_task_func(void *pvParameters)
{
    ESP_LOGI(TAG, "Touch init success");
    while (1)
    {
        uint32_t events = 0;

        bool notified = xTaskNotifyWait(
            0, UINT32_MAX, &events,
            display_driver.is_sleep ? portMAX_DELAY : pdMS_TO_TICKS(30));

        if (!notified || (events & SCREEN_ON_EVENT))
        {
            esp_err_t err = touch_manager.read_touch_data();
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Touch read failed: %s", esp_err_to_name(err));
            }
        }
    }
    vTaskDelete(nullptr);
}

void gui_task_func(void *pvParameters)
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(screen_manager.draw());
    vTaskDelay(pdMS_TO_TICKS(500));
    while (1)
    {
        uint32_t events = 0;

        bool notified = xTaskNotifyWait(
            0, UINT32_MAX, &events, portMAX_DELAY);

        if (events & SCREEN_OFF_EVENT)
        {
            display_driver.sleep();
        }
        else
        {
            screen_manager.handle_events(events);
        }

        power_saver_manager.handle_events(events);
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
    xTaskNotify(gui_task_handle, UPDATE_TIME_EVENT, eSetBits);

    ESP_LOGI(TAG, "Boot Wi-Fi sync completed. Deleting task.");

    time_sync_handle = NULL;
    vTaskDelete(NULL);
}

static void button_single_click_event_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Button single click!");
    if (display_driver.is_sleep)
    {
        display_driver.wake();
    }
    else
    {
        display_driver.sleep();
    }
    display_driver.reset_screen_timeout_timer();
}

static int on_sleep_enter(int64_t sleep_time_us, void *arg)
{
    sleep_enter_count++;
    return 0;
}

static int on_sleep_exit(int64_t slept_time_us, void *arg)
{
    sleep_exit_count++;
    if (esp_clk_cpu_freq() == 40000000)
    {
        cpu_40hz_count++;
    }
    return 0;
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(i2c_manager.init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    ESP_ERROR_CHECK(nvs_manager.init());

    ESP_ERROR_CHECK(battery_manager.init());

    ESP_ERROR_CHECK(display_driver.init());
    ESP_ERROR_CHECK(display_driver.clear());

    ESP_ERROR_CHECK(screen_manager.init());

    int retry_count = 0;
    bool is_touch_init = false;
    while (retry_count <= 3)
    {
        esp_err_t touch_err = touch_manager.init();
        if (touch_err != ESP_OK)
        {
            retry_count += 1;
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        else
        {
            is_touch_init = true;
            break;
        }
    }

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
    }
    else
    {
        ret = iot_button_register_cb(
            gpio_btn, BUTTON_SINGLE_CLICK, NULL, button_single_click_event_cb, NULL);
        ESP_ERROR_CHECK(ret);
    }

    if (is_touch_init)
    {
        xTaskCreate(
            touch_task_func,
            "TOUCH_TASK",
            4096,
            NULL,
            1,
            &touch_task_handle);
    }

    xTaskCreate(
        time_sync_task_func,
        "TIME_SYNC",
        4096,
        NULL,
        1,
        &time_sync_handle);

    xTaskCreatePinnedToCore(
        gui_task_func,
        "GUI TASK",
        16384,
        NULL,
        2,
        &gui_task_handle,
        1);

    xTaskCreatePinnedToCore(
        background_task_func,
        "BACKGROUND TASK",
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

    // esp_pm_sleep_cbs_register_config_t cb_config = {
    //     .enter_cb = on_sleep_enter,
    //     .exit_cb = on_sleep_exit,
    //     .enter_cb_user_arg = NULL,
    //     .exit_cb_user_arg = NULL,
    //     .enter_cb_prior = 5, // Default mid-level priority
    //     .exit_cb_prior = 5   // Default mid-level priority
    // };

    // esp_err_t err = esp_pm_light_sleep_register_cbs(&cb_config);
    // if (err != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to register sleep callbacks: %s", esp_err_to_name(err));
    //     return;
    // }
    // ESP_LOGI(TAG, "Sleep callbacks registered successfully.");
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
}
