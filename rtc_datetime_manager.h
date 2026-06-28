#pragma once

#include "globals.h"
#include "wifi_manager.h"
#include <time.h>
#include <SensorPCF85063.hpp>

extern void rtc_init();
extern int getDayOfWeek(int year, int month, int day);
extern void fetch_and_sync_time();
extern void update_datetime_ui();