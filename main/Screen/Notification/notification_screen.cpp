#include <format>
#include <string>
#include "notification_screen.h"
#include "Screen/screen_manager.h"
#include "Common/globals.h"
#include <cstring>

static constexpr const char *TAG = "NOTIFICATION_SCREEN";

esp_err_t NotificationScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");
    
    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(50, 40, 310, 420, BLACK_COLOR),
        TAG, "Failed to reset screen");
    
    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");
    
    return ESP_OK;
}

esp_err_t NotificationScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");
    return ESP_OK;
}

esp_err_t NotificationScreen::draw()
{
    ESP_RETURN_ON_ERROR(
        graphics.draw_text(190, 251, "Notification", freesans_30, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw notification text");

    return ESP_OK;
}


esp_err_t NotificationScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Return back to Menu Page 2
        return screen_manager.change_screen(2);
    }
    return ESP_OK;
}