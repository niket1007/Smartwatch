#include "lvgl_manager.h"

#include "Arduino_GFX_Library.h"
#include "globals.h"

// ==========================================================
// Module-level variables
// ==========================================================
uint32_t screenWidth;
uint32_t screenHeight;

// ==========================================================
// LVGL LOG CALLBACK
// ==========================================================
#if LV_USE_LOG != 0
void lvgl_log_print_cb(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  usb_serial.println(buf);
  usb_serial.flush();
}
#endif

// ==========================================================
// LVGL GLUE: DISPLAY ENGINE
// ==========================================================
void lvgl_display_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    gfx->draw16bitRGBBitmap(
        area->x1,
        area->y1,
        (uint16_t *)px_map,
        w,
        h
    );

    lv_display_flush_ready(disp);
}

// ==========================================================
// LVGL GLUE: DISPLAY ROUNDER (CO5300 requires even pixel alignment)
// ==========================================================
void lvgl_rounder_event_cb(lv_event_t *e) {
  lv_area_t *area = (lv_area_t *)lv_event_get_param(e);
  uint16_t x1 = area->x1;
  uint16_t x2 = area->x2;
  uint16_t y1 = area->y1;
  uint16_t y2 = area->y2;

  // round the start of coordinate down to the nearest 2M number
  area->x1 = (x1 >> 1) << 1;
  area->y1 = (y1 >> 1) << 1;
  // round the end of coordinate up to the nearest 2N+1 number
  area->x2 = ((x2 >> 1) << 1) + 1;
  area->y2 = ((y2 >> 1) << 1) + 1;
}

// ==========================================================
// LVGL GLUE: TOUCH ENGINE
// ==========================================================
void lvgl_touch_read_cb(lv_indev_t * indev, lv_indev_data_t * data) {
  int16_t touchX, touchY;
  if (lvgl_get_touch(touchX, touchY)) {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = touchX;
    data->point.y = touchY;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// ==========================================================
// LVGL TICK WRAPPER
// ==========================================================
uint32_t lvgl_tick_function() {
  return (uint32_t)millis();
}

// ==========================================================
// LVGL MANAGER INIT
// ==========================================================
void lvgl_manager_init() {
  // ACTIVATE LVGL V9 MATRIX
  lv_init();
  lv_tick_set_cb(lvgl_tick_function);

  // register print function for debugging
#if LV_USE_LOG != 0
  lv_log_register_print_cb(lvgl_log_print_cb);
#endif

  usb_serial.println("LVL9 initialised");

  screenWidth = gfx->width();
  screenHeight = gfx->height();

  // Create the Screen Buffer
  static uint8_t draw_buf1[LCD_WIDTH * 80];
  static uint8_t draw_buf2[LCD_WIDTH * 80];
  lv_display_t * disp = lv_display_create(screenWidth, screenHeight);
  lv_display_set_flush_cb(disp, lvgl_display_flush_cb);
  lv_display_set_buffers(disp, draw_buf1, draw_buf2, sizeof(draw_buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_add_event_cb(disp, lvgl_rounder_event_cb, LV_EVENT_INVALIDATE_AREA, NULL);

  // Connect the Touch Input
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, lvgl_touch_read_cb);

  // LOAD THE EEZ STUDIO UI
  ui_init();
}
