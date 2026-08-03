#pragma once

#define XPOWERS_CHIP_AXP2101

#include "XPowersLib.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

class BatteryManager {
    private:
        static int pmu_register_read(
            uint8_t devAddr, 
            uint8_t regAddr, 
            uint8_t *data, 
            uint8_t len);

        static int pmu_register_write_byte(
            uint8_t devAddr, 
            uint8_t regAddr, 
            uint8_t *data, 
            uint8_t len);
        
        XPowersPMU PMU;
        
        int old_battery_percentage_ = 0;
        int battery_percentage_ = 0;

        bool old_is_charging_ = false;
        bool is_charging_ = false;
    public:
        esp_err_t init();
        esp_err_t refresh();

        int get_battery_percentage();
        bool is_charging();
};