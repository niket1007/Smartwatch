#include "graphics.h"
#include "Common/globals.h"
#include <cmath>

#include <inttypes.h> // Required for PRIu32

static constexpr const char *TAG = "Graphics";

constexpr uint16_t MIN_PIXEL_SIZE = 2;
constexpr uint16_t MAX_PIXEL_SIZE = 5;

esp_err_t Graphics::draw_pixel(
    int16_t x, int16_t y, uint16_t color, uint16_t size)
{
    if (x < 0 || x >= LCD_WIDTH ||
        y < 0 || y >= LCD_HEIGHT ||
        x + size > LCD_WIDTH ||
        y + size > LCD_HEIGHT)
    {
        ESP_LOGW(TAG, "Pixel out of bounds");
        return ESP_ERR_INVALID_ARG;
    }

    if (size < MIN_PIXEL_SIZE || size > MAX_PIXEL_SIZE)
    {
        ESP_LOGE(TAG, "Width out of bound (Supported Range: 2 to 5)");
        return ESP_ERR_INVALID_ARG;
    }

    static uint16_t buf[MAX_PIXEL_SIZE * MAX_PIXEL_SIZE];

    std::fill_n(buf, size * size, color);

    ESP_RETURN_ON_ERROR(
        display_driver.draw(x, y, x + size, y + size, buf),
        TAG, "Failed to draw pixel");

    return ESP_OK;
}

esp_err_t Graphics::fill_rect(
    int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color)
{

    if (
        x < 0 || x >= LCD_WIDTH ||
        y < 0 || y >= LCD_HEIGHT ||
        width == 0 || x + width > LCD_WIDTH ||
        height == 0 || y + height > LCD_HEIGHT)
    {
        ESP_LOGW(TAG, "Rectangle out of bounds");
        return ESP_ERR_INVALID_ARG;
    }

    constexpr int LINES = 10;
    static uint16_t buf[LCD_WIDTH * LINES];

    std::fill_n(buf, width * LINES, color);

    for (int curr_y = y; curr_y < (y + height); curr_y += LINES)
    {
        ESP_RETURN_ON_ERROR(
            display_driver.draw(
                x, curr_y, x + width, std::min(curr_y + LINES, y + height), buf),
            TAG, "Failed to draw bitmap");
    }

    return ESP_OK;
}

esp_err_t Graphics::draw_line(
    int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, uint16_t width)
{
    // Generalized Bresenham line algorithm (Alois Zingl)
    // Handles all 8 octants using integer arithmetic.

    if (x1 < 0 || x1 >= LCD_WIDTH ||
        y1 < 0 || y1 >= LCD_HEIGHT ||
        x2 < 0 || x2 >= LCD_WIDTH ||
        y2 < 0 || y2 >= LCD_HEIGHT)
    {
        ESP_LOGW(TAG, "Line out of bounds");
        return ESP_ERR_INVALID_ARG;
    }

    if (width < MIN_PIXEL_SIZE || width > MAX_PIXEL_SIZE)
    {
        ESP_LOGE(TAG, "Width out of bound (Supported Range: 2 to 5)");
        return ESP_ERR_INVALID_ARG;
    }

    int16_t dx = std::abs(x2 - x1);
    int16_t dy = -std::abs(y2 - y1);

    int16_t stepping_dir_x = x1 < x2 ? 1 : -1;
    int16_t stepping_dir_y = y1 < y2 ? 1 : -1;

    int16_t error = dx + dy;
    int16_t error_2 = 0;

    while (true)
    {
        ESP_RETURN_ON_ERROR(draw_pixel(x1, y1, color, width), TAG, "Failed to draw pixel");
        error_2 = 2 * error;

        if (error_2 >= dy)
        {
            if (x1 == x2)
                break;
            error += dy;
            x1 += stepping_dir_x;
        }
        if (error_2 <= dx)
        {
            if (y1 == y2)
                break;
            error += dx;
            y1 += stepping_dir_y;
        }
    }

    return ESP_OK;
}

