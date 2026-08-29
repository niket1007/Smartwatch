#include "rtc_manager.h"
#include "Common/globals.h"

static const char* TAG = "BLUETOOTH_MANAGER";

esp_err_t RTCManager::init() 
{
  if (!rtc.begin(Wire, IIC_SDA, IIC_SCL)) 
  {
    return ESP_FAIL;
  } 
  return ESP_OK;
}

esp_err_t RTCManager::sync() 
{
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER, BACKUP_NTP_SERVER);

    struct tm timeinfo;

    if (!getLocalTime(&timeinfo, 15000))
    {
        usb_serial.println("Failed to obtain time from NTP server.");
        return ESP_FAIL;
    } 
    else 
    {
        usb_serial.println("NTP Time Fetched Successfully!");
    //   if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(1000))) 
    //   {
        rtc.setDateTime(timeinfo.tm_year + 1900, 
                        timeinfo.tm_mon + 1, 
                        timeinfo.tm_mday, 
                        timeinfo.tm_hour, 
                        timeinfo.tm_min, 
                        timeinfo.tm_sec);
        // xSemaphoreGive(i2c_mutex);
        usb_serial.println("RTC updated with latest time");
    //   }
    }

    return ESP_OK;
//   return disconnect_wifi();
}

RTC_DateTime RTCManager::get_current_datetime() {
    // If already acquired then wait for 50ms, then also acquired then
    // if (!xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(50))) {
    //     return false;
    // }

    return rtc.getDateTime();

    // xSemaphoreGive(i2c_mutex);
}

// void update_datetime_ui()
// {
//     if(!load_current_datetime()) return;

//     // Validate month
//     int month = current_datetime.getMonth();
//     if (month < 1 || month > 12) {
//         month = 1;
//     }

//     // Validate day of week
    
//     uint8_t day_of_week = current_datetime.getWeek();

//     if (day_of_week < 0 || day_of_week > 6) {
//         day_of_week = 0;
//     }

//     static const char *days[] = {
//         "Sunday","Monday","Tuesday","Wednesday",
//         "Thursday","Friday","Saturday"
//     };

//     static const char *months[] = {
//         "Jan","Feb","Mar","Apr","May","Jun",
//         "Jul","Aug","Sep","Oct","Nov","Dec"
//     };

//     int day = current_datetime.getDay();
//     int year = current_datetime.getYear();

//     char date_str[30];
//     snprintf(
//         date_str,
//         sizeof(date_str),
//         "%02d - %s - %04d",
//         day,
//         months[month - 1],
//         year
//     );

//     int hour = current_datetime.getHour();
//     const char *ampm = "AM";

//     if (hour >= 12) {
//         ampm = "PM";
//         if (hour > 12) hour -= 12;
//     } else if (hour == 0) {
//         hour = 12;
//     }

//     char time_str[10];
//     snprintf(
//         time_str,
//         sizeof(time_str),
//         "%02d : %02d",
//         hour,
//         current_datetime.getMinute()
//     );

//     if (objects.day_label)
//         lv_label_set_text(objects.day_label, days[day_of_week]);

//     if (objects.date_label)
//         lv_label_set_text(objects.date_label, date_str);

//     if (objects.time_label)
//         lv_label_set_text(objects.time_label, time_str);

//     if (objects.am_pm_label)
//         lv_label_set_text(objects.am_pm_label, ampm);
    
//     if(objects.calendar_obj) {
//         if(cal_year != year || cal_day != day || cal_month != month) {
//             // usb_serial.println("Updated the calendar data");
//             cal_year = year;
//             cal_month = month;
//             cal_day = day;
//             lv_calendar_set_today_date(objects.calendar_obj, cal_year, cal_month, cal_day);
//             lv_calendar_set_month_shown(objects.calendar_obj, cal_year, cal_month);
//         }
//     }
// }