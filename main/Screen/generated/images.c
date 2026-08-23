#include "images.h"

const ext_img_desc_t images[18] = {
    { "battery_charging", &img_battery_charging },
    { "info", &img_info },
    { "notification", &img_notification },
    { "weather", &img_weather },
    { "calendar", &img_calendar },
    { "navigation", &img_navigation },
    { "alarm", &img_alarm },
    { "music", &img_music },
    { "settings", &img_settings },
    { "passkey", &img_passkey },
    { "phone_default", &img_phone_default },
    { "phone_reject", &img_phone_reject },
    { "phone_in_progress", &img_phone_in_progress },
    { "phone_incoming", &img_phone_incoming },
    { "phone_outgoing", &img_phone_outgoing },
    { "phone_reject_action", &img_phone_reject_action },
    { "phone_accept", &img_phone_accept },
    { "bluetooth_connected", &img_bluetooth_connected },
};