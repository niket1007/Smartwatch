#include "weather_screen.h"
#include "Screen/screen_manager.h"
#include "Common/globals.h"

static constexpr const char *TAG = "WEATHER_SCREEN";

esp_err_t WeatherScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(50, 40, 310, 420, BLACK_COLOR),
        TAG, "Failed to reset screen");

    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    return ESP_OK;
}

esp_err_t WeatherScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");
    return ESP_OK;
}

esp_err_t WeatherScreen::draw_rain_block()
{
    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(80, 330, 250, 110, BLACK_COLOR),
        TAG, "Failed to fill the rect"
    );

    ESP_RETURN_ON_ERROR(
        graphics.draw_icon(80, 350, &icon_rain_chance, WHITE_COLOR),
        TAG, "Failed to draw rain icon");

    std::string rain = std::to_string(old_weather_data.rain) + "0%";
    ESP_RETURN_ON_ERROR(
        graphics.draw_text(200, 400, rain.c_str(), freesans_50, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw rain text");

    return ESP_OK;
}

esp_err_t WeatherScreen::draw_wind_block()
{
    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(80, 330, 250, 110, BLACK_COLOR),
        TAG, "Failed to fill the rect"
    );

    ESP_RETURN_ON_ERROR(
        graphics.draw_icon(80, 350, &icon_wind, WHITE_COLOR),
        TAG, "Failed to draw wind icon");

    std::string wind = std::to_string(old_weather_data.wind) + "0km/h";
    ESP_RETURN_ON_ERROR(
        graphics.draw_text(170, 400, wind.c_str(), freesans_50, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw wind text");

    return ESP_OK;
}

esp_err_t WeatherScreen::draw()
{
    ESP_RETURN_ON_ERROR(
        graphics.draw_icon(160, 50, &icon_weather, WHITE_COLOR),
        TAG, "Failed to draw weather condition icon");

    ESP_RETURN_ON_ERROR(
        graphics.draw_text(130, 170, old_weather_data.text.c_str(), freesans_40, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw condition text");

    std::string temp = std::to_string(old_weather_data.temp) + "0C";
    ESP_RETURN_ON_ERROR(
        graphics.draw_text(140, 250, temp.c_str(), freesans_80, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw temperature text");

    ESP_RETURN_ON_ERROR(
        graphics.draw_text(130, 300, old_weather_data.loc.c_str(), freesans_40, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw location text");

    ESP_RETURN_ON_ERROR(
        graphics.draw_round_rect(60, 320, 290, 140, 30, WHITE_COLOR),
        TAG, "Failed to draw rain container");

    if (toggle_rain_wind)
    {
        draw_rain_block();
    }
    else
    {
        draw_wind_block();
    }

    return ESP_OK;
}

esp_err_t WeatherScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Return back to Menu Page 2
        return screen_manager.change_screen(2);
    }
    if (events & SINGLE_TAP_EVENT)
    {
        toggle_rain_wind = !toggle_rain_wind;
        if (toggle_rain_wind)
        {
            draw_rain_block();
        }
        else
        {
            draw_wind_block();
        }
    }
    return ESP_OK;
}