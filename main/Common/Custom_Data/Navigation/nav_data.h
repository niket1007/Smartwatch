#pragma once

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct NavData
{
private:
    std::string direction;
    std::string estimated_time;
    std::string instructions;
    int distance;

    bool direction_dirty;
    bool eta_dirty;
    bool instr_dirty;
    bool distance_dirty;

    SemaphoreHandle_t nav_mutex_ = nullptr;

public:
    NavData()
    {
        nav_mutex_ = xSemaphoreCreateMutex();

        direction = "Navigation";
        distance = 0;
        instructions = "";
        estimated_time = "--:-- --";

        direction_dirty = false;
        eta_dirty = false;
        instr_dirty = false;
        distance_dirty = false;
    }

    std::string get_directions() const
    {
        xSemaphoreTake(nav_mutex_, portMAX_DELAY);
        std::string dir = direction;
        xSemaphoreGive(nav_mutex_);
        return dir;
    }

    bool is_direction_diirty()
    {
        xSemaphoreTake(nav_mutex_, portMAX_DELAY);
        bool flag = direction_dirty;
        xSemaphoreGive(nav_mutex_);
        return flag;
    }

    std::string get_eta() const
    {
        xSemaphoreTake(nav_mutex_, portMAX_DELAY);
        std::string eta = estimated_time;
        xSemaphoreGive(nav_mutex_);
        return eta;
    }

    bool is_eta_diirty()
    {
        xSemaphoreTake(nav_mutex_, portMAX_DELAY);
        bool flag = eta_dirty;
        xSemaphoreGive(nav_mutex_);
        return flag;
    }

    std::string get_instructions() const
    {
        xSemaphoreTake(nav_mutex_, portMAX_DELAY);
        std::string s = instructions;
        xSemaphoreGive(nav_mutex_);
        return s;
    }

    bool is_instr_dirty()
    {
        xSemaphoreTake(nav_mutex_, portMAX_DELAY);
        bool flag = instr_dirty;
        xSemaphoreGive(nav_mutex_);
        return flag;
    }

    int get_distance()
    {
        xSemaphoreTake(nav_mutex_, portMAX_DELAY);
        int dist = distance;
        xSemaphoreGive(nav_mutex_);
        return dist;
    }

    bool is_distance_diirty()
    {
        xSemaphoreTake(nav_mutex_, portMAX_DELAY);
        bool flag = distance_dirty;
        xSemaphoreGive(nav_mutex_);
        return flag;
    }

    void update(
        std::string dir,
        std::string eta,
        std::string instr,
        int dist)
    {
        xSemaphoreTake(nav_mutex_, portMAX_DELAY);

        direction_dirty = direction != dir;
        direction = dir;

        distance_dirty = distance != dist;
        distance = dist;

        eta_dirty = estimated_time != eta;
        estimated_time = eta;

        instr_dirty = instructions != instr;
        instructions = instr;

        xSemaphoreGive(nav_mutex_);
    }
};

extern NavData nav_data;
