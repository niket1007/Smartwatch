#pragma once

#include "XPowersLib.h"
#include "esp_err.h"

class BatteryManager
{
private:
    int current_batt_percentage_;
    bool curent_charging_status_;

    XPowersAXP2101 power;

public:
    esp_err_t init();
    esp_err_t refresh();

    int get_battery_percentage();
    bool is_charging();
};