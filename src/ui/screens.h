#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_HOME_SCREEN = 1,
    SCREEN_ID_SETTINGS_SCREEN = 2,
    SCREEN_ID_WEATHER_SCREEN = 3,
    SCREEN_ID_NEWS_SCREEN = 4,
    SCREEN_ID_APP_DRAWER_SCREEN = 5,
    _SCREEN_ID_LAST = 5
};

typedef struct _objects_t {
    lv_obj_t *home_screen;
    lv_obj_t *settings_screen;
    lv_obj_t *weather_screen;
    lv_obj_t *news_screen;
    lv_obj_t *app_drawer_screen;
    lv_obj_t *parent_container;
    lv_obj_t *wifi_bluetooth_container;
    lv_obj_t *battery_container;
    lv_obj_t *wifi_label;
    lv_obj_t *bluetooth_label;
    lv_obj_t *time_label;
    lv_obj_t *date_label;
    lv_obj_t *day_label;
    lv_obj_t *am_pm_label;
    lv_obj_t *battery_percentage_bar;
    lv_obj_t *battery_status_label;
    lv_obj_t *battery_percentage_label;
    lv_obj_t *obj0;
    lv_obj_t *parent_container_1;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *obj9;
    lv_obj_t *high_temp_label;
    lv_obj_t *obj10;
    lv_obj_t *low_temp_label;
    lv_obj_t *obj11;
    lv_obj_t *humidity_label;
    lv_obj_t *parent_container_2;
} objects_t;

extern objects_t objects;

void create_screen_home_screen();
void tick_screen_home_screen();

void create_screen_settings_screen();
void tick_screen_settings_screen();

void create_screen_weather_screen();
void tick_screen_weather_screen();

void create_screen_news_screen();
void tick_screen_news_screen();

void create_screen_app_drawer_screen();
void tick_screen_app_drawer_screen();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/