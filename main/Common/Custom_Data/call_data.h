#pragma once

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct CallData
{
private:
    std::string name;
    std::string phone_number;
    std::string status; // accept/incoming/outgoing/reject/start/end

    SemaphoreHandle_t call_mutex_ = nullptr;

public:
    CallData()
    {
        call_mutex_ = xSemaphoreCreateMutex();

        name = "Phone";
        phone_number = "+911234567890";
        status = "incoming";
    }

    std::string get_contact_name() const
    {
        xSemaphoreTake(call_mutex_, portMAX_DELAY);
        std::string cn = name;
        xSemaphoreGive(call_mutex_);
        return cn;
    }

    std::string get_contact_number() const
    {
        xSemaphoreTake(call_mutex_, portMAX_DELAY);
        std::string cn = phone_number;
        xSemaphoreGive(call_mutex_);
        return cn;
    }

    std::string get_contact_status() const
    {
        xSemaphoreTake(call_mutex_, portMAX_DELAY);
        std::string cs = status;
        xSemaphoreGive(call_mutex_);
        return cs;
    }

    void update(
        std::string contact_name,
        std::string contact_number,
        std::string cmd)
    {
        xSemaphoreTake(call_mutex_, portMAX_DELAY);
        
        name = contact_name;
        phone_number = contact_number;
        status = cmd;

        xSemaphoreGive(call_mutex_);
    }
};

extern CallData call_data;