esp_err_t Graphics::draw_rect(
    int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color, uint16_t border_width)
{

    if (
        x < 0 || x >= LCD_WIDTH ||
        y < 0 || y >= LCD_HEIGHT ||
        width == 0 || x + width > LCD_WIDTH ||
        height == 0 || y + height > LCD_HEIGHT)
    {
        ESP_LOGW(TAG, "Rectangle out of bounds");
        return ESP_ERR_INVALID_ARG;
    }

    if (border_width < MIN_PIXEL_SIZE || border_width > MAX_PIXEL_SIZE)
    {
        ESP_LOGE(TAG, "Border Width out of bound (Supported Range: 2 to 5)");
        return ESP_ERR_INVALID_ARG;
    }

    if (width == 1)
        return draw_line(x, y, x, y + height - 1, color);
    if (height == 1)
        return draw_line(x, y, x + width - 1, y, color);

    // Create a line from (x, y) to (x+width-1, y) => Top Edge
    ESP_RETURN_ON_ERROR(
        draw_line(x, y, x + width - 1, y, color, border_width),
        TAG, "Failed to draw top edge");

    // Create a line from (x, y) to (x, y+height-1)
    ESP_RETURN_ON_ERROR(
        draw_line(x, y, x, y + height - 1, color, border_width),
        TAG, "Failed to draw left edge");

    // Create a line from (x, y+height-1) to (x+width-1, y+height-1)
    ESP_RETURN_ON_ERROR(
        draw_line(x, y + height - 1, x + width - 1, y + height - 1, color, border_width),
        TAG, "Failed to draw bottom edge");

    // Create a line from (x+width-1, y) to (x+width-1, y+height-1)
    ESP_RETURN_ON_ERROR(
        draw_line(x + width - 1, y, x + width - 1, y + height - 1, color, border_width),
        TAG, "Failed to draw right edge");

    return ESP_OK;
}

esp_err_t Graphics::plot_circle_octants(
    int16_t x, int16_t y, int16_t x_c, int16_t y_c, uint16_t color, uint16_t border_width)
{
    ESP_RETURN_ON_ERROR(
        draw_pixel(x_c + x, y_c + y, color, border_width),
        TAG, "Failed to draw circle point");
    ESP_RETURN_ON_ERROR(
        draw_pixel(x_c - x, y_c + y, color, border_width),
        TAG, "Failed to draw circle point");
    ESP_RETURN_ON_ERROR(
        draw_pixel(x_c + x, y_c - y, color, border_width),
        TAG, "Failed to draw circle point");
    ESP_RETURN_ON_ERROR(
        draw_pixel(x_c - x, y_c - y, color, border_width),
        TAG, "Failed to draw circle point");

    ESP_RETURN_ON_ERROR(
        draw_pixel(x_c + y, y_c + x, color, border_width),
        TAG, "Failed to draw circle point");
    ESP_RETURN_ON_ERROR(
        draw_pixel(x_c - y, y_c + x, color, border_width),
        TAG, "Failed to draw circle point");
    ESP_RETURN_ON_ERROR(
        draw_pixel(x_c + y, y_c - x, color, border_width),
        TAG, "Failed to draw circle point");
    ESP_RETURN_ON_ERROR(
        draw_pixel(x_c - y, y_c - x, color, border_width),
        TAG, "Failed to draw circle point");

    return ESP_OK;
}

esp_err_t Graphics::draw_circle(
    int16_t x, int16_t y, uint16_t radius, uint16_t color, uint16_t border_width)
{
    if (
        x < 0 || x >= LCD_WIDTH ||
        y < 0 || y >= LCD_HEIGHT ||
        radius == 0 ||
        x < radius || y < radius ||
        x + radius >= LCD_WIDTH || y + radius >= LCD_HEIGHT)
    {
        ESP_LOGW(TAG, "Circle out of bounds");
        return ESP_ERR_INVALID_ARG;
    }

    if (border_width < MIN_PIXEL_SIZE || border_width > MAX_PIXEL_SIZE)
    {
        ESP_LOGE(TAG, "Border Width out of bound (Supported Range: 2 to 5)");
        return ESP_ERR_INVALID_ARG;
    }

    int16_t x_a = 0, y_a = radius;
    int16_t decision_parameter = 1 - radius;

    while (x_a <= y_a)
    {
        esp_err_t err = plot_circle_octants(x_a, y_a, x, y, color, border_width);
        if (err != ESP_OK)
            return err;

        x_a += 1;

        if (decision_parameter < 0)
        {
            decision_parameter += (2 * x_a) + 1;
        }
        else
        {
            y_a = y_a - 1;
            decision_parameter += (2 * x_a) - (2 * y_a) + 1;
        }
    }
    return ESP_OK;
}

