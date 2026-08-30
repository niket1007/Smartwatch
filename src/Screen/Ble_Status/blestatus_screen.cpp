#include "blestatus_screen.h"
#include "Screen/Generated/ui.h"
#include "Common/globals.h"

static constexpr const char *TAG = "BLESTATUS_SCREEN";

esp_err_t BleStatusScreen::on_enter() 
{
    if(display_manager.is_sleeping())
    {
        display_manager.wake();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    lv_label_set_text(
        objects.ble_con_status_label, get_status_text().c_str());

    return ESP_OK;
}

esp_err_t BleStatusScreen::on_exit() 
{
    return ESP_OK;
}

std::string BleStatusScreen::get_status_text()
{
    BLE_STATUS ble_status = bluetooth_manager.get_ble_status();

    switch(ble_status)
    {
        case BLE_STATUS::DISCONNECTED:
        {
            return "DISCONNECTED";
        }
        case BLE_STATUS::PAIRED:
        {
            return "CONNECTED";
        }
        case BLE_STATUS::PAIRING:
        {
            std::string passkey = "PAIRING\n" + std::to_string(bluetooth_manager.get_passkey());
            return passkey;
        }
        case BLE_STATUS::PAIRTING_FAILED:
        {    
            return "PAIRING FAILED";
        }
        default:
        {
            return "UNKNOWN";
        }
    }
}

esp_err_t BleStatusScreen::update_screen()
{
    lv_label_set_text(
        objects.ble_con_status_label, get_status_text().c_str());

    return ESP_OK;
}

esp_err_t BleStatusScreen::handle_events(uint32_t events) 
{
    if(events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Home Screen
        return screen_manager.change_screen(SCREEN_ID_HOME);
    }

    if(events & BLE_STATUS_EVENT)
    {
        return update_screen();
    }
    
    return ESP_OK;
}