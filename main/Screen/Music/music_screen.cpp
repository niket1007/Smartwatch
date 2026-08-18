#include <format>
#include <string>
#include "music_screen.h"
#include "Screen/screen_manager.h"
#include "Common/globals.h"
#include <cstring>

static constexpr const char *TAG = "MUSIC_SCREEN";

esp_err_t MusicScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    return ESP_OK;
}

esp_err_t MusicScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");
    return ESP_OK;
}

esp_err_t MusicScreen::draw_music_name()
{
    std::string name = music_data.get_music_name();
    int text_width = graphics.get_text_width(name.c_str(), freesans_50);

    const int TEXT_AREA_CENTER_X = screen_manager.get_reset_screen_mid();
    int x = TEXT_AREA_CENTER_X - (text_width / 2);

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(x - 10, 50, text_width + 20, 70, BLACK_COLOR),
        TAG, "Failed to fill music block");

    ESP_RETURN_ON_ERROR(
        graphics.draw_text(x, 100, name.c_str(), freesans_50, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw music name");

    return ESP_OK;
}

esp_err_t MusicScreen::draw_artist_names()
{
    std::string name = music_data.get_artist_names();
    int text_width = graphics.get_text_width(name.c_str(), freesans_50);

    const int TEXT_AREA_CENTER_X = screen_manager.get_reset_screen_mid();
    int x = TEXT_AREA_CENTER_X - (text_width / 2);

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(x - 10, 130, text_width + 20, 70, BLACK_COLOR),
        TAG, "Failed to fill artist block");

    ESP_RETURN_ON_ERROR(
        graphics.draw_text(x, 180, name.c_str(), freesans_50, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw artist names");

    return ESP_OK;
}

esp_err_t MusicScreen::draw()
{
    ESP_RETURN_ON_ERROR(
        draw_music_name(), TAG, "Failed to draw music container");

    ESP_RETURN_ON_ERROR(
        draw_artist_names(), TAG, "Failed to draw artist container");

    // Symbols
    {
        for (const auto &container : icon_containers)
        {
            ESP_RETURN_ON_ERROR(
                graphics.draw_round_rect(
                    container.x1, container.y1,
                    container.width, container.height,
                    30, WHITE_COLOR),
                TAG, "Failed to draw container");
        }

        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(65, 220, &icon_volume_up, WHITE_COLOR),
            TAG, "Failed to draw volume up icon");

        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(255, 220, &icon_volume_down, WHITE_COLOR),
            TAG, "Failed to draw volume down icon");

        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(65, 350, &icon_prev, WHITE_COLOR),
            TAG, "Failed to draw previous icon");
        
        ESP_RETURN_ON_ERROR(
            graphics.draw_icon(255, 350, &icon_next, WHITE_COLOR),
            TAG, "Failed to draw next icon");
    }

    return ESP_OK;
}

esp_err_t MusicScreen::refresh(bool music, bool artist)
{
    if (music)
    {
        ESP_RETURN_ON_ERROR(
            draw_music_name(), TAG, "Failed to draw music container");
    }

    if (artist)
    {
        ESP_RETURN_ON_ERROR(
            draw_artist_names(), TAG, "Failed to draw artist container");
    }

    return ESP_OK;
}

esp_err_t MusicScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Return back to Menu Page 2
        return screen_manager.change_screen(3);
    }
    return ESP_OK;
}