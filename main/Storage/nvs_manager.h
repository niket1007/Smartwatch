#pragma once

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

class NVSManager
{
private:
    bool is_init_ = false;

public:
    esp_err_t init();
    esp_err_t deinit();

    bool is_initialised() const;
};