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
    for (const auto &container : notif_containers)
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_round_rect(
                container.x1, container.y1,
                container.width, container.height,
                30, WHITE_COLOR),
            TAG, "Failed to draw container box");
    }

    // Call Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(75, 155, "CALL", freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw call text");

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(290, 155, "00", freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw call notif text");
    }

    // Message Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(75, 275, "MESSAGE", freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw message text");

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(290, 275, "00", freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw message notif count text");
    }

    // App Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(75, 395, "APP", freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw app icon");

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(290, 395, "00", freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw app count text");
    }

    return ESP_OK;
}

esp_err_t NotificationScreen::identify_tap()
{
    uint16_t tap_x = touch_manager.get_single_tap_x();
    uint16_t tap_y = touch_manager.get_single_tap_y();

    if (notif_containers[0].contains(tap_x, tap_y))
    {
        // Call Notification Screen Id = 41
        ESP_LOGI(TAG, "Call clicked");
        ESP_RETURN_ON_ERROR(
            screen_manager.change_screen(41),
            TAG, "Failed to switch to Call Notification screen");
    }
    else if (notif_containers[1].contains(tap_x, tap_y))
    {
        // Message Notification Screen Id = 42
        ESP_LOGI(TAG, "Message clicked");
        ESP_RETURN_ON_ERROR(
            screen_manager.change_screen(42),
            TAG, "Failed to switch to Message Notification screen");
    }
    else if (notif_containers[2].contains(tap_x, tap_y))
    {
        // App Notification Screen Id = 43
        ESP_LOGI(TAG, "App clicked");
        ESP_RETURN_ON_ERROR(
            screen_manager.change_screen(43),
            TAG, "Failed to switch to App Notification screen");
    }
    return ESP_OK;
}

esp_err_t NotificationScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Return back to Menu Page 2
        return screen_manager.change_screen(2);
    }
    if (events & SINGLE_TAP_EVENT)
    {
        return identify_tap();
    }
    return ESP_OK;
}