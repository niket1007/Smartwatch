#pragma once

#include "Screen/screen.h"
#include <string>

class HomeScreen : public Screen
{
private:
    std::string date = "";
    std::string time = "";
    std::string weekday = "";
    bool charging_status = false;
    int battery_percentage = 0;
    
    esp_err_t update_date(std::string date);
    esp_err_t update_time(std::string time);
    esp_err_t update_weekday(std::string weekday);
    esp_err_t update_battery_percentage(int percentage);
    esp_err_t update_battery_icon(bool status, int percentage);
    
public:
    ~HomeScreen() = default;
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;
    esp_err_t handle_events(uint32_t events) override;
};