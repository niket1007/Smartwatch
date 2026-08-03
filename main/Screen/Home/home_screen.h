#pragma once

#include <string>
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "Screen/screen.h"
#include "Graphics/Icons/generated/icon.h"
#include "Graphics/Icons/generated/icon_globals.h"

class HomeScreen : public Screen
{
private:
    icon_t old_icon = icon_battery_default;
    icon_t get_icon(bool charging, int percent);

    char old_day_str[16] = "";
    char old_date_str[32] = "";
    char old_time_str[16] = "";

public:
    int screen_id = 1;
    
    esp_err_t on_enter();
    esp_err_t on_exit();

    esp_err_t draw() override;
    esp_err_t draw_battery_icon();
    esp_err_t draw_battery_label();
    esp_err_t draw_datetime_labels();

    esp_err_t handle_events(uint32_t events) override;
};