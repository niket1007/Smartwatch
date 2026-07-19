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
    SCREEN_ID_WEATHER_SCREEN = 2,
    SCREEN_ID_NOTIFICATION_SCREEN = 3,
    SCREEN_ID_SETTINGS_SCREEN = 4,
    SCREEN_ID_BLE_PASSKEY_SCREEN = 5,
    SCREEN_ID_CALENDAR_SCREEN = 6,
    SCREEN_ID_CALL_NOTIFICATION_SCREEN = 7,
    _SCREEN_ID_LAST = 7
};

typedef struct _objects_t {
    lv_obj_t *home_screen;
    lv_obj_t *weather_screen;
    lv_obj_t *notification_screen;
    lv_obj_t *settings_screen;
    lv_obj_t *ble_passkey_screen;
    lv_obj_t *calendar_screen;
    lv_obj_t *call_notification_screen;
    lv_obj_t *parent_container;
    lv_obj_t *time_label;
    lv_obj_t *date_label;
    lv_obj_t *day_label;
    lv_obj_t *am_pm_label;
    lv_obj_t *battery_container;
    lv_obj_t *battery_status_label;
    lv_obj_t *battery_percentage_bar;
    lv_obj_t *battery_percentage_label;
    lv_obj_t *weather_container;
    lv_obj_t *temp_label;
    lv_obj_t *weather_type_label;
    lv_obj_t *loc_label;
    lv_obj_t *refresh_label;
    lv_obj_t *high_temp_container;
    lv_obj_t *obj0;
    lv_obj_t *high_temp_label;
    lv_obj_t *low_temp_container;
    lv_obj_t *low_const_label;
    lv_obj_t *low_temp_label;
    lv_obj_t *humidity_container;
    lv_obj_t *humidity_const_label;
    lv_obj_t *humidity_label;
    lv_obj_t *aqi_container;
    lv_obj_t *wind_const_label;
    lv_obj_t *wind_label;
    lv_obj_t *rain_chances_container;
    lv_obj_t *rain_chance_const_label;
    lv_obj_t *rain_chance_label;
    lv_obj_t *notification_tab;
    lv_obj_t *notification_show_tab;
    lv_obj_t *music_tab;
    lv_obj_t *track_name_label;
    lv_obj_t *artist_label;
    lv_obj_t *music_sub_container;
    lv_obj_t *duration_label;
    lv_obj_t *navigation_tab;
    lv_obj_t *dir_label;
    lv_obj_t *nav_eta_container;
    lv_obj_t *obj1;
    lv_obj_t *eta_datetime_label;
    lv_obj_t *settings_tabview;
    lv_obj_t *ble_tab;
    lv_obj_t *obj2;
    lv_obj_t *ble_heading_label_1;
    lv_obj_t *ble_const_label_1;
    lv_obj_t *ble_device_name;
    lv_obj_t *ble_user_switch;
    lv_obj_t *obj3;
    lv_obj_t *wifi_heading_label;
    lv_obj_t *wifi_ct_const_label;
    lv_obj_t *wifi_ssid_name;
    lv_obj_t *wifi_const_label_1;
    lv_obj_t *wifi_tab;
    lv_obj_t *wifi_const_label;
    lv_obj_t *wifi_pass_text_field;
    lv_obj_t *wifi_ssid_dropdown;
    lv_obj_t *wifi_save_button;
    lv_obj_t *wifi_refresh_button;
    lv_obj_t *systen_info_label;
    lv_obj_t *obj4;
    lv_obj_t *brightness_label;
    lv_obj_t *brightness_bar;
    lv_obj_t *obj5;
    lv_obj_t *interval_label;
    lv_obj_t *rbs_label;
    lv_obj_t *ub_label;
    lv_obj_t *udt_label;
    lv_obj_t *udt_interval_label;
    lv_obj_t *ub_interval_label;
    lv_obj_t *rbs_interval_label;
    lv_obj_t *obj6;
    lv_obj_t *st_label;
    lv_obj_t *st_interval_label;
    lv_obj_t *power_saver_tab;
    lv_obj_t *psm_const_label;
    lv_obj_t *psm_row_1;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *psm_row_2;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
    lv_obj_t *psm_row_3;
    lv_obj_t *obj11;
    lv_obj_t *obj12;
    lv_obj_t *psm_row_4;
    lv_obj_t *obj13;
    lv_obj_t *obj14;
    lv_obj_t *psm_row_5;
    lv_obj_t *obj15;
    lv_obj_t *obj16;
    lv_obj_t *about_tab;
    lv_obj_t *about_row_1;
    lv_obj_t *obj17;
    lv_obj_t *obj18;
    lv_obj_t *about_row_2;
    lv_obj_t *obj19;
    lv_obj_t *obj20;
    lv_obj_t *about_row_3;
    lv_obj_t *obj21;
    lv_obj_t *obj22;
    lv_obj_t *about_row_4;
    lv_obj_t *obj23;
    lv_obj_t *obj24;
    lv_obj_t *about_row_5;
    lv_obj_t *obj25;
    lv_obj_t *obj26;
    lv_obj_t *about_row_6;
    lv_obj_t *obj27;
    lv_obj_t *obj28;
    lv_obj_t *about_row_7;
    lv_obj_t *obj29;
    lv_obj_t *obj30;
    lv_obj_t *diagnostic_tab;
    lv_obj_t *diag_row_1;
    lv_obj_t *obj31;
    lv_obj_t *lsat_label;
    lv_obj_t *settings_loading_screen;
    lv_obj_t *settings_loading_spinner;
    lv_obj_t *obj32;
    lv_obj_t *ble_passkey_container;
    lv_obj_t *obj33;
    lv_obj_t *ble_passkey_label;
    lv_obj_t *calendar_obj;
    lv_obj_t *call_notif_container;
    lv_obj_t *call_type_label;
    lv_obj_t *caller_name_label;
    lv_obj_t *phone_number_label;
    lv_obj_t *call_r_button_container;
    lv_obj_t *reject_call_button_1;
    lv_obj_t *obj34;
    lv_obj_t *call_ar_button_container;
    lv_obj_t *reject_call_button;
    lv_obj_t *obj35;
    lv_obj_t *accept_call_button;
    lv_obj_t *obj36;
} objects_t;

extern objects_t objects;

void create_screen_home_screen();
void tick_screen_home_screen();

void create_screen_weather_screen();
void tick_screen_weather_screen();

void create_screen_notification_screen();
void tick_screen_notification_screen();

void create_screen_settings_screen();
void tick_screen_settings_screen();

void create_screen_ble_passkey_screen();
void tick_screen_ble_passkey_screen();

void create_screen_calendar_screen();
void tick_screen_calendar_screen();

void create_screen_call_notification_screen();
void tick_screen_call_notification_screen();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/