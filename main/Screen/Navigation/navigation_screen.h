#pragma once

#include "Screen/screen.h"
#include <string>
#include "lvgl.h"

class NavigationScreen : public Screen
{   
private:
    const lv_image_dsc_t *get_direction_icon(std::string action);
    esp_err_t update();

public:
    ~NavigationScreen() = default;
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;
    esp_err_t handle_events(uint32_t events) override;
};