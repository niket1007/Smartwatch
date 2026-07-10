#pragma once

#include "globals.h"
#include <WiFi.h>

extern void get_wifi_credentials();
extern bool connect_to_wifi();
extern void disconnect_wifi();
extern void scan_and_save_nearby_wifi();
extern void update_wifi_dropdown();
