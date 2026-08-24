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

    bool is_name_dirty;
    bool is_number_dirty;
    bool is_status_dirty;

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

    bool is_call_name_dirty() const
    {
        xSemaphoreTake(call_mutex_, portMAX_DELAY);
        bool flag = is_name_dirty;
        xSemaphoreGive(call_mutex_);
        return flag;
    }

    std::string get_contact_number() const
    {
        xSemaphoreTake(call_mutex_, portMAX_DELAY);
        std::string cn = phone_number;
        xSemaphoreGive(call_mutex_);
        return cn;
    }

    bool is_call_number_dirty() const
    {
        xSemaphoreTake(call_mutex_, portMAX_DELAY);
        bool flag = is_number_dirty;
        xSemaphoreGive(call_mutex_);
        return flag;
    }

    std::string get_contact_status() const
    {
        xSemaphoreTake(call_mutex_, portMAX_DELAY);
        std::string cs = status;
        xSemaphoreGive(call_mutex_);
        return cs;
    }

    bool is_call_status_dirty() const
    {
        xSemaphoreTake(call_mutex_, portMAX_DELAY);
        bool flag = is_status_dirty;
        xSemaphoreGive(call_mutex_);
        return flag;
    }

    void update(
        std::string contact_name,
        std::string contact_number,
        std::string cmd)
    {
        xSemaphoreTake(call_mutex_, portMAX_DELAY);
        
        is_name_dirty = name != contact_name;
        name = contact_name;

        is_number_dirty = phone_number != contact_number;
        phone_number = contact_number;

        is_status_dirty = status != cmd;
        status = cmd;

        xSemaphoreGive(call_mutex_);
    }
};

extern CallData call_data;
