#pragma once

#include "Screen/screen.h"
#include <string>

class WeatherScreen : public Screen
{  
private:
    esp_err_t update_screen();
    
public:
    ~WeatherScreen() = default;
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;
    esp_err_t handle_events(uint32_t events) override;
};