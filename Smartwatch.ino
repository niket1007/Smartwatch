#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include <Wire.h>
#include "HWCDC.h"
#include <SensorPCF85063.hpp>
#include <WiFi.h>
#include <time.h>
#include "XPowersLib.h"
#include <lvgl.h>

extern "C" {
  #include "src/ui/ui.h"
}

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
  LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

Arduino_GFX *gfx = new Arduino_CO5300(bus, LCD_RESET /* RST */,
                                      0 /* rotation */,  LCD_WIDTH, LCD_HEIGHT,
                                      22 /* col_offset1 */,
                                      0 /* row_offset1 */,
                                      0 /* col_offset2 */,
                                      0 /* row_offset2 */);

// Task handles
TaskHandle_t TaskGuiHandle = NULL;
TaskHandle_t TaskBackgroundHandle = NULL;

SemaphoreHandle_t i2c_mutex;

SensorPCF85063 rtc;
HWCDC USBSerial;
XPowersAXP2101 power;

volatile int global_battery_percentage = 0;
volatile bool global_is_charging = false;

const char* ssid     = "Amit 1st Floor";
const char* password = "12345689";

// Timezone Settings (UTC + 5:30 for India = 19800 seconds)
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;
const char* ntp_server = "pool.ntp.org";
const char* backup_ntp_server = "time.nist.gov";

// Global references for day-of-week calculation from `tm` struct
int getDayOfWeek(int year, int month, int day) {
  struct tm t = {0};
  t.tm_year = year - 1900;
  t.tm_mon = month - 1;
  t.tm_mday = day;
  mktime(&t);
  return t.tm_wday; // 0 = Sunday, 1 = Monday, ..., 6 = Saturday
}

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
// WIFI fetches the date time and update the rtc
// ==========================================================
void wifi_fetch_time() {
  USBSerial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    USBSerial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    USBSerial.println("\nWi-Fi Connected!");
    configTime(gmtOffset_sec, daylightOffset_sec, ntp_server, backup_ntp_server);
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 15000)) {
      USBSerial.println("Failed to obtain time from NTP server.");
    } else {
      USBSerial.println("NTP Time Fetched Successfully!");
      rtc.setDateTime(timeinfo.tm_year + 1900, 
                      timeinfo.tm_mon + 1, 
                      timeinfo.tm_mday, 
                      timeinfo.tm_hour, 
                      timeinfo.tm_min, 
                      timeinfo.tm_sec);
      USBSerial.println("Hardware RTC Updated!");
    }
    
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    USBSerial.println("Wi-Fi Powered Down.");
  } else {
    USBSerial.println("\nWi-Fi Connection Failed. Relying on existing RTC time.");
  }
}

