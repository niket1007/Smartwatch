#pragma once

#include "Screen/screen.h"

class ScreenManager
{
private:
    Screen *current_screen = nullptr;

    // Home Screen
    int current_screen_id = 1;

    // Home, Menu_Page1, Menu_Page2
    int total_screen = 3;

    int reset_screen_x = 50;
    int reset_screen_width = 310;
    int reset_screen_y = 40;
    int reset_screen_height = 420;

    Screen *get_screen_instance_(int screen_id);

public:
    esp_err_t init();
    esp_err_t draw();
    esp_err_t reset();
    int get_reset_screen_mid();
    esp_err_t change_screen(int new_screen_id);
    esp_err_t handle_events(uint32_t events);
};