- **GadgetBridge**
    1. Bluetooth Payload Detail: https://www.espruino.com/Gadgetbridge
    2. Banglejs GadgetBrige Code
        2.1) https://codeberg.org/Freeyourgadget/Gadgetbridge/src/commit/c42cc91586a6e6d31863ee7fe637f01a5845941c/app/src/main/java/nodomain/freeyourgadget/gadgetbridge/service/devices/banglejs/BangleJSDeviceSupport.java

- **DEBUGGING**
    1. For enabling esp_pm_dump_locks, below config need to be set
        1. CONFIG_PM_PROFILING=y
        2. CONFIG_PM_TRACE=y
    Code Example: 
    ```
    #include "esp_pm.h"
    esp_pm_dump_locks(stdout);
    ```
- **Light Sleep Callbacks**
1. Firstly, set the below config
    1. CONFIG_PM_LIGHT_SLEEP_CALLBACKS=y

2. Code Example
```
static uint64_t sleep_enter_count = 0;
static uint64_t sleep_exit_count = 0;

static uint64_t total_sleep_us = 0;
static uint64_t last_sleep_start_us = 0;
static uint64_t longest_sleep_us = 0;

static int on_sleep_enter(int64_t sleep_time_us, void *arg)
{
    sleep_enter_count++;

    last_sleep_start_us = esp_timer_get_time();

    return 0;
}

static int on_sleep_exit(int64_t slept_time_us, void *arg)
{
    sleep_exit_count++;

    const uint64_t sleep_end_us = esp_timer_get_time();

    if (sleep_end_us >= last_sleep_start_us)
    {
        const uint64_t sleep_duration_us =
            sleep_end_us - last_sleep_start_us;

        total_sleep_us += sleep_duration_us;

        if (sleep_duration_us > longest_sleep_us)
        {
            longest_sleep_us = sleep_duration_us;
        }
    }
    return 0;
}

esp_pm_sleep_cbs_register_config_t cb_config = {
    .enter_cb = on_sleep_enter,
    .exit_cb = on_sleep_exit,
    .enter_cb_user_arg = NULL,
    .exit_cb_user_arg = NULL,
    .enter_cb_prior = 5, // Default mid-level priority
    .exit_cb_prior = 5   // Default mid-level priority
};

esp_err_t err = esp_pm_light_sleep_register_cbs(&cb_config);
if (err != ESP_OK)
{
    ESP_LOGE(TAG, "Failed to register sleep callbacks: %s", esp_err_to_name(err));
    return;
}
ESP_LOGI(TAG, "Sleep callbacks registered successfully.");
```
