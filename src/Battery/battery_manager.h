#pragma once

#include "XPowersLib.h"
#include "esp_err.h"

class BatteryManager
{
private:
    int current_batt_percentage_ = -1;
    bool curent_charging_status_ = false;
    uint32_t current_touch_event_status = 0;
    uint32_t current_ble_event_status = 0;

    XPowersAXP2101 power;

public:
    esp_err_t init();
    esp_err_t refresh();

    int get_battery_percentage();
    bool is_charging();
};