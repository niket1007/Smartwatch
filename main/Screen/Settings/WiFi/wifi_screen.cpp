#include <format>
#include <string>
#include "wifi_screen.h"
#include "Screen/screen_manager.h"
#include "Common/globals.h"
#include <cstring>

static constexpr char *TAG = "WIFI_SCREEN";

esp_err_t WIFIScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    return ESP_OK;
}

esp_err_t WIFIScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");
    return ESP_OK;
}

esp_err_t WIFIScreen::draw()
{
    ESP_RETURN_ON_ERROR(
        graphics.draw_text(50, 150, "Note: WiFi is used only\nfor time sync", freesans_30, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw wifi note text");

    {
        std::string ssid = wifi_manager.get_ssid();

        if (ssid.empty())
        {
            ssid = "No Connection";
        }

        int text_width = graphics.get_text_width(ssid.c_str(), freesans_40);

        const int TEXT_AREA_CENTER_X = screen_manager.get_reset_screen_mid();
        int x = TEXT_AREA_CENTER_X - (text_width / 2);

        ESP_LOGI(TAG, "Text Width: %d, X: %d", text_width, x);

        ESP_RETURN_ON_ERROR(
            graphics.draw_text(x, 320, ssid.c_str(), freesans_40, WHITE_COLOR, BLACK_COLOR),
            TAG, "Failed to draw wifi ssid name");
    }
    return ESP_OK;
}

esp_err_t WIFIScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Return back to Settings
        return screen_manager.change_screen(10);
    }
    return ESP_OK;
}