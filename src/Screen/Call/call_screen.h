#pragma once

#include "Screen/screen.h"
#include <string>

class CallScreen : public Screen
{
private:
    esp_err_t update_screen();

    esp_err_t update_icons(std::string status);

public:
    ~CallScreen() = default;
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;
    esp_err_t handle_events(uint32_t events) override;
};