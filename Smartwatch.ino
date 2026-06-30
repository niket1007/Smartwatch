#include "Arduino_DriveBus_Library.h"
#include "XPowersLib.h"
#include <lvgl.h>

#include "globals.h"
#include "navigation_manager.h"
#include "rtc_datetime_manager.h"
#include "battery_manager.h"
#include "lvgl_manager.h"

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
  LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

Arduino_GFX *gfx = new Arduino_CO5300(bus, LCD_RESET /* RST */,
                                      0 /* rotation */,  LCD_WIDTH, LCD_HEIGHT,
                                      22 /* col_offset1 */,
                                      0 /* row_offset1 */,
                                      0 /* col_offset2 */,
                                      0 /* row_offset2 */);

// ==========================================================
// FT3168 TOUCH CONTROLLER (via Arduino_DriveBus_Library)
// ==========================================================
std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
  std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

void Arduino_IIC_Touch_Interrupt(void);

std::unique_ptr<Arduino_IIC> FT3168(new Arduino_FT3x68(IIC_Bus, FT3168_DEVICE_ADDRESS,
                                                       DRIVEBUS_DEFAULT_VALUE, TP_INT, Arduino_IIC_Touch_Interrupt));

void Arduino_IIC_Touch_Interrupt(void) {
  FT3168->IIC_Interrupt_Flag = true;
}

// ################# Variable Start #################
HWCDC usb_serial;
SemaphoreHandle_t i2c_mutex;

// Task handles
TaskHandle_t task_gui_handle = NULL;
TaskHandle_t task_background_handle = NULL;

// Interval variables
int wifi_interval = 10;
unsigned long previous_millis = 0;
// ################# Variable End #################


// ==========================================================
// FT3168 TOUCH DRIVER (Thread-Safe)
// ==========================================================
bool lvgl_get_touch(int16_t &x, int16_t &y) {
  bool is_touched = false;

  // MUST lock the I2C bus because this runs on Core 1!
  if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(10))) {

    if (FT3168->IIC_Interrupt_Flag == true) {
      FT3168->IIC_Interrupt_Flag = false;

      x = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
      y = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);
      is_touched = true;
    }

    // Always give the lock back so Core 0 can read the battery!
    xSemaphoreGive(i2c_mutex);
  }

  return is_touched;
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
void task_background(void *pvParameters) {
  // Sync time via Wi-Fi & update RTC
  fetch_and_sync_time();

  while(1) {
    // unsigned long currentMillis = millis();
    read_battery_sensor();

    // if(currentMillis - previous_millis >= wifi_interval) {
    //   previous_millis = currentMillis;
    //   usb_serial.println("Wifi turneed on and fetched the data");
    // }
    vTaskDelay(pdMS_TO_TICKS(3000)); // Run every 3 seconds
  }
}

// ==========================================================
// Task for Core 1: Handling the LVGL UI engine
// ==========================================================
void task_gui(void *pvParameters) {
  while(1) {
    lv_timer_handler(); // Drive the GUI engine

    static uint32_t last = 0;
    if (millis() - last > 2000)
    {
        last = millis();

        usb_serial.printf(
            "GUI Stack Free = %u bytes\n",
            uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t)
        );
    }
    vTaskDelay(pdMS_TO_TICKS(5));  // 5ms delay as used in standard LVGL loops
  }
}

// ==========================================================
// MAIN SETUP
// ==========================================================
void setup() { 

  usb_serial.begin(115200);
  delay(1000);
  usb_serial.println("Booting LVGL V9 OS...");

  power_init();

  // Init I2C bus and wake up FT3168 touch controller
  Wire.begin(IIC_SDA, IIC_SCL);

  while (FT3168->begin() == false) {
    usb_serial.println("FT3168 initialization fail");
    delay(2000);
  }
  usb_serial.println("FT3168 initialization successfully");

  FT3168->IIC_Write_Device_State(FT3168->Arduino_IIC_Touch::Device::TOUCH_POWER_MODE,
                                 FT3168->Arduino_IIC_Touch::Device_Mode::TOUCH_POWER_MONITOR);

#ifdef GFX_EXTRA_PRE_INIT
  GFX_EXTRA_PRE_INIT();
#endif

  // Init Screen
  if (!gfx->begin()) {
    usb_serial.println("Screen init failed!");
  }
  gfx->fillScreen(0x0000);

  rtc_init();

  // Init the lock
  i2c_mutex = xSemaphoreCreateMutex();
  usb_serial.println("Lock initialised");

  // Initializes LVGL, display, touch indev, EEZ Studio UI
  lvgl_manager_init();
  // Create the 1-second recurring clock update timer
  lv_timer_create(clock_timer_cb, 1000, NULL);

  // Create Background Task, pinned to Core 0
  xTaskCreatePinnedToCore(
     task_background,       // Task function
     "TaskBackground",      // Name of task
     4096,                  // Stack size
     NULL,                  // Parameter
     1,                     // Priority
     &task_background_handle, // Task handle
     0                      // Pin to core 0
   );

  // Create GUI Task, pinned to Core 1
  xTaskCreatePinnedToCore(
    task_gui,              // Task function
    "TaskGui",             // Name of task
    16384,                 // Stack size
    NULL,                  // Parameter
    2,                     // Priority (higher priority for smooth UI)
    &task_gui_handle,        // Task handle
    1                    // Pin to core 1
  );
}

void loop() {}