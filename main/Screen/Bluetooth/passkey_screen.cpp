#include "passkey_screen.h"
#include "Common/globals.h"
#include "Screen/screen_manager.h"
#include "Graphics/Icons/generated/icon_globals.h"
#include "Graphics/Fonts/generated/font_globals.h"

static constexpr char *TAG = "PASSKEY_SCREEN";

esp_err_t PassKeyScreen::on_enter()
{
    ESP_LOGI(TAG, "on enter called");

    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    return ESP_OK;
}

esp_err_t PassKeyScreen::on_exit()
{
    ESP_LOGI(TAG, "on exit called");
    return ESP_OK;
}

esp_err_t PassKeyScreen::draw()
{
    ESP_RETURN_ON_ERROR(
        graphics.draw_icon(150, 150, &icon_passkey, WHITE_COLOR),
        TAG, "Failed to draw passkey icon");
    
    
    ESP_RETURN_ON_ERROR(
        graphics.draw_text(120, 300, "000000", freesans_50, 0x07F0, BLACK_COLOR),
        TAG, "Failed to draw passkey text");
    
    return ESP_OK;
}

esp_err_t PassKeyScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Return back to home screen (id: 1)
        return screen_manager.change_screen(1);
    }
    return ESP_OK;
}