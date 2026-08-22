#include "call_screen.h"

#include "Common/globals.h"
#include "Screen/screen_manager.h"
#include "Graphics/Icons/generated/icon_globals.h"
#include "Graphics/Fonts/generated/font_globals.h"

static constexpr const char *TAG = "CALL_SCREEN";

esp_err_t CallScreen::on_enter()
{
    ESP_LOGI(TAG, "on enter called");

    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    return ESP_OK;
}

esp_err_t CallScreen::on_exit()
{
    ESP_LOGI(TAG, "on exit called");
    return ESP_OK;
}

esp_err_t CallScreen::draw_contact_information()
{
    const int TEXT_AREA_CENTER_X = screen_manager.get_reset_screen_mid();

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(50, 170, 310, 120, BLACK_COLOR),
        TAG, "Failed to fill contact info container");

    const std::string contact_name = call_data.get_contact_name();
    int text_width = graphics.get_text_width(contact_name.c_str(), freesans_50);
    int x = TEXT_AREA_CENTER_X - (text_width / 2);

    ESP_RETURN_ON_ERROR(
        graphics.draw_text(x, 220, contact_name.c_str(),
                           freesans_50, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw contact name");

    const std::string number = call_data.get_contact_number();
    text_width = graphics.get_text_width(number.c_str(), freesans_40);
    x = TEXT_AREA_CENTER_X - (text_width / 2);

    ESP_RETURN_ON_ERROR(
        graphics.draw_text(x, 270, number.c_str(),
                           freesans_40, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw contact number");

    return ESP_OK;
}

esp_err_t CallScreen::draw_status_icons()
{
    icon_t icon = icon_phone_default;

    std::string status = call_data.get_contact_status();
    if (status == "accept" || status == "start")
    {
        icon = icon_phone_call;
    }
    else if (status == "incoming")
    {
        icon = icon_phone_incoming;
    }
    else if (status == "outgoing")
    {
        icon = icon_phone_outgoing;
    }
    else if (status == "end")
    {
        icon = icon_phone_end;
    }
    else
    {
        icon = icon_phone_default;
    }

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(130, 40, 110, 110, BLACK_COLOR),
        TAG, "Failed to fill staus container");

    ESP_RETURN_ON_ERROR(
        graphics.draw_icon(150, 50, &icon, WHITE_COLOR),
        TAG, "Failed to draw status icon");

    return ESP_OK;
}

esp_err_t CallScreen::draw_action_icons()
{
    std::string status = call_data.get_contact_status();

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(50, 300, 310, 120, BLACK_COLOR),
        TAG, "Failed to fill the action icons container");

    if (status == "incoming")
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_round_rect(
                icon_containers[0].x1, icon_containers[0].y1, 
                icon_containers[0].width, icon_containers[0].height, 30, WHITE_COLOR),
            TAG, "Failed to draw accept icon container");

        ESP_RETURN_ON_ERROR(
            graphics.draw_round_rect(
                icon_containers[1].x1, icon_containers[1].y1, 
                icon_containers[1].width, icon_containers[1].height, 30, WHITE_COLOR),
            TAG, "Failed to draw reject icon container");
        
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(60, 305, &icon_phone_default, GREEN_COLOR),
            TAG, "Failed to draw accept icon");
        
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(250, 305, &icon_phone_reject, WHITE_COLOR),
            TAG, "Failed to draw reject icon");
    }
    else if (status == "outgoing" || status == "accept" || status == "start")
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_round_rect(
                icon_containers[2].x1, icon_containers[2].y1, 
                icon_containers[2].width, icon_containers[2].height, 30, WHITE_COLOR),
            TAG, "Failed to draw reject icon container");
        
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(155, 305, &icon_phone_reject, WHITE_COLOR),
            TAG, "Failed to draw reject icon");
    }
    return ESP_OK;
}

esp_err_t CallScreen::draw()
{
    ESP_RETURN_ON_ERROR(
        draw_status_icons(),
        TAG, "Failed to draw status icon");

    ESP_RETURN_ON_ERROR(
        draw_contact_information(),
        TAG, "Failed to draw contact informations");

    ESP_RETURN_ON_ERROR(
        draw_action_icons(),
        TAG, "Failed to draw action icons");

    return ESP_OK;
}

esp_err_t CallScreen::refresh(bool call_info, bool call_status)
{
    if (call_info)
    {
        ESP_RETURN_ON_ERROR(
            draw_contact_information(),
            TAG, "Failed to draw contact informations");
    }

    if (call_status)
    {
        ESP_RETURN_ON_ERROR(
            draw_status_icons(),
            TAG, "Failed to draw status icon");

        ESP_RETURN_ON_ERROR(
            draw_action_icons(),
            TAG, "Failed to draw action icons");
    }
    return ESP_OK;
}

esp_err_t CallScreen::identify_tap()
{
    uint16_t tap_x = touch_manager.get_single_tap_x();
    uint16_t tap_y = touch_manager.get_single_tap_y();

    if (icon_containers[0].contains(tap_x, tap_y))
    {
        ESP_LOGI(TAG, "Incoming accept clicked");
    }
    else if (icon_containers[1].contains(tap_x, tap_y))
    {
        ESP_LOGI(TAG, "Incoming reject clicked");
    }
    else if (icon_containers[2].contains(tap_x, tap_y))
    {
        ESP_LOGI(TAG, "Outgoing reject clicked");
    }
    return ESP_OK;
}

esp_err_t CallScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Return back to home screen (id: 1)
        return screen_manager.change_screen(1);
    }
    if(events & SINGLE_TAP_EVENT)
    {
        return identify_tap();
    }
    return ESP_OK;
}