#include <format>
#include <string>
#include "bluetooth_screen.h"
#include "Screen/screen_manager.h"
#include "Common/globals.h"
#include <cstring>

static constexpr const char *TAG = "BLUETOOTH_SCREEN";

esp_err_t BluetoothScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    return ESP_OK;
}

esp_err_t BluetoothScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");
    return ESP_OK;
}

esp_err_t BluetoothScreen::draw()
{
    std::string message = "0";
    ESP_RETURN_ON_ERROR(
        graphics.draw_text(50, 150, message.c_str(), freesans_50, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw birghtness text");
    return ESP_OK;
}

esp_err_t BluetoothScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Return back to Settings
        return screen_manager.change_screen(10);
    }
    return ESP_OK;
}