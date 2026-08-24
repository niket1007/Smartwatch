#pragma once

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct MusicData
{
private:
    std::string music_name;
    std::string artist_names;

    SemaphoreHandle_t music_mutex_ = nullptr;

public:
    MusicData()
    {
        music_mutex_ = xSemaphoreCreateMutex();

        music_name = "Music";
        artist_names = "Artists";
    }

    std::string get_music_name() const
    {
        xSemaphoreTake(music_mutex_, portMAX_DELAY);
        std::string name = music_name;
        xSemaphoreGive(music_mutex_);
        return name;
    }

    std::string get_artist_names() const
    {
        xSemaphoreTake(music_mutex_, portMAX_DELAY);
        std::string names = artist_names;
        xSemaphoreGive(music_mutex_);
        return names;
    }

    void update(
        std::string mn,
        std::string ans)
    {
        xSemaphoreTake(music_mutex_, portMAX_DELAY);
        
        music_name = mn;
        artist_names = ans;

        xSemaphoreGive(music_mutex_);
    }
};

extern MusicData music_data;
