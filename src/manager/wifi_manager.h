#pragma once

#include "globals.h"
#include <WiFi.h>

extern bool connect_to_wifi();
extern void disconnect_wifi();
extern void scan_and_save_nearby_wifi();
extern void update_wifi_dropdown();
extern void update_wifi_settings_details();
