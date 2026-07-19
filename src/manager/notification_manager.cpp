#include "notification_manager.h"
#include "screen_navigation_manager.h"
#include "weather_manager.h"
#include "call_screen_manager.h"

MusicNotification ms_notif = {
    "Music",
    "",
    "-M -S"
};

NavNotificaton nav_notif = {
    ""
    "",
    "",
    ""
};

void handleNotification(String payload) {
    if (payload.startsWith("GB(") && payload.endsWith(")")) {
        payload = payload.substring(4, payload.length() - 1);
    }
    // usb_serial.printf("[RAW] %s\n", payload.c_str());

    JsonDocument json_doc;
    DeserializationError error = deserializeJson(json_doc, payload);
    if(error) {
        usb_serial.print(F("deserializeJson() failed: "));
        usb_serial.println(error.f_str());
        return;
    }

    if(!json_doc.isNull() ) {
        String type = json_doc["t"] | "";
        if(type == "musicinfo") {
            /*
                GadgetBridge Payload: 
                {"t":"musicinfo","artist":"Ali & Shjr & Haider Ali","album":"Heer","track":"Heer","dur":250,"c":-1,"n":-1}
            */
            int duration = json_doc["dur"] | 0;
            update_music_notification_details(
                String(json_doc["track"]),
                String(json_doc["artist"]),
                duration
            );
        }
        else if(type == "call") {
            /*
                GadgetBridge Payload
                {"t":"call","cmd":"incoming","name":"XXXX","number":"XXXXXXXXXX"}
                {"t":"call","cmd":"end","name":"XXXX","number":"XXXXXXXXXX"}
                {"t":"call","cmd":"outgoing","name":"XXXX","number":"XXXXXXXXXX"}
            */
            usb_serial.println("---- Call Notification ----");
            usb_serial.println("Type: " + String(json_doc["cmd"]));
            usb_serial.println("Name:   " + String(json_doc["name"]));
            usb_serial.println("Number:  " + String(json_doc["number"]));
            usb_serial.println("-----------------------");

            update_call_notification_details(
                String(json_doc["cmd"]),
                String(json_doc["name"]),
                String(json_doc["number"])
            );
        }
        else if(type == "weather") {
            /*
                GadgetBridge Payload
                {"t":"weather","v":1,"temp":310,"hi":311,"lo":302,"hum":39,
                "rain":20,"uv":4,"code":803,"txt":"Partly cloudy",
                "wind":13.68,"wdir":295,"loc":"Gurgaon, Sushant Lok 2"}
            */

            // temp provided is in Kelvin and C = K - 273.15
            float temp = json_doc["temp"].as<float>() - 273.15f;
            float hi = json_doc["hi"].as<float>() - 273.15f;
            float low = json_doc["lo"].as<float>() - 273.15f;
            float wind = json_doc["wind"].as<float>();
            update_weather_details(
                String(temp, 2),
                String(hi, 2),
                String(low, 2),
                String(json_doc["hum"]),
                String(json_doc["rain"]),
                String(wind, 2),
                String(json_doc["txt"]),
                String(json_doc["loc"])
            );
            
        }
        else if(type == "nav") {
            /*
                GadgetBridge Payload
                {"t":"nav","instr":"Head southeast",
                "distance":"0�m","action":"continue","eta":"10:41\u202fpm"}
                {"t":"nav"} -> When navigation is ended
            */

            if(json_doc.containsKey("instr")) {
                update_nav_notification_details(
                    String(json_doc["instr"]),
                    String(json_doc["distance"]),
                    String(json_doc["action"]),
                    String(json_doc["eta"])
                );
            } 
            else {
                update_nav_notification_details(
                    "",
                    "--",
                    "Navigation",
                    "--"
                );
            }
        }
    }

}



// ==========================================================
// Music Tab related changes
// ==========================================================
void update_music_notification_details(const String &track_name, const String &artists, int duration) {
    ms_notif.artists = artists;
    ms_notif.track_name = track_name;
    ms_notif.duration = String(duration / 60) + "M " + String(duration % 60) + "S";

    ts_var.is_music_event_received = true;
}

void update_music_tab_ui() {
    if(objects.track_name_label) {
        lv_label_set_text(objects.track_name_label, ms_notif.track_name.c_str());
    }

    if(objects.artist_label) {
        lv_label_set_text(objects.artist_label, ms_notif.artists.c_str());
    }
    
    if(objects.duration_label) {
        lv_label_set_text(objects.duration_label, ms_notif.duration.c_str());
    }

    ts_var.is_music_event_received = false;
}


// ==========================================================
// Navigation Tab related changes
// ==========================================================
void update_nav_notification_details(
    const String &instr, const String &distance, const String &action, const String &eta) {
    
    nav_notif.instr = instr;
    nav_notif.distance = distance;
    nav_notif.action = action;
    nav_notif.eta = eta;

    ts_var.is_nav_event_received = true;
}


void update_navigation_tab_ui() {
    if(objects.eta_datetime_label) {
        lv_label_set_text(objects.eta_datetime_label, nav_notif.eta.c_str());
    }

    if(objects.dir_label) {
        lv_label_set_text(objects.dir_label, nav_notif.action.c_str());
    }

    ts_var.is_nav_event_received = false;
}