#include "Arduino_GFX_Library.h"
#include "XPowersLib.h"
#include <lvgl.h>

#include "globals.h"
#include "navigation_manager.h"
#include "rtc_datetime_manager.h"
#include "battery_manager.h"

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
  LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

Arduino_GFX *gfx = new Arduino_CO5300(bus, LCD_RESET /* RST */,
                                      0 /* rotation */,  LCD_WIDTH, LCD_HEIGHT,
                                      22 /* col_offset1 */,
                                      0 /* row_offset1 */,
                                      0 /* col_offset2 */,
                                      0 /* row_offset2 */);



// ################# Variable Start #################
HWCDC USBSerial;
SemaphoreHandle_t i2c_mutex;

// Task handles
TaskHandle_t TaskGuiHandle = NULL;
TaskHandle_t TaskBackgroundHandle = NULL;

// Interval variables
int wifi_interval = 10;
unsigned long previous_millis = 0;
// ################# Variable End #################

// ==========================================================
// CUSTOM TOUCH DRIVER (Thread-Safe)
// ==========================================================
bool getTouch(int16_t &x, int16_t &y) {
  bool is_touched = false;

  // 1. MUST lock the I2C bus because this runs on Core 1!
  if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(10))) { 
    
    Wire.beginTransmission(TOUCH_I2C_ADD);
    Wire.write(0x02);
    
    // 2. Only request data if the touch chip actually acknowledges us
    if (Wire.endTransmission() == 0) { 
      Wire.requestFrom((uint8_t)TOUCH_I2C_ADD, (uint8_t)5);
      
      if (Wire.available() >= 5) {
        uint8_t touches = Wire.read() & 0x0F;
        uint8_t x_high  = Wire.read() & 0x0F;
        uint8_t x_low   = Wire.read();
        uint8_t y_high  = Wire.read() & 0x0F;
        uint8_t y_low   = Wire.read();
        
        if (touches > 0) {
          x = (x_high << 8) | x_low;
          y = (y_high << 8) | y_low;
          is_touched = true;
        }
      }
    }
    
    // 3. Always give the lock back so Core 0 can read the battery!
    xSemaphoreGive(i2c_mutex); 
  }
  
  return is_touched;
}

// ==========================================================
// LVGL GLUE: DISPLAY ENGINE
// ==========================================================
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t * px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  
  USBSerial.printf(
      "Flush (%d,%d)-(%d,%d) w=%d h=%d\n",
      area->x1,
      area->y1,
      area->x2,
      area->y2,
      w,
      h
  );
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
  lv_display_flush_ready(disp);
}

// ==========================================================
// LVGL GLUE: TOUCH ENGINE
// ==========================================================
void my_touch_read(lv_indev_t * indev, lv_indev_data_t * data) {
  int16_t touchX, touchY;
  if (digitalRead(TP_INT) == LOW && getTouch(touchX, touchY)) {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = touchX;
    data->point.y = touchY;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// ==========================================================
// CLOCK BACKEND TIMER CALLBACK
// ==========================================================
static void clock_timer_cb(lv_timer_t *timer) {
  update_datetime_ui();
  update_battery_ui();
}

// ==========================================================
// Task for Core 0: Handling sensors/time in the background
// ==========================================================
void vTaskBackground(void *pvParameters) {
  // Sync time via Wi-Fi & update RTC
  fetch_and_sync_time();

  while(1) {
    // unsigned long currentMillis = millis();
    read_battery_sensor();

    // if(currentMillis - previous_millis >= wifi_interval) {
    //   previous_millis = currentMillis;
    //   USBSerial.println("Wifi turneed on and fetched the data");
    // }
    vTaskDelay(pdMS_TO_TICKS(3000)); // Run every 3 seconds
  }
}

// ==========================================================
// Task for Core 1: Handling the LVGL UI engine
// ==========================================================
void vTaskGui(void *pvParameters) {
  while(1) {
    lv_timer_handler(); // Drive the GUI engine

    static uint32_t last = 0;
    if (millis() - last > 2000)
    {
        last = millis();

        USBSerial.printf(
            "GUI Stack Free = %u bytes\n",
            uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t)
        );
    }
    vTaskDelay(pdMS_TO_TICKS(5));  // 5ms delay as used in standard LVGL loops
  }
}

// ==========================================================
// LVGL TICK WRAPPER
// ==========================================================
uint32_t my_tick_function() {
  return (uint32_t)millis();
}

// ==========================================================
// MAIN SETUP
// ==========================================================
void setup() {

  // Set the CPU Frequency to 160MHz
  // setCpuFrequencyMhz(160); 

  USBSerial.begin(115200);
  delay(1000);
  USBSerial.println("Booting LVGL V9 OS...");

  power_init();

  // Wake up Touch Hardware
  pinMode(TP_RESET, OUTPUT);
  digitalWrite(TP_RESET, LOW);
  delay(20);
  digitalWrite(TP_RESET, HIGH);
  delay(50);
  pinMode(TP_INT, INPUT_PULLUP);

#ifdef GFX_EXTRA_PRE_INIT
  GFX_EXTRA_PRE_INIT();
#endif

  // Init Screen
  if (!gfx->begin()) {
    USBSerial.println("Screen init failed!");
  }
  gfx->fillScreen(0x0000);

  rtc_init();

  // Init the lock
  i2c_mutex = xSemaphoreCreateMutex();
  USBSerial.println("Lock initialised");

  // ACTIVATE LVGL V9 MATRIX
  lv_init();
  lv_tick_set_cb(my_tick_function); 
  USBSerial.println("LVL9 initialised");

  // Create the Screen Buffer
  static uint8_t draw_buf1[LCD_WIDTH * 20 * 2];
  lv_display_t * disp = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf1, NULL, sizeof(draw_buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Connect the Touch Input
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touch_read);

  // LOAD THE EEZ STUDIO UI
  ui_init();
  ui_tick();
  USBSerial.println("UI initialised");

  USBSerial.printf("Home     : %p\n", objects.home_screen);
  USBSerial.printf("Weather  : %p\n", objects.weather_screen);
  USBSerial.printf("News     : %p\n", objects.news_screen);
  USBSerial.printf("Settings : %p\n", objects.settings_screen);
  USBSerial.printf("Drawer   : %p\n", objects.app_drawer_screen);

  USBSerial.printf("Heap: %u\n", ESP.getFreeHeap());
  USBSerial.printf(
    "Largest DMA Block: %u\n",
    heap_caps_get_largest_free_block(MALLOC_CAP_DMA)
  );  

  // Create the 1-second recurring clock update timer
  lv_timer_create(clock_timer_cb, 1000, NULL);

  // Create Background Task, pinned to Core 0
  xTaskCreatePinnedToCore(
    vTaskBackground,       // Task function
    "TaskBackground",      // Name of task
    4096,                  // Stack size
    NULL,                  // Parameter
    1,                     // Priority
    &TaskBackgroundHandle, // Task handle
    0                      // Pin to core 0
  );

  // Create GUI Task, pinned to Core 1
  xTaskCreatePinnedToCore(
    vTaskGui,              // Task function
    "TaskGui",             // Name of task
    16384,                 // Stack size
    NULL,                  // Parameter
    2,                     // Priority (higher priority for smooth UI)
    &TaskGuiHandle,        // Task handle
    1                    // Pin to core 1
  );
}

void loop() {}