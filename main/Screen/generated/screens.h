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
    SCREEN_ID_MENU_SCREEN_ONE = 2,
    SCREEN_ID_MENU_SCREEN_TWO = 3,
    SCREEN_ID_CALL = 4,
    SCREEN_ID_BLE_STATUS = 5,
    _SCREEN_ID_LAST = 5
};

typedef struct _objects_t {
    lv_obj_t *home;
    lv_obj_t *menu_screen_one;
    lv_obj_t *menu_screen_two;
    lv_obj_t *call;
    lv_obj_t *ble_status;
    lv_obj_t *parent_container;
    lv_obj_t *time_label;
    lv_obj_t *date_label;
    lv_obj_t *day_label;
    lv_obj_t *am_pm_label;
    lv_obj_t *battery_container;
    lv_obj_t *battery_icon;
    lv_obj_t *battery_percentage_bar;
    lv_obj_t *battery_percentage_label;
    lv_obj_t *menu_one_pc;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *menu_two_pc;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *call_container;
    lv_obj_t *caller_name;
    lv_obj_t *phone_number;
    lv_obj_t *phone_status_icon;
    lv_obj_t *icoming_icon_one;
    lv_obj_t *icoming_icon_two;
    lv_obj_t *outgoing_icon;
    lv_obj_t *blestatus_pc;
    lv_obj_t *ble_con_status_label;
} objects_t;

extern objects_t objects;

void create_screen_home();
void tick_screen_home();

void create_screen_menu_screen_one();
void tick_screen_menu_screen_one();

void create_screen_menu_screen_two();
void tick_screen_menu_screen_two();

void create_screen_call();
void tick_screen_call();

void create_screen_ble_status();
void tick_screen_ble_status();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/