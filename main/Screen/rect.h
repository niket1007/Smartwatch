#pragma once

struct Rect
{
    int x1, y1, width, height;

    bool contains(uint16_t x, uint16_t y) const
    {
        return (x >= x1 && x < (x1 + width) && y >= y1 && y < (y1 + height));
    }
};