esp_err_t Graphics::fill_circle_spans(
    int16_t x, int16_t y, int16_t x_c, int16_t y_c, uint16_t color)
{
    ESP_RETURN_ON_ERROR(
        draw_line(x_c - x, y_c + y, x_c + x, y_c + y, color),
        TAG, "Failed to fill circle span");

    ESP_RETURN_ON_ERROR(
        draw_line(x_c - x, y_c - y, x_c + x, y_c - y, color),
        TAG, "Failed to fill circle span");

    ESP_RETURN_ON_ERROR(
        draw_line(x_c - y, y_c + x, x_c + y, y_c + x, color),
        TAG, "Failed to fill circle span");

    ESP_RETURN_ON_ERROR(
        draw_line(x_c - y, y_c - x, x_c + y, y_c - x, color),
        TAG, "Failed to fill circle span");

    return ESP_OK;
}

esp_err_t Graphics::fill_circle(
    int16_t x, int16_t y, uint16_t radius, uint16_t color)
{
    if (
        x < 0 || x >= LCD_WIDTH ||
        y < 0 || y >= LCD_HEIGHT ||
        radius == 0 ||
        x < radius || y < radius ||
        x + radius >= LCD_WIDTH || y + radius >= LCD_HEIGHT)
    {
        ESP_LOGW(TAG, "Circle out of bounds");
        return ESP_ERR_INVALID_ARG;
    }

    int16_t x_a = 0, y_a = radius;
    int16_t decision_parameter = 1 - radius;

    while (x_a <= y_a)
    {
        esp_err_t err = fill_circle_spans(x_a, y_a, x, y, color);
        if (err != ESP_OK)
            return err;

        x_a += 1;

        if (decision_parameter < 0)
        {
            decision_parameter += (2 * x_a) + 1;
        }
        else
        {
            y_a = y_a - 1;
            decision_parameter += (2 * x_a) - (2 * y_a) + 1;
        }
    }
    return ESP_OK;
}

esp_err_t Graphics::draw_round_rect_arcs(
    int16_t tlx, int16_t tly, int16_t trx, int16_t try_, int16_t blx, int16_t bly,
    int16_t brx, int16_t bry, int16_t x, int16_t y, uint16_t color, uint16_t border_width)
{
    // Draw Top Left Arc
    ESP_RETURN_ON_ERROR(
        draw_pixel(tlx - x, tly - y, color, border_width),
        TAG, "Failed to draw top left arc");

    ESP_RETURN_ON_ERROR(
        draw_pixel(tlx - y, tly - x, color, border_width),
        TAG, "Failed to draw top left arc");

    // Draw Top Right Arc
    ESP_RETURN_ON_ERROR(
        draw_pixel(trx + x, try_ - y, color, border_width),
        TAG, "Failed to draw top right arc");

    ESP_RETURN_ON_ERROR(
        draw_pixel(trx + y, try_ - x, color, border_width),
        TAG, "Failed to draw top right arc");

    // Draw Bottom Left Arc
    ESP_RETURN_ON_ERROR(
        draw_pixel(blx - y, bly + x, color, border_width),
        TAG, "Failed to draw bottom left arc (1)");

    ESP_RETURN_ON_ERROR(
        draw_pixel(blx - x, bly + y, color, border_width),
        TAG, "Failed to draw bottom left arc (2)");

    // Draw Bottom Right Arc
    ESP_RETURN_ON_ERROR(
        draw_pixel(brx + x, bry + y, color, border_width),
        TAG, "Failed to draw bottom right arc (1)");

    ESP_RETURN_ON_ERROR(
        draw_pixel(brx + y, bry + x, color, border_width),
        TAG, "Failed to draw bottom right arc (2)");

    return ESP_OK;
}

