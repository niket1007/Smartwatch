#include "battery_manager.h"
#include "Common/globals.h"

static const char *TAG = "BATTERY_MANAGER";

esp_err_t BatteryManager::init()
{
    if (power.begin(Wire, AXP2101_SLAVE_ADDRESS, IIC_SDA, IIC_SCL))
    {
        power.disableTSPinMeasure();
        power.enableBattDetection();
        // power.enableBattVoltageMeasure();
        // power.enableVbusVoltageMeasure();
        power.enableGauge();

        power.disableALDO1(); // Handles power to speaker and mic on board

        errored = false;

        return ESP_OK;
    }

    errored = true;
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

            if (gui_task_handle != nullptr)
            {
                uint32_t events = 0;

                if (!display_manager.is_sleeping())
                {
                    events |= BAT_PERCENT_CHG_EVENT;
                }

                if (current_batt_percentage_ < 20 &&
                    current_touch_event_status != TOUCH_DISABLED_EVENT)
                {
                    events |= TOUCH_DISABLED_EVENT;
                    current_touch_event_status = TOUCH_DISABLED_EVENT;
                }
                else if (current_batt_percentage_ >= 25 &&
                        current_touch_event_status != TOUCH_ENABLED_EVENT)
                {
                    events |= TOUCH_ENABLED_EVENT;
                    current_touch_event_status = TOUCH_ENABLED_EVENT;
                }

                if (events != 0)
                {
                    xTaskNotify(gui_task_handle, events, eSetBits);
                    // usb_serial.println("Batt Event sent to gui");
                }
            }

            if(background_task_handle != nullptr)
            {
                uint32_t event = 0;
                if ((current_batt_percentage_ < 20) and 
                (current_ble_event_status != BLUETOOTH_DEINIT_EVENT))
                {
                    event = BLUETOOTH_DEINIT_EVENT;
                    current_ble_event_status = BLUETOOTH_DEINIT_EVENT;
                }
                else if ((current_batt_percentage_ >= 25) and 
                (current_ble_event_status != BLUETOOTH_INIT_EVENT))
                {
                    event = BLUETOOTH_INIT_EVENT;
                    current_ble_event_status = BLUETOOTH_INIT_EVENT;
                }

                if (event != 0)
                {
                    xTaskNotify(background_task_handle, event, eSetBits);
                    // usb_serial.println("Batt Event sent to background");
                }
            }

            // usb_serial.printf("Battery Percentage %d\n", current_batt_percentage_);
        }
        // usb_serial.printf(
        //     "Battery Percentage %d %d\n", current_batt_percentage_, battery_percentage_);
    }

    bool is_charging_ = power.isVbusIn() || power.isCharging();
    if (is_charging_ != curent_charging_status_)
    {
        curent_charging_status_ = is_charging_;

        if (gui_task_handle != nullptr and !display_manager.is_sleeping())
        {
            xTaskNotify(
                gui_task_handle, BAT_CHARGE_CHG_EVENT, eSetBits);
            // usb_serial.println("BAT_CHARGE_CHG_EVENT sent to gui");
        }
        // usb_serial.printf("Is Charging: %s\n", curent_charging_status_ ? "YES" : "NO");
    }
    // usb_serial.printf(
    //     "Is Charging: %s %s\n", 
    //     curent_charging_status_ ? "YES" : "NO",
    //     is_charging_ ? "YES" : "NO");

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
