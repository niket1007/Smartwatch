#include <format>
#include <string>
#include "info_screen.h"
#include "Screen/screen_manager.h"
#include "Common/globals.h"
#include <cstring>

static constexpr const char *TAG = "INFO_SCREEN";

esp_err_t InfoScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    return ESP_OK;
}

esp_err_t InfoScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");
    return ESP_OK;
}

esp_err_t InfoScreen::draw()
{
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(50, 100, "Code by:", freesans_30, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(210, 100, "NAJ", freesans_30, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");
    }

    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(50, 170, "Company:", freesans_30, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(210, 170, "Waveshare", freesans_30, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");
    }

    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(50, 240, "Screen:", freesans_30, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(210, 240, "Amoled", freesans_30, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");
    }

    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(50, 310, "Screen Size:", freesans_30, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(230, 310, "2.06 inch", freesans_30, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");
    }

    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(50, 390, "Resolution:", freesans_30, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(210, 390, "402 X 510", freesans_30, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");
    }

    return ESP_OK;
}

esp_err_t InfoScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Return back to Menu Page 2
        return screen_manager.change_screen(3);
    }
    return ESP_OK;
}