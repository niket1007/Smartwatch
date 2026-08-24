#pragma once

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct WeatherData
{
private:
    float temp;
    float rain;
    float wind;
    float high;
    float low;
    float humidity;
    int condition_code;
    std::string condition_text;
    std::string loc;

    bool temp_dirty = false;
    bool high_dirty = false;
    bool low_dirty = false;
    bool humidity_dirty = false;
    bool rain_dirty = false;
    bool wind_dirty = false;
    bool code_dirty = false;
    bool text_dirty = false;
    bool loc_dirty = false;

    SemaphoreHandle_t weather_mutex_ = nullptr;

public:
    WeatherData()
    {
        weather_mutex_ = xSemaphoreCreateMutex();

        temp = 0;
        rain = 0;
        wind = 0;
        high = 0;
        low = 0;
        humidity = 0;
        condition_text = "Weather";
        loc = "Location";
    }

    float get_temp() const
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        float temperature = temp;
        xSemaphoreGive(weather_mutex_);
        return temperature;
    }

    bool is_temp_dirty()
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        bool flag = temp_dirty;
        xSemaphoreGive(weather_mutex_);
        return flag;
    }

    float get_high() const
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        float high_t = high;
        xSemaphoreGive(weather_mutex_);
        return high_t;
    }

    bool is_high_dirty()
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        bool flag = high_dirty;
        xSemaphoreGive(weather_mutex_);
        return flag;
    }

    float get_low() const
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        float low_t = low;
        xSemaphoreGive(weather_mutex_);
        return low_t;
    }

    bool is_low_dirty()
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        bool flag = low_dirty;
        xSemaphoreGive(weather_mutex_);
        return flag;
    }

    float get_humidity() const
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        float humidity_t = humidity;
        xSemaphoreGive(weather_mutex_);
        return humidity_t;
    }

    bool is_humidity_dirty()
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        bool flag = humidity_dirty;
        xSemaphoreGive(weather_mutex_);
        return flag;
    }

    float get_rain_percentage() const
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        float rp = rain;
        xSemaphoreGive(weather_mutex_);
        return rp;
    }

    bool is_rain_dirty()
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        bool flag = rain_dirty;
        xSemaphoreGive(weather_mutex_);
        return flag;
    }

    float get_wind_speed() const
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        float ws = wind;
        xSemaphoreGive(weather_mutex_);
        return ws;
    }

    bool is_wind_dirty()
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        bool flag = wind_dirty;
        xSemaphoreGive(weather_mutex_);
        return flag;
    }

    std::string get_condition_text() const
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        std::string ct = condition_text;
        xSemaphoreGive(weather_mutex_);
        return ct;
    }

    bool is_text_dirty()
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        bool flag = text_dirty;
        xSemaphoreGive(weather_mutex_);
        return flag;
    }

    std::string get_location() const
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        std::string location = loc;
        xSemaphoreGive(weather_mutex_);
        return location;
    }

    bool is_loc_dirty()
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);
        bool flag = loc_dirty;
        xSemaphoreGive(weather_mutex_);
        return flag;
    }

    void update(
        float temperature,
        float high_t,
        float low_t,
        float humidity_percent,
        float rain_percentage,
        float wind_speed,
        std::string text,
        std::string location)
    {
        xSemaphoreTake(weather_mutex_, portMAX_DELAY);

        temp_dirty = temp != temperature;
        temp = temperature;

        high_dirty = high != high_t;
        high = high_t;

        low_dirty = low != low_t;
        low = low_t;

        humidity_dirty = humidity != humidity_percent;
        humidity = humidity_percent;

        rain_dirty = rain != rain_percentage;
        rain = rain_percentage;
        
        wind_dirty = wind != wind_speed;
        wind = wind_speed;

        text_dirty = condition_text != text;
        condition_text = text;

        loc_dirty = loc != location;
        loc = location;

        xSemaphoreGive(weather_mutex_);
    }

};

extern WeatherData weather_data;
