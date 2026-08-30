#pragma once

#include <stdint.h>

#include "esp_err.h"
#include "screen.h"

#include "Screen/Menu/menu_screen.h"
#include "Screen/Settings/settings_screen.h"

class ScreenManager
{
private:
    int current_screen_id = -1;
    Screen *current_screen = nullptr;
    bool transition_running_ = false;

    Screen *get_screen_instance(int id);
    esp_err_t navigate(int direction);

public:
    esp_err_t init();
    esp_err_t load_default_screen();
    esp_err_t change_screen(int new_screen_id);
    esp_err_t handle_events(uint32_t events);
    int get_current_screen_id();

    MenuScreen *get_current_menu_screen();
    SettingsScreen *get_current_settings_screen();
};