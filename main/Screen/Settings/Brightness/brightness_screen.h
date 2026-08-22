#pragma once

#include <string>
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "Screen/screen.h"
#include "Screen/rect.h"
#include "Graphics/Icons/generated/icon.h"
#include "Graphics/Icons/generated/icon_globals.h"

class BrightnessScreen : public Screen
{
    static constexpr Rect icon_containers[2] = {
        {150, 50, 115, 115},  // Minus
        {150, 320, 115, 115}, // Plus
    };

    uint32_t current_brightness = 0;
    bool is_updating = false;

    esp_err_t identify_tap();
    esp_err_t update_brightness(bool sign);
    esp_err_t draw_brightness_text();

public:
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;

    esp_err_t draw() override;
    esp_err_t handle_events(uint32_t events) override;
};