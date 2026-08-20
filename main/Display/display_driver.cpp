#include "display_driver.h"
#include "Common/globals.h"
#include "esp_timer.h"

static const sh8601_lcd_init_cmd_t lcd_init_cmds[] = {
    {0x11, (uint8_t[]){0x00}, 0, 120},
    {0xC4, (uint8_t[]){0x80}, 1, 0},
    {0x44, (uint8_t[]){0x01, 0xD1}, 2, 0},
    {0x35, (uint8_t[]){0x00}, 1, 0},
    {0x53, (uint8_t[]){0x20}, 1, 10},
    {0x63, (uint8_t[]){0xFF}, 1, 10},
    {0x51, (uint8_t[]){0x00}, 1, 10},
    {0x2A, (uint8_t[]){0x00, 0x16, 0x01, 0xAF}, 4, 0},
    {0x2B, (uint8_t[]){0x00, 0x00, 0x01, 0xF5}, 4, 0},
    {0x29, (uint8_t[]){0x00}, 0, 10},
    {0x51, (uint8_t[]){0xFF}, 1, 0},
};

static constexpr const char *TAG = "Display_Driver";

IRAM_ATTR static bool on_color_trans_done_cb(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    SemaphoreHandle_t flush_sem = (SemaphoreHandle_t)user_ctx;
    BaseType_t need_yield = pdFALSE;
    xSemaphoreGiveFromISR(flush_sem, &need_yield);
    return (need_yield == pdTRUE);
}

esp_err_t DisplayDriver::init()
{
    ESP_LOGI(TAG, "init method invoked");

    flush_sem_ = xSemaphoreCreateBinary();
    if (flush_sem_ == nullptr)
    {
        ESP_LOGE(TAG, "Failed to create flush semaphore");
        return ESP_ERR_NO_MEM;
    }

    draw_mutex_ = xSemaphoreCreateMutex();

    dma_buffer_ = (uint16_t *)heap_caps_malloc(
        LCD_WIDTH * DMA_BUFFER_LINES * sizeof(uint16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (dma_buffer_ == nullptr)
    {
        ESP_LOGE(TAG, "Failed to allocate DMA buffer");
        return ESP_ERR_NO_MEM;
    }

    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = LCD_SCLK;
    buscfg.data0_io_num = LCD_SDIO0;
    buscfg.data1_io_num = LCD_SDIO1;
    buscfg.data2_io_num = LCD_SDIO2;
    buscfg.data3_io_num = LCD_SDIO3;
    buscfg.max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * LCD_BITS_PER_PIXEL / 8;

    ESP_RETURN_ON_ERROR(
        spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");

    const esp_lcd_panel_io_spi_config_t io_config = SH8601_PANEL_IO_QSPI_CONFIG(
        LCD_CS, on_color_trans_done_cb, flush_sem_);

    sh8601_vendor_config_t vendor_config = {
        .init_cmds = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]),
        .flags = {
            .use_qspi_interface = 1,
        },
    };

    ESP_ERROR_CHECK(
        esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle_));

    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RESET,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = LCD_BITS_PER_PIXEL,
        .vendor_config = &vendor_config,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_sh8601(io_handle_, &panel_config, &panel_handle_));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle_));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle_));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle_, 0x16, 0));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle_, true));

    return ESP_OK;
}

