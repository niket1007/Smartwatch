#include <string>
#include "Common/globals.h"
#include "Graphics/Fonts/generated/font_globals.h"
#include "menu_screen_page_2.h"

static constexpr const char *TAG = "MENU_SCREEN_PAGE_2";

esp_err_t MenuScreenPage2::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    return ESP_OK;
}

esp_err_t MenuScreenPage2::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");
    return ESP_OK;
}

esp_err_t MenuScreenPage2::draw()
{
    for (const auto &container : icon_containers)
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_round_rect(
                container.x1, container.y1,
                container.width, container.height,
                30, WHITE_COLOR),
            TAG, "Failed to draw container box");
    }

    // Alarm Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(75, 100, &icon_alarm, WHITE_COLOR),
            TAG, "Failed to draw alarm icon");

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(160, 160, "Alarm", freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw alarm text");
    }

    // Navigation Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(70, 220, &icon_nav_default, WHITE_COLOR),
            TAG, "Failed to draw navigation icon");

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(160, 280, "Navigation", freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw navigation text");
    }

    // Music Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(75, 340, &icon_music, WHITE_COLOR),
            TAG, "Failed to draw music icon");

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(160, 400, "Music", freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw music text");
    }

    return ESP_OK;
}

void MenuScreenPage2::identify_tap()
{
    uint16_t tap_x = touch_manager.get_single_tap_x();
    uint16_t tap_y = touch_manager.get_single_tap_y();

    if (icon_containers[0].contains(tap_x, tap_y))
    {
        ESP_LOGI(TAG, "Alarm clicked");
    }
    else if (icon_containers[1].contains(tap_x, tap_y))
    {
        ESP_LOGI(TAG, "Navigation clicked");
    }
    else if (icon_containers[2].contains(tap_x, tap_y))
    {
        ESP_LOGI(TAG, "Music clicked");
    }
}

esp_err_t MenuScreenPage2::handle_events(uint32_t events)
{
    ESP_LOGI(TAG, "handle_events called");
    if (events & SINGLE_TAP_EVENT)
    {
        identify_tap();
    }
    return ESP_OK;
}