#include "weather_screen.h"
#include "Screen/screen_manager.h"
#include "Common/globals.h"

static constexpr char *TAG = "WEATHER_SCREEN";

esp_err_t WeatherScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    return ESP_OK;
}

esp_err_t WeatherScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");
    return ESP_OK;
}

esp_err_t WeatherScreen::reset_rain_wind_block()
{
    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(80, 340, 260, 110, BLACK_COLOR),
        TAG, "Failed to fill the rect");

    return ESP_OK;
}

esp_err_t WeatherScreen::draw_rain_block()
{
    reset_rain_wind_block();

    ESP_RETURN_ON_ERROR(
        graphics.draw_icon(70, 350, &icon_weather_rain, WHITE_COLOR),
        TAG, "Failed to draw rain icon");

    std::string rain = std::to_string(old_weather_data.rain) + "0.00%";
    ESP_RETURN_ON_ERROR(
        graphics.draw_text(200, 410, rain.c_str(), freesans_40, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw rain text");

    return ESP_OK;
}

esp_err_t WeatherScreen::draw_wind_block()
{
    reset_rain_wind_block();

    ESP_RETURN_ON_ERROR(
        graphics.draw_icon(70, 350, &icon_weather_wind, WHITE_COLOR),
        TAG, "Failed to draw wind icon");

    std::string wind = std::to_string(old_weather_data.wind) + "0 km/h";
    ESP_RETURN_ON_ERROR(
        graphics.draw_text(200, 410, wind.c_str(), freesans_40, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw wind text");

    return ESP_OK;
}

esp_err_t WeatherScreen::draw_temperature()
{
    std::string temp = std::to_string(old_weather_data.temp) + "0C";

    int text_width = graphics.get_text_width(temp.c_str(), freesans_80);
    // The screen manager clears the area
    // Therefore the center of the drawable area half of that
    const int TEXT_AREA_CENTER_X = screen_manager.get_reset_screen_mid();
    int x = TEXT_AREA_CENTER_X - (text_width / 2);

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(x - 10, 180, text_width + 20, 80, BLACK_COLOR),
        TAG, "Failed to fill temperature block");

    ESP_RETURN_ON_ERROR(
        graphics.draw_text(x, 250, temp.c_str(), freesans_80, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw temperature text");
    
    return ESP_OK;
}

esp_err_t WeatherScreen::draw_condition()
{
    int text_width = graphics.get_text_width(old_weather_data.text.c_str(), freesans_40);
    // The screen manager clears the area
    // Therefore the center of the drawable area half of that
    const int TEXT_AREA_CENTER_X = screen_manager.get_reset_screen_mid();
    int x = TEXT_AREA_CENTER_X - (text_width / 2);

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(x - 10, 130, text_width + 20, 60, BLACK_COLOR),
        TAG, "Failed to fill condition block");

    ESP_RETURN_ON_ERROR(
        graphics.draw_text(
            x, 170, old_weather_data.text.c_str(), 
            freesans_40, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw condition text");
    
    return ESP_OK;
}

esp_err_t WeatherScreen::draw_location()
{
    int text_width = graphics.get_text_width(old_weather_data.loc.c_str(), freesans_40);
    // The screen manager clears the area
    // Therefore the center of the drawable area half of that
    const int TEXT_AREA_CENTER_X = screen_manager.get_reset_screen_mid();
    int x = TEXT_AREA_CENTER_X - (text_width / 2);

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(x - 10, 260, text_width + 20, 50, BLACK_COLOR),
        TAG, "Failed to fill location block");

    ESP_RETURN_ON_ERROR(
        graphics.draw_text(x, 300, old_weather_data.loc.c_str(), freesans_40, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw location text");
    
    return ESP_OK;
}

esp_err_t WeatherScreen::draw_icon()
{
    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(150, 40, 110, 120, BLACK_COLOR),
        TAG, "Failed to fill icon block");

    ESP_RETURN_ON_ERROR(
        graphics.draw_icon(160, 40, &icon_weather_cloud, WHITE_COLOR),
        TAG, "Failed to draw icon");

    return ESP_OK;
}

esp_err_t WeatherScreen::draw()
{
    ESP_RETURN_ON_ERROR(
        draw_icon(), TAG, "Failed to draw condition icon");

    ESP_RETURN_ON_ERROR(
        draw_condition(), TAG, "Failed to draw condition");

    ESP_RETURN_ON_ERROR(
        draw_temperature(), TAG, "Failed to draw temperature");

    ESP_RETURN_ON_ERROR(
        draw_location(), TAG, "Failed to draw location");


    ESP_RETURN_ON_ERROR(
        graphics.draw_round_rect(60, 330, 290, 130, 30, WHITE_COLOR),
        TAG, "Failed to draw rain-wind container");

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