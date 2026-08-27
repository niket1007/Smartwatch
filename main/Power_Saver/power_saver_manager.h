#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

struct UserDefined
{
    int brightness_percentage;
    int battery;
    bool is_manual_override;
};

class PowerSaverManager
{
private:
    bool is_touch_enabled_ = true;
    uint32_t current_brightness_percentage_ = 0;
    uint32_t current_screen_timeout_ = 0; // 0 sec

    UserDefined user_defined_option = {0, 0, false};
    SemaphoreHandle_t brightness_mutex_ = nullptr;

    esp_err_t change_brightness_value(int batt);
    esp_err_t change_screen_timeout_value(int batt);
    void clear_user_defined_options();

public:
    esp_err_t init();
    uint32_t get_screen_timeout();
    uint32_t get_brightness_percentage();
    esp_err_t set_user_defined_options(int brightness_percent);
    bool is_touch_enabled();
    esp_err_t update_bluetooth_state();
    esp_err_t handle_events(uint32_t events);
};