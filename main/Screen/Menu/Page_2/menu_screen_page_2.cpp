#include <string>
#include "Common/globals.h"
#include "Graphics/Fonts/generated/font_globals.h"
#include "menu_screen_page_2.h"

static constexpr char *TAG = "MENU_SCREEN_PAGE_2";

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
                30, WHITE_COLOR, 4U),
            TAG, "Failed to draw container box");
    }

    // Alarm Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(60, 95, &icon_alarm, WHITE_COLOR),
            TAG, "Failed to draw alarm icon");
    }

    // Music Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(250, 95, &icon_music, WHITE_COLOR),
            TAG, "Failed to draw music icon");
    }

    // Settings Container Content
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(60, 305, &icon_settings, WHITE_COLOR),
            TAG, "Failed to draw setting icon");
    }

    return ESP_OK;
}

esp_err_t MenuScreenPage2::identify_tap()
{
    uint16_t tap_x = touch_manager.get_single_tap_x();
    uint16_t tap_y = touch_manager.get_single_tap_y();

    if (icon_containers[0].contains(tap_x, tap_y))
    {
        ESP_LOGI(TAG, "Alarm clicked");
    }
    else if (icon_containers[1].contains(tap_x, tap_y))
    {
        ESP_LOGI(TAG, "Music clicked");
    }
    else if (icon_containers[2].contains(tap_x, tap_y))
    {
        // Settings Screen Id = 10
        ESP_RETURN_ON_ERROR(
            screen_manager.change_screen(10),
            TAG, "Failed to switch to Settings screen");
    }
    return ESP_OK;
}

esp_err_t MenuScreenPage2::handle_events(uint32_t events)
{
    ESP_LOGI(TAG, "handle_events called");
    if (events & SINGLE_TAP_EVENT)
    {
        return identify_tap();
    }
    return ESP_OK;
}