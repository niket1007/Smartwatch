#pragma once

#include <string>
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "Screen/screen.h"
#include "Graphics/Icons/generated/icon.h"
#include "Graphics/Icons/generated/icon_globals.h"
#include "Screen/Menu/rect.h"

class MenuScreenPage1 : public Screen
{
private:
    static constexpr Rect icon_containers[3] = {
        {60, 90, 300, 100},   // Notification
        {60, 210, 300, 100},  // Weather
        {60, 330, 300, 100}   // Calendar
    };

    void identify_tap();

public:
    int screen_id = 2;

    esp_err_t on_enter() override;
    esp_err_t on_exit() override;

    esp_err_t draw() override;
    esp_err_t handle_events(uint32_t events) override;
};