esp_err_t Graphics::draw_round_rect(
    int16_t x, int16_t y, int16_t width, int16_t height,
    int16_t r, uint16_t color, uint16_t border_width)
{

    if (r < 0 || r > std::min(width, height) / 2)
    {
        ESP_LOGW(TAG, "Invalid radius");
        return ESP_ERR_INVALID_ARG;
    }

    if (
        x < 0 || x >= LCD_WIDTH ||
        y < 0 || y >= LCD_HEIGHT ||
        width == 0 || x + width > LCD_WIDTH ||
        height == 0 || y + height > LCD_HEIGHT)
    {
        ESP_LOGW(TAG, "Round Rectangle out of bounds");
        return ESP_ERR_INVALID_ARG;
    }

    if (border_width < MIN_PIXEL_SIZE || border_width > MAX_PIXEL_SIZE)
    {
        ESP_LOGE(TAG, "Border Width out of bound (Supported Range: 2 to 5)");
        return ESP_ERR_INVALID_ARG;
    }

    // Draw 4 Lines
    ESP_RETURN_ON_ERROR(
        draw_line(x + r, y,
                  (x + width - 1) - r, y, color, border_width),
        TAG, "Failed to draw top line (round_rectangle)");

    ESP_RETURN_ON_ERROR(
        draw_line(x + r, y + height - 1,
                  (x + width - 1) - r, (y + height - 1), color, border_width),
        TAG, "Failed to draw bottom line (round_rectangle)");

    ESP_RETURN_ON_ERROR(
        draw_line(x, y + r, x,
                  (y + height - 1) - r, color, border_width),
        TAG, "Failed to draw left line (round_rectangle)");

    ESP_RETURN_ON_ERROR(
        draw_line((x + width - 1), y + r,
                  (x + width - 1), (y + height - 1) - r, color, border_width),
        TAG, "Failed to draw right line (round_rectangle)");

    // Draw 4 arc
    int16_t x_a = 0, y_a = r;
    int16_t decision_parameter = 1 - r;

    while (x_a <= y_a)
    {
        esp_err_t err = draw_round_rect_arcs(
            x + r, y + r,
            (x + width - 1) - r, y + r,
            x + r, (y + height - 1) - r,
            (x + width - 1) - r, (y + height - 1) - r,
            x_a, y_a,
            color, border_width);

        if (err != ESP_OK)
            return err;

        x_a += 1;

        if (decision_parameter < 0)
        {
            decision_parameter += (2 * x_a) + 1;
        }
        else
        {
            y_a -= 1;
            decision_parameter += (2 * x_a) - (2 * y_a) + 1;
        }
    }

    return ESP_OK;
}

esp_err_t Graphics::fill_round_rect_arcs(
    int16_t tlx, int16_t tly, int16_t trx, int16_t try_, int16_t blx, int16_t bly,
    int16_t brx, int16_t bry, int16_t x, int16_t y, uint16_t color)
{
    // Draw Top Left Arc
    ESP_RETURN_ON_ERROR(
        draw_line(tlx - y, tly - x, tlx, tly - x, color),
        TAG, "Failed to draw top left arc");

    ESP_RETURN_ON_ERROR(
        draw_line(tlx - x, tly - y, tlx, tly - y, color),
        TAG, "Failed to draw top left arc");

    // Draw Top Right Arc
    ESP_RETURN_ON_ERROR(
        draw_line(trx, try_ - x, trx + y, try_ - x, color),
        TAG, "Failed to draw top right arc");

    ESP_RETURN_ON_ERROR(
        draw_line(trx, try_ - y, trx + x, try_ - y, color),
        TAG, "Failed to draw top right arc");

    // Draw Bottom Left Arc
    ESP_RETURN_ON_ERROR(
        draw_line(blx - y, bly + x, blx, bly + x, color),
        TAG, "Failed to draw bottom left arc (1)");

    ESP_RETURN_ON_ERROR(
        draw_line(blx - x, bly + y, blx, bly + y, color),
        TAG, "Failed to draw bottom left arc (2)");

    // Draw Bottom Right Arc
    ESP_RETURN_ON_ERROR(
        draw_line(brx, bry + x, brx + y, bry + x, color),
        TAG, "Failed to fill bottom right arc (1)");

    ESP_RETURN_ON_ERROR(
        draw_line(brx, bry + y, brx + x, bry + y, color),
        TAG, "Failed to fill bottom right arc (2)");

    return ESP_OK;
}

