#pragma once

#include "Screen/screen.h"

// Screen Imports

#include "Home/home_screen.h"
#include "Menu/Page_1/menu_screen_page_1.h"
#include "Menu/Page_2/menu_screen_page_2.h"
#include "Notification/notification_screen.h"
#include "Weather/weather_screen.h"
#include "Screen/Info/info_screen.h"
#include "Screen/Music/music_screen.h"

#include "Screen/Notification/Notif_Type_Screens/notif_call_screen.h"
#include "Screen/Notification/Notif_Type_Screens/notif_message_screen.h"
#include "Screen/Notification/Notif_Type_Screens/notif_other_screen.h"
#include "Screen/Notification/Ble_Type_Screens/passkey_screen.h"
#include "Screen/Notification/Ble_Type_Screens/call_screen.h"

#include "Screen/Settings/settings_screen.h"
#include "Screen/Settings/Brightness/brightness_screen.h"
#include "Screen/Settings/Bluetooth/bluetooth_screen.h"
#include "Screen/Settings/WiFi/wifi_screen.h"


class ScreenManager
{
private:
    Screen *current_screen = nullptr;

    // Home Screen
    int current_screen_id = 1;

    // Home, Menu_Page1, Menu_Page2
    int total_screen = 3;

    int reset_screen_x = 50;
    int reset_screen_width = 310;
    int reset_screen_y = 40;
    int reset_screen_height = 420;

    Screen *get_screen_instance_(int screen_id);

public:
    esp_err_t init();
    esp_err_t draw();
    esp_err_t reset();
    int get_reset_screen_mid();
    esp_err_t change_screen(int new_screen_id);
    esp_err_t handle_events(uint32_t events);
};