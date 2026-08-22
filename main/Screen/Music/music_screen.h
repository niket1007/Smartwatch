#pragma once

#include <string>
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "Screen/screen.h"
#include "Screen/rect.h"
#include "Graphics/Icons/generated/icon.h"
#include "Graphics/Icons/generated/icon_globals.h"
#include "Common/Custom_Data/music_data.h"

class MusicScreen : public Screen
{
private:
    static constexpr Rect icon_containers[4] = {
        {60, 220, 100, 100},  // Volume Up
        {250, 220, 100, 100}, // Volume Down
        {60, 350, 100, 100}, // Prev
        {250, 350, 100, 100} // Next
    };

    esp_err_t identify_tap();
    esp_err_t draw_music_name();
    esp_err_t draw_artist_names();

public:
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;

    esp_err_t draw() override;
    esp_err_t refresh(bool music = false, bool artist = false);
    esp_err_t handle_events(uint32_t events) override;
};