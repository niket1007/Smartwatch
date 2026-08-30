#include "developer_screen.h"
#include "Screen/Generated/ui.h"
#include "Common/globals.h"

static constexpr const char *TAG = "DEVELOPER_SCREEN";


esp_err_t DeveloperScreen::on_enter()
{
    lv_label_set_text(
        objects.wifi_dev_status_label, 
        wifi_manager.errored ? "Error" : "Success"
    );

    lv_label_set_text(
        objects.rtc_dev_status_label, 
        rtc_manager.errored ? "Error" : "Success"
    );

    lv_label_set_text(
        objects.ble_init_dev_status_label, 
        bluetooth_manager.init_errored ? "Error" : "Success"
    );

    lv_label_set_text(
        objects.ble_deinit_dev_status_label, 
        bluetooth_manager.deinit_errored ? "Error" : "Success"
    );

    lv_label_set_text(
        objects.battery_dev_status_label, 
        battery_manager.errored ? "Error" : "Success"
    );

    lv_label_set_text(
        objects.ls_dev_status_label, 
        light_sleep_enable ? "True" : "False"
    );

    return ESP_OK;
}


esp_err_t DeveloperScreen::on_exit()
{
    return ESP_OK;
}


esp_err_t DeveloperScreen::handle_events(uint32_t events)
{
    if(events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // SETTINGS
        return screen_manager.change_screen(SCREEN_ID_SETTINGS);
    }
    return ESP_OK;
}