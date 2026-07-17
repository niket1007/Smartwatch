#pragma once

#include "globals.h"

struct MusicNotification {
    String track_name;
    String artists;
    String duration;
};

void update_music_notification_details(const String &track_name, const String &artists, int duration);
void update_music_tab_ui();

