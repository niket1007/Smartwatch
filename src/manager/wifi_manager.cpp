#include "wifi_manager.h"
#include "preferences_manager.h"
#include "ble_manager.h"

const int WIFI_MAX_ATTEMPTS = 20;

bool connect_to_wifi() {
    int attempts = 0;

    usb_serial.println("Initiating Wifi Connection");

    fetch_wifi_credentials();
    if(gv.ssid.isEmpty() && gv.password.isEmpty()) {
        return false;
    }

    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    usb_serial.printf("SSID: %s; Password: %s\n",gv.ssid, gv.password);
    WiFi.begin(gv.ssid.c_str(), gv.password.c_str());
    
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_MAX_ATTEMPTS) {
        delay(500);
        usb_serial.print(".");
        attempts++;
    }

    return WiFi.status() == WL_CONNECTED;
}

void disconnect_wifi() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    usb_serial.println("Wi-Fi Powered Down.");
    delay(200);
}

void scan_and_save_nearby_wifi() {
    // usb_serial.println("Scanning and Saving nearby wifi");
    if(ts_var.wifi_refresh == 1) {

        if(gv.ble_is_powered_on) {
            ble_manager_deinit();
        }

        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        
        int n = WiFi.scanNetworks();

        gv.wifi_list = "";
        
        for(int index = 0; index < n; index++) {
            gv.wifi_list += WiFi.SSID(index) + "\n";
        }
        // usb_serial.println(gv.wifi_list);

        ts_var.wifi_refresh = 2;

        disconnect_wifi();

        if(!gv.ble_is_powered_on) {
            ble_manager_init();
        }
    }
}

void update_wifi_dropdown() {
    if(!gv.wifi_list.isEmpty()) {
        lv_dropdown_set_options(objects.wifi_ssid_dropdown, gv.wifi_list.c_str());
    }
    lv_obj_add_flag(objects.settings_loading_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(objects.settings_loading_spinner, LV_OBJ_FLAG_HIDDEN);
    ts_var.wifi_refresh = 0;
}

void update_wifi_settings_details() {
    if(gv.ssid.isEmpty()) {
        lv_label_set_text(objects.wifi_ssid_name, "No Device");
        return;
    }
    
    if(!gv.ssid.equals(lv_label_get_text(objects.wifi_ssid_name))) {
        lv_label_set_text(objects.wifi_ssid_name, gv.ssid.c_str());
    }
    
}

void action_wifi_refresh_button_clicked(lv_event_t * e) {
//   usb_serial.println("Wifi refresh button clicked");
  ts_var.wifi_refresh = 1;
  lv_obj_remove_flag(objects.settings_loading_screen, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(objects.settings_loading_spinner, LV_OBJ_FLAG_HIDDEN);
  if(task_background_handle != NULL) xTaskNotifyGive(task_background_handle);
}

void action_wifi_save_button_clicked(lv_event_t * e) {
    // usb_serial.println("Wifi save button clicked");

    // Guard against empty lists
    if (gv.wifi_list.isEmpty()) {
        // usb_serial.println("Dropdown empty, aborting save.");
        return; 
    }

    char ssid_buffer[64] = {0};
    lv_dropdown_get_selected_str(objects.wifi_ssid_dropdown, ssid_buffer, sizeof(ssid_buffer));

    const char* password = lv_textarea_get_text(objects.wifi_pass_text_field);

    // usb_serial.printf("Saving SSID: [%s]\n", ssid_buffer);
    // usb_serial.printf("Saving Password: [%s]\n", password);
    
    store_wifi_credentials(String(ssid_buffer), password);

    // usb_serial.println("Credentials saved. Rebooting system to apply changes...");

    delay(1000); // Give the serial buffer and screen time to flush
    
    // Trigger a clean hardware-level software reset
    ESP.restart(); 
}