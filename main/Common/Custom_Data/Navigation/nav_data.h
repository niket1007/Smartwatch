#pragma once

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct NavData
{
private:
    std::string direction;
    std::string estimated_time;
    int distance;

    SemaphoreHandle_t nav_mutex_ = nullptr;

public:
    NavData()
    {
        nav_mutex_ = xSemaphoreCreateMutex();

        direction = "";
        distance = 0;
        estimated_time = "Navigation";
    }

    std::string get_directions() const
    {
        xSemaphoreTake(nav_mutex_, portMAX_DELAY);
        std::string dir = direction;
        xSemaphoreGive(nav_mutex_);
        return dir;
    }

    std::string get_eta() const
    {
        xSemaphoreTake(nav_mutex_, portMAX_DELAY);
        std::string eta = estimated_time;
        xSemaphoreGive(nav_mutex_);
        return eta;
    }

    int get_distance()
    {
        xSemaphoreTake(nav_mutex_, portMAX_DELAY);
        int dist = distance;
        xSemaphoreGive(nav_mutex_);
        return dist;
    }

    void update(
        std::string dir,
        std::string eta,
        int dist)
    {
        xSemaphoreTake(nav_mutex_, portMAX_DELAY);

        direction = dir;
        distance = dist;
        estimated_time = eta;

        xSemaphoreGive(nav_mutex_);
    }
};

extern NavData nav_data;
