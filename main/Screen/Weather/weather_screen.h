#pragma once

#include <string>
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "Screen/screen.h"
#include "Graphics/Icons/generated/icon.h"
#include "Graphics/Icons/generated/icon_globals.h"
#include "Common/Custom_Data/weather_data.h"

class WeatherScreen : public Screen
{
private:
    bool toggle_rain_wind = true;

    icon_t get_icon(std::string);
    // WeatherData old_weather_data = {
    //     0, 0, 0, 100, "Weather", "Location"};
    esp_err_t draw_rain_block();
    esp_err_t draw_wind_block();
    esp_err_t reset_rain_wind_block();
    esp_err_t draw_temperature();
    esp_err_t draw_condition();
    esp_err_t draw_location();
    esp_err_t draw_icon();

public:
    esp_err_t on_enter();
    esp_err_t on_exit();

    esp_err_t draw() override;
    esp_err_t handle_events(uint32_t events) override;
};