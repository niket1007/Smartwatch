#include "settings_screen.h"
#include "Screen/Generated/ui.h"
#include "Screen/Generated/actions.h"
#include "Common/globals.h"

static constexpr const char *TAG = "SETTINGS_SCREEN";


esp_err_t SettingsScreen::on_enter()
{
    // const int percent_int = power_saver_manager.get_brightness_percentage();
    // std::string percent = std::to_string(percent_int) + "%";

    // lv_label_set_text(objects.brightness_label, percent.c_str());
    // lv_slider_set_value(objects.brightness_bar, percent_int, LV_ANIM_OFF);

    // std::string ssid = wifi_manager.get_ssid().c_str();
    // ssid = ssid.empty() ? "No Connection" : ssid;
    // lv_label_set_text(objects.wifi_name_label, ssid.c_str());

    // std::string ble_device_name = bluetooth_manager.get_connected_device_name();
    // lv_label_set_text(objects.ble_name_label, ble_device_name.c_str());

    return ESP_OK;
}


esp_err_t SettingsScreen::on_exit()
{
    return ESP_OK;
}


esp_err_t SettingsScreen::handle_events(uint32_t events)
{
    if(events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Menu Screen 2
        return screen_manager.change_screen(SCREEN_ID_MENU_SCREEN_TWO);
    }
    return ESP_OK;
}