esp_err_t Graphics::fill_round_rect(
    int16_t x, int16_t y, int16_t width, int16_t height,
    int16_t r, uint16_t color)
{

    if (r < 0 || r > std::min(width, height) / 2)
    {
        ESP_LOGW(TAG, "Invalid radius");
        return ESP_ERR_INVALID_ARG;
    }

    if (
        x < 0 || x >= LCD_WIDTH ||
        y < 0 || y >= LCD_HEIGHT ||
        width == 0 || x + width > LCD_WIDTH ||
        height == 0 || y + height > LCD_HEIGHT)
    {
        ESP_LOGW(TAG, "Round Rectangle out of bounds");
        return ESP_ERR_INVALID_ARG;
    }

    // Fill center rectangle
    ESP_RETURN_ON_ERROR(
        fill_rect(x + r, y, width - (2 * r), height, color),
        TAG, "Failed to fill center rectangle (round_rectangle)");

    // Fill left rectangle
    ESP_RETURN_ON_ERROR(
        fill_rect(x, y + r, r, height - (2 * r), color),
        TAG, "Failed to fill left rectangle (round_rectangle)");

    // Fill right rectangle
    ESP_RETURN_ON_ERROR(
        fill_rect((x + width - 1) - r, y + r, r, height - (2 * r), color),
        TAG, "Failed to fill right rectangle (round_rectangle)");

    // Fill arcs
    int16_t x_a = 0, y_a = r;
    int16_t decision_parameter = 1 - r;

    while (x_a <= y_a)
    {
        esp_err_t err = fill_round_rect_arcs(
            x + r, y + r,
            (x + width - 1) - r, y + r,
            x + r, (y + height - 1) - r,
            (x + width - 1) - r, (y + height - 1) - r,
            x_a, y_a,
            color);

        if (err != ESP_OK)
            return err;

        x_a += 1;

        if (decision_parameter < 0)
        {
            decision_parameter += (2 * x_a) + 1;
        }
        else
        {
            y_a -= 1;
            decision_parameter += (2 * x_a) - (2 * y_a) + 1;
        }
    }

    return ESP_OK;
}

esp_err_t Graphics::draw_icon(
    int16_t x,
    int16_t y,
    const icon_t *icon,
    uint16_t color)
{
    if (!icon)
        return ESP_ERR_INVALID_ARG;

    for (uint16_t row = 0; row < icon->height; row++)
    {
        for (uint16_t col = 0; col < icon->width; col++)
        {
            uint8_t coverage = icon->data[row * icon->stride + col];

            // Ignore nearly transparent pixels
            if (coverage < 16)
                continue;

            ESP_ERROR_CHECK(draw_pixel(
                x + col,
                y + row,
                color));
        }
    }

    return ESP_OK;
}

