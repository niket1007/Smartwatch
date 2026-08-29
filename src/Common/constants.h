#pragma once

#include <cstdint>

/*
*   GUI Events
*   Events received by GUI Task.
*/

// BATTERY 
constexpr uint32_t BAT_PERCENT_CHG_EVENT = 1U << 0;
constexpr uint32_t BAT_CHARGE_CHG_EVENT = 1U << 1;

// TIME EVENT
constexpr uint32_t UPDATE_TIME_EVENT = 1U << 2;

// SWIPE Gesture event attached to each screen and its parent container
constexpr uint32_t SWIPE_LEFT_EVENT = 1U << 3;
constexpr uint32_t SWIPE_RIGHT_EVENT = 1U << 4;
constexpr uint32_t SINGLE_TAP_EVENT = 1U << 5;

// BLE
constexpr uint32_t BLE_STATUS_EVENT = 1U << 6;
constexpr uint32_t CALL_SCREEN_EVENT = 1U << 7;

// MENU SCREEN
constexpr uint32_t MENU_OPTION_CLICKED_EVENT = 1U << 8;

// WEATHER SCREEN
constexpr uint32_t WEATHER_UI_UPDATE_EVENT = 1U << 9;

// HOME SCREEN
constexpr uint32_t BACK_TO_HOME_EVENT = 1U << 10;

// NAVIGATION SCREEN
constexpr uint32_t NAV_SCREEN_UDPATE_EVENT = 1U << 11;

// DISPLAY
constexpr uint32_t SCREEN_ON_EVENT = 1U << 12;
constexpr uint32_t SCREEN_OFF_EVENT = 1U << 13;
constexpr uint32_t BOOT_BUTTON_CLICK_EVENT = 1U << 14;

// TOUCH
constexpr uint32_t TOUCH_DISABLED_EVENT = 1U << 15;
constexpr uint32_t TOUCH_ENABLED_EVENT = 1U << 16;


/*
*   Other Events
*   Events received by other tasks [RTC Time Sync Task, Background Task]
*/

// BG WAKE
constexpr uint32_t BG_WAKE_UP = 1U << 0;

// WIFI
constexpr uint32_t WIFI_CONNECTED_EVENT = 1U << 1;
constexpr uint32_t WIFI_FAILED_EVENT = 1U << 2;

// BLUETOOTH
constexpr uint32_t BLUETOOTH_INIT_EVENT = 1U << 3;
constexpr uint32_t BLUETOOTH_DEINIT_EVENT = 1U << 4;
constexpr uint32_t ACCEPT_CALL_EVENT = 1U << 5;
constexpr uint32_t REJECT_CALL_EVENT = 1U << 6;
constexpr uint32_t SEND_BATTERY_DATA_EVENT = 1U << 7;
constexpr uint32_t INIT_WEATHER_FETCH_EVENT = 1U << 8;

// ---------------------- Timeout -----------------
constexpr uint32_t UPDATE_TIME_TIMER = 3000; //3 sec