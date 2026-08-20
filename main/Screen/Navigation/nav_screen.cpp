#include "nav_screen.h"

#include "Common/globals.h"
#include "Screen/screen_manager.h"
#include "Graphics/Icons/generated/icon_globals.h"
#include "Graphics/Fonts/generated/font_globals.h"

static constexpr const char *TAG = "NAV_SCREEN";

esp_err_t NavScreen::on_enter()
{
    ESP_LOGI(TAG, "on enter called");

    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    return ESP_OK;
}

esp_err_t NavScreen::on_exit()
{
    ESP_LOGI(TAG, "on exit called");
    return ESP_OK;
}

esp_err_t NavScreen::draw_directions()
{
    std::string direction = nav_data.get_directions();

    icon_t icon = icon_nav_default;

    if (direction == "continue")
    {
        icon = icon_arrow_upward;
        direction = "Continue";
    }
    else if (direction == "left")
    {
        icon = icon_arrow_left;
        direction = "Left";
    }
    else if (direction == "left_slight")
    {
        icon = icon_turn_slight_left;
        direction = "Slight Left";
    }
    else if (direction == "left_sharp")
    {
        icon = icon_turn_sharp_left;
        direction = "Sharp Left";
    }
    else if (direction == "right")
    {
        icon = icon_arrow_right;
        direction = "Right";
    }
    else if (direction == "right_slight")
    {
        icon = icon_turn_slight_right;
        direction = "Slight Right";
    }
    else if (direction == "right_sharp")
    {
        icon = icon_turn_sharp_right;
        direction = "Sharp Right";
    }
    else if (direction == "keep_left")
    {
        icon = icon_keep_left;
        direction = "Keep Left";
    }
    else if (direction == "keep_right")
    {
        icon = icon_keep_right;
        direction = "Keep Right";
    }
    else if (direction == "uturn_left")
    {
        icon = icon_u_turn_left;
        direction = "U-Turn Left";
    }
    else if (direction == "uturn_right")
    {
        icon = icon_u_turn_right;
        direction = "U-Turn Right";
    }
    else if (direction == "offroute")
    {
        icon = icon_offroute;
        direction = "Offroute";
    }
    else if (direction == "roundabout_left")
    {
        icon = icon_roundabout_left;
        direction = "Roundabout Left";
    }
    else if (direction == "roundabout_right")
    {
        icon = icon_roundabout_right;
        direction = "Roundabout Right";
    }
    else if (direction == "roundabout_straight")
    {
        icon = icon_arrow_upward;
        direction = "Roundabout Straight";
    }
    else if (direction == "roundabout_uturn")
    {
        icon = icon_u_turn_right;
        direction = "Roundabout U-Turn";
    }
    else if (direction == "finish")
    {
        icon = icon_finish;
        direction = "Finished";
    }
    else
    {
        icon = icon_nav_default;
    }

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(130, 50, 150, 100, BLACK_COLOR),
        TAG, "Failed to fill the direction icon container");

    ESP_RETURN_ON_ERROR(
        graphics.draw_icon(150, 50, &icon, WHITE_COLOR),
        TAG, "Failed to draw direction icon");

    const int TEXT_AREA_CENTER_X = screen_manager.get_reset_screen_mid();
    int text_width = graphics.get_text_width(direction.c_str(), freesans_30);
    int x = TEXT_AREA_CENTER_X - (text_width / 2);

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(x - 10, 170, text_width + 20, 60, BLACK_COLOR),
        TAG, "Failed to fill the direction text container");

    ESP_RETURN_ON_ERROR(
        graphics.draw_text(
            x, 220, direction.c_str(), freesans_30, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw direction text");

    return ESP_OK;
}

esp_err_t NavScreen::draw_distance()
{
    const std::string distance = std::to_string(nav_data.get_distance()) + "m";

    const int TEXT_AREA_CENTER_X = screen_manager.get_reset_screen_mid();
    int text_width = graphics.get_text_width(distance.c_str(), freesans_40);
    int x = TEXT_AREA_CENTER_X - (text_width / 2);

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(x - 10, 250, text_width + 20, 60, BLACK_COLOR),
        TAG, "Failed to fill the distance container");

    ESP_RETURN_ON_ERROR(
        graphics.draw_text(
            x, 300, distance.c_str(), freesans_40, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw distance text");

    return ESP_OK;
}

esp_err_t NavScreen::draw_eta()
{
    const int TEXT_AREA_CENTER_X = screen_manager.get_reset_screen_mid();
    
    std::string eta = nav_data.get_eta();

    int text_width = graphics.get_text_width(eta.c_str(), freesans_40);
    int x = TEXT_AREA_CENTER_X - (text_width / 2);

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(x - 10, 350, text_width + 20, 60, BLACK_COLOR),
        TAG, "Failed to fill the eta container");

    ESP_RETURN_ON_ERROR(
        graphics.draw_text(
            x, 400, eta.c_str(), freesans_40, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw eta text");
    
    return ESP_OK;
}

esp_err_t NavScreen::draw()
{
    ESP_RETURN_ON_ERROR(
        draw_directions(),
        TAG, "Failed to draw directions");

    ESP_RETURN_ON_ERROR(
        draw_distance(),
        TAG, "Failed to draw distance");
    
    ESP_RETURN_ON_ERROR(
        draw_eta(),
        TAG, "Failed to draw eta");

    return ESP_OK;
}

esp_err_t NavScreen::refresh(
    bool dir_signs, bool dist, bool eta)
{
    if (dir_signs)
    {
        ESP_RETURN_ON_ERROR(
            draw_directions(),
            TAG, "Failed to draw directions");
    }

    if (dist)
    {
        ESP_RETURN_ON_ERROR(
            draw_distance(),
            TAG, "Failed to draw distance");
    }

    if(eta)
    {
        ESP_RETURN_ON_ERROR(
            draw_eta(),
            TAG, "Failed to draw eta");
    }
    return ESP_OK;
}

esp_err_t NavScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Return back to Main Menu Page 1 (id: 2)
        return screen_manager.change_screen(2);
    }
    return ESP_OK;
}