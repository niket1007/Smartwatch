#include "Screen/Home/home_screen.h"
#include "Screen/generated/ui.h"
#include "Screen/generated/images.h"
#include "Common/globals.h"

#include "esp_log.h"
#include <time.h>
#include <sys/time.h>
#include <cstring>

static constexpr const char *TAG = "HOME_SCREEN";

esp_err_t HomeScreen::on_enter()
{
    // ESP_LOGI(TAG, "on_enter called");

    ESP_RETURN_ON_ERROR(
        update_bat_charging_fields(),
        TAG, "Failed to update charging fields");
    ESP_RETURN_ON_ERROR(
        update_bat_percent_fields(),
        TAG, "Failed to update percentage fields");
    ESP_RETURN_ON_ERROR(
        update_datetime_fields(),
        TAG, "Failed to update datetime fields");

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
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char time_str[16];
    char date_str[32];
    char day_str[16];

    strftime(
        time_str, sizeof(time_str), "%I:%M %p", &timeinfo); // e.g., "09:46 PM"

    strftime(
        date_str, sizeof(date_str), "%b %d, %Y", &timeinfo); // e.g., "Jul 28, 2026"

    strftime(
        day_str, sizeof(day_str), "%A", &timeinfo); // e.g., "Tuesday"

    if (std::strcmp(old_day_str, day_str) != 0)
    {
        std::strncpy(old_day_str, day_str, sizeof(old_day_str) - 1);

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

    if (objects.calendar_calendar)
    {
        if ((cal_year != timeinfo.tm_year) or
            (cal_day != timeinfo.tm_mday) or (cal_month != timeinfo.tm_mon))
        {
            cal_year = timeinfo.tm_year;
            cal_month = timeinfo.tm_mon;
            cal_day = timeinfo.tm_mday;
            lv_calendar_set_today_date(
                objects.calendar_calendar, cal_year, cal_month, cal_day);
            // lv_calendar_set_month_shown(
            //     objects.calendar_calendar, cal_year, cal_month);
        }
    }

    return ESP_OK;
}

esp_err_t HomeScreen::on_exit()
{
    // ESP_LOGI(TAG, "on_exit called");

    return ESP_OK;
}

esp_err_t HomeScreen::handle_events(uint32_t events)
{
    if (events & BAT_CHARGE_CHG_EVENT)
    {
        ESP_RETURN_ON_ERROR(
            update_bat_charging_fields(),
            TAG, "Failed to update charging fields");
    }

    if (events & BAT_PERCENT_CHG_EVENT)
    {
        ESP_RETURN_ON_ERROR(
            update_bat_percent_fields(),
            TAG, "Failed to update percentage fields");
    }

    if (events & UPDATE_TIME_EVENT)
    {
        ESP_RETURN_ON_ERROR(
            update_datetime_fields(),
            TAG, "Failed to update datetime fields");
    }

    return ESP_OK;
}