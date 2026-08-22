#pragma once

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "Screen/screen.h"
#include "Common/Custom_Data/nav_data.h"

class NavScreen : public Screen
{
private:
    esp_err_t draw_directions();
    esp_err_t draw_distance();
    esp_err_t draw_eta();

public:
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;

    esp_err_t draw() override;
    esp_err_t refresh(
        bool dir_signs = false,
        bool dist = false,
        bool eta = false);
    esp_err_t handle_events(uint32_t events) override;
};