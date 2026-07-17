#include "battery_manager.h"

XPowersAXP2101 power;

static int battery_percentage = 0;
static bool is_charging = false;

static portMUX_TYPE battery_mux = portMUX_INITIALIZER_UNLOCKED;

const uint32_t COLOR_CHARGING = 0x00FF00;
const uint32_t COLOR_DISCHARGING = 0x155dfc;

int get_battery_percentage() {
    portENTER_CRITICAL(&battery_mux);
    int val = battery_percentage;
    portEXIT_CRITICAL(&battery_mux);
    return val;
}

void set_battery_percentage(int percentage) {
    portENTER_CRITICAL(&battery_mux);
    battery_percentage = percentage;
    portEXIT_CRITICAL(&battery_mux);
}

bool is_battery_charging() {
    portENTER_CRITICAL(&battery_mux);
    bool val = is_charging;
    portEXIT_CRITICAL(&battery_mux);
    return val;
}

void set_battery_charging(bool charging) {
    portENTER_CRITICAL(&battery_mux);
    is_charging = charging;
    portEXIT_CRITICAL(&battery_mux);
}

void power_init() {
    usb_serial.print("Initializing AXP2101 PMU...");
    if (power.begin(Wire, AXP2101_SLAVE_ADDRESS, IIC_SDA, IIC_SCL)) {
        usb_serial.println(" SUCCESS!");
        power.disableTSPinMeasure();
        power.enableBattDetection();
        power.enableBattVoltageMeasure();
        power.enableVbusVoltageMeasure();
        power.enableGauge();
    } else {
        usb_serial.println(" FAILED! (Chip not found on I2C bus)");
    }
}

int read_pmu_battery_percent() {
    // 1. Check if PMU sees the battery
    if (!power.isBatteryConnect()) {
    usb_serial.print("[PMU Warning] Battery not physically detected! ");
    }
    int percentage = power.getBatteryPercent();
    // usb_serial.println(percentage);
    return percentage;
}

void read_battery_sensor() {
    int calculated_percentage = 0;
    bool charging_status = false;   

    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(500))) { // Wait up to 500ms for lock
      calculated_percentage = read_pmu_battery_percent();
      charging_status = power.isCharging();
      xSemaphoreGive(i2c_mutex);
    } else {
    //   usb_serial.println("Warning: I2C Mutex Lock Timeout in Task 0!");
    }
    // usb_serial.printf("Is Charging: %s\n", charging_status ? "YES" : "NO");

    if (calculated_percentage != -1) {
        set_battery_percentage(calculated_percentage);
    }
    set_battery_charging(charging_status);
}

void update_battery_ui()
{
    int current_percentage = get_battery_percentage();
    bool charging = is_battery_charging();

    char batt_str[10];
    snprintf(batt_str, sizeof(batt_str), "%d%%", current_percentage);

    if (objects.battery_percentage_label) {
        lv_label_set_text(objects.battery_percentage_label, batt_str);
    }

    if (objects.battery_percentage_bar) {
        lv_bar_set_value(
            objects.battery_percentage_bar, get_battery_percentage(), LV_ANIM_OFF );
    }

    if (is_battery_charging()) {
        if (objects.battery_percentage_bar) {
            lv_obj_set_style_bg_color(
                objects.battery_percentage_bar, 
                lv_color_hex(COLOR_CHARGING), LV_PART_INDICATOR | LV_STATE_DEFAULT);
        }

        if (objects.battery_status_label) {
            lv_label_set_text(objects.battery_status_label, "Charging");
        }
    }
    else {
        if (objects.battery_percentage_bar) {
            lv_obj_set_style_bg_color(
                objects.battery_percentage_bar, 
                lv_color_hex(COLOR_DISCHARGING), LV_PART_INDICATOR | LV_STATE_DEFAULT);
        }

        if (objects.battery_status_label) {
            lv_label_set_text( objects.battery_status_label, "Battery");
        }
    }
}

void send_battery_related_information() {
    int batt = get_battery_percentage();
    bool charging_now = is_battery_charging();

    JsonDocument json_doc;
    json_doc["t"] = "status";
    json_doc["bat"] = batt;
    json_doc["chg"] = charging_now ? 1: 0;
    ble_send_to_phone(json_doc);
}