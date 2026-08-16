#pragma once

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

class PowerSaverManager
{
private:
    uint32_t current_brightness_percentage_ = 0;
    uint32_t current_screen_timeout_ = 0; // 15 sec
    bool is_touch_enabled_ = true;

    esp_err_t change_brightness_value(int batt);
    esp_err_t change_screen_timeout_value(int batt);
    esp_err_t touch_toggled(int batt);

public:
    esp_err_t init();
    uint32_t get_screen_timeout();
    uint32_t get_brightness_percentage();
    esp_err_t handle_events(uint32_t events);
};