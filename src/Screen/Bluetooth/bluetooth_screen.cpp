#include "bluetooth_screen.h"
#include "Screen/Generated/ui.h"
#include "Common/globals.h"

static constexpr const char *TAG = "BLUETOOTH_SCREEN";


esp_err_t BluetoothScreen::on_enter()
{
    return ESP_OK;
}


esp_err_t BluetoothScreen::on_exit()
{
    return ESP_OK;
}


esp_err_t BluetoothScreen::handle_events(uint32_t events)
{
    if(events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // SETTINGS
        return screen_manager.change_screen(SCREEN_ID_SETTINGS);
    }
    return ESP_OK;
}