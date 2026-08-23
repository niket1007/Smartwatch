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

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[20];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/