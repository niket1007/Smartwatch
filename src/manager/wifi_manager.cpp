#include "wifi_manager.h"

Wifi_Global_Variables wifi_gv = {
    "", // ssid 
    "", // password
    ""  // wifi_list
};

const int WIFI_MAX_ATTEMPTS = 20;

void get_wifi_credentials() {
    preferences.begin("wifi-config", false);
    wifi_gv.ssid = preferences.getString("SSID", "");
    wifi_gv.password = preferences.getString("PASSWORD", "");
    preferences.end();
}

bool connect_to_wifi() {
    int attempts = 0;

    usb_serial.println("Initiating Wifi Connection");

    get_wifi_credentials();
    if(wifi_gv.ssid.isEmpty() && wifi_gv.password.isEmpty()) {
        return false;
    }

    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    usb_serial.printf("SSID: %s; Password: %s\n",wifi_gv.ssid, wifi_gv.password);
    WiFi.begin(wifi_gv.ssid.c_str(), wifi_gv.password.c_str());
    
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
    usb_serial.println("Scanning and Saving nearby wifi");
    if(ts_var.wifi_refresh == 1) {

        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        
        int n = WiFi.scanNetworks();

        wifi_gv.wifi_list = "";
        
        for(int index = 0; index < n; index++) {
            wifi_gv.wifi_list += WiFi.SSID(index) + "\n";
        }
        usb_serial.println(wifi_gv.wifi_list);

        ts_var.wifi_refresh = 2;

        disconnect_wifi();
    }
}

void update_wifi_dropdown() {
    if(!wifi_gv.wifi_list.isEmpty()) {
        lv_dropdown_set_options(objects.wifi_ssid_dropdown, wifi_gv.wifi_list.c_str());
    }
    lv_obj_add_flag(objects.settings_loading_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(objects.settings_loading_spinner, LV_OBJ_FLAG_HIDDEN);
    ts_var.wifi_refresh = 0;
}

void update_wifi_settings_details() {
    if(wifi_gv.ssid.isEmpty()) {
        lv_label_set_text(objects.wifi_ssid_name, "No Device");
    }
    else {
        lv_label_set_text(objects.wifi_ssid_name, wifi_gv.ssid.c_str());
    }
}

void action_wifi_refresh_button_clicked(lv_event_t * e) {
  usb_serial.println("Wifi refresh button clicked");
  ts_var.wifi_refresh = 1;
  lv_obj_remove_flag(objects.settings_loading_screen, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(objects.settings_loading_spinner, LV_OBJ_FLAG_HIDDEN);
  if(task_background_handle != NULL) xTaskNotifyGive(task_background_handle);
}

void action_wifi_save_button_clicked(lv_event_t * e) {
    usb_serial.println("Wifi save button clicked");

    // Guard against empty lists
    if (wifi_gv.wifi_list.isEmpty()) {
        usb_serial.println("Dropdown empty, aborting save.");
        return; 
    }

    char ssid_buffer[64] = {0};
    lv_dropdown_get_selected_str(objects.wifi_ssid_dropdown, ssid_buffer, sizeof(ssid_buffer));

    const char* password = lv_textarea_get_text(objects.wifi_pass_text_field);

    usb_serial.printf("Saving SSID: [%s]\n", ssid_buffer);
    usb_serial.printf("Saving Password: [%s]\n", password);
    
    preferences.begin("wifi-config", false);
    preferences.putString("SSID", ssid_buffer);
    preferences.putString("PASSWORD", password);
    preferences.end();

    usb_serial.println("Credentials saved. Rebooting system to apply changes...");

    delay(1000); // Give the serial buffer and screen time to flush
    
    // Trigger a clean hardware-level software reset
    ESP.restart(); 
}