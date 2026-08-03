#include <string>
#include "Common/globals.h"
#include "Graphics/Fonts/generated/font_globals.h"
#include "menu_screen_page_1.h"

static constexpr const char *TAG = "MENU_SCREEN_PAGE_1";

esp_err_t MenuScreenPage1::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(50, 40, 310, 420, BLACK_COLOR),
        TAG, "Failed to reset screen");
    
    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    return ESP_OK;
}

esp_err_t MenuScreenPage1::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");
    return ESP_OK;
}

esp_err_t MenuScreenPage1::draw()
{
    for (const auto& container : icon_containers) {
        ESP_RETURN_ON_ERROR(
            graphics.draw_round_rect(
                container.x1, container.y1, 
                container.width, container.height, 
                30, WHITE_COLOR),
            TAG, "Failed to draw container box");
    }

    // Notification Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(75, 100, &icon_notifications, WHITE_COLOR),
            TAG, "Failed to draw notification icon");
        
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(160, 160, "Notification", freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw notification text");
    }

    // Weather Container Content
    {    
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(75, 220, &icon_weather, WHITE_COLOR),
            TAG, "Failed to draw weather icon");
        
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(180, 280, "Weather", freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw weather text");
    }

    // Calendar Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(75, 340, &icon_calendar, WHITE_COLOR),
            TAG, "Failed to draw calendar icon");
        
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(180, 400, "Calendar", freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw calendar text");
    }

    return ESP_OK;
}

void MenuScreenPage1::identify_tap()
{
    uint16_t tap_x = touch_manager.get_single_tap_x();
    uint16_t tap_y = touch_manager.get_single_tap_y();

    if (icon_containers[0].contains(tap_x, tap_y))
    {
        ESP_LOGI(TAG, "Notification clicked");
    }
    else if (icon_containers[1].contains(tap_x, tap_y))
    {
        ESP_LOGI(TAG, "Weather clicked");
    }
    else if (icon_containers[2].contains(tap_x, tap_y))
    {
        ESP_LOGI(TAG, "Calendar clicked");
    }
}

esp_err_t MenuScreenPage1::handle_events(uint32_t events)
{
    ESP_LOGI(TAG, "handle_events called");
    if (events & SINGLE_TAP_EVENT)
    {
        identify_tap();
    }
    return ESP_OK;
}