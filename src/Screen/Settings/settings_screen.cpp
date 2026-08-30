#include "settings_screen.h"
#include "Screen/Generated/ui.h"
#include "Screen/Generated/actions.h"
#include "Common/globals.h"

static constexpr const char *TAG = "SETTINGS_SCREEN";

esp_err_t SettingsScreen::on_enter()
{
    return ESP_OK;
}

void SettingsScreen::set_selected_option_id(int id)
{
    selected_option_id = id;
}

esp_err_t SettingsScreen::on_exit()
{
    return ESP_OK;
}

esp_err_t SettingsScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Menu Screen 2
        return screen_manager.change_screen(SCREEN_ID_MENU_SCREEN_TWO);
    }

    if (events & SETTINGS_OPTION_CLICKED_EVENT)
    {
        int screen_id = SCREEN_ID_BRIGHTNESS;
        switch (selected_option_id)
        {
            case 1:
            {
                screen_id = SCREEN_ID_BRIGHTNESS;
                break;
            }
            case 2:
            {
                screen_id = SCREEN_ID_BLUETOOTH;
                break;
            }
            case 3:
            {
                screen_id = SCREEN_ID_WIFI;
                break;
            }
            case 4:
            {
                screen_id = SCREEN_ID_DEVELOPER;
                break;
            }
        }
        return screen_manager.change_screen(screen_id);
    }

    return ESP_OK;
}

void action_settings_click_func(lv_event_t *e)
{
    int icon_id = (int)(intptr_t)lv_event_get_user_data(e);

    SettingsScreen *settings_screen = screen_manager.get_current_settings_screen();
    if (settings_screen == nullptr)
    {
        return;
    }
    settings_screen->set_selected_option_id(icon_id);

    if (gui_task_handle != nullptr)
    {
        xTaskNotify(gui_task_handle, SETTINGS_OPTION_CLICKED_EVENT, eSetBits);
    }
}