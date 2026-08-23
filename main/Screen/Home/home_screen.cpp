#include "Screen/Home/home_screen.h"

#include "esp_log.h"
#include "Screen/generated/ui.h"

static constexpr const char *TAG = "HOME_SCREEN";


esp_err_t HomeScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");
    return ESP_OK;
}


esp_err_t HomeScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");

    return ESP_OK;
}


esp_err_t HomeScreen::handle_events(uint32_t events)
{
    (void)events;

    return ESP_OK;
}