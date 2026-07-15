#include "settings_screen_manager.h"


void update_shutdow_approx_label() {
    if(objects.lsat_label) {
        String datetime = fetch_power_off_time();
        if(datetime.isEmpty()) {
            lv_label_set_text(objects.lsat_label, "Not Computed");
            return;
        }
        lv_label_set_text(objects.lsat_label, datetime.c_str());
    }
}