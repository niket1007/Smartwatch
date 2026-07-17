#pragma once

#include "globals.h"
#include "wifi_manager.h"
#include <time.h>
#include <SensorPCF85063.hpp>

extern RTC_DateTime current_datetime;

extern bool load_current_datetime();
extern void rtc_init();
extern void fetch_and_sync_time();
extern void update_datetime_ui();