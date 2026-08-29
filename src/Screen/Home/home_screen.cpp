#include "home_screen.h"
#include "Screen/Generated/ui.h"
#include "Screen/Generated/images.h"
#include "Common/globals.h"

#include <time.h>
#include <sys/time.h>
#include <cstring>

static constexpr const char *TAG = "HOME_SCREEN";

esp_err_t HomeScreen::on_enter()
{
    esp_err_t ret = ESP_OK;

    ret = update_bat_charging_fields();
    if(ret != ESP_OK)    
        usb_serial.println("Failed to update charging fields");
    
    ret = update_bat_percent_fields();
    if(ret != ESP_OK)    
        usb_serial.println("Failed to update percentage fields");
    
    ret = update_datetime_fields();
    if(ret != ESP_OK)    
        usb_serial.println("Failed to update datetime fields");

    // TODO: Unhide the notification container and show number of notifications
    return ESP_OK;
}

const lv_image_dsc_t *HomeScreen::get_battery_icon(bool charging, int percent)
{
    if (charging)
        return &img_battery_charging;

    if (percent >= 0 && percent <= 20)
        return &img_battery_0_20;

    if (percent >= 21 && percent <= 49)
        return &img_battery_21_49;

    if (percent >= 50 && percent <= 79)
        return &img_battery_50_79;

    if (percent >= 80 && percent <= 100)
        return &img_battery_80_100;

    return &img_battery_0_20;
}

esp_err_t HomeScreen::update_bat_percent_fields()
{
    const int percent_int = battery_manager.get_battery_percentage();
    bool chg = battery_manager.is_charging();

    std::string percent = std::to_string(percent_int) + "%";
    if (percent.length() == 2)
    {
        percent = "0" + percent;
    }

    lv_label_set_text(
        objects.battery_percentage_label, percent.c_str());

    lv_bar_set_value(objects.battery_percentage_bar, percent_int, LV_ANIM_OFF);

    if (!chg)
    {
        lv_image_set_src(
            objects.battery_icon, get_battery_icon(chg, percent_int));
    }

    return ESP_OK;
}

esp_err_t HomeScreen::update_bat_charging_fields()
{
    bool chg = battery_manager.is_charging();
    const int percent_int = battery_manager.get_battery_percentage();

    lv_image_set_src(
        objects.battery_icon, get_battery_icon(chg, percent_int));

    return ESP_OK;
}

esp_err_t HomeScreen::update_datetime_fields()
{
    RTC_DateTime datetime = rtc_manager.get_current_datetime();

    int month = datetime.getMonth();
    if (month < 1 || month > 12) {
        month = 1;
    }

    // Validate day of week
    
    uint8_t day_of_week = datetime.getWeek();

    if (day_of_week < 0 || day_of_week > 6) {
        day_of_week = 0;
    }

    int day = datetime.getDay();
    int year = datetime.getYear();

    char date_str[32];
    snprintf(
        date_str,
        sizeof(date_str),
        "%02d - %s - %04d",
        day,
        months[month - 1],
        year
    );

    int hour = datetime.getHour();
    const char *ampm = "AM";

    if (hour >= 12) {
        ampm = "PM";
        if (hour > 12) hour -= 12;
    } else if (hour == 0) {
        hour = 12;
    }

    char time_str[16];
    snprintf(
        time_str,
        sizeof(time_str),
        "%02d : %02d %s",
        hour,
        datetime.getMinute(),
        ampm
    );


    if (std::strcmp(old_day_str, days[day_of_week]) != 0)
    {
        std::strncpy(old_day_str, days[day_of_week], sizeof(old_day_str) - 1);

        lv_label_set_text(objects.day_label, old_day_str);
    }

    if (std::strcmp(old_time_str, time_str) != 0)
    {
        std::strncpy(old_time_str, time_str, sizeof(old_time_str) - 1);

        lv_label_set_text(objects.time_label, old_time_str);
    }

    if (std::strcmp(old_date_str, date_str) != 0)
    {
        std::strncpy(old_date_str, date_str, sizeof(old_date_str) - 1);

        lv_label_set_text(objects.date_label, old_date_str);
    }

    return ESP_OK;
}

esp_err_t HomeScreen::on_exit()
{
    return ESP_OK;
}

esp_err_t HomeScreen::handle_events(uint32_t events)
{
    usb_serial.println("HomeScreen :: handle_events called");

    esp_err_t ret = ESP_OK;
    if (events & BAT_CHARGE_CHG_EVENT)
    {
        ret = update_bat_charging_fields();
        if(ret != ESP_OK)
            usb_serial.println("Failed to update charging fields");
    }

    if (events & BAT_PERCENT_CHG_EVENT)
    {
        ret = update_bat_percent_fields();
        if(ret != ESP_OK)    
            usb_serial.println("Failed to update percentage fields");
    }

    if (events & UPDATE_TIME_EVENT)
    {
        ret = update_datetime_fields();
        if(ret != ESP_OK)    
            usb_serial.println("Failed to update datetime fields");
    }

    return ESP_OK;
}