#include "navigation_manager.h"

// ################# Variables Start #################
int previous_vertical_screen = 0;
int total_screens = 3; // Only those screens which has left and right gesture enabled
// ################# Variables End #################


lv_obj_t* get_screen(int number) {

  switch(number) {
    case HOME_SCREEN:
      return objects.home_screen;
      break;
    case WEATHER_SCREEN:
      return objects.weather_screen;
      break;
    case SETTINGS_SCREEN:
      return objects.settings_screen;
      break;
    case NOTIFICATION_SCREEN:
      return objects.notification_screen;
      break;
    case BLE_PASSKEY_SCREEN:
      return objects.ble_passkey_screen;
      break;
    case CALENDAR_SCREEN:
      return objects.calendar_screen;
      break;
    case CALL_NOTIF_SCREEN:
      return objects.call_notification_screen;
      break;
    default:
      return objects.home_screen;
  }
}

static bool transitionRunning = false;

void navigate_to_screen(int screen)
{
    if (transitionRunning)
    {
      return;
    }

    transitionRunning = true;

    lv_obj_t *scr = get_screen(screen);

    if (scr == nullptr)
    {
      // usb_serial.printf("Screen is null %d\n", screen);
      transitionRunning = false;
      return;
    }
    gv.active_screen_id = screen;
    lv_screen_load(scr);

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

  if(get_battery_percentage() > 20) {
    int current_screen = (int)(intptr_t)lv_event_get_user_data(e);

    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

    int next_screen = current_screen;

    if(previous_vertical_screen < 0) {
      previous_vertical_screen = 0;
      // usb_serial.println("Previous Vertical Screen default to 0");
    }

    // Vertical Movement
    if(dir == LV_DIR_BOTTOM) {
      // App Drawer -> Main Screens 
      if(current_screen == -20) {
        next_screen = previous_vertical_screen;
        // usb_serial.printf("Bottom Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
      }
      // Main Screens -> Settings
      else if(current_screen >= 0) {
        previous_vertical_screen = current_screen;
        next_screen = -10;
        // usb_serial.printf("Bottom Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
      }
    }
    else if(dir == LV_DIR_TOP) {
      // Settings -> Main Screens
      if (current_screen == -10) {
        next_screen = previous_vertical_screen;
        previous_vertical_screen = 0;
        // usb_serial.printf("Top Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
      }
      // Main Screens -> App drawer
      else if (current_screen >= 0) {
        previous_vertical_screen = current_screen;
        next_screen = -20;
        // usb_serial.printf("Top Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
      }
    }
    // Horizontal Movement
    else if(dir == LV_DIR_LEFT && current_screen >= 0) {
      next_screen = (current_screen + 1) % total_screens;
      // usb_serial.printf("Left Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
    }
    else if(dir == LV_DIR_RIGHT && current_screen >= 0) {
      next_screen = (current_screen - 1 + total_screens) % total_screens;
      // usb_serial.printf("Right Direction Swipe: Current Screen: %d and Next Screen: %d\n", current_screen, next_screen);
    }

    if(next_screen != current_screen)
    {
        // usb_serial.printf(
        //     "Current:%d Previous:%d Next:%d Gesture:%d\n",
        //     current_screen,
        //     previous_vertical_screen,
        //     next_screen,
        //     dir
        // );

        navigate_to_screen(next_screen);
    }
  } 
  else {
    if (gv.active_screen_id != HOME_SCREEN) {
      navigate_to_screen(HOME_SCREEN);
    }
  }
}