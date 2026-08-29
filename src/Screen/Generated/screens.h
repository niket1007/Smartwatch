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
    SCREEN_ID_NOTIFICATION = 6,
    SCREEN_ID_MUSIC = 7,
    SCREEN_ID_WEATHER = 8,
    SCREEN_ID_NAVIGATION = 9,
    SCREEN_ID_ALARM = 10,
    SCREEN_ID_CALENDAR = 11,
    SCREEN_ID_SETTINGS = 12,
    SCREEN_ID_INFO = 13,
    SCREEN_ID_BRIGHTNESS = 14,
    SCREEN_ID_BLUETOOTH = 15,
    _SCREEN_ID_LAST = 15
};

typedef struct _objects_t {
    lv_obj_t *home;
    lv_obj_t *menu_screen_one;
    lv_obj_t *menu_screen_two;
    lv_obj_t *call;
    lv_obj_t *ble_status;
    lv_obj_t *notification;
    lv_obj_t *music;
    lv_obj_t *weather;
    lv_obj_t *navigation;
    lv_obj_t *alarm;
    lv_obj_t *calendar;
    lv_obj_t *settings;
    lv_obj_t *info;
    lv_obj_t *brightness;
    lv_obj_t *bluetooth;
    lv_obj_t *parent_container;
    lv_obj_t *status_container;
    lv_obj_t *notif_count_label;
    lv_obj_t *time_label;
    lv_obj_t *date_label;
    lv_obj_t *day_label;
    lv_obj_t *battery_container;
    lv_obj_t *battery_icon;
    lv_obj_t *battery_percentage_bar;
    lv_obj_t *battery_percentage_label;
    lv_obj_t *menu_one_pc;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *menu_two_pc;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *call_container;
    lv_obj_t *caller_name;
    lv_obj_t *phone_number;
    lv_obj_t *phone_status_icon;
    lv_obj_t *accept_call_icon;
    lv_obj_t *reject_call_icon_one;
    lv_obj_t *reject_call_icon_two;
    lv_obj_t *blestatus_pc;
    lv_obj_t *ble_con_status_label;
    lv_obj_t *notification_pc;
    lv_obj_t *obj8;
    lv_obj_t *obj9;
    lv_obj_t *calls_notif_count;
    lv_obj_t *obj10;
    lv_obj_t *obj11;
    lv_obj_t *messages_notif_count;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
    lv_obj_t *others_notif_count;
    lv_obj_t *music_pc;
    lv_obj_t *music_name_label;
    lv_obj_t *artist_name_label;
    lv_obj_t *volume_down_icon;
    lv_obj_t *volume_up_icon;
    lv_obj_t *music_next_icon;
    lv_obj_t *music_prev_icon;
    lv_obj_t *music_pause_icon;
    lv_obj_t *music_play_icon;
    lv_obj_t *weather_pc;
    lv_obj_t *temp_label;
    lv_obj_t *weather_type_label;
    lv_obj_t *wloc_label;
    lv_obj_t *wrefresh_label;
    lv_obj_t *high_temp_container;
    lv_obj_t *obj14;
    lv_obj_t *high_temp_label;
    lv_obj_t *low_temp_container;
    lv_obj_t *low_const_label;
    lv_obj_t *low_temp_label;
    lv_obj_t *humidity_container;
    lv_obj_t *humidity_const_label;
    lv_obj_t *humidity_label;
    lv_obj_t *wind_container;
    lv_obj_t *wind_const_label;
    lv_obj_t *wind_label;
    lv_obj_t *rain_chances_container;
    lv_obj_t *rain_chance_const_label;
    lv_obj_t *rain_chance_label;
    lv_obj_t *navigation_pc;
    lv_obj_t *direction_icon;
    lv_obj_t *direction_label;
    lv_obj_t *direction_instr_label;
    lv_obj_t *distance_label;
    lv_obj_t *obj15;
    lv_obj_t *obj16;
    lv_obj_t *eta_label;
    lv_obj_t *alarm_pc;
    lv_obj_t *calendar_pc;
    lv_obj_t *calendar_calendar;
    lv_obj_t *settings_pc;
    lv_obj_t *obj17;
    lv_obj_t *obj18;
    lv_obj_t *obj19;
    lv_obj_t *info_pc;
    lv_obj_t *obj20;
    lv_obj_t *obj21;
    lv_obj_t *obj22;
    lv_obj_t *obj23;
    lv_obj_t *obj24;
    lv_obj_t *obj25;
    lv_obj_t *brightness_pc;
    lv_obj_t *brightness_slider;
    lv_obj_t *brightness_percent_label;
    lv_obj_t *bluetooth_pc;
    lv_obj_t *obj26;
    lv_obj_t *ble_device_name;
    lv_obj_t *ble_toggle_button;
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

void create_screen_notification();
void tick_screen_notification();

void create_screen_music();
void tick_screen_music();

void create_screen_weather();
void tick_screen_weather();

void create_screen_navigation();
void tick_screen_navigation();

void create_screen_alarm();
void tick_screen_alarm();

void create_screen_calendar();
void tick_screen_calendar();

void create_screen_settings();
void tick_screen_settings();

void create_screen_info();
void tick_screen_info();

void create_screen_brightness();
void tick_screen_brightness();

void create_screen_bluetooth();
void tick_screen_bluetooth();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/