#include "passkey_screen.h"
#include "Common/globals.h"
#include "Screen/screen_manager.h"
#include "Graphics/Icons/generated/icon_globals.h"
#include "Graphics/Fonts/generated/font_globals.h"

static constexpr const char *TAG = "PASSKEY_SCREEN";

esp_err_t PassKeyScreen::on_enter()
{
    ESP_LOGI(TAG, "on enter called");

    if(display_driver.is_sleep)
    {
        int ret = display_driver.wake();
        if(ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to wake screen up");
        }
        display_driver.reset_screen_timeout_timer();
    }

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
    
    std::string passkey = std::to_string(bluetooth_manager.get_passkey());
    ESP_RETURN_ON_ERROR(
        graphics.draw_text(
            120, 300, passkey.c_str(), freesans_50, WHITE_COLOR, BLACK_COLOR),
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