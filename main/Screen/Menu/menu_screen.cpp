#include "Screen/Menu/menu_screen.h"
#include "Screen/generated/ui.h"
#include "Screen/generated/actions.h"
#include "Common/globals.h"

#include "esp_log.h"

static constexpr const char *TAG = "MENU_SCREEN";

esp_err_t MenuScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");
    return ESP_OK;
}


esp_err_t MenuScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");

    return ESP_OK;
}

void MenuScreen::set_selected_option_id(int id)
{
    selected_option_id = id;
}

int MenuScreen::get_selected_option_id()
{
    return selected_option_id;
}

esp_err_t MenuScreen::handle_events(uint32_t events)
{
    if(events & MENU_OPTION_CLICKED_EVENT)
    {
        ESP_LOGI(TAG, "handle_events called");
        int id = get_selected_option_id();
        int screen_id = SCREEN_ID_MENU_SCREEN_ONE;
        switch(id)
        {
            case 1:
            {
                screen_id = SCREEN_ID_NOTIFICATION;
                break;
            }
            case 2:
            {
                screen_id = SCREEN_ID_WEATHER;
                break;
            }
            case 3:
            {
                screen_id = SCREEN_ID_CALENDAR;
                break;
            }
            case 4:
            {
                screen_id = SCREEN_ID_NAVIGATION;
                break;
            }
            case 5:
            {
                screen_id = SCREEN_ID_ALARM;
                break;
            }
            case 6:
            {
                screen_id = SCREEN_ID_MUSIC;
                break;
            }
            case 7:
            {
                screen_id = SCREEN_ID_SETTINGS;
                break;
            }
            case 8:
            {
                screen_id = SCREEN_ID_INFO;
                break;
            }
        }
        ESP_LOGI(TAG, "Screen ID: %d", screen_id);

        return screen_manager.change_screen(screen_id);
    }
    return ESP_OK;
}

void action_menu_click_func(lv_event_t *e)
{
    int icon_id = (int)(intptr_t)lv_event_get_user_data(e);

    ESP_LOGI(TAG, "Clicked function called id %d", icon_id);

    MenuScreen *menu_screen = screen_manager.get_current_menu_screen();
    if(menu_screen == nullptr)
    {
        return;
    }
    menu_screen->set_selected_option_id(icon_id);

    if(gui_task_handle != nullptr)
    {
        xTaskNotify(gui_task_handle, MENU_OPTION_CLICKED_EVENT, eSetBits);
    }
    
}