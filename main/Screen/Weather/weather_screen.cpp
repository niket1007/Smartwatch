#include "weather_screen.h"
#include "Screen/generated/ui.h"
#include "Common/globals.h"

#include "esp_log.h"

static constexpr const char *TAG = "WEATHER_SCREEN";


esp_err_t WeatherScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");
    return ESP_OK;
}


esp_err_t WeatherScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");

    return ESP_OK;
}


esp_err_t WeatherScreen::handle_events(uint32_t events)
{
    if(events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Menu Screen 1
        return screen_manager.change_screen(SCREEN_ID_MENU_SCREEN_ONE);
    }
    return ESP_OK;
}