#include "rtc_manager.h"

#include "esp_sntp.h"
#include "Common/globals.h"

static constexpr const char *TAG = "RTC_MANAGER";

uint8_t RTCManager::dec2bcd(uint8_t val)
{
    return ((val / 10) << 4) + (val % 10);
}

uint8_t RTCManager::bcd2dec(uint8_t val)
{
    return ((val >> 4) * 10) + (val & 0x0F);
}

esp_err_t RTCManager::init()
{
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.nist.gov");
    esp_sntp_setservername(2, "time.google.com");
    esp_sntp_init();

    int retry = 0;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < MAX_RETRIES)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, MAX_RETRIES);
        vTaskDelay(RETRY_DELAY);
    }

    if (retry < MAX_RETRIES)
    {
        // Set timezone to Indian Standard Time (UTC+5:30)
        setenv("TZ", "IST-5:30", 1);
        tzset();

        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);

        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "Time successfully synced! Current RTC time: %s", strftime_buf);

        if (sync_PCF85063_rtc(&timeinfo) == ESP_OK)
        {
            ESP_LOGI(TAG, "Successfully synced PCF85063 RTC");
        }
        else
        {
            ESP_LOGI(TAG, "Failed to sync PCF85063 rtc");
        }
        return ESP_OK;
    }
    else
    {
        ESP_LOGW(TAG, "Time sync failed. Proceeding with unsynced RTC.");
        return ESP_FAIL;
    }
}

esp_err_t RTCManager::sync_PCF85063_rtc(struct tm *tminfo)
{
    if (i2c_manager.rtc_dev_handle == NULL)
        return ESP_FAIL;

    uint8_t data[8];
    data[0] = 0x04;

    data[1] = dec2bcd(tminfo->tm_sec) & 0x7F;
    data[2] = dec2bcd(tminfo->tm_min);
    data[3] = dec2bcd(tminfo->tm_hour);
    data[4] = dec2bcd(tminfo->tm_mday);
    data[5] = dec2bcd(tminfo->tm_wday);
    data[6] = dec2bcd(tminfo->tm_mon + 1);
    data[7] = dec2bcd(tminfo->tm_year % 100);

    return i2c_master_transmit(i2c_manager.rtc_dev_handle, data, sizeof(data), -1);
}

esp_err_t RTCManager::get_PCF85063_rtc()
{
    if (i2c_manager.rtc_dev_handle == NULL)
        return ESP_FAIL;

    uint8_t reg_addr = 0x04;
    uint8_t data[7];
    struct tm timeinfo;

    esp_err_t err = i2c_master_transmit_receive(
        i2c_manager.rtc_dev_handle, &reg_addr, 1, data, sizeof(data), -1);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get time from PCF85063 rtc");
        return err;
    }

    timeinfo.tm_sec = bcd2dec(data[0] & 0x7F);
    timeinfo.tm_min = bcd2dec(data[1] & 0x7F);
    timeinfo.tm_hour = bcd2dec(data[2] & 0x3F);
    timeinfo.tm_mday = bcd2dec(data[3] & 0x3F);
    timeinfo.tm_wday = bcd2dec(data[4] & 0x07);
    // PCF85063 stores months as 1-12, tm_mon expects 0-11
    timeinfo.tm_mon = bcd2dec(data[5] & 0x1F) - 1;
    timeinfo.tm_year = bcd2dec(data[6]) + 100;

    // Set the internal system time
    time_t rtc_timestamp = mktime(&timeinfo);
    struct timeval tv;
    tv.tv_sec = rtc_timestamp;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);

    return ESP_OK;
}

esp_err_t RTCManager::deinit()
{
    esp_sntp_stop();
    return ESP_OK;
}