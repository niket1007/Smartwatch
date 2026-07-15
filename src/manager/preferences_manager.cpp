#include "preferences_manager.h"
#include "Preferences.h"
#include "rtc_datetime_manager.h"

Preferences prefernces;

void store_power_off_time() {
    if(!load_current_datetime()) return;

    char datetime_str[20];
    snprintf(datetime_str,
            sizeof(datetime_str),
            "%04d-%02d-%02d %02d:%02d:%02d",
            current_datetime.getYear(),
            current_datetime.getMonth(),
            current_datetime.getDay(),
            current_datetime.getHour(),
            current_datetime.getMinute(),
            current_datetime.getSecond());


    preferences.begin("SETTINGS", false);
    preferences.putString("SHUTDOWN_TIME", datetime_str);
    preferences.end();
}


String fetch_power_off_time() {

    preferences.begin("SETTINGS", true);
    String shutdown_time = preferences.getString("SHUTDOWN_TIME");
    preferences.end();
    
    return shutdown_time;
}


