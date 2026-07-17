#pragma once

#include "globals.h"

extern void store_power_off_time();
extern String fetch_power_off_time();

extern void store_wifi_credentials(String ssid, String password);
extern void fetch_wifi_credentials();