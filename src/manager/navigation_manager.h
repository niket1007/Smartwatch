#pragma once

#include "globals.h"
#include <lvgl.h>

extern int active_screen;

extern lv_obj_t* get_screen(int number);
extern void action_navigate_gesture(lv_event_t * e);
void navigate_to_screen(int screen, lv_screen_load_anim_t anim);