#include "navigation_manager.h"

// ################# Variables Start #################
int previous_vertical_screen = 0;
int total_screens = 3; // Only those screens which has left and right gesture enabled
// ################# Variables End #################


lv_obj_t* get_screen(int number) {
    if(number == 0) {
        return objects.home_screen;
    }
    else if(number == 1) {
        return objects.weather_screen;
    }
    else if(number == 2) {
        return objects.news_screen;
    }
    else if(number == -10) {
        return objects.settings_screen;
    }
    else if(number == -20) {
        return objects.app_drawer_screen;
    }
    else {
        return objects.home_screen;
    }
}

void action_action_navigate_gesture(lv_event_t * e) {

  // if(global_battery_percentage > 30) {
    int current_screen = (int)(intptr_t)lv_event_get_user_data(e);

    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

    int next_screen = current_screen;
    lv_screen_load_anim_t animate = LV_SCR_LOAD_ANIM_NONE;

    if(previous_vertical_screen < 0) {
      previous_vertical_screen = 0;
      USBSerial.println("Previous Vertical Screen default to 0");
    }

    // Vertical Movement
    if(dir == LV_DIR_BOTTOM) {
      // App Drawer -> Main Screens 
      if(current_screen == -20) {
        next_screen = previous_vertical_screen;
        animate = LV_SCR_LOAD_ANIM_MOVE_BOTTOM;
        USBSerial.printf("Bottom Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
      }
      // Main Screens -> Settings
      else if(current_screen >= 0) {
        previous_vertical_screen = current_screen;
        next_screen = -10;
        animate = LV_SCR_LOAD_ANIM_MOVE_BOTTOM;
        USBSerial.printf("Bottom Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
      }
    }
    else if(dir == LV_DIR_TOP) {
      // Settings -> Main Screens
      if (current_screen == -10) {
        next_screen = previous_vertical_screen;
        previous_vertical_screen = 0;
        animate = LV_SCR_LOAD_ANIM_MOVE_TOP;
        USBSerial.printf("Top Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
      }
      // Main Screens -> App drawer
      else if (current_screen >= 0) {
        previous_vertical_screen = current_screen;
        next_screen = -20;
        animate = LV_SCR_LOAD_ANIM_MOVE_TOP;
        USBSerial.printf("Top Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
      }
    }
    // Horizontal Movement
    else if(dir == LV_DIR_LEFT && current_screen >= 0) {
      next_screen = (current_screen + 1) % total_screens;
      animate = LV_SCR_LOAD_ANIM_MOVE_LEFT;
      USBSerial.printf("Left Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
    }
    else if(dir == LV_DIR_RIGHT && current_screen >= 0) {
      next_screen = (current_screen - 1 + total_screens) % total_screens;
      animate = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
      USBSerial.printf("Right Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
    }

    if(next_screen != current_screen)
    {
        USBSerial.printf(
            "Current:%d Previous:%d Next:%d Gesture:%d\n",
            current_screen,
            previous_vertical_screen,
            next_screen,
            dir
        );

        lv_screen_load_anim(
            get_screen(next_screen),
            animate,
            250,
            0,
            false
        );
    }
  // }
}