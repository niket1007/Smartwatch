#include "brightness_screen.h"
#include "Screen/Generated/ui.h"
#include "Screen/Generated/actions.h"
#include "Common/globals.h"

static constexpr const char *TAG = "BRIGHTNESS_SCREEN";


esp_err_t BrightnessScreen::on_enter()
{
    int brightness_percent = power_saver_manager.get_brightness_percentage();

    lv_slider_set_value(objects.brightness_percent_label, brightness_percent, LV_ANIM_OFF);

    lv_label_set_text(
        objects.brightness_percent_label,
        std::to_string(brightness_percent).c_str());
    return ESP_OK;
}


esp_err_t BrightnessScreen::on_exit()
{
    return ESP_OK;
}


esp_err_t BrightnessScreen::handle_events(uint32_t events)
{
    if(events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // SETTINGS
        return screen_manager.change_screen(SCREEN_ID_SETTINGS);
    }
    return ESP_OK;
}

void action_brightness_change_event(lv_event_t *e)
{
    int brightness_percent = lv_slider_get_value(objects.brightness_slider);

    lv_obj_add_state(objects.brightness_slider, LV_STATE_DISABLED);

    power_saver_manager.set_user_defined_options(brightness_percent);

    lv_obj_remove_state(objects.brightness_slider, LV_STATE_DISABLED);

    lv_label_set_text(
        objects.brightness_percent_label,
        std::to_string(brightness_percent).c_str());

}