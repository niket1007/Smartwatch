#include "rtc_datetime_manager.h"

SensorPCF85063 rtc;
RTC_DateTime current_datetime;

int cal_year = 0;
int cal_month = 0;
int cal_day = 0;

// Timezone Settings (UTC + 5:30 for India = 19800 seconds)
const long  GMT_OFFSET_SEC = 19800;
const int   DAYLIGHT_OFFSET_SEC = 0;
const char* NTP_SERVER = "pool.ntp.org";
const char* BACKUP_NTP_SERVER = "time.nist.gov";


void rtc_init() {
  if (!rtc.begin(Wire, IIC_SDA, IIC_SCL)) {
    usb_serial.println("RTC failed!");
  } else {
    usb_serial.println("RTC OK!");
  }
}

// Global references for day-of-week calculation from `tm` struct
int get_day_of_week(int year, int month, int day) {
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
    usb_serial.println("\nWi-Fi Connected!");
    
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER, BACKUP_NTP_SERVER);
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 15000)) {
      usb_serial.println("Failed to obtain time from NTP server.");
    } else {
      usb_serial.println("NTP Time Fetched Successfully!");
      if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(1000))) {
        rtc.setDateTime(timeinfo.tm_year + 1900, 
                        timeinfo.tm_mon + 1, 
                        timeinfo.tm_mday, 
                        timeinfo.tm_hour, 
                        timeinfo.tm_min, 
                        timeinfo.tm_sec);
        xSemaphoreGive(i2c_mutex);
        usb_serial.println("Hardware RTC Updated!");
      }
    }
  } else {
    usb_serial.println("\nWi-Fi Connection Failed. Relying on existing RTC time.");
  }
  disconnect_wifi();
}

bool load_current_datetime() {
    // If already acquired then wait for 50ms, then also acquired then
    if (!xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(50))) {
        return false;
    }

    current_datetime = rtc.getDateTime();

    xSemaphoreGive(i2c_mutex);

    return true;
}

void update_datetime_ui()
{
    if(!load_current_datetime()) return;

    // Validate month
    int month = current_datetime.getMonth();
    if (month < 1 || month > 12) {
        month = 1;
    }

    // Validate day of week
    
    uint8_t day_of_week = current_datetime.getWeek();
    
    get_day_of_week(
        current_datetime.getYear(),
        current_datetime.getMonth(),
        current_datetime.getDay());

    if (day_of_week < 0 || day_of_week > 6) {
        day_of_week = 0;
    }

    static const char *days[] = {
        "Sunday","Monday","Tuesday","Wednesday",
        "Thursday","Friday","Saturday"
    };

    static const char *months[] = {
        "Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"
    };

    int day = current_datetime.getDay();
    int year = current_datetime.getYear();

    char date_str[30];
    snprintf(
        date_str,
        sizeof(date_str),
        "%02d - %s - %04d",
        day,
        months[month - 1],
        year
    );

    int hour = current_datetime.getHour();
    const char *ampm = "AM";

    if (hour >= 12) {
        ampm = "PM";
        if (hour > 12) hour -= 12;
    } else if (hour == 0) {
        hour = 12;
    }

    char time_str[10];
    snprintf(
        time_str,
        sizeof(time_str),
        "%02d : %02d",
        hour,
        current_datetime.getMinute()
    );

    if (objects.day_label)
        lv_label_set_text(objects.day_label, days[day_of_week]);

    if (objects.date_label)
        lv_label_set_text(objects.date_label, date_str);

    if (objects.time_label)
        lv_label_set_text(objects.time_label, time_str);

    if (objects.am_pm_label)
        lv_label_set_text(objects.am_pm_label, ampm);
    
    if(objects.calendar_obj) {
        if(cal_year != year || cal_day != day || cal_month != month) {
            // usb_serial.println("Updated the calendar data");
            cal_year = year;
            cal_month = month;
            cal_day = day;
            lv_calendar_set_today_date(objects.calendar_obj, cal_year, cal_month, cal_day);
            lv_calendar_set_month_shown(objects.calendar_obj, cal_year, cal_month);
        }
    }
}