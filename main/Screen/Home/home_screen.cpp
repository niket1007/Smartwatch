#include <format>
#include <string>
#include "home_screen.h"
#include "Common/globals.h"
#include <time.h>
#include <sys/time.h>
#include <cstring>

static constexpr char *TAG = "HOME_SCREEN";

esp_err_t HomeScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    ESP_RETURN_ON_ERROR(
        draw_datetime_labels(), TAG, "Failed to draw datetime labels");

    ESP_RETURN_ON_ERROR(
        draw_battery_label(), TAG, "Failed to draw battery labels");

    ESP_RETURN_ON_ERROR(
        draw_battery_icon(), TAG, "Failed to draw battery icons");

    return ESP_OK;
}

esp_err_t HomeScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");
    return ESP_OK;
}

icon_t HomeScreen::get_icon(bool charging, int percent)
{
    if (charging)
        return icon_battery_charging;

    if (percent >= 0 && percent <= 10)
        return icon_battery_0_10;

    if (percent >= 11 && percent <= 20)
        return icon_battery_11_20;

    if (percent >= 21 && percent <= 30)
        return icon_battery_21_30;

    if (percent >= 31 && percent <= 50)
        return icon_battery_31_50;

    if (percent >= 51 && percent <= 70)
        return icon_battery_51_70;

    if (percent >= 71 && percent <= 90)
        return icon_battery_71_90;

    if (percent >= 91 && percent <= 100)
        return icon_battery_91_100;

    return icon_battery_default;
}

esp_err_t HomeScreen::draw_battery_icon()
{
    bool is_charging = battery_manager.is_charging();
    int bat_percent = battery_manager.get_battery_percentage();

    icon_t icon = get_icon(is_charging, bat_percent);

    if (old_icon.data != icon.data)
    {
        old_icon = icon;
        ESP_RETURN_ON_ERROR(
            graphics.fill_rect(80, 360, 100, 80, BLACK_COLOR),
            TAG, "Failed to re-draw battery symbol rect");

        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(80, 350, &icon, is_charging ? GREEN_COLOR : WHITE_COLOR),
            TAG, "Failed to re-draw battery symbol");
    }
    return ESP_OK;
}

esp_err_t HomeScreen::draw_battery_label()
{
    int bat_percent = battery_manager.get_battery_percentage();
    std::string percentage_str = "";

    if (bat_percent < 10)
    {
        percentage_str = std::format("0{}%", bat_percent);
    }
    else
    {
        percentage_str = std::format("{}%", bat_percent);
    }

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(230, 370, 95, 60, BLACK_COLOR),
        TAG, "Failed to re-draw battery percentage rect");

    ESP_RETURN_ON_ERROR(
        graphics.draw_text(240, 415, percentage_str.c_str(), freesans_40, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to re-draw battery percentage label");

    return ESP_OK;
}

esp_err_t HomeScreen::draw()
{
    // Containers
    // ESP_LOGI(TAG, "Drawing main container");
    ESP_RETURN_ON_ERROR(
        graphics.draw_round_rect(10, 10, 390, 480, 100, WHITE_COLOR),
        TAG, "Failed to draw main container");

    // ESP_LOGI(TAG, "Drawing battery container");
    ESP_RETURN_ON_ERROR(
        graphics.draw_round_rect(50, 340, 310, 120, 50, WHITE_COLOR),
        TAG, "Failed to draw battery container");

    return ESP_OK;
}

esp_err_t HomeScreen::draw_datetime_labels()
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char time_str[16];
    char date_str[32];
    char day_str[16];
    const int TEXT_AREA_CENTER_X = screen_manager.get_reset_screen_mid();
    int text_width = 0; 
    int x = 0;

    strftime(
        time_str, sizeof(time_str), "%I:%M %p", &timeinfo); // e.g., "09:46 PM"

    strftime(
        date_str, sizeof(date_str), "%b %d, %Y", &timeinfo); // e.g., "Jul 28, 2026"

    strftime(
        day_str, sizeof(day_str), "%A", &timeinfo); // e.g., "Tuesday"

    if (std::strcmp(old_day_str, day_str) != 0)
    {
        std::strncpy(old_day_str, day_str, sizeof(old_day_str) - 1);

        text_width = graphics.get_text_width(day_str, freesans_40);
        x = TEXT_AREA_CENTER_X - (text_width / 2);

        ESP_RETURN_ON_ERROR(
            graphics.fill_rect(x-10, 100, text_width+20, 40, BLACK_COLOR),
            TAG, "Failed to re-draw weekday rect");

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(x, 130, day_str, freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to re-draw weekday label");
    }

    if (std::strcmp(old_time_str, time_str) != 0)
    {
        std::strncpy(old_time_str, time_str, sizeof(old_time_str) - 1);

        text_width = graphics.get_text_width(time_str, freesans_50);
        x = TEXT_AREA_CENTER_X - (text_width / 2);

        ESP_RETURN_ON_ERROR(
            graphics.fill_rect(x-10, 145, text_width+20, 60, BLACK_COLOR),
            TAG, "Failed to re-draw time rect");

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(x, 190, time_str, freesans_50, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to re-draw time label");
    }

    if (std::strcmp(old_date_str, date_str) != 0)
    {
        std::strncpy(old_date_str, date_str, sizeof(old_date_str) - 1);

        text_width = graphics.get_text_width(date_str, freesans_40);
        x = TEXT_AREA_CENTER_X - (text_width / 2);

        ESP_RETURN_ON_ERROR(
            graphics.fill_rect(x-10, 200, text_width+20, 50, BLACK_COLOR),
            TAG, "Failed to re-draw date rect");

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(x, 240, date_str, freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to re-draw date label");
    }

    return ESP_OK;
}

esp_err_t HomeScreen::handle_events(uint32_t events)
{
    if (events & (BAT_PERCENT_CHG_EVENT | SCREEN_ON_EVENT))
    {
        ESP_RETURN_ON_ERROR(
            draw_battery_label(), TAG, "Failed to draw battery label");
    }

    if (events & (BAT_PERCENT_CHG_EVENT | BAT_CHARGE_CHG_EVENT | SCREEN_ON_EVENT))
    {
        ESP_RETURN_ON_ERROR(
            draw_battery_icon(), TAG, "Failed to draw battery icon");
    }

    if (events & (UPDATE_TIME_EVENT | SCREEN_ON_EVENT))
    {
        ESP_RETURN_ON_ERROR(
            draw_datetime_labels(), TAG, "Failed to draw datetime labels");
    }

    return ESP_OK;
}