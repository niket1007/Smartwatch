#include "rtc_manager.h"
#include "Common/globals.h"

static const char* TAG = "BLUETOOTH_MANAGER";

esp_err_t RTCManager::init() 
{
  if (!rtc.begin(Wire, IIC_SDA, IIC_SCL)) 
  {
    errored = true;
    return ESP_FAIL;
  } 

  errored = false;
  return ESP_OK;
}

esp_err_t RTCManager::sync() 
{
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER, BACKUP_NTP_SERVER);

    struct tm timeinfo;

    if (!getLocalTime(&timeinfo, 15000))
    {
        errored = true;
        usb_serial.println("Failed to obtain time from NTP server.");
        return ESP_FAIL;
    } 
    else 
    {
        usb_serial.println("NTP Time Fetched Successfully!");
        rtc.setDateTime(timeinfo.tm_year + 1900, 
                        timeinfo.tm_mon + 1, 
                        timeinfo.tm_mday, 
                        timeinfo.tm_hour, 
                        timeinfo.tm_min, 
                        timeinfo.tm_sec);
        // xSemaphoreGive(i2c_mutex);
        usb_serial.println("RTC updated with latest time");

    }

    errored = false;
    return ESP_OK;
}

RTC_DateTime RTCManager::get_current_datetime() {
    return rtc.getDateTime();
}