esp_err_t DisplayDriver::clear(uint16_t color)
{
    if (panel_handle_ == nullptr)
    {
        ESP_LOGE(TAG, "Panel handle is not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    std::fill_n(dma_buffer_, LCD_WIDTH * DMA_BUFFER_LINES, color);

    for (int y = 0; y < LCD_HEIGHT; y += DMA_BUFFER_LINES)
    {
        ESP_RETURN_ON_ERROR(
            esp_lcd_panel_draw_bitmap(
                panel_handle_, 0, y,
                LCD_WIDTH, std::min(y + DMA_BUFFER_LINES, LCD_HEIGHT), dma_buffer_),
            TAG, "Failed to draw bitmap");

        // FIX 1: Wait for this specific chunk to finish BEFORE queueing the next one.
        // This keeps the DMA queue from overflowing and keeps the semaphore in sync.
        xSemaphoreTake(flush_sem_, portMAX_DELAY);
    }

    return ESP_OK;
}
esp_err_t DisplayDriver::set_brightness(uint8_t percent)
{
    if (panel_handle_ == nullptr)
    {
        ESP_LOGE(TAG, "Panel handle is not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (percent > 100)
    {
        ESP_LOGE(TAG, "Invalid brightness percentage. Should be between 0 and 100.");
        return ESP_ERR_INVALID_ARG;
    }

    brightness_value_ = (uint8_t)(percent * 255 / 100);

    uint32_t lcd_cmd = 0x51;
    lcd_cmd &= 0xff;
    lcd_cmd <<= 8;
    lcd_cmd |= 0x02 << 24;
    uint8_t param = brightness_value_;
    ESP_RETURN_ON_ERROR(
        esp_lcd_panel_io_tx_param(io_handle_, lcd_cmd, &param, 1),
        TAG, "Failed to set brightness");

    return ESP_OK;
}

uint8_t DisplayDriver::get_brightness() const
{
    return static_cast<uint8_t>(brightness_value_ * 100 / 255);
}

esp_err_t DisplayDriver::draw(
    int16_t x_start, int16_t y_start, int16_t x_end, int16_t y_end, const uint16_t *color_data)
{
    if (panel_handle_ == nullptr)
    {
        ESP_LOGE(TAG, "Panel handle is not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(draw_mutex_, portMAX_DELAY);

    if (x_start >= x_end || x_start < 0 || y_start >= y_end || y_start < 0 ||
        x_end > LCD_WIDTH || y_end > LCD_HEIGHT)
    {
        // ESP_LOGE(TAG, "Draw region outbounds"); // Consider commenting this out so it doesn't spam your console

        // FIX 2: Always give the mutex back before returning early
        xSemaphoreGive(draw_mutex_);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = esp_lcd_panel_draw_bitmap(
        panel_handle_, x_start, y_start, x_end, y_end, color_data);

    if (err != ESP_OK)
    {
        // FIX 2b: Prevent deadlock if the SPI driver itself throws an error
        xSemaphoreGive(draw_mutex_);
        return err;
    }

    xSemaphoreTake(flush_sem_, portMAX_DELAY);
    xSemaphoreGive(draw_mutex_);

    return ESP_OK;
}

esp_err_t DisplayDriver::sleep()
{
    ESP_LOGI(TAG, "Sleep func called; is_sleep: %d", is_sleep);
    if (is_sleep)
    {
        return ESP_OK;
    }

    ESP_RETURN_ON_ERROR(
        set_brightness(0), TAG, "Failed to set brightness to 0%%");

    ESP_RETURN_ON_ERROR(
        esp_lcd_panel_disp_off(panel_handle_, true),
        TAG, "Failed display off");

    ESP_RETURN_ON_ERROR(
        esp_lcd_panel_io_tx_param(io_handle_, 0x10, NULL, 0),
        TAG, "Failed display sleep");

    vTaskDelay(pdMS_TO_TICKS(120));

    is_sleep = true;
    return ESP_OK;
}

esp_err_t DisplayDriver::wake()
{
    ESP_LOGI(TAG, "Wake func called; is_sleep: %d", is_sleep);
    if (!is_sleep)
    {
        return ESP_OK;
    }

    ESP_RETURN_ON_ERROR(
        esp_lcd_panel_io_tx_param(io_handle_, 0x11, NULL, 0),
        TAG, "Failed to wake up display");

    vTaskDelay(pdMS_TO_TICKS(120));

    ESP_RETURN_ON_ERROR(
        esp_lcd_panel_disp_off(panel_handle_, false),
        TAG, "Failed display on");

    uint32_t brightness = power_saver_manager.get_brightness_percentage();
    ESP_RETURN_ON_ERROR(
        set_brightness(brightness), TAG, "Failed to set brightness");

    is_sleep = false;

    if (background_task_handle != nullptr)
    {
        xTaskNotify(background_task_handle, SCREEN_ON_EVENT, eSetBits);
    }

    if (touch_task_handle != nullptr)
    {
        xTaskNotify(touch_task_handle, SCREEN_ON_EVENT, eSetBits);
    }

    if (gui_task_handle != nullptr)
    {
        xTaskNotify(gui_task_handle, UPDATE_TIME_EVENT, eSetBits);
    }

    return ESP_OK;
}

esp_err_t DisplayDriver::reset_screen_timeout_timer(int64_t timer)
{
    if (timer == 0)
        timer = esp_timer_get_time() / 1000;
    screen_timeout_ = timer;
    return ESP_OK;
}

int64_t DisplayDriver::get_screen_timeout_timer()
{
    return screen_timeout_;
}