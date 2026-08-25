#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_gesture_func(lv_event_t * e);
extern void action_menu_click_func(lv_event_t * e);
extern void action_weather_refresh_clicked(lv_event_t * e);
extern void action_call_click_func(lv_event_t * e);
extern void action_settings_change_brightness(lv_event_t * e);
extern void action_setting_set_wifi_cred(lv_event_t * e);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/