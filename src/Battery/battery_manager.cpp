#include "battery_manager.h"
#include "Common/globals.h"

static const char *TAG = "BATTERY_MANAGER";

esp_err_t BatteryManager::init()
{
    if (power.begin(Wire, AXP2101_SLAVE_ADDRESS, IIC_SDA, IIC_SCL))
    {
        power.disableTSPinMeasure();
        power.enableBattDetection();
        power.enableBattVoltageMeasure();
        power.enableVbusVoltageMeasure();
        power.enableGauge();

        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t BatteryManager::refresh()
{
    if (power.isBatteryConnect())
    {
        int battery_percentage_ = power.getBatteryPercent();
        if (battery_percentage_ < 0)
            battery_percentage_ = 0;

        if (battery_percentage_ > 100)
            battery_percentage_ = 100;

        
        
        if (battery_percentage_ != current_batt_percentage_)
        {
            current_batt_percentage_ = battery_percentage_;

            // if (gui_task_handle != nullptr)
            // {
            //     xTaskNotify(
            //         gui_task_handle, BAT_PERCENT_CHG_EVENT, eSetBits);
            // }

            ESP_LOGI(TAG, "getBatteryPercent: %d %%", current_batt_percentage_);
            // usb_serial.printf("Battery Percentage %d\n", current_batt_percentage_);
        }
    }

    bool is_charging_ = power.isVbusIn() || power.isCharging();
    if (is_charging_ != curent_charging_status_)
    {
        curent_charging_status_ = is_charging_;

        // if (gui_task_handle != nullptr)
        // {
        //     xTaskNotify(
        //         gui_task_handle, BAT_CHARGE_CHG_EVENT, eSetBits);
        // }
        ESP_LOGI(TAG, "is_charging: %d %%", is_charging_);

        // usb_serial.printf("Is Charging: %s\n", curent_charging_status_ ? "YES" : "NO");
    }
    return ESP_OK;
}

int BatteryManager::get_battery_percentage()
{
    return current_batt_percentage_;
}

bool BatteryManager::is_charging()
{
    return curent_charging_status_;
}