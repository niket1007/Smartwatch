#include <string>
#include "Common/globals.h"
#include "Graphics/Fonts/generated/font_globals.h"
#include "menu_screen_page_1.h"

static constexpr char *TAG = "MENU_SCREEN_PAGE_1";

esp_err_t MenuScreenPage1::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

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
    for (const auto &container : icon_containers)
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_round_rect(
                container.x1, container.y1,
                container.width, container.height,
                30, WHITE_COLOR, 4U),
            TAG, "Failed to draw container box");
    }

    // Notification Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(80, 95, &icon_notification, WHITE_COLOR),
            TAG, "Failed to draw notification icon");
    }

    // Weather Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(250, 95, &icon_weather, WHITE_COLOR),
            TAG, "Failed to draw weather icon");
    }

    // Calendar Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(80, 275, &icon_calendar, WHITE_COLOR),
            TAG, "Failed to draw calendar icon");
    }

    // Navigation Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(250, 275, &icon_navigation, WHITE_COLOR),
            TAG, "Failed to draw navigation icon");
    }

    return ESP_OK;
}

esp_err_t MenuScreenPage1::identify_tap()
{
    uint16_t tap_x = touch_manager.get_single_tap_x();
    uint16_t tap_y = touch_manager.get_single_tap_y();

    if (icon_containers[0].contains(tap_x, tap_y))
    {
        // Notification Screen Id = 4
        ESP_RETURN_ON_ERROR(
            screen_manager.change_screen(4),
            TAG, "Failed to switch to Notification screen");
    }
    else if (icon_containers[1].contains(tap_x, tap_y))
    {
        // Weather Screen Id = 5
        ESP_RETURN_ON_ERROR(
            screen_manager.change_screen(5),
            TAG, "Failed to switch to Weather screen");
    }
    else if (icon_containers[2].contains(tap_x, tap_y))
    {
        ESP_LOGI(TAG, "Calendar clicked");
    }
    return ESP_OK;
}

esp_err_t MenuScreenPage1::handle_events(uint32_t events)
{
    ESP_LOGI(TAG, "handle_events called");
    if (events & SINGLE_TAP_EVENT)
    {
        return identify_tap();
    }
    return ESP_OK;
}