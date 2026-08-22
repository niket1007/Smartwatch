#pragma once

#include "stdint.h"
#include "esp_err.h"
#include "Icons/generated/icon_globals.h"
#include "Fonts/generated/font_globals.h"

class Graphics
{

private:
    static constexpr uint16_t MAX_GLYPH_WIDTH = 64;
    static constexpr uint16_t MAX_GLYPH_HEIGHT = 64;

    uint16_t glyph_buffer[MAX_GLYPH_WIDTH * MAX_GLYPH_HEIGHT];

    esp_err_t plot_circle_octants(
        int16_t x,
        int16_t y,
        int16_t x_c,
        int16_t y_c,
        uint16_t color,
        uint16_t border_width);

    esp_err_t fill_circle_spans(
        int16_t x,
        int16_t y,
        int16_t x_c,
        int16_t y_c,
        uint16_t color);

    esp_err_t draw_round_rect_arcs(
        int16_t tlx, int16_t tly,
        int16_t trx, int16_t try_,
        int16_t blx, int16_t bly,
        int16_t brx, int16_t bry,
        int16_t x, int16_t y,
        uint16_t color,
        uint16_t border_width);

    esp_err_t fill_round_rect_arcs(
        int16_t tlx, int16_t tly,
        int16_t trx, int16_t try_,
        int16_t blx, int16_t bly,
        int16_t brx, int16_t bry,
        int16_t x, int16_t y,
        uint16_t color);

    esp_err_t draw_alpha_bitmap(
        int16_t x,
        int16_t y,
        uint16_t width,
        uint16_t height,
        const uint8_t *bitmap,
        uint16_t color,
        uint16_t bg_color);

public:
    esp_err_t draw_pixel(
        int16_t x,
        int16_t y,
        uint16_t color,
        uint16_t size = 2);

    esp_err_t draw_rect(
        int16_t x,
        int16_t y,
        uint16_t width,
        uint16_t height,
        uint16_t color,
        uint16_t border_width = 2);

    esp_err_t draw_circle(
        int16_t x,
        int16_t y,
        uint16_t radius,
        uint16_t color,
        uint16_t border_width = 2);

    esp_err_t fill_rect(
        int16_t x,
        int16_t y,
        uint16_t width,
        uint16_t height,
        uint16_t color);

    esp_err_t fill_circle(
        int16_t x,
        int16_t y,
        uint16_t radius,
        uint16_t color);

    esp_err_t draw_line(
        int16_t x1,
        int16_t y1,
        int16_t x2,
        int16_t y2,
        uint16_t color,
        uint16_t width = 2);

    esp_err_t draw_round_rect(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        int16_t r,
        uint16_t color,
        uint16_t border_width = 2);

    esp_err_t fill_round_rect(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        int16_t r,
        uint16_t color);

    esp_err_t draw_icon(
        int16_t x,
        int16_t y,
        const icon_t *icon,
        uint16_t color);

    esp_err_t draw_char(
        int16_t x,
        int16_t baseline_y,
        char c,
        const font_t &font,
        uint16_t color,
        uint16_t bg_color,
        uint16_t *advance);

    esp_err_t draw_text(
        int16_t x,
        int16_t y,
        const char *text,
        const font_t &font,
        uint16_t color,
        uint16_t bg_color);

    int get_text_width(const char *text, const font_t &font);
};