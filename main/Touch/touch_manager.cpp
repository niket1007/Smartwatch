#include "touch_manager.h"
#include "Common/globals.h"

static constexpr const char *TAG = "TOUCH_MANAGER";

esp_err_t TouchManager::init()
{
    vTaskDelay(pdMS_TO_TICKS(100));

    gpio_config_t int_gpio_cfg = {
        .pin_bit_mask = (1ULL << GPIO_NUM_38),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&int_gpio_cfg));

    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_WIDTH,
        .y_max = LCD_HEIGHT,
        .rst_gpio_num = GPIO_NUM_NC,
        .int_gpio_num = GPIO_NUM_38,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
    tp_io_config.scl_speed_hz = 400000;

    esp_err_t err = esp_lcd_new_panel_io_i2c(i2c_manager.i2c_bus_handle, &tp_io_config, &tp_io_handle);
    if (err != ESP_OK) {
        return err;
    }
    
    err = esp_lcd_touch_new_i2c_ft5x06(
        tp_io_handle,
        &tp_cfg,
        &touch_out_handle);

    if (err != ESP_OK) {
        esp_lcd_panel_io_del(tp_io_handle);
        return err;
    }

    ESP_LOGI(TAG, "touch_new returned: %s (%d)", esp_err_to_name(err), err);
    return ESP_OK;
}

TouchGesture TouchManager::process_touch()
{
    int16_t delta_x = (int16_t)last_x_ - (int16_t)start_x_;
    int16_t delta_y = (int16_t)last_y_ - (int16_t)start_y_;

    uint16_t abs_delta_x = abs(delta_x);
    uint16_t abs_delta_y = abs(delta_y);

    if (abs_delta_x >= SWIPE_MIN_DISTANCE && abs_delta_x > abs_delta_y)
    {
        if (delta_x > 0) {
            ESP_LOGI(TAG, "Gesture Detected: SWIPE RIGHT");
            return TouchGesture::SWIPE_RIGHT;
        } else {
            ESP_LOGI(TAG, "Gesture Detected: SWIPE LEFT");
            return TouchGesture::SWIPE_LEFT;
        }
    }
    else if (abs_delta_x <= TAP_MAX_DISTANCE && abs_delta_y <= TAP_MAX_DISTANCE)
    {
        return TouchGesture::SINGLE_TAP;
    }

    return TouchGesture::NONE;
}

esp_err_t TouchManager::read_touch_data()
{
    // INT pin is Active LOW (0 = Touched, 1 = Idle)
    bool is_hardware_touching = (gpio_get_level(GPIO_NUM_38) == 0);

    // FINGER LIFTED EVENT
    if (!is_hardware_touching && is_touching_)
    {
        is_touching_ = false;
        TouchGesture gesture = process_touch();

        if (gesture == TouchGesture::SWIPE_LEFT)
        {
            ESP_LOGI(TAG, "Left swipe gesture");
            if (gui_task_handle != nullptr) {
                xTaskNotify(gui_task_handle, SWIPE_LEFT_EVENT, eSetBits);
            }
        }
        else if (gesture == TouchGesture::SWIPE_RIGHT)
        {
            ESP_LOGI(TAG, "Right swipe gesture");
            if (gui_task_handle != nullptr) {
                xTaskNotify(gui_task_handle, SWIPE_RIGHT_EVENT, eSetBits);
            }
        }
        else if (gesture == TouchGesture::SINGLE_TAP)
        {
            tap_x_ = start_x_;
            tap_y_ = start_y_;
            ESP_LOGI(TAG, "Single Tap at (%d, %d)", tap_x_, tap_y_);
            if (gui_task_handle != nullptr) {
                xTaskNotify(gui_task_handle, SINGLE_TAP_EVENT, eSetBits);
            }
        }

        return ESP_OK;
    }

    // IDLE STATE (No touch active)
    if (!is_hardware_touching)
    {
        return ESP_OK;
    }

    esp_err_t ret = esp_lcd_touch_read_data(touch_out_handle);

    if (ret == ESP_OK)
    {
        uint16_t x[1], y[1], strength[1];
        uint8_t points = 0;

        if (esp_lcd_touch_get_coordinates(touch_out_handle, x, y, strength, &points, 1) && points > 0)
        {
            if (!is_touching_)
            {
                is_touching_ = true;
                start_x_ = x[0];
                start_y_ = y[0];
            }

            last_x_ = x[0];
            last_y_ = y[0];

            display_driver.reset_screen_timeout_timer();
        }
    }

    return ret;
}