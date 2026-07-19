#pragma once

#include "globals.h"

struct MusicNotification {
    String track_name;
    String artists;
    String duration;
};

struct NavNotificaton {
    String instr;
    String distance;
    String action;
    String eta;
};

void handleNotification(String payload);

void update_music_notification_details(
    const String &track_name, const String &artists, int duration);
void update_music_tab_ui();

void update_nav_notification_details(
    const String &instr, const String &distance, const String &action, const String &eta);
void update_navigation_tab_ui();
