#pragma once

#include "globals.h"
#include <Arduino.h>
#include <lvgl.h>

// Screen dimensions
extern uint32_t screenWidth;
extern uint32_t screenHeight;

// Initializes LVGL core (lv_init, tick cb, log cb), creates the display +
// buffers + flush/rounder callbacks, creates the touch indev, loads the
// EEZ Studio UI (ui_init)
void lvgl_manager_init();