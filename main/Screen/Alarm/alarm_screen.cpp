#include "alarm_screen.h"
#include "Screen/generated/ui.h"
#include "Common/globals.h"

#include "esp_log.h"

static constexpr const char *TAG = "ALARM_SCREEN";


esp_err_t AlarmScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");
    return ESP_OK;
}


esp_err_t AlarmScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");

    return ESP_OK;
}


esp_err_t AlarmScreen::handle_events(uint32_t events)
{
    if(events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Menu Screen 2
        return screen_manager.change_screen(SCREEN_ID_MENU_SCREEN_TWO);
    }
    return ESP_OK;
}