// ==========================================================
// CLOCK BACKEND TIMER CALLBACK
// ==========================================================
static void clock_timer_cb(lv_timer_t *timer) {
  // Use RTC object methods to get current time
  RTC_DateTime datetime;
  if (xSemaphoreTake(i2c_mutex, portMAX_DELAY)) {
    datetime = rtc.getDateTime(); 
  
    // Format Day string
    const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    int day_of_week = getDayOfWeek(datetime.getYear(), datetime.getMonth(), datetime.getDay());
    lv_label_set_text(objects.day_label, days[day_of_week]);

    // Format Date string (12 - Mar - 2026)
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char date_str[30];
    snprintf(date_str, sizeof(date_str), "%02d - %s - %04d", datetime.getDay(), months[datetime.getMonth() - 1], datetime.getYear());
    lv_label_set_text(objects.date_label, date_str);

    // Format Time (12-hour format) & AM/PM determination
    int display_hour = datetime.getHour();
    char am_pm_str[3];
    
    if (display_hour >= 12) {
      strcpy(am_pm_str, "PM");
      if (display_hour > 12) {
        display_hour -= 12;
      }
    } else {
      strcpy(am_pm_str, "AM");
      if (display_hour == 0) {
        display_hour = 12;
      }
    }

    char time_str[10];
    snprintf(time_str, sizeof(time_str), "%02d : %02d", display_hour, datetime.getMinute()); // Format 02: 24
    
    lv_label_set_text(objects.time_label, time_str);
    lv_label_set_text(objects.am_pm_label, am_pm_str);

    xSemaphoreGive(i2c_mutex); // Always give the lock back!
  }

  // Battery UI Updates
  static char batt_str[10];
  snprintf(batt_str, sizeof(batt_str), "%d%%", global_battery_percentage);
  lv_label_set_text(objects.battery_percentage_label, batt_str); // Matches `obj0` pointing to label string buffer exported in screens.h
  
  // Update the bar length
  lv_bar_set_value(objects.battery_percentage_bar, global_battery_percentage, LV_ANIM_ON);
  
  // Update Colors and Status text
  if (global_is_charging) {
    // Set bar to Green 0x00FF00
    lv_obj_set_style_bg_color(objects.battery_percentage_bar, lv_color_hex(0x00FF00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_label_set_text(objects.battery_status_label, "Charging");
  } else {
    // Set bar back to default blue color #155dfc
    lv_obj_set_style_bg_color(objects.battery_percentage_bar, lv_color_hex(0x155dfc), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_label_set_text(objects.battery_status_label, "Charged");
  }
}

// ==========================================================
// Calculate battery percentage
// ==========================================================
// ==========================================================
// Calculate battery percentage (Diagnostic Version)
// ==========================================================
int get_accurate_battery_percentage() {
  // 1. Check if PMU sees the battery
  if (!power.isBatteryConnect()) {
    USBSerial.print("[PMU Warning] Battery not physically detected! ");
    // We intentionally WON'T return -1 here right now so we can test the voltage reader
  }
  int percentage = power.getBatteryPercent();
  USBSerial.println(percentage)
  return percentage;
}

// ==========================================================
// Task for Core 0: Handling sensors/time in the background
// ==========================================================
void vTaskBackground(void *pvParameters) {
  // Sync time via Wi-Fi & update RTC
  wifi_fetch_time();

  while(1) {
    int calculated_percentage = 0;
    bool charging_status = false; 

    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(500))) { // Wait up to 500ms for lock
      calculated_percentage = get_accurate_battery_percentage();
      charging_status = power.isCharging();
      xSemaphoreGive(i2c_mutex);
    } else {
      USBSerial.println("Warning: I2C Mutex Lock Timeout in Task 0!");
    }

    USBSerial.print("Debug - Calc %: ");
    USBSerial.print(calculated_percentage);
    USBSerial.print(" | Is Charging: ");
    USBSerial.println(charging_status ? "YES" : "NO");

    if (calculated_percentage != -1) {
      global_battery_percentage = calculated_percentage;
    }
    global_is_charging = charging_status;

    vTaskDelay(pdMS_TO_TICKS(5000)); // Run every 5 seconds
  }
}

// ==========================================================
// Task for Core 1: Handling the LVGL UI engine
// ==========================================================
void vTaskGui(void *pvParameters) {
  while(1) {
    lv_timer_handler(); // Drive the GUI engine
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
  USBSerial.begin(115200);
  delay(1000);
  USBSerial.println("Booting LVGL V9 OS...");

  // Init PMU to stop the safety shutdown (Must happen first)
  Wire.begin(IIC_SDA, IIC_SCL);
  Wire.setClock(100000); 

  USBSerial.print("Initializing AXP2101 PMU...");
  if (power.begin(Wire, AXP2101_SLAVE_ADDRESS, IIC_SDA, IIC_SCL)) {
    USBSerial.println(" SUCCESS!");
    power.disableTSPinMeasure();
 
    power.enableBattDetection();
    power.enableBattVoltageMeasure();
    power.enableVbusVoltageMeasure(); // Force USB voltage reading on
    power.enableGauge();

  } else {
    USBSerial.println(" FAILED! (Chip not found on I2C bus)");
  }

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

  // Init RTC Clock Chip
  if (!rtc.begin(Wire, IIC_SDA, IIC_SCL)) {
    USBSerial.println("RTC failed!");
  } else {
    USBSerial.println("RTC OK!");
  }

  // Init the lock
  i2c_mutex = xSemaphoreCreateMutex();

  // ACTIVATE LVGL V9 MATRIX
  lv_init();
  lv_tick_set_cb(my_tick_function); 

  // Create the Screen Buffer
  static uint8_t draw_buf[LCD_WIDTH * 120 * 2];
  lv_display_t * disp = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Connect the Touch Input
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touch_read);

  // LOAD THE EEZ STUDIO UI
  ui_init();

  // Create the 1-second recurring clock update timer
  lv_timer_create(clock_timer_cb, 1000, NULL);

  // Create Background Task, pinned to Core 0
  xTaskCreatePinnedToCore(
    vTaskBackground,       // Task function
    "TaskBackground",      // Name of task
    8192,                  // Stack size
    NULL,                  // Parameter
    1,                     // Priority
    &TaskBackgroundHandle, // Task handle
    0                      // Pin to core 0
  );

  // Create GUI Task, pinned to Core 1
  xTaskCreatePinnedToCore(
    vTaskGui,              // Task function
    "TaskGui",             // Name of task
    8192,                  // Stack size
    NULL,                  // Parameter
    2,                     // Priority (higher priority for smooth UI)
    &TaskGuiHandle,        // Task handle
    1                      // Pin to core 1
  );
}

void loop() {
           
}