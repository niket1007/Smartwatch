#pragma once

#include "Screen/screen.h"
#include <string>

class WifiScreen : public Screen
{  
private:
    void update_saved_ssid_label();
    
public:
    ~WifiScreen() = default;
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;
    esp_err_t handle_events(uint32_t events) override;
};