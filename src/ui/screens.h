#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_HOME = 1,
    _SCREEN_ID_LAST = 1
};

typedef struct _objects_t {
    lv_obj_t *home;
    lv_obj_t *parent_container;
    lv_obj_t *wifi_bluetooth_container;
    lv_obj_t *battery_container;
    lv_obj_t *wifi_label;
    lv_obj_t *bluetooth_label;
    lv_obj_t *time_label;
    lv_obj_t *date_label;
    lv_obj_t *day_label;
    lv_obj_t *am_pm_label;
    lv_obj_t *right_day_border;
    lv_obj_t *left_day_border;
    lv_obj_t *battery_percentage_bar;
    lv_obj_t *battery_status_label;
    lv_obj_t *battery_percentage_label;
} objects_t;

extern objects_t objects;

void create_screen_home();
void tick_screen_home();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/