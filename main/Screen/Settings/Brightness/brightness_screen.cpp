#include <format>
#include <string>
#include "brightness_screen.h"
#include "Screen/screen_manager.h"
#include "Common/globals.h"
#include <cstring>

static constexpr const char *TAG = "BRIGHTNESS_SCREEN";

esp_err_t BrightnessScreen::on_enter()
{
    ESP_LOGI(TAG, "on_enter called");

    current_brightness = power_saver_manager.get_brightness_percentage();

    ESP_RETURN_ON_ERROR(
        draw(), TAG, "Failed to draw");

    return ESP_OK;
}

esp_err_t BrightnessScreen::on_exit()
{
    ESP_LOGI(TAG, "on_exit called");
    return ESP_OK;
}

esp_err_t BrightnessScreen::update_brightness(bool sign)
{
    uint32_t new_brightness = current_brightness;

    if (!sign)
    {
        if (new_brightness <= 10)
            return ESP_OK;
        new_brightness -= 10;
    }
    else
    {
        if (new_brightness >= 100)
            return ESP_OK;
        new_brightness += 10;
    }

    is_updating = true;

    esp_err_t ret = power_saver_manager.set_user_defined_options(new_brightness);

    is_updating = false;

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set user brightness");
        return ret;
    }

    current_brightness = new_brightness;

    return draw_brightness_text();
}

esp_err_t BrightnessScreen::draw_brightness_text()
{
    std::string percent_str = std::to_string(current_brightness) + "%";

    int text_width = graphics.get_text_width(percent_str.c_str(), freesans_50);

    const int TEXT_AREA_CENTER_X = screen_manager.get_reset_screen_mid();
    int x = TEXT_AREA_CENTER_X - (text_width / 2);

    ESP_RETURN_ON_ERROR(
        graphics.fill_rect(140, 210, 140, 70, BLACK_COLOR),
        TAG, "Failed to fill percent block");

    ESP_RETURN_ON_ERROR(
        graphics.draw_text(
            x + 10, 260, percent_str.c_str(),
            freesans_50, WHITE_COLOR, BLACK_COLOR),
        TAG, "Failed to draw percent text");

    return ESP_OK;
}

esp_err_t BrightnessScreen::draw()
{
    for (const auto &container : icon_containers)
    {
        ESP_RETURN_ON_ERROR(
            graphics.draw_round_rect(
                container.x1, container.y1,
                container.width, container.height,
                30, WHITE_COLOR, 4U),
            TAG, "Failed to draw container box");
    }

    ESP_RETURN_ON_ERROR(
        graphics.draw_icon(160, 55, &icon_minus, WHITE_COLOR),
        TAG, "Failed to draw minus icon");

    ESP_RETURN_ON_ERROR(
        draw_brightness_text(), TAG, "Failed to draw minus icon");

    ESP_RETURN_ON_ERROR(
        graphics.draw_icon(160, 325, &icon_plus, WHITE_COLOR),
        TAG, "Failed to draw plus icon");

    return ESP_OK;
}

esp_err_t BrightnessScreen::identify_tap()
{
    uint16_t tap_x = touch_manager.get_single_tap_x();
    uint16_t tap_y = touch_manager.get_single_tap_y();

    if (icon_containers[0].contains(tap_x, tap_y))
    {
        ESP_LOGI(TAG, "Minus button clicked");
        return update_brightness(false);
    }
    else if (icon_containers[1].contains(tap_x, tap_y))
    {
        ESP_LOGI(TAG, "Plus button clicked");
        return update_brightness(true);
    }

    return ESP_OK;
}

esp_err_t BrightnessScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Return back to Settings
        return screen_manager.change_screen(10);
    }

    if ((events & SINGLE_TAP_EVENT) && !is_updating)
    {
        return identify_tap();
    }
    return ESP_OK;
}