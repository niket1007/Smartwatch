#pragma once

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "Screen/screen.h"

class PassKeyScreen: public Screen
{
    public:
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;

    esp_err_t draw() override;
    esp_err_t handle_events(uint32_t events) override;
};