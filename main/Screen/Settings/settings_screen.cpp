#include <string>
#include "Common/globals.h"
#include "Graphics/Fonts/generated/font_globals.h"
#include "settings_screen.h"

static constexpr char *TAG = "SETTINGS_SCREEN";

esp_err_t SettingsScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    return ESP_OK;
}

esp_err_t SettingsScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");
    return ESP_OK;
}

esp_err_t SettingsScreen::draw()
{
    for (const auto &container : icon_containers)
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_round_rect(
                container.x1, container.y1,
                container.width, container.height,
                30, WHITE_COLOR, 4U),
            TAG, "Failed to draw container box");
    }

    // Bluetooth Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(60, 95, &icon_bluetooth, WHITE_COLOR),
            TAG, "Failed to draw bluetooth icon");
    }

    // Wifi Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(250, 95, &icon_wifi, WHITE_COLOR),
            TAG, "Failed to draw wifi icon");
    }

    // Brightness Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(60, 305, &icon_brightness, WHITE_COLOR),
            TAG, "Failed to draw brightness icon");
    }

    return ESP_OK;
}

esp_err_t SettingsScreen::identify_tap()
{
    uint16_t tap_x = touch_manager.get_single_tap_x();
    uint16_t tap_y = touch_manager.get_single_tap_y();

    if (icon_containers[0].contains(tap_x, tap_y))
    {
        // Bluetooth Screen Id = 101
        ESP_RETURN_ON_ERROR(
            screen_manager.change_screen(101),
            TAG, "Failed to switch to Bluetooth screen");
    }
    else if (icon_containers[1].contains(tap_x, tap_y))
    {
        // Wifi Screen Id = 102
        ESP_RETURN_ON_ERROR(
            screen_manager.change_screen(102),
            TAG, "Failed to switch to Wifi screen");
    }
    else if (icon_containers[2].contains(tap_x, tap_y))
    {
        // Brightness Screen Id = 103
        ESP_RETURN_ON_ERROR(
            screen_manager.change_screen(103),
            TAG, "Failed to switch to Brightness screen");
    }
    return ESP_OK;
}

esp_err_t SettingsScreen::handle_events(uint32_t events)
{
    ESP_LOGI(TAG, "handle_events called");
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Return back to Main Page 2
        return screen_manager.change_screen(3);
    }
    if (events & SINGLE_TAP_EVENT)
    {
        return identify_tap();
    }
    return ESP_OK;
}