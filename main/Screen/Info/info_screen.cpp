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
    const int TEXT_AREA_CENTER_X = screen_manager.get_reset_screen_mid();

    int text_width = 0;
    int x = 0;
    std::string content = "";

    {   
        content = "NSW";
        text_width = graphics.get_text_width(content.c_str(), freesans_40);
        x = TEXT_AREA_CENTER_X - (text_width / 2);
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(
                x, 80, content.c_str(), freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");
    }

    {   
        content = "Code by: NAJ";
        text_width = graphics.get_text_width(content.c_str(), freesans_30);
        x = TEXT_AREA_CENTER_X - (text_width / 2);
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(
                x, 140, content.c_str(), freesans_30, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");
    }

    {
        content = "Company: Waveshare";
        text_width = graphics.get_text_width(content.c_str(), freesans_30);
        x = TEXT_AREA_CENTER_X - (text_width / 2);
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(x, 210, content.c_str(), freesans_30, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");
    }

    {
        content = "Screen: Amoled";
        text_width = graphics.get_text_width(content.c_str(), freesans_30);
        x = TEXT_AREA_CENTER_X - (text_width / 2);
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(x, 280, content.c_str(), freesans_30, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");
    }

    {
        content = "Screen Size: 2.06 inch";
        text_width = graphics.get_text_width(content.c_str(), freesans_30);
        x = TEXT_AREA_CENTER_X - (text_width / 2);
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(x, 350, content.c_str(), freesans_30, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw text");
    }

    {
        content = "Resolution: 402 X 510";
        text_width = graphics.get_text_width(content.c_str(), freesans_30);
        x = TEXT_AREA_CENTER_X - (text_width / 2);
        ESP_RETURN_ON_ERROR(
            graphics.draw_text(x, 420, content.c_str(), freesans_30, WHITE_COLOR, BLACK_COLOR),
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