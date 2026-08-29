#pragma once

#include "Screen/screen.h"
#include <string>
#include "lvgl.h"

class HomeScreen : public Screen
{
private:
    char old_day_str[16] = "";
    char old_date_str[32] = "";
    char old_time_str[16] = "";

    static constexpr char *days[7] = {
        "Sunday","Monday","Tuesday","Wednesday",
        "Thursday","Friday","Saturday"
    };

    static constexpr char *months[12] = {
        "Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"
    };

    esp_err_t update_bat_percent_fields();
    esp_err_t update_bat_charging_fields();
    esp_err_t update_datetime_fields();
    const lv_image_dsc_t *get_battery_icon(bool charging, int percent);
    
public:
    ~HomeScreen() = default;
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;
    esp_err_t handle_events(uint32_t events) override;
};