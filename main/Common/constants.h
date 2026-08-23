#pragma once

#include <cstdint>

// ---------------------- Event -----------------
// BATTERY
constexpr uint32_t BAT_PERCENT_CHG_EVENT = 1U << 0;
constexpr uint32_t BAT_CHARGE_CHG_EVENT = 1U << 1;

// SCREEN ON
constexpr uint32_t SCREEN_ON_EVENT = 1U << 2;
constexpr uint32_t UPDATE_TIME_EVENT = 1U << 3;

// WIFI
constexpr uint32_t WIFI_CONNECTED_EVENT = 1U << 4;
constexpr uint32_t WIFI_FAILED_EVENT = 1U << 5;

// SCREEN OFF
constexpr uint32_t SCREEN_OFF_EVENT = 1U << 6;

// SWIPE
constexpr uint32_t SWIPE_LEFT_EVENT = 1U << 7;
constexpr uint32_t SWIPE_RIGHT_EVENT = 1U << 8;
constexpr uint32_t SINGLE_TAP_EVENT = 1U << 9;

// BLUETOOTH SCREEN
constexpr uint32_t BLUETOOTH_INIT = 1U << 10;
constexpr uint32_t BLE_STATUS_EVENT = 1U << 11;
constexpr uint32_t CALL_SCREEN_EVENT = 1U << 13;
constexpr uint32_t UPDATE_CALL_SCREEN_EVENT = 1U << 14;

// MENU SCREEN
constexpr uint32_t MENU_OPTION_CLICKED_EVENT = 1U << 15;

// ---------------------- Timeout -----------------
constexpr uint32_t SCREEN_TIMEOUT = 15000;
constexpr uint32_t UPDATE_TIME_TIMER = 30000;