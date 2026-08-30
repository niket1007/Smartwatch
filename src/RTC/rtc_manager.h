#pragma once

#include "esp_err.h"
#include <time.h>
#include <SensorPCF85063.hpp>

class RTCManager
{
    private:
        SensorPCF85063 rtc;
        RTC_DateTime current_datetime;

        const long  GMT_OFFSET_SEC = 19800;
        const int   DAYLIGHT_OFFSET_SEC = 0;
        const char* NTP_SERVER = "pool.ntp.org";
        const char* BACKUP_NTP_SERVER = "time.nist.gov";

        int cal_year = 0;
        int cal_month = 0;
        int cal_day = 0;


    public:
        bool errored;
        
        esp_err_t init();
        esp_err_t sync();

        RTC_DateTime get_current_datetime();
};