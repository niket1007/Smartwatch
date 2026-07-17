#include "weather_manager.h"
#include "ble_manager.h"
#include "ArduinoJson.h"

Weather_Details wd_notif = {
    "--°C",
    "--°C",
    "--°C",
    "--%",
    "---%",
    "--.--\nkm/h",
    "-----------",
    "-----------"
};

void send_weather_update_command() {
    JsonDocument json_doc;
    json_doc["t"] = "weather";
    json_doc["v"] = 1;

    bool result = ble_send_to_phone(json_doc);
    if(!result) {
        ts_var.weather_refresh = 0;
    } else {
        ts_var.weather_refresh = 2;
    }
}

void update_weather_details(
    const String &temp, const String &hi, const String &lo, const String &hum, 
    const String &rain, const String &wind, const String &txt, const String &loc) {
    wd_notif.temp = temp + "°C";
    wd_notif.hi = hi + "°C";
    wd_notif.lo = lo + "°C";
    wd_notif.hum = hum + "%";
    wd_notif.rain = rain + "%";
    wd_notif.wind = wind + "\nkm/h";
    wd_notif.txt = txt;
    wd_notif.loc = String(LV_SYMBOL_GPS) + " " + loc;

    ts_var.weather_refresh = 3;
}

void action_weather_refresh_clicked(lv_event_t * e) {
    if(ts_var.weather_refresh == 0) {
        usb_serial.println("Refresh weather screen called");
        ts_var.weather_refresh = 1;
    }
}

void update_weather_screen_ui() {

    usb_serial.println("Update weather screen called");
    
    if(objects.temp_label) {
        lv_label_set_text(objects.temp_label, wd_notif.temp.c_str());
    }

    if(objects.high_temp_label) {
        lv_label_set_text(objects.high_temp_label, wd_notif.hi.c_str());
    }

    if(objects.low_temp_label) {
        lv_label_set_text(objects.low_temp_label, wd_notif.lo.c_str());
    }

    if(objects.humidity_label) {
        lv_label_set_text(objects.humidity_label, wd_notif.hum.c_str());
    }

    if(objects.wind_label) {
        lv_label_set_text(objects.wind_label, wd_notif.wind.c_str());
    }

    if(objects.rain_chance_label) {
        lv_label_set_text(objects.rain_chance_label, wd_notif.rain.c_str());
    }

    if(objects.loc_label) {
        lv_label_set_text(objects.loc_label, wd_notif.loc.c_str());
    }

    if(objects.weather_type_label) {
        lv_label_set_text(objects.weather_type_label, wd_notif.txt.c_str());
    }

    ts_var.weather_refresh = 0;
}