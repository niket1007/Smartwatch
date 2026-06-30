#include "Arduino_DriveBus_Library.h"
#include "XPowersLib.h"
#include <lvgl.h>

#include "globals.h"
#include "src/manager/navigation_manager.h"
#include "src/manager/rtc_datetime_manager.h"
#include "src/manager/battery_manager.h"
#include "src/manager/lvgl_manager.h"

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

volatile bool lvgl_boot_complete = false;

// Interval variables
int wifi_interval = 10;
unsigned long previous_millis_datetime = 0;
unsigned long previous_millis_battery = 0;
unsigned long previous_millis_read_battery = 0;

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
  // Reserved for internal layout synchronization hooks
}

// ==========================================================
// Isolated One-Time Network Time Synchronization Task
// ==========================================================
void task_time_sync_worker(void *pvParameters) {
  while (!lvgl_boot_complete) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  
  vTaskDelay(pdMS_TO_TICKS(500)); 
  
  usb_serial.println("[WiFi] UI Boot Complete. Launching network sync safely...");
  // fetch_and_sync_time();
  usb_serial.println("[WiFi] Time sync completed successfully. Terminating worker thread.");
  
  vTaskDelete(NULL); 
}

// ==========================================================
// Task for Core 0: Handling hardware sensors on a dedicated core
// ==========================================================
// ==========================================================
// Task for Core 0: Handling sensors in the background
// ==========================================================
void task_background(void *pvParameters) {
  // CRITICAL FLUSH PROTECTION BUFFER:
  // Let Core 1 completely process setup handoff, enter task_gui, 
  // and load the initial watch layout arrays into stable memory.
  vTaskDelay(pdMS_TO_TICKS(1500)); 

  unsigned long last_battery_check = 0;

  while(1) {
    unsigned long now = millis();

    // Use our clean non-blocking tracking timer structure
    if (now - last_battery_check >= 5000) {
      last_battery_check = now;
      
      // Thread-safe lock ensures no collision happens with touch scanning
      if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(50))) {
        read_battery_sensor();
        xSemaphoreGive(i2c_mutex);
      }
    }

    // Keep this short 50ms block so the IDLE0 task can clear stack canaries
    vTaskDelay(pdMS_TO_TICKS(50)); 
  }
}

// ==========================================================
// Task for Core 1: Dedicated entirely to the LVGL UI Engine
// ==========================================================
void task_gui(void *pvParameters) {
  // lv_timer_handler();
  
  lvgl_boot_complete = true;
  usb_serial.println("[System] LVGL engine initialized & running.");

  while(1) {
    lv_timer_handler(); // Paint and update screen objects layout
    unsigned long now = millis();

    // Simply flash data onto the UI components from local variable memory state
    if(now - previous_millis_battery >= 2000 && active_screen == 0) {
      previous_millis_battery = now;
      update_battery_ui();
    }

    if(now - previous_millis_datetime >= 30000  && active_screen == 0) {
      previous_millis_datetime = now;
      update_datetime_ui();
    }

    vTaskDelay(pdMS_TO_TICKS(5));  
  }
}

// ==========================================================
// MAIN SETUP
// ==========================================================
void setup() { 

  usb_serial.begin(115200);
  delay(1000);
  usb_serial.println("Booting LVGL V9 OS...");

  i2c_mutex = xSemaphoreCreateMutex();
  usb_serial.println("Lock initialised");

  power_init();

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

  if (!gfx->begin()) {
    usb_serial.println("Screen init failed!");
  }
  gfx->fillScreen(0x0000);

  rtc_init();
  fetch_and_sync_time();

  lvgl_manager_init();

  // Create Background Sensor Monitoring Task, pinned to Core 0
  xTaskCreatePinnedToCore(
    task_background,       
    "TaskBackground",      
    4096,                  
    NULL,                  
    1,                     
    &task_background_handle, 
    0                      
  );

  // Create GUI Task, pinned to Core 1
  xTaskCreatePinnedToCore(
    task_gui,              
    "TaskGui",             
    16384,                 
    NULL,                  
    2,                     
    &task_gui_handle,        
    1                    
  );
}

void loop() {
  // Clean empty main execution loop context shifted by the RTOS kernels
}