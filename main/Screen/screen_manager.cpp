#include "screen_manager.h"
#include "Common/globals.h"
#include "Common/constants.h"

#include "Home/home_screen.h"
#include "Menu/Page_1/menu_screen_page_1.h"
#include "Menu/Page_2/menu_screen_page_2.h"
#include "Notification/notification_screen.h"
#include "Weather/weather_screen.h"

#include "Screen/Notification/Notif_Type_Screens/notif_call_screen.h"
#include "Screen/Notification/Notif_Type_Screens/notif_message_screen.h"
#include "Screen/Notification/Notif_Type_Screens/notif_other_screen.h"
#include "Screen/Notification/Ble_Type_Screens/passkey_screen.h"

#include "Screen/Settings/settings_screen.h"
#include "Screen/Settings/Brightness/brightness_screen.h"
#include "Screen/Settings/Bluetooth/bluetooth_screen.h"
#include "Screen/Settings/WiFi/wifi_screen.h"
#include "Screen/Settings/Info/info_screen.h"

static constexpr char *TAG = "SCREEN_MANAGER";

esp_err_t ScreenManager::init()
{
    ESP_LOGI(TAG, "init called");
    if (current_screen == nullptr)
        current_screen = new HomeScreen();
    return ESP_OK;
}

esp_err_t ScreenManager::draw()
{
    ESP_LOGI(TAG, "draw called");
    if (current_screen != nullptr)
        return current_screen->draw();
    return ESP_OK;
}

Screen *ScreenManager::get_screen_instance_(int screen_id)
{
    switch (screen_id)
    {
    case 1:
        return new HomeScreen();
        break;
    case 2:
        return new MenuScreenPage1();
        break;
    case 3:
        return new MenuScreenPage2();
        break;
    case 4:
        return new NotificationScreen();
        break;
    case 41:
        return new NotifCallScreen();
        break;
    case 42:
        return new NotifMessageScreen();
        break;
    case 43:
        return new NotifOtherScreen();
        break;
    case 5:
        return new WeatherScreen();
        break;
    case 6:
        return new PassKeyScreen();
        break;
    case 10:
        return new SettingsScreen();
        break;
    case 101:
        return new BluetoothScreen();
        break;
    case 102:
        return new WIFIScreen();
        break;
    case 103:
        return new BrightnessScreen();
        break;
    case 104:
        return new InfoScreen();
        break;
    default:
        return new HomeScreen();
    }
}

esp_err_t ScreenManager::reset()
{
    // Reset the screen
    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(
            reset_screen_x, 
            reset_screen_y, 
            reset_screen_width, 
            reset_screen_height, 
            BLACK_COLOR),
        TAG, "Failed to reset screen");
    return ESP_OK;
}

int ScreenManager::get_reset_screen_mid()
{
    return (reset_screen_width / 2) + reset_screen_x;
}

esp_err_t ScreenManager::change_screen(int new_screen_id)
{

    ESP_RETURN_ON_ERROR(
        current_screen->on_exit(), TAG, "Failed to exit old screen");

    // vTaskDelay(pdMS_TO_TICKS(50));
    current_screen = get_screen_instance_(new_screen_id);
    current_screen_id = new_screen_id;

    reset();

    ESP_RETURN_ON_ERROR(
        current_screen->on_enter(), TAG, "Failed to enter new screen");
    return ESP_OK;
}

esp_err_t ScreenManager::handle_events(uint32_t events)
{
    ESP_LOGI(TAG, "handle events called");
    if (current_screen_id <= total_screen && current_screen_id >= 1)
    {
        if (events & SWIPE_RIGHT_EVENT)
        {
            int new_screen_id = ((current_screen_id - 2 + total_screen) % total_screen) + 1;
            ESP_LOGI(TAG, "Swipe right; new_screen_id: %d", new_screen_id);

            ESP_RETURN_ON_ERROR(
                change_screen(new_screen_id), TAG, "Failed to switch to previous screen");
            return ESP_OK;
        }

        if (events & SWIPE_LEFT_EVENT)
        {
            int new_screen_id = (current_screen_id % total_screen) + 1;
            ESP_LOGI(TAG, "Swipe left; new_screen_id: %d", new_screen_id);

            ESP_RETURN_ON_ERROR(
                change_screen(new_screen_id), TAG, "Failed to switch to next screen");
            return ESP_OK;
        }
    }
    if (current_screen != nullptr)
        return current_screen->handle_events(events);
    return ESP_OK;
}