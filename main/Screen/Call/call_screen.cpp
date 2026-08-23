#include "call_screen.h"
#include "Screen/generated/ui.h"
#include "Common/globals.h"

#include "esp_log.h"

static constexpr const char *TAG = "CALL_SCREEN";

esp_err_t CallScreen::on_enter() 
{
    ESP_LOGI(TAG, "on_enter called");
    return ESP_OK;
}

esp_err_t CallScreen::on_exit() 
{
    ESP_LOGI(TAG, "on_exit called");
    return ESP_OK;
}

esp_err_t CallScreen::update_screen()
{
    return ESP_OK;
}

esp_err_t CallScreen::handle_events(uint32_t events) 
{
    if(events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Home Screen
        return screen_manager.change_screen(SCREEN_ID_HOME);
    }

    if(events & CALL_SCREEN_EVENT)
    {
        return update_screen();
    }
    
    return ESP_OK;
}