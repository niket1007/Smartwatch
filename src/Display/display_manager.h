#pragma once 

#include "esp_err.h"
#include "Arduino_DriveBus_Library.h"

class DisplayManager
{
    private:
        unsigned long screen_timeout_timer_ = 0;
        bool current_sleep_status = false;

    public:
        void init();

        esp_err_t sleep();
        esp_err_t wake();

        bool is_sleeping();
        unsigned long get_screen_timeout_timer();
        void reset_screen_timeout_timer(unsigned long time = 0);

        void set_brightness(uint32_t brightness_percent);
};