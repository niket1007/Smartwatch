#include "i2c_manager.h"
#include "bsp/esp32_s3_touch_amoled_2_06.h"

#include "esp_err.h"
#include "esp_check.h"

static const char *TAG = "I2C_MANAGER";

esp_err_t I2CManager::init()
{
    ESP_RETURN_ON_ERROR(
        bsp_i2c_init(), TAG, "Failed to initialise the i2c");

    i2c_bus_handle = bsp_i2c_get_handle();

    // POWER
    i2c_device_config_t pmu_dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x34,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = 0}};

    ESP_RETURN_ON_ERROR(
        i2c_master_bus_add_device(
            i2c_bus_handle, &pmu_dev_config, &pmu_dev_handle),
        TAG, "Failed to add PMU device");

    // RTC
    i2c_device_config_t rtc_dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x51,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = 0}};

    ESP_RETURN_ON_ERROR(
        i2c_master_bus_add_device(i2c_bus_handle, &rtc_dev_config, &rtc_dev_handle),
        TAG, "Failed to add RTC device");

    return ESP_OK;
}