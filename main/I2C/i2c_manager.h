#pragma once

#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

class I2CManager {
    public:
        i2c_master_bus_handle_t i2c_bus_handle = NULL;
        i2c_master_dev_handle_t pmu_dev_handle = NULL;
        i2c_master_dev_handle_t rtc_dev_handle = NULL;

        esp_err_t init();
};