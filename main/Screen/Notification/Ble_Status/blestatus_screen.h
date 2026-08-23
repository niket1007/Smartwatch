#pragma once

#include "Screen/screen.h"
#include <string>

class BleStatusScreen : public Screen
{
private:
    std::string passkey = "";

    std::string get_status_text();
    esp_err_t update_screen();

public:
    ~BleStatusScreen() = default;
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;
    esp_err_t handle_events(uint32_t events) override;
};