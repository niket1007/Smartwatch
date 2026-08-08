#pragma once

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <time.h>
#include <sys/time.h>

class RTCManager
{
private:
    static constexpr int MAX_RETRIES = 15;
    static constexpr TickType_t RETRY_DELAY = pdMS_TO_TICKS(2000);

    uint8_t dec2bcd(uint8_t val);
    uint8_t bcd2dec(uint8_t val);

public:
    esp_err_t init();
    esp_err_t sync_PCF85063_rtc(struct tm *tminfo);
    esp_err_t get_PCF85063_rtc();
    esp_err_t deinit();
};