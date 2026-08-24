#include "call_screen.h"
#include "Screen/generated/ui.h"
#include "Screen/generated/images.h"
#include "Screen/generated/actions.h"
#include "Common/globals.h"
#include "Common/Custom_Data/Call/call_data.h"

#include "esp_log.h"

static constexpr const char *TAG = "CALL_SCREEN";

esp_err_t CallScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

    display_manager.set_screen_timeout_enabled(false);
    if (display_manager.is_sleeping())
    {
        display_manager.wake();
        vTaskDelay(10);
    }

    lv_label_set_text(
        objects.caller_name, call_data.get_contact_name().c_str());

    lv_label_set_text(
        objects.phone_number, call_data.get_contact_number().c_str());

    ESP_RETURN_ON_ERROR(
        update_icons(call_data.get_contact_status()), TAG, "Failed to update call screen icons");

    return ESP_OK;
}

esp_err_t CallScreen::update_icons(std::string status)
{
    if (status == "incoming")
    {
        lv_image_set_src(objects.phone_status_icon, &img_phone_incoming);

        lv_obj_remove_flag(objects.accept_call_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(objects.accept_call_icon, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_remove_flag(objects.reject_call_icon_one, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(objects.reject_call_icon_one, LV_OBJ_FLAG_CLICKABLE);
        
        lv_obj_add_flag(objects.reject_call_icon_two, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(objects.reject_call_icon_two, LV_OBJ_FLAG_CLICKABLE);
    }
    else if (status == "outgoing" or status == "accept" or status == "start")
    {
        lv_image_set_src(
            objects.phone_status_icon,
            status == "outgoing" ? &img_phone_outgoing : &img_phone_in_progress);
        
        lv_obj_add_flag(objects.accept_call_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(objects.accept_call_icon, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_add_flag(objects.reject_call_icon_one, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(objects.reject_call_icon_one, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_remove_flag(objects.reject_call_icon_two, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(objects.reject_call_icon_two, LV_OBJ_FLAG_CLICKABLE);
    }
    else if (status == "reject" or status == "end")
    {
        lv_image_set_src(objects.phone_status_icon, &img_phone_reject);
        
        lv_obj_add_flag(objects.accept_call_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(objects.accept_call_icon, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_add_flag(objects.reject_call_icon_one, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(objects.reject_call_icon_one, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_add_flag(objects.reject_call_icon_two, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(objects.reject_call_icon_two, LV_OBJ_FLAG_CLICKABLE);
    }
    else
    {
        lv_image_set_src(objects.phone_status_icon, &img_phone_default);

        lv_obj_add_flag(objects.accept_call_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(objects.accept_call_icon, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_add_flag(objects.reject_call_icon_one, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(objects.reject_call_icon_one, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_add_flag(objects.reject_call_icon_two, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(objects.reject_call_icon_two, LV_OBJ_FLAG_CLICKABLE);
    }
    return ESP_OK;
}

esp_err_t CallScreen::update_screen()
{
    ESP_LOGI(TAG, "update_screen called");

    ESP_LOGI(TAG, "is name dirty %d", call_data.is_call_name_dirty());
    if (call_data.is_call_name_dirty())
    {
        lv_label_set_text(
            objects.caller_name, call_data.get_contact_name().c_str());
    }
    ESP_LOGI(TAG, "is number dirty %d", call_data.is_call_number_dirty());
    if (call_data.is_call_number_dirty())
    {
        lv_label_set_text(
            objects.phone_number, call_data.get_contact_number().c_str());
    }
    ESP_LOGI(TAG, "is status dirty %d", call_data.is_call_status_dirty());
    if (call_data.is_call_status_dirty())
    {
        ESP_RETURN_ON_ERROR(
            update_icons(call_data.get_contact_status()),
            TAG, "Failed to update call screen icons");
    }

    return ESP_OK;
}

esp_err_t CallScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");

    display_manager.set_screen_timeout_enabled(true);

    return ESP_OK;
}

esp_err_t CallScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Home Screen
        return screen_manager.change_screen(SCREEN_ID_HOME);
    }

    if (events & CALL_SCREEN_EVENT)
    {
        return update_screen();
    }

    return ESP_OK;
}

void action_call_click_func(lv_event_t *e)
{
    
    int event = (int)(intptr_t)lv_event_get_user_data(e);
    uint32_t notify_event = 0;

    ESP_LOGI(TAG, "Click event called %d", event);

    if(event == 1)
    {
        notify_event = ACCEPT_CALL_EVENT;
    }
    else if(event == 2)
    {
        notify_event = REJECT_CALL_EVENT;
    }
    else 
    {
        return;
    }

    if(background_task_handle != nullptr)
    {
        xTaskNotify(background_task_handle, notify_event, eSetBits);
    }
}