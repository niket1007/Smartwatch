#pragma once

#include <string>
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "Screen/screen.h"
#include "Graphics/Icons/generated/icon.h"
#include "Graphics/Icons/generated/icon_globals.h"
#include "Screen/rect.h"

class SettingsScreen : public Screen
{
private:

    static constexpr Rect icon_containers[4] = {
        {50, 90, 115, 115},  // Bluetooth
        {240, 90, 115, 115}, // Wifi
        {50, 300, 115, 115},  // Brightness
        {240, 300, 115, 115} // Info
    };

    esp_err_t identify_tap();

public:
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;

    esp_err_t draw() override;
    esp_err_t handle_events(uint32_t events) override;
};