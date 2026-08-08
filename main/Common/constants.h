#pragma once

#include <cstdint>

#define LCD_WIDTH 410
#define LCD_HEIGHT 502

// ---------------------- Color -----------------
constexpr int BLACK_COLOR = 0x0000;
constexpr int GREEN_COLOR = 0x001F;
constexpr int WHITE_COLOR = 0xFFFF;

// ---------------------- Event -----------------
// BATTERY
constexpr uint32_t BAT_PERCENT_CHG_EVENT = 1U << 0;
constexpr uint32_t BAT_CHARGE_CHG_EVENT = 1U << 1;

// SCREEN ON
constexpr uint32_t SCREEN_ON_EVENT = 1U << 2;
constexpr uint32_t UPDATE_TIME_EVENT = 1U << 3;

// WIFI
constexpr uint32_t WIFI_CONNECTED_EVENT = 1U << 0;
constexpr uint32_t WIFI_FAILED_EVENT = 1U << 1;

// SCREEN OFF
constexpr uint32_t SCREEN_OFF_EVENT = 1U << 4;

// SWIPE
constexpr uint32_t SWIPE_LEFT_EVENT = 1U << 5;
constexpr uint32_t SWIPE_RIGHT_EVENT = 1U << 6;
constexpr uint32_t SINGLE_TAP_EVENT = 1U << 7;

// ---------------------- Timeout -----------------
constexpr uint32_t SCREEN_TIMEOUT = 15000;
constexpr uint32_t UPDATE_TIME_TIMER = 30000;