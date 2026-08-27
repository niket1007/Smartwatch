#include "rtc_manager.h"

#include "esp_sntp.h"
#include "esp_netif_sntp.h"
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
    setenv("TZ", "IST-5:30", 1);
    tzset();

    esp_sntp_config_t config =
        ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");

    config.start = false;
    config.wait_for_sync = true;
    config.smooth_sync = false;

    esp_err_t err = esp_netif_sntp_init(&config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize SNTP: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_netif_sntp_start();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start SNTP: %s", esp_err_to_name(err));
        esp_netif_sntp_deinit();
        return err;
    }

    ESP_LOGI(TAG, "SNTP started. Waiting for synchronization...");

    err = esp_netif_sntp_sync_wait(SNTP_SYNC_TIMEOUT);
    if (err != ESP_OK)
    {
        if (err == ESP_ERR_NOT_FINISHED)
        {
            ESP_LOGW(
                TAG,
                "SNTP synchronization is still in progress after timeout.");
        }
        else if (err == ESP_ERR_TIMEOUT)
        {
            ESP_LOGW(
                TAG,
                "SNTP synchronization timed out after %lu seconds.",
                (unsigned long)(SNTP_SYNC_TIMEOUT / configTICK_RATE_HZ));
        }
        else
        {
            ESP_LOGE(
                TAG,
                "SNTP synchronization failed: %s",
                esp_err_to_name(err));
        }

        esp_netif_sntp_deinit();
        return ESP_FAIL;
    }

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "Time successfully synced! Current RTC time: %s", strftime_buf);

    err = sync_PCF85063_rtc(&timeinfo);
    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to sync PCF85063 RTC: %s",
            esp_err_to_name(err));

        esp_netif_sntp_deinit();
        return err;
    }

    err = sync_PCF85063_rtc(&timeinfo);
    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to sync PCF85063 RTC: %s", esp_err_to_name(err));

        esp_netif_sntp_deinit();
        return err;
    }

    ESP_LOGI(TAG, "Successfully synced PCF85063 RTC");

    esp_netif_sntp_deinit();

    return ESP_OK;
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