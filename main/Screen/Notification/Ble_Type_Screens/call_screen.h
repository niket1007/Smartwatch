#pragma once

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "Screen/rect.h"
#include "Screen/screen.h"
#include "Common/Custom_Data/call_data.h"

class CallScreen : public Screen
{
private:

    static constexpr Rect icon_containers[3] = {
        {50, 300, 115, 115}, // Incoming - Accept
        {240, 300, 115, 115}, // Incoming - Reject
        {145, 300, 115, 115} // Outgoing - Reject
    };

    esp_err_t draw_contact_information();
    esp_err_t draw_status_icons();
    esp_err_t draw_action_icons();

    esp_err_t identify_tap();

public:
    esp_err_t on_enter() override;
    esp_err_t on_exit() override;

    esp_err_t draw() override;
    esp_err_t refresh(bool call_info = false, bool call_status = false);
    esp_err_t handle_events(uint32_t events) override;
};