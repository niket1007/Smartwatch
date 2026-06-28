#include "rtc_datetime_manager.h"

SensorPCF85063 rtc;

// Timezone Settings (UTC + 5:30 for India = 19800 seconds)
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;
const char* ntp_server = "pool.ntp.org";
const char* backup_ntp_server = "time.nist.gov";


void rtc_init() {
  if (!rtc.begin(Wire, IIC_SDA, IIC_SCL)) {
    USBSerial.println("RTC failed!");
  } else {
    USBSerial.println("RTC OK!");
  }
}

// Global references for day-of-week calculation from `tm` struct
int getDayOfWeek(int year, int month, int day) {
  struct tm t = {0};
  t.tm_year = year - 1900;
  t.tm_mon = month - 1;
  t.tm_mday = day;
  mktime(&t);
  return t.tm_wday; // 0 = Sunday, 1 = Monday, ..., 6 = Saturday
}

void fetch_and_sync_time() {
  bool is_connected = connect_to_wifi();

  if (is_connected) {
    USBSerial.println("\nWi-Fi Connected!");
    configTime(gmtOffset_sec, daylightOffset_sec, ntp_server, backup_ntp_server);
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 15000)) {
      USBSerial.println("Failed to obtain time from NTP server.");
    } else {
      USBSerial.println("NTP Time Fetched Successfully!");
      rtc.setDateTime(timeinfo.tm_year + 1900, 
                      timeinfo.tm_mon + 1, 
                      timeinfo.tm_mday, 
                      timeinfo.tm_hour, 
                      timeinfo.tm_min, 
                      timeinfo.tm_sec);
      USBSerial.println("Hardware RTC Updated!");
    }
    disconnect_wifi();
  } else {
    USBSerial.println("\nWi-Fi Connection Failed. Relying on existing RTC time.");
  }
}

void update_datetime_ui() {
  RTC_DateTime datetime;
  if (xSemaphoreTake(i2c_mutex, portMAX_DELAY)) {
    datetime = rtc.getDateTime(); 
  
    // Format Day string
    const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    int day_of_week = getDayOfWeek(datetime.getYear(), datetime.getMonth(), datetime.getDay());
    lv_label_set_text(objects.day_label, days[day_of_week]);

    // Format Date string (12 - Mar - 2026)
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char date_str[30];
    snprintf(date_str, sizeof(date_str), "%02d - %s - %04d", datetime.getDay(), months[datetime.getMonth() - 1], datetime.getYear());
    lv_label_set_text(objects.date_label, date_str);

    // Format Time (12-hour format) & AM/PM determination
    int display_hour = datetime.getHour();
    char am_pm_str[3];
    
    if (display_hour >= 12) {
      strcpy(am_pm_str, "PM");
      if (display_hour > 12) {
        display_hour -= 12;
      }
    } else {
      strcpy(am_pm_str, "AM");
      if (display_hour == 0) {
        display_hour = 12;
      }
    }

    char time_str[10];
    snprintf(time_str, sizeof(time_str), "%02d : %02d", display_hour, datetime.getMinute()); // Format 02: 24
    
    lv_label_set_text(objects.time_label, time_str);
    lv_label_set_text(objects.am_pm_label, am_pm_str);

    xSemaphoreGive(i2c_mutex); // Always give the lock back!
  }
}