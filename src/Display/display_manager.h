#pragma once 


class DisplayManager
{
    private:
        int screen_timeout_timer_ = 0;
        bool current_sleep_status = false;

    public:
        esp_err_t sleep();
        esp_err_t wake();

        bool is_sleeping();
}