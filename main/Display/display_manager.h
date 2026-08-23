#pragma once

#include "esp_err.h"
#include "lvgl.h"

class DisplayManager
{
private:
    lv_display_t *display_pointer = nullptr;
    bool is_sleep = false;
    int64_t screen_timeout_ = 0;
    bool screen_timeout_enabled_ = true;

    void update_configs();

public:
    esp_err_t init();
    bool lock(uint32_t timeout_ms);
    void unlock();

    bool is_sleeping();
    esp_err_t sleep();
    esp_err_t wake();

    esp_err_t set_brightness(int brightness_percentage);
    int get_brightness();

    esp_err_t reset_screen_timeout_timer(int64_t timer = 0);
    int64_t get_screen_timeout_timer();

    void set_screen_timeout_enabled(bool enabled);
    bool is_screen_timeout_enabled() const;
};