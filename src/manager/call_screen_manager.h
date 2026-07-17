#pragma once

#include "globals.h"

struct CallNotification {
    String type;
    String name;
    String number;
};

void update_call_notification_details(const String &type, const String &name, const String &number);
void update_call_screen_ui();

void send_accept_call_signal();
void send_reject_call_signal();