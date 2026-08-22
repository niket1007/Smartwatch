#pragma once

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct WeatherData
{
private:
    int temp;
    int rain;
    int wind;
    int condition_code;
    std::string condition_text;
    std::string loc;

    SemaphoreHandle_t weather_mutex_ = nullptr;

public:
    WeatherData()
    {
        weather_mutex_ = xSemaphoreCreateMutex();

        temp = 0;
        rain = 0;
        wind = 0;
        condition_code = 0;
        condition_text = "Weather";
        loc = "Location";
    }

    int get_temp() const
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        int temperature = temp;
        xSemaphoreGive(weather_mutex_);
        return temperature;
    } 

    int get_rain_percentage() const
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        int rp = rain;
        xSemaphoreGive(weather_mutex_);
        return rp;
    }

    int get_wind_speed() const
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        int ws = wind;
        xSemaphoreGive(weather_mutex_);
        return ws;
    }

    int get_condition_code() const
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        int code = condition_code;
        xSemaphoreGive(weather_mutex_);
        return code;
    }

    std::string get_condition_text() const
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        std::string ct = condition_text;
        xSemaphoreGive(weather_mutex_);
        return ct;
    }

    std::string get_location() const
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        std::string location = loc;
        xSemaphoreGive(weather_mutex_);
        return location;
    }

    void update(
        int temperature,
        int rain_percentage,
        int wind_speed,
        int code,
        std::string text,
        std::string location)
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);

        temp = temperature;
        rain = rain_percentage;
        wind = wind_speed;
        condition_code = code;
        condition_text = text;
        loc = location;

        xSemaphoreGive(weather_mutex_);
    }

};

extern WeatherData weather_data;
