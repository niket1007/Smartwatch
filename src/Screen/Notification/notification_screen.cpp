#include "notification_screen.h"
#include "Screen/Generated/ui.h"
#include "Common/globals.h"

static constexpr const char *TAG = "NOTIFICATION_SCREEN";


esp_err_t NotificationScreen::on_enter()
{
    return ESP_OK;
}


esp_err_t NotificationScreen::on_exit()
{
    return ESP_OK;
}


esp_err_t NotificationScreen::handle_events(uint32_t events)
{
    if(events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Menu Screen 1
        return screen_manager.change_screen(SCREEN_ID_MENU_SCREEN_ONE);
    }
    return ESP_OK;
}