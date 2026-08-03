#include "i2c_manager.h"

#define PMU_INPUT_PIN (gpio_num_t) CONFIG_PMU_INTERRUPT_PIN
#define PMU_INPUT_PIN_SEL (1ULL << PMU_INPUT_PIN)

#define I2C_MASTER_NUM (i2c_port_num_t) CONFIG_I2C_MASTER_PORT_NUM
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY
#define I2C_MASTER_SDA_IO (gpio_num_t) CONFIG_PMU_I2C_SDA
#define I2C_MASTER_SCL_IO (gpio_num_t) CONFIG_PMU_I2C_SCL
#define I2C_MASTER_TIMEOUT_MS 1000

static const char *TAG = "I2C_MANAGER";

esp_err_t I2CManager::init()
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = 1,
            .allow_pd = 0}};

    i2c_new_master_bus(&bus_config, &i2c_bus_handle);

    // POWER
    i2c_device_config_t pmu_dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x34,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = 0}};

    i2c_master_bus_add_device(i2c_bus_handle, &pmu_dev_config, &pmu_dev_handle);

    // RTC
    i2c_device_config_t rtc_dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x51,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = 0}};

    i2c_master_bus_add_device(i2c_bus_handle, &rtc_dev_config, &rtc_dev_handle);

    return ESP_OK;
}