#pragma once

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct NotifData
{
private:
    std::string direction;
    std::string estimated_time;
    int distance;

    SemaphoreHandle_t notif_mutex_ = nullptr;

public:
    NotifData()
    {
        notif_mutex_ = xSemaphoreCreateMutex();

        direction = "";
        distance = 0;
        estimated_time = "Navigation";
    }

    std::string get_directions() const
    {
        xSemaphoreTake(notif_mutex_, portMAX_DELAY);
        std::string dir = direction;
        xSemaphoreGive(notif_mutex_);
        return dir;
    }

    std::string get_eta() const
    {
        xSemaphoreTake(notif_mutex_, portMAX_DELAY);
        std::string eta = estimated_time;
        xSemaphoreGive(notif_mutex_);
        return eta;
    }

    int get_distance()
    {
        xSemaphoreTake(notif_mutex_, portMAX_DELAY);
        int dist = distance;
        xSemaphoreGive(notif_mutex_);
        return dist;
    }

    void update(
        std::string dir,
        std::string eta,
        int dist)
    {
        xSemaphoreTake(notif_mutex_, portMAX_DELAY);

        direction = dir;
        distance = dist;
        estimated_time = eta;

        xSemaphoreGive(notif_mutex_);
    }
};

extern NotifData notif_data;
