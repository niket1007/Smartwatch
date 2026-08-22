#pragma once

#include <string>
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "Screen/screen.h"
#include "Graphics/Icons/generated/icon.h"
#include "Graphics/Icons/generated/icon_globals.h"

class NotifOtherScreen : public Screen
{
private:
    int32_t notif_counter = 0;
    int32_t MAX_NOTIF = 20;

    esp_err_t show_next_notif();

public:
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;

    esp_err_t draw() override;
    esp_err_t handle_events(uint32_t events) override;
};