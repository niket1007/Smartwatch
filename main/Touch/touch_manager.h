#pragma once

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_lcd_touch_ft5x06.h"
#include "Common/constants.h"

enum class TouchGesture {
    NONE,
    SINGLE_TAP,
    SWIPE_LEFT,
    SWIPE_RIGHT
};

class TouchManager
{
    private:
        esp_lcd_touch_handle_t touch_out_handle = nullptr;

        bool is_touching_ = false;
        uint16_t start_x_ = 0;
        uint16_t start_y_ = 0;
        uint16_t last_x_ = 0;
        uint16_t last_y_ = 0;

        // Store Single Tap coordinates
        uint16_t tap_x_ = 0;
        uint16_t tap_y_ = 0;

        static constexpr uint16_t SWIPE_MIN_DISTANCE = 50; 
        static constexpr uint16_t TAP_MAX_DISTANCE = 20;

        TouchGesture process_touch();

    public:
        esp_err_t init();
        esp_err_t read_touch_data();

        uint16_t get_single_tap_x() const { return tap_x_; }
        uint16_t get_single_tap_y() const { return tap_y_; }
};