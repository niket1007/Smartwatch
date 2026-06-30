#include "rtc_datetime_manager.h"

SensorPCF85063 rtc;

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
      rtc.setDateTime(timeinfo.tm_year + 1900, 
                      timeinfo.tm_mon + 1, 
                      timeinfo.tm_mday, 
                      timeinfo.tm_hour, 
                      timeinfo.tm_min, 
                      timeinfo.tm_sec);
      usb_serial.println("Hardware RTC Updated!");
    }
    disconnect_wifi();
  } else {
    usb_serial.println("\nWi-Fi Connection Failed. Relying on existing RTC time.");
  }
}

void update_datetime_ui()
{
    if (!xSemaphoreTake(i2c_mutex, portMAX_DELAY)) {
        return;
    }

    RTC_DateTime datetime = rtc.getDateTime();

    xSemaphoreGive(i2c_mutex);

    // Validate month
    int month = datetime.getMonth();
    if (month < 1 || month > 12) {
        month = 1;
    }

    // Validate day of week
    int day_of_week = get_day_of_week(
        datetime.getYear(),
        datetime.getMonth(),
        datetime.getDay());

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

    char date_str[30];
    snprintf(
        date_str,
        sizeof(date_str),
        "%02d - %s - %04d",
        datetime.getDay(),
        months[month - 1],
        datetime.getYear()
    );

    int hour = datetime.getHour();
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
        datetime.getMinute()
    );

    if (objects.day_label)
        lv_label_set_text(objects.day_label, days[day_of_week]);

    if (objects.date_label)
        lv_label_set_text(objects.date_label, date_str);

    if (objects.time_label)
        lv_label_set_text(objects.time_label, time_str);

    if (objects.am_pm_label)
        lv_label_set_text(objects.am_pm_label, ampm);
}