esp_err_t Graphics::draw_alpha_bitmap(
    int16_t x, int16_t y, uint16_t width, uint16_t height, const uint8_t *bitmap, uint16_t color, uint16_t bg_color)
{
    if (bitmap == nullptr)
        return ESP_ERR_INVALID_ARG;

    int16_t x0 = std::max<int16_t>(0, x);
    int16_t y0 = std::max<int16_t>(0, y);
    int16_t x1 = std::min<int16_t>(LCD_WIDTH, x + width);
    int16_t y1 = std::min<int16_t>(LCD_HEIGHT, y + height);

    if (x0 >= x1 || y0 >= y1)
        return ESP_OK;

    // Calculate SH8601 Even-Aligned Boundaries to prevent DMA crashes
    int16_t aligned_x0 = (x0 / 2) * 2; // Round down to even
    int16_t aligned_y0 = (y0 / 2) * 2;
    int16_t aligned_x1 = ((x1 + 1) / 2) * 2; // Round up to even
    int16_t aligned_y1 = ((y1 + 1) / 2) * 2;

    const uint16_t aligned_width = aligned_x1 - aligned_x0;
    const uint16_t aligned_height = aligned_y1 - aligned_y0;

    const uint16_t src_r = (color >> 11) & 0x1F;
    const uint16_t src_g = (color >> 5) & 0x3F;
    const uint16_t src_b = color & 0x1F;

    const uint16_t bg_r = (bg_color >> 11) & 0x1F;
    const uint16_t bg_g = (bg_color >> 5) & 0x3F;
    const uint16_t bg_b = bg_color & 0x1F;

    const uint16_t draw_width = x1 - x0;
    const uint16_t draw_height = y1 - y0;

    static uint16_t *glyph_buffers[2] = {nullptr, nullptr};
    static uint8_t buf_idx = 0;

    if (!glyph_buffers[0])
    {
        // Allocate buffer large enough for aligned bounds
        glyph_buffers[0] = (uint16_t *)heap_caps_malloc(100 * 100 * sizeof(uint16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
        glyph_buffers[1] = (uint16_t *)heap_caps_malloc(100 * 100 * sizeof(uint16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    }

    buf_idx = (buf_idx + 1) % 2;
    uint16_t *current_buffer = glyph_buffers[buf_idx];

    // Pre-fill the aligned buffer with the background color to prevent artifacting
    for (int i = 0; i < aligned_width * aligned_height; i++)
    {
        current_buffer[i] = bg_color;
    }

    // Map the font bitmap onto the aligned buffer
    for (int16_t row = 0; row < draw_height; row++)
    {
        int16_t src_row = row + (y0 - y);
        int16_t dest_row = row + (y0 - aligned_y0); // Calculate offset inside aligned buffer

        for (int16_t col = 0; col < draw_width; col++)
        {
            int16_t src_col = col + (x0 - x);
            int16_t dest_col = col + (x0 - aligned_x0); // Calculate offset inside aligned buffer

            uint32_t bit_index = src_row * width + src_col;
            uint32_t byte_index = bit_index / 2;

            uint8_t shift = (bit_index % 2 == 0) ? 4 : 0;
            uint8_t alpha_4bpp = (bitmap[byte_index] >> shift) & 0x0F;

            uint8_t alpha = (alpha_4bpp * 255) / 15;

            if (alpha > 0)
            {
                // Blend font color with background color
                uint16_t r = (bg_r * (255 - alpha) + src_r * alpha) / 255;
                uint16_t g = (bg_g * (255 - alpha) + src_g * alpha) / 255;
                uint16_t b = (bg_b * (255 - alpha) + src_b * alpha) / 255;

                current_buffer[dest_row * aligned_width + dest_col] = (r << 11) | (g << 5) | b;
            }
        }
    }

    // Send the strictly hardware-aligned boundaries and the padded buffer
    return display_driver.draw(aligned_x0, aligned_y0, aligned_x1, aligned_y1, current_buffer);
}

esp_err_t Graphics::draw_char(
    int16_t x, int16_t baseline_y, char c, const font_t &font, uint16_t color, uint16_t bg_color, uint16_t *advance)
{
    if (advance == nullptr || !font.bitmap || !font.glyphs)
        return ESP_ERR_INVALID_ARG;

    *advance = 0;
    uint32_t ch = static_cast<uint8_t>(c);

    if (ch < font.first_char || ch > font.last_char)
        return ESP_ERR_NOT_FOUND;

    const glyph_t *glyph = &font.glyphs[(ch - font.first_char) + 1];

    if (glyph->width > 0 && glyph->height > 0)
    {
        ESP_RETURN_ON_ERROR(
            draw_alpha_bitmap(
                x + glyph->x_offset,
                baseline_y - glyph->y_offset - glyph->height,
                glyph->width,
                glyph->height,
                font.bitmap + glyph->bitmap_offset,
                color,
                bg_color),
            TAG,
            "draw_alpha_bitmap");
    }

    *advance = glyph->advance;
    return ESP_OK;
}

esp_err_t Graphics::draw_text(
    int16_t x, int16_t y, const char *text, const font_t &font, uint16_t color, uint16_t bg_color)
{
    if (text == nullptr)
    {
        return ESP_ERR_INVALID_ARG;
    }

    int16_t cursor_x = x;
    int16_t cursor_y = y;

    while (*text)
    {
        char ch = *text++;

        switch (ch)
        {
        case '\r':
            // Ignore carriage return
            break;

        case '\n':
            cursor_x = x;
            cursor_y += font.line_height;
            break;

        default:
        {
            uint16_t advance = 0;
            esp_err_t err = draw_char(cursor_x, cursor_y, ch, font, color, bg_color, &advance);
            if (err != ESP_OK)
            {
                return err;
            }
            cursor_x += advance;
            break;
        }
        }
    }

    return ESP_OK;
}

int Graphics::get_text_width(
    const char *text,
    const font_t &font)
{
    if (text == nullptr ||
        font.bitmap == nullptr ||
        font.glyphs == nullptr)
    {
        return 0;
    }

    int width = 0;

    while (*text)
    {
        char c = *text++;

        // This function is intended for a single line
        if (c == '\r' || c == '\n')
        {
            return 0;
        }

        uint32_t codepoint = static_cast<uint8_t>(c);

        if (codepoint < font.first_char ||
            codepoint > font.last_char)
        {
            return 0;
        }

        const glyph_t *glyph =
            &font.glyphs[(codepoint - font.first_char) + 1];

        width += glyph->advance;
    }

    return width;
}