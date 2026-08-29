#pragma once

#include "Screen/screen.h"
#include <string>

class MenuScreen : public Screen
{
private:
    int selected_option_id = 0;

public:
    ~MenuScreen() = default;
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;
    esp_err_t handle_events(uint32_t events) override;

    void set_selected_option_id(int id);
    int get_selected_option_id();
};