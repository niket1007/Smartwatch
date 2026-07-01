#include "navigation_manager.h"

// ################# Variables Start #################
int previous_vertical_screen = 0;
int total_screens = 3; // Only those screens which has left and right gesture enabled
int active_screen = 0;
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

static bool transitionRunning = false;

void navigate_to_screen(int screen, lv_screen_load_anim_t anim)
{
    if (transitionRunning)
    {
        return;
    }

    transitionRunning = true;

    lv_obj_t *scr = get_screen(screen);

    if (scr == nullptr)
    {
        transitionRunning = false;
        return;
    }
    active_screen = screen;
    lv_screen_load(scr);
    // lv_screen_load_anim(scr, anim, 250, 0, false);

    lv_timer_create(
        [](lv_timer_t *timer)
        {
            transitionRunning = false;
            lv_timer_delete(timer);
        },
        300,
        NULL
    );
}

void action_navigate_gesture(lv_event_t * e) {

  if(get_battery_percentage() >= 20) {
    int current_screen = (int)(intptr_t)lv_event_get_user_data(e);

    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

    int next_screen = current_screen;
    lv_screen_load_anim_t animate = LV_SCR_LOAD_ANIM_NONE;

    if(previous_vertical_screen < 0) {
      previous_vertical_screen = 0;
      usb_serial.println("Previous Vertical Screen default to 0");
    }

    // Vertical Movement
    if(dir == LV_DIR_BOTTOM) {
      // App Drawer -> Main Screens 
      if(current_screen == -20) {
        next_screen = previous_vertical_screen;
        animate = LV_SCR_LOAD_ANIM_MOVE_BOTTOM;
        usb_serial.printf("Bottom Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
      }
      // Main Screens -> Settings
      else if(current_screen >= 0) {
        previous_vertical_screen = current_screen;
        next_screen = -10;
        animate = LV_SCR_LOAD_ANIM_MOVE_BOTTOM;
        usb_serial.printf("Bottom Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
      }
    }
    else if(dir == LV_DIR_TOP) {
      // Settings -> Main Screens
      if (current_screen == -10) {
        next_screen = previous_vertical_screen;
        previous_vertical_screen = 0;
        animate = LV_SCR_LOAD_ANIM_MOVE_TOP;
        usb_serial.printf("Top Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
      }
      // Main Screens -> App drawer
      else if (current_screen >= 0) {
        previous_vertical_screen = current_screen;
        next_screen = -20;
        animate = LV_SCR_LOAD_ANIM_MOVE_TOP;
        usb_serial.printf("Top Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
      }
    }
    // Horizontal Movement
    else if(dir == LV_DIR_LEFT && current_screen >= 0) {
      next_screen = (current_screen + 1) % total_screens;
      animate = LV_SCR_LOAD_ANIM_MOVE_LEFT;
      usb_serial.printf("Left Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
    }
    else if(dir == LV_DIR_RIGHT && current_screen >= 0) {
      next_screen = (current_screen - 1 + total_screens) % total_screens;
      animate = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
      usb_serial.printf("Right Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
    }

    if(next_screen != current_screen)
    {
        usb_serial.printf(
            "Current:%d Previous:%d Next:%d Gesture:%d\n",
            current_screen,
            previous_vertical_screen,
            next_screen,
            dir
        );

        lv_obj_t *scr = get_screen(next_screen);

        if (scr == nullptr) {
          usb_serial.println("ERROR: get_screen returned nullptr");
          return;
        }
        usb_serial.println("Before async");

        navigate_to_screen(next_screen, animate);

        usb_serial.println("After async");
    }
  }
}