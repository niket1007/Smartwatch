#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_interface.h"
#include "driver/spi_master.h"
#include "esp_lcd_sh8601.h"
#include <algorithm>
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

#define LCD_SDIO0 4
#define LCD_SDIO1 5
#define LCD_SDIO2 6
#define LCD_SDIO3 7
#define LCD_SCLK 11
#define LCD_CS 12
#define LCD_RESET 8
#define LCD_HOST SPI2_HOST
#define LCD_BITS_PER_PIXEL 16

class DisplayDriver
{
private:
    static constexpr int DMA_BUFFER_LINES = 10;
    uint16_t *dma_buffer_ = nullptr;

    esp_lcd_panel_handle_t panel_handle_ = nullptr;
    uint8_t brightness_value_ = 255;

    SemaphoreHandle_t flush_sem_ = nullptr;
    SemaphoreHandle_t draw_mutex_;

    int64_t screen_timeout_ = 0;

public:
    bool is_sleep = false;

    esp_lcd_panel_io_handle_t io_handle_ = nullptr;

    DisplayDriver() = default;

    DisplayDriver(const DisplayDriver &) = delete;
    DisplayDriver &operator=(const DisplayDriver &) = delete;

    esp_err_t init();
    esp_err_t clear(uint16_t color = 0x0000);
    esp_err_t set_brightness(uint8_t percent);
    uint8_t get_brightness() const;

    esp_err_t draw(
        int16_t x_start,
        int16_t y_start,
        int16_t x_end,
        int16_t y_end,
        const uint16_t *color_data);

    esp_err_t sleep();
    esp_err_t wake();

    esp_err_t reset_screen_timeout_timer(int64_t timer = 0);
    int64_t get_screen_timeout_timer();
};