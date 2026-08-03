#include "screen_manager.h"
#include "Common/constants.h"

static constexpr const char *TAG = "SCREEN_MANAGER";

esp_err_t ScreenManager::init()
{
    ESP_LOGI(TAG, "init called");
    if(current_screen == nullptr)
        current_screen = new HomeScreen();
    return ESP_OK;
}

esp_err_t ScreenManager::draw()
{
    ESP_LOGI(TAG, "draw called");
    if(current_screen != nullptr)
        return current_screen->draw();
    return ESP_OK;
}

esp_err_t ScreenManager::change_screen(Screen* new_screen)
{
    if(new_screen == nullptr)
    {
        ESP_LOGE(TAG, "Invalid new screen");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_RETURN_ON_ERROR(
        current_screen->on_exit(), TAG, "Failed to exit old screen");
    
    // vTaskDelay(pdMS_TO_TICKS(50));
    current_screen = new_screen;
    
    ESP_RETURN_ON_ERROR(
        current_screen->on_enter(), TAG, "Failed to enter new screen");
    return ESP_OK;
}

esp_err_t ScreenManager::handle_events(uint32_t events)
{
    ESP_LOGI(TAG, "handle events called");
    if(events & SWIPE_RIGHT_EVENT)
    {
        ESP_LOGI(TAG, "Swipe right");
        ESP_RETURN_ON_ERROR(
            change_screen(new HomeScreen()), TAG, "Failed to switch to previous screen");
        return ESP_OK;
    }

    if(events & SWIPE_LEFT_EVENT)
    {
        ESP_LOGI(TAG, "Swipe left");
        ESP_RETURN_ON_ERROR(
            change_screen(new MenuScreenPage1()), TAG, "Failed to switch to next screen");
        return ESP_OK;
    }

    if(current_screen!= nullptr)
        return current_screen->handle_events(events);
    return ESP_OK;
}