#include "wifi_screen.h"
#include "Screen/Generated/ui.h"
#include "Screen/Generated/actions.h"
#include "Common/globals.h"

static constexpr const char *TAG = "WIFI_SCREEN";


esp_err_t WifiScreen::on_enter()
{
    
    update_saved_ssid_label();

    // If WIFI_CRED_STORED_EVENT is missed
    lv_obj_clear_state(objects.wifi_save_cred_button, LV_STATE_DISABLED);

    return ESP_OK;
}

void WifiScreen::update_saved_ssid_label()
{
    std::string ssid = wifi_manager.get_ssid();
    
    if(ssid.empty())
    {
        ssid = "-----------";
    }
    
    ssid = "SAVED\n" + ssid;

    lv_label_set_text(objects.saved_ssid_label, ssid.c_str());
}


esp_err_t WifiScreen::on_exit()
{
    return ESP_OK;
}


esp_err_t WifiScreen::handle_events(uint32_t events)
{
    if(events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // SETTINGS
        return screen_manager.change_screen(SCREEN_ID_SETTINGS);
    }

    if(events & WIFI_CRED_STORED_EVENT)
    {
        update_saved_ssid_label();
        lv_obj_clear_state(objects.wifi_save_cred_button, LV_STATE_DISABLED);
    }
    return ESP_OK;
}

void action_wifi_password_text_area_click(lv_event_t *e)
{
    // usb_serial.println("action_wifi_password_text_area_click called");
    lv_keyboard_set_textarea(objects.wifi_keyboard, objects.wifi_password_text_area);
}

void action_ssid_text_area_click(lv_event_t *e)
{
    // usb_serial.println("action_ssid_text_area_click called");
    lv_keyboard_set_textarea(objects.wifi_keyboard, objects.wifi_ssid_text_area);
}

void action_save_wifi_creds(lv_event_t *e)
{
    std::string ssid = lv_textarea_get_text(objects.wifi_ssid_text_area);
    std::string password = lv_textarea_get_text(objects.wifi_password_text_area);

    // usb_serial.printf("%s; %s\n", ssid.c_str(), password.c_str());

    wifi_manager.update_local_credentials(ssid, password);

    lv_obj_add_state(objects.wifi_save_cred_button, LV_STATE_DISABLED);

    if(background_task_handle != nullptr)
    {
        xTaskNotify(background_task_handle, WIFI_SAVE_CRED_EVENT, eSetBits);
    }
}