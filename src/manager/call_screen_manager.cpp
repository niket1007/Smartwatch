#include "call_screen_manager.h"
#include "ble_manager.h"
#include "navigation_manager.h"

CallNotification call_notif = {
    "",
    "",
    ""
};

void update_call_notification_details(const String &type, const String &name, const String &number) {
    call_notif.type = type.equals("start") ? "In Call" : type;
    call_notif.name = name.isEmpty() ? "Unknown" : name;
    call_notif.number = number;

    if(name.equals(number)) {
        call_notif.name = "";
    }

    // Update the task state variable
    ts_var.show_call_notif_screen = type.equals("end") ? 2 : 1;

    // Notify GUI to get up
    if(task_gui_handle != NULL) xTaskNotifyGive(task_gui_handle);
}

void update_call_screen_ui() {

    if(gv.active_screen_id != CALL_NOTIF_SCREEN) {
        usb_serial.println("Load Call Notification screen");
        navigate_to_screen(CALL_NOTIF_SCREEN);
    }

    if(ts_var.show_call_notif_screen == 1) {
        if(objects.call_type_label) {
            call_notif.type.toUpperCase();
            String label = String(LV_SYMBOL_CALL) + " " + call_notif.type;
            lv_label_set_text(objects.call_type_label, label.c_str());
        }

        if(objects.caller_name_label) {
            lv_label_set_text(objects.caller_name_label, call_notif.name.c_str());
        }

        if(objects.phone_number_label) {
            lv_label_set_text(objects.phone_number_label, call_notif.number.c_str());
        }
        
        if(call_notif.type.equals("OUTGOING") || call_notif.type.equals("IN CALL")) {
            lv_obj_add_flag(objects.call_ar_button_container, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(objects.call_r_button_container, LV_OBJ_FLAG_HIDDEN);
        } 
        else if(call_notif.type.equals("INCOMING")) {
            lv_obj_add_flag(objects.call_r_button_container, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(objects.call_ar_button_container, LV_OBJ_FLAG_HIDDEN);
        }
    }
    else {
        ts_var.show_call_notif_screen = 0;

        lv_timer_create([](lv_timer_t *timer) {
            navigate_to_screen(HOME_SCREEN);    // Transition back to home
            lv_timer_delete(timer);   // Destroy this one-shot timer
        }, 1000, NULL);
    }      
}

void action_accept_call_clicked(lv_event_t * e) {
    ts_var.accept_call_signal_init = 1;
}

void send_accept_call_signal() {
    JsonDocument json_doc;
    json_doc["t"] = "call";
    json_doc["n"] = "ACCEPT";
    ble_send_to_phone(json_doc);
}

void action_reject_call_clicked(lv_event_t * e) {
    ts_var.reject_call_signal_init = 1;
}

void send_reject_call_signal() {
    JsonDocument json_doc;
    json_doc["t"] = "call";
    json_doc["n"] = "REJECT";
    ble_send_to_phone(json_doc);
}