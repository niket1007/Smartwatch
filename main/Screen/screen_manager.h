#pragma once

#include "Screen/screen.h"
#include "Home/home_screen.h"
#include "Menu/Page_1/menu_screen_page_1.h"

class ScreenManager
{
    private:
        Screen* current_screen = nullptr;
    public:
        esp_err_t init();
        esp_err_t draw();
        esp_err_t change_screen(Screen* new_screen);
        esp_err_t handle_events(uint32_t events);
};