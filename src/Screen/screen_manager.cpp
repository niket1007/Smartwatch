#include "screen_manager.h"
#include "Screen/Generated/ui.h"
#include "Screen/Generated/screens.h"
#include "Screen/Generated/actions.h"
#include "Common/globals.h"

// Screen Includes
#include "Screen/Home/home_screen.h"
#include "Screen/Menu/menu_screen.h"
#include "Screen/Ble_Status/blestatus_screen.h"
#include "Screen/Alarm/alarm_screen.h"
#include "Screen/Calendar/calendar_screen.h"
#include "Screen/Navigation/navigation_screen.h"
#include "Screen/Call/call_screen.h"
#include "Screen/Info/info_screen.h"
#include "Screen/Music/music_screen.h"
#include "Screen/Notification/notification_screen.h"
#include "Screen/Weather/weather_screen.h"
#include "Screen/Settings/settings_screen.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static constexpr const char *TAG = "SCREEN_MANAGER";

MenuScreen *ScreenManager::get_current_menu_screen()
{
    if (current_screen_id != SCREEN_ID_MENU_SCREEN_ONE &&
        current_screen_id != SCREEN_ID_MENU_SCREEN_TWO)
    {
        return nullptr;
    }

    return static_cast<MenuScreen *>(current_screen);
}

esp_err_t ScreenManager::init()
{
    usb_serial.println("ScreenManager :: init called");

    create_screens();

    usb_serial.println("EEZ Home Screen initialized");

    return ESP_OK;
}

Screen *ScreenManager::get_screen_instance(int id)
{
    switch (id)
    {
    case SCREEN_ID_HOME:
    {
        return new HomeScreen();
    }
    case SCREEN_ID_MENU_SCREEN_ONE:
    case SCREEN_ID_MENU_SCREEN_TWO:
    {
        return new MenuScreen();
    }
    case SCREEN_ID_NOTIFICATION:
    {
        return new NotificationScreen();
    }
    case SCREEN_ID_WEATHER:
    {
        return new WeatherScreen();
    }
    case SCREEN_ID_CALENDAR:
    {
        return new CalendarScreen();
    }
    case SCREEN_ID_NAVIGATION:
    {
        return new NavigationScreen();
    }
    case SCREEN_ID_ALARM:
    {
        return new AlarmScreen();
    }
    case SCREEN_ID_MUSIC:
    {
        return new MusicScreen();
    }
    case SCREEN_ID_SETTINGS:
    {
        return new SettingsScreen();
    }
    case SCREEN_ID_INFO:
    {
        return new InfoScreen();
    }
    case SCREEN_ID_BLE_STATUS:
    {
        return new BleStatusScreen();
    }
    case SCREEN_ID_CALL:
    {
        return new CallScreen();
    }
    default:
    {
        return new HomeScreen();
    }
    }
}

int ScreenManager::get_current_screen_id()
{
    return current_screen_id;
}

esp_err_t ScreenManager::load_default_screen()
{
    current_screen_id = SCREEN_ID_HOME;

    loadScreen(SCREEN_ID_HOME);

    current_screen = get_screen_instance(current_screen_id);

    if (current_screen == nullptr)
    {
        usb_serial.println("Failed to create HomeScreen");
        return ESP_ERR_NO_MEM;
    }

    return current_screen->on_enter();
}

esp_err_t ScreenManager::change_screen(int new_screen_id)
{
    if (transition_running_)
    {
        usb_serial.println("Screen transition already in progress");
        return ESP_ERR_INVALID_STATE;
    }

    if (new_screen_id == current_screen_id)
    {
        return ESP_OK;
    }

    transition_running_ = true;

    usb_serial.printf("Changing screen: %d -> %d\n",
        current_screen_id, new_screen_id);

    Screen *new_screen = get_screen_instance(new_screen_id);

    if (new_screen == nullptr)
    {
        usb_serial.printf(
            "Failed to create screen: %d\n",
            new_screen_id);

        transition_running_ = false;
        return ESP_ERR_NOT_FOUND;
    }

    if (current_screen != nullptr)
    {
        esp_err_t err = current_screen->on_exit();

        if (err != ESP_OK)
        {
            delete new_screen;
            transition_running_ = false;
            return err;
        }
    }

    loadScreen(static_cast<ScreensEnum>(new_screen_id));

    delete current_screen;

    current_screen = new_screen;
    current_screen_id = new_screen_id;

    esp_err_t err = current_screen->on_enter();

    transition_running_ = false;

    return err;
}

esp_err_t ScreenManager::navigate(int direction)
{
    int next_screen = current_screen_id;

    if (direction == SWIPE_LEFT_EVENT)
    {
        switch (current_screen_id)
        {
        case SCREEN_ID_HOME:
            next_screen = SCREEN_ID_MENU_SCREEN_ONE;
            break;

        case SCREEN_ID_MENU_SCREEN_ONE:
            next_screen = SCREEN_ID_MENU_SCREEN_TWO;
            break;

        case SCREEN_ID_MENU_SCREEN_TWO:
            next_screen = SCREEN_ID_HOME;
            break;

        default:
            return ESP_ERR_INVALID_STATE;
        }
    }
    else if (direction == SWIPE_RIGHT_EVENT)
    {
        switch (current_screen_id)
        {
        case SCREEN_ID_HOME:
            next_screen = SCREEN_ID_MENU_SCREEN_TWO;
            break;

        case SCREEN_ID_MENU_SCREEN_ONE:
            next_screen = SCREEN_ID_HOME;
            break;

        case SCREEN_ID_MENU_SCREEN_TWO:
            next_screen = SCREEN_ID_MENU_SCREEN_ONE;
            break;

        default:
            return ESP_ERR_INVALID_STATE;
        }
    }

    if (next_screen == current_screen_id)
    {
        return ESP_OK;
    }

    return change_screen(next_screen);
}

esp_err_t ScreenManager::handle_events(uint32_t events)
{
    usb_serial.println("ScreenManager handl_events called");
    if ((events & BACK_TO_HOME_EVENT) and
        (current_screen_id != SCREEN_ID_HOME))
    {
        return change_screen(SCREEN_ID_HOME);
    }

    if (current_screen_id >= 1 and current_screen_id <= 3)
    {
        if (events & SWIPE_LEFT_EVENT)
        {
            return navigate(SWIPE_LEFT_EVENT);
        }

        if (events & SWIPE_RIGHT_EVENT)
        {
            return navigate(SWIPE_RIGHT_EVENT);
        }
    }

    if ((events & BLE_STATUS_EVENT) and (current_screen_id != SCREEN_ID_BLE_STATUS))
    {
        usb_serial.println("Ble status event called");
        return change_screen(SCREEN_ID_BLE_STATUS);
    }

    if ((events & CALL_SCREEN_EVENT) and (current_screen_id != SCREEN_ID_CALL))
    {
        usb_serial.println("Call screen event called");
        return change_screen(SCREEN_ID_CALL);
    }

    if (current_screen != nullptr)
        return current_screen->handle_events(events);

    return ESP_OK;
}

// LVGL Gesture Event
void action_gesture_func(lv_event_t *e)
{
    // if (power_saver_manager.is_touch_enabled())
    // {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

        if (dir == LV_DIR_LEFT)
        {
            if (gui_task_handle != nullptr)
            {
                xTaskNotify(
                    gui_task_handle,
                    SWIPE_LEFT_EVENT,
                    eSetBits);
            }
        }
        else if (dir == LV_DIR_RIGHT)
        {
            if (gui_task_handle != nullptr)
            {
                xTaskNotify(
                    gui_task_handle,
                    SWIPE_RIGHT_EVENT,
                    eSetBits);
            }
        }
    // }
    // else
    // {
    //     if ((gui_task_handle != nullptr) and
    //         (screen_manager.get_current_screen_id() != SCREEN_ID_HOME))
    //     {
    //         xTaskNotify(
    //             gui_task_handle, BACK_TO_HOME_EVENT, eSetBits);
    //     }
    // }
}
