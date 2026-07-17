#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_navigate_gesture(lv_event_t * e);
extern void action_wifi_save_button_clicked(lv_event_t * e);
extern void action_enable_power_saver_mode(lv_event_t * e);
extern void action_wifi_refresh_button_clicked(lv_event_t * e);
extern void action_weather_refresh_clicked(lv_event_t * e);
extern void action_accept_call_clicked(lv_event_t * e);
extern void action_reject_call_clicked(lv_event_t * e);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/