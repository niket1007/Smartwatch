#pragma once

#include <stdint.h>

#include "esp_err.h"
#include "screen.h"

#include "Screen/Menu/menu_screen.h"

class ScreenManager
{
private:
    bool initialized_ = false;
    int current_screen_id = -1;
    Screen *current_screen = nullptr;

    Screen *get_screen_instance(int id);
    esp_err_t navigate(int direction);

public:
    esp_err_t init();
    esp_err_t change_screen(int new_screen_id);
    esp_err_t handle_events(uint32_t events);
    esp_err_t load_default_screen();

    MenuScreen *get_current_menu_screen();
};