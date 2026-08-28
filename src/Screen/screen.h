#pragma once

#include "esp_err.h"

class Screen
{
public:
    virtual ~Screen() = default;

    virtual esp_err_t on_enter() = 0;
    virtual esp_err_t on_exit() = 0;
    virtual esp_err_t handle_events(uint32_t events) = 0;
};