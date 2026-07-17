#include "notification_manager.h"
#include "navigation_manager.h"

MusicNotification ms_notif = {
    "Music",
    "",
    "-M -S"
};

void update_music_notification_details(const String &track_name, const String &artists, int duration) {
    ms_notif.artists = artists;
    ms_notif.track_name = track_name;
    ms_notif.duration = String(duration / 60) + "M " + String(duration % 60) + "S";
}

void update_music_tab_ui() {
    if(objects.track_name_label) {
        ms_notif.track_name.toUpperCase();
        lv_label_set_text(objects.track_name_label, ms_notif.track_name.c_str());
    }

    if(objects.artist_label) {
        ms_notif.artists.toUpperCase();
        lv_label_set_text(objects.artist_label, ms_notif.artists.c_str());
    }
    
    if(objects.duration_label) {
        lv_label_set_text(objects.duration_label, ms_notif.duration.c_str());
    }
}

