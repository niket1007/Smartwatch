
#pragma once

typedef struct
{
    uint32_t bitmap_offset;
    uint16_t width;
    uint16_t height;
    int16_t x_offset;
    int16_t y_offset;
    uint16_t advance;
} glyph_t;

typedef struct
{
    const uint8_t *bitmap;
    const glyph_t *glyphs;
    uint32_t first_char;
    uint32_t last_char;
    uint16_t line_height;
} font_t;
