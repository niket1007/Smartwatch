#include "battery_manager.h"

XPowersAXP2101 power;
volatile int global_battery_percentage = 0;
volatile bool global_is_charging = false;

void power_init() {
    // Init PMU to stop the safety shutdown (Must happen first)
    Wire.begin(IIC_SDA, IIC_SCL);
    Wire.setClock(100000); 

    USBSerial.print("Initializing AXP2101 PMU...");
    if (power.begin(Wire, AXP2101_SLAVE_ADDRESS, IIC_SDA, IIC_SCL)) {
    USBSerial.println(" SUCCESS!");
    power.disableTSPinMeasure();
    power.enableBattDetection();
    power.enableBattVoltageMeasure();
    power.enableVbusVoltageMeasure(); // Force USB voltage reading on
    power.enableGauge();
    } else {
    USBSerial.println(" FAILED! (Chip not found on I2C bus)");
    }
}

int get_accurate_battery_percentage() {
    // 1. Check if PMU sees the battery
    if (!power.isBatteryConnect()) {
    USBSerial.print("[PMU Warning] Battery not physically detected! ");
    }
    int percentage = power.getBatteryPercent();
    USBSerial.println(percentage);
    return percentage;
}

void read_battery_sensor() {
    int calculated_percentage = 0;
    bool charging_status = false; 

    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(500))) { // Wait up to 500ms for lock
      calculated_percentage = get_accurate_battery_percentage();
      charging_status = power.isCharging();
      xSemaphoreGive(i2c_mutex);
    } else {
      USBSerial.println("Warning: I2C Mutex Lock Timeout in Task 0!");
    }
    USBSerial.printf("Is Charging: %s\n", charging_status ? "YES" : "NO");

    if (calculated_percentage != -1) {
      global_battery_percentage = calculated_percentage;
    }
    global_is_charging = charging_status;
}

void update_battery_ui() {
    static char batt_str[10];
    snprintf(batt_str, sizeof(batt_str), "%d%%", global_battery_percentage);
    lv_label_set_text(objects.battery_percentage_label, batt_str);

    // Update the bar length
    lv_bar_set_value(objects.battery_percentage_bar, global_battery_percentage, LV_ANIM_ON);

    // Update Colors and Status text
    if (global_is_charging) {
    // Set bar to Green 0x00FF00
    lv_obj_set_style_bg_color(objects.battery_percentage_bar, lv_color_hex(0x00FF00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_label_set_text(objects.battery_status_label, "Charging");
    } else {
    // Set bar back to default blue color #155dfc
    lv_obj_set_style_bg_color(objects.battery_percentage_bar, lv_color_hex(0x155dfc), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_label_set_text(objects.battery_status_label, "Charged");
    }
}