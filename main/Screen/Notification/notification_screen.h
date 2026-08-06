#pragma once

#include <string>
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "Screen/screen.h"
#include "Graphics/Icons/generated/icon.h"
#include "Graphics/Icons/generated/icon_globals.h"

class NotificationScreen : public Screen
{
public:
    
    esp_err_t on_enter();
    esp_err_t on_exit();

    esp_err_t draw() override;
    esp_err_t handle_events(uint32_t events) override;
};