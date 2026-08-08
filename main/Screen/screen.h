#pragma once

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

class Screen
{
public:
    virtual ~Screen() = default;

    virtual esp_err_t on_enter() = 0;
    virtual esp_err_t on_exit() = 0;
    virtual esp_err_t draw() = 0;

    virtual esp_err_t handle_events(uint32_t events) = 0;
};