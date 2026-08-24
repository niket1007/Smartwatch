#include "weather_screen.h"
#include "Screen/generated/ui.h"
#include "Screen/generated/actions.h"
#include "Common/globals.h"
#include "Common/Custom_Data/Weather/weather_data.h"

#include "esp_log.h"
#include <format>

static constexpr const char *TAG = "WEATHER_SCREEN";

esp_err_t WeatherScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

    lv_obj_add_flag(objects.wrefresh_label, LV_OBJ_FLAG_CLICKABLE);
    
    std::string temp = std::format("{:.2f}", weather_data.get_temp()) + "°C";
    lv_label_set_text(objects.temp_label, temp.c_str());

    std::string high = std::format("{:.2f}", weather_data.get_high()) + "°C";
    lv_label_set_text(objects.high_temp_label, high.c_str());

    std::string low = std::format("{:.2f}", weather_data.get_low()) + "°C";
    lv_label_set_text(objects.low_temp_label, low.c_str());

    std::string humidity = std::format("{:.2f}", weather_data.get_humidity()) + "%";
    lv_label_set_text(objects.humidity_label, humidity.c_str());

    std::string wind = std::format("{:.2f}", weather_data.get_wind_speed()) + "\nkm/h";
    lv_label_set_text(objects.wind_label, wind.c_str());

    std::string rain = std::format("{:.1f}", weather_data.get_rain_percentage()) + "%";
    lv_label_set_text(objects.rain_chance_label, rain.c_str());

    lv_label_set_text(
        objects.weather_type_label, weather_data.get_condition_text().c_str());

    lv_label_set_text(
        objects.wloc_label, ("\uF124 " + weather_data.get_location()).c_str());

    return ESP_OK;
}

esp_err_t WeatherScreen::update_screen()
{
    lv_obj_add_flag(objects.wrefresh_label, LV_OBJ_FLAG_CLICKABLE);
    if (weather_data.is_temp_dirty())
    {
        std::string temp = std::format("{:.2f}", weather_data.get_temp()) + "°C";
        lv_label_set_text(objects.temp_label, temp.c_str());
    }

    if (weather_data.is_high_dirty())
    {
        std::string high = std::format("{:.2f}", weather_data.get_high()) + "°C";
        lv_label_set_text(objects.high_temp_label, high.c_str());
    }

    if (weather_data.is_low_dirty())
    {
        std::string low = std::format("{:.2f}", weather_data.get_low()) + "°C";
        lv_label_set_text(objects.low_temp_label, low.c_str());
    }

    if (weather_data.is_humidity_dirty())
    {
        std::string humidity = std::format("{:.2f}", weather_data.get_humidity()) + "%";
        lv_label_set_text(objects.humidity_label, humidity.c_str());
    }

    if (weather_data.is_wind_dirty())
    {
        std::string wind = std::format("{:.2f}", weather_data.get_wind_speed()) + "\nkm/h";
        lv_label_set_text(objects.wind_label, wind.c_str());
    }

    if (weather_data.is_rain_dirty())
    {
        std::string rain = std::format("{:.2f}", weather_data.get_rain_percentage()) + "%";
        lv_label_set_text(objects.rain_chance_label, rain.c_str());
    }

    if (weather_data.is_text_dirty())
    {
        lv_label_set_text(
            objects.weather_type_label, weather_data.get_condition_text().c_str());
    }

    if (weather_data.is_loc_dirty())
    {
        lv_label_set_text(
            objects.wloc_label, ("\uF124 " + weather_data.get_location()).c_str());
    }
    return ESP_OK;
}

esp_err_t WeatherScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");

    return ESP_OK;
}

esp_err_t WeatherScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Menu Screen 1
        return screen_manager.change_screen(SCREEN_ID_MENU_SCREEN_ONE);
    }

    if (events & WEATHER_UI_UPDATE_EVENT)
    {
        return update_screen();
    }

    return ESP_OK;
}

void action_weather_refresh_clicked(lv_event_t *e)
{
    if(background_task_handle != nullptr)
    {
        xTaskNotify(background_task_handle, INIT_WEATHER_FETCH_EVENT,eSetBits);
        lv_obj_remove_flag(objects.wrefresh_label, LV_OBJ_FLAG_CLICKABLE);
    }
}