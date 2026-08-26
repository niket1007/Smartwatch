#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_battery_charging;
extern const lv_img_dsc_t img_info;
extern const lv_img_dsc_t img_notification;
extern const lv_img_dsc_t img_weather;
extern const lv_img_dsc_t img_calendar;
extern const lv_img_dsc_t img_navigation;
extern const lv_img_dsc_t img_alarm;
extern const lv_img_dsc_t img_music;
extern const lv_img_dsc_t img_settings;
extern const lv_img_dsc_t img_passkey;
extern const lv_img_dsc_t img_phone_default;
extern const lv_img_dsc_t img_phone_reject;
extern const lv_img_dsc_t img_phone_in_progress;
extern const lv_img_dsc_t img_phone_incoming;
extern const lv_img_dsc_t img_phone_outgoing;
extern const lv_img_dsc_t img_phone_reject_action;
extern const lv_img_dsc_t img_phone_accept;
extern const lv_img_dsc_t img_bluetooth_connected;
extern const lv_img_dsc_t img_wifi;
extern const lv_img_dsc_t img_brightness;
extern const lv_img_dsc_t img_battery_0_20;
extern const lv_img_dsc_t img_battery_21_49;
extern const lv_img_dsc_t img_battery_50_79;
extern const lv_img_dsc_t img_battery_80_100;
extern const lv_img_dsc_t img_play;
extern const lv_img_dsc_t img_pause;
extern const lv_img_dsc_t img_next;
extern const lv_img_dsc_t img_prev;
extern const lv_img_dsc_t img_volume_plus;
extern const lv_img_dsc_t img_volume_minus;
extern const lv_img_dsc_t img_arrow_right;
extern const lv_img_dsc_t img_arrow_left;
extern const lv_img_dsc_t img_arrow_up;
extern const lv_img_dsc_t img_keep_left;
extern const lv_img_dsc_t img_keep_right;
extern const lv_img_dsc_t img_slight_right;
extern const lv_img_dsc_t img_slight_left;
extern const lv_img_dsc_t img_sharp_right;
extern const lv_img_dsc_t img_sharp_left;
extern const lv_img_dsc_t img_u_turn_right;
extern const lv_img_dsc_t img_u_turn_left;
extern const lv_img_dsc_t img_roundabout_left;
extern const lv_img_dsc_t img_roundabout_right;
extern const lv_img_dsc_t img_roundabout_straight;
extern const lv_img_dsc_t img_roundabout_uturn;
extern const lv_img_dsc_t img_offroute;
extern const lv_img_dsc_t img_nav_finish;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[47];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/