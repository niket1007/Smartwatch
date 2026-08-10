#include <format>
#include <string>
#include "notif_other_screen.h"
#include "Screen/screen_manager.h"
#include "Common/globals.h"
#include <cstring>

static constexpr char *TAG = "MESSAGE_NOTIF_SCREEN";

esp_err_t NotifOtherScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    return ESP_OK;
}

esp_err_t NotifOtherScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");
    return ESP_OK;
}

esp_err_t NotifOtherScreen::draw()
{
    std::string message = std::to_string(notif_counter);
    ESP_RETURN_ON_ERROR(
        graphics.draw_text(50, 150, message.c_str(), freesans_50, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw notification text");
    return ESP_OK;
}

esp_err_t NotifOtherScreen::show_next_notif()
{
    notif_counter++;

    screen_manager.reset();

    std::string message = std::to_string(notif_counter);
    ESP_RETURN_ON_ERROR(
        graphics.draw_text(50, 150, message.c_str(), freesans_50, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw notification text");

    return ESP_OK;
}

esp_err_t NotifOtherScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Return back to Notification
        return screen_manager.change_screen(4);
    }
    if (events & SINGLE_TAP_EVENT)
    {
        return show_next_notif();
    }
    return ESP_OK;
}