#include "info_screen.h"
#include "Screen/Generated/ui.h"
#include "Common/globals.h"

static constexpr const char *TAG = "INFO_SCREEN";


esp_err_t InfoScreen::on_enter()
{
    return ESP_OK;
}


esp_err_t InfoScreen::on_exit()
{
    return ESP_OK;
}


esp_err_t InfoScreen::handle_events(uint32_t events)
{
    if(events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Menu Screen 2
        return screen_manager.change_screen(SCREEN_ID_MENU_SCREEN_TWO);
    }
    return ESP_OK;
}