#include "Arduino_DriveBus_Library.h"
#include "XPowersLib.h"
#include <lvgl.h>
#include "esp_pm.h"          // Automatic light sleep + dynamic freq scaling BLE

#include "globals.h"
#include "manager/navigation_manager.h"
#include "manager/rtc_datetime_manager.h"
#include "manager/battery_manager.h"
#include "manager/lvgl_manager.h"
#include "manager/ble_manager.h"

#define BOOT_BUTTON_PIN 0 // Onboard BOOT button wired to GPIO0

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

uint32_t ble_passkey;

// ESP Power Management Locks (The Two-Lock Strategy)
esp_pm_lock_handle_t sleep_lock;
esp_pm_lock_handle_t speed_lock;

// Task handles
TaskHandle_t task_gui_handle = NULL;
TaskHandle_t task_background_handle = NULL;

// Interval variables
int wifi_interval = 10;
int battery_update_ui_interval = 5000;
int datetime_update_ui_interval = 3000;
int battery_sensor_read_interval = 2000;
int screen_timeout_interval = 15000;
int battery_data_to_phone_interval = 10000;
unsigned long previous_millis_datetime = 0;
unsigned long previous_millis_battery = 0;
unsigned long previous_millis_read_battery = 0;
unsigned long previous_millis_screen_timeout = 0;
unsigned long previous_millis_battery_to_phone = 0;


// Screen and Button State variables
volatile bool power_button_pressed = false;
static bool screen_state = true; 
uint8_t screen_brightness = 0xff;

// ################# Variable End #################


// ==========================================================
// PHYSICAL BOOT BUTTON INTERRUPT SERVICE ROUTINE (ISR)
// ==========================================================
void IRAM_ATTR bootButtonISR() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  
  // 200ms software debounce
  if (interrupt_time - last_interrupt_time > 200) { 
    power_button_pressed = true;
  }
  last_interrupt_time = interrupt_time;
}

// ==========================================================
// CO5300 LOW-LEVEL DISPLAY POWER / BACKLIGHT FUNCTIONS
// ==========================================================
void set_screen_brightness(uint8_t brightness_val) {
   #if defined(ARDUINO_ARCH_ESP32) && defined(CO5300_QSPI_BRIGHTNESS)
      gfx->setBrightness(brightness_val);
   #else
      // Send command 0x51 (brightness register) followed by the brightness value
      bus->beginWrite();
      bus->writeC8D8(0x51, brightness_val);
      bus->endWrite();
   #endif
}

void turn_on_screen() {
    // ACQUIRE LOCKS: Prevent sleep and force 240MHz for smooth UI
    esp_pm_lock_acquire(sleep_lock);
    esp_pm_lock_acquire(speed_lock);

    // Wake up display driver (Command 0x29: Display On)
    bus->beginWrite();
    bus->writeCommand(0x29);
    bus->endWrite();
    
    // Full Brightness is 255 and hexadecimal value is 0xff
    // 70% brightness is 255 * 0.70 = 178 => hexadecimal value of 178 is 0xB2
    set_screen_brightness(screen_brightness);
    screen_state = true;
}

void turn_off_screen() {
    set_screen_brightness(0x00); // Backlight off
    
    // Put display in sleep / display off mode (Command 0x28: Display Off)
    bus->beginWrite();
    bus->writeCommand(0x28);
    bus->endWrite();
    
    screen_state = false;

    // RELEASE LOCKS: Allow CPU to drop to 40MHz and enter Light Sleep
    esp_pm_lock_release(speed_lock);
    esp_pm_lock_release(sleep_lock);
}

void check_power_button_action() {
  if (power_button_pressed) {
    power_button_pressed = false;
    if (screen_state) {
      turn_off_screen();
    } else {
      turn_on_screen();
      previous_millis_screen_timeout = millis();
    }
  }
}
// ==========================================================
// FT3168 TOUCH DRIVER (Thread-Safe)
// ==========================================================
bool lvgl_get_touch(int16_t &x, int16_t &y) {
  bool is_touched = false;

  // Touch disabled if screen is off
  if (!screen_state) {
    return false;
  }

  // MUST lock the I2C bus because this runs on Core 1!
  if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(10))) {

    if (FT3168->IIC_Interrupt_Flag == true) {
      FT3168->IIC_Interrupt_Flag = false;

      x = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
      y = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);
      is_touched = true;
      previous_millis_screen_timeout = millis();
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
// Task for Core 0: Handling sensors in the background
// ==========================================================
void task_background(void *pvParameters) {
  vTaskDelay(pdMS_TO_TICKS(1500)); 

  unsigned long last_battery_check = 0;
  unsigned long last_bat_to_phone_check = 0;

  while(1) {
    unsigned long now = millis();

    if (now - last_battery_check >= battery_sensor_read_interval) {
      last_battery_check = now;
      read_battery_sensor();
    }

    // if (now - last_bat_to_phone_check >= battery_data_to_phone_interval) {
    //   last_bat_to_phone_check = now;
    //   send_battery_percentage_to_phone();
    // }
    
    vTaskDelay(pdMS_TO_TICKS(1000)); // 1000 ms
  }
}

// ==========================================================
// Task for Core 1: Dedicated entirely to the LVGL UI Engine
// ==========================================================
void task_gui(void *pvParameters) {
  usb_serial.println("[System] LVGL engine initialized & running.");
  uint8_t last_brightness_value = 0xFF;
  previous_millis_screen_timeout = millis();

  while(1) {
    if(screen_state) {
      int bat_percent = get_battery_percentage();
      if(bat_percent >= 70) {
        screen_brightness = 0xb2; // 70% brightness = 255 *0.70 = 178 (0xb2 hexadecimal)
        battery_update_ui_interval = 5000;
        datetime_update_ui_interval = 2000;
        battery_sensor_read_interval = 2000;

        screen_timeout_interval = 15000;
      }
      else if(bat_percent >= 40 && bat_percent < 70) {
        screen_brightness = 0x7F; // 50% brightness = 255 *0.50 = 175 (0x7F hexadecimal)
        // Intervals doubled (slower)
        battery_update_ui_interval = 10000; 
        datetime_update_ui_interval = 4000;
        battery_sensor_read_interval = 4000;

        screen_timeout_interval = 10000;
      }
      else if(bat_percent >=20 && bat_percent < 40) {
        screen_brightness = 0x66; // 40% brightness = 255 *0.40 = 102 (0x66 hexadecimal)
        // Intervals tripled (slower)
        battery_update_ui_interval = 15000; 
        datetime_update_ui_interval = 6000;
        battery_sensor_read_interval = 6000;

        screen_timeout_interval = 5000;
      }
      else {
        screen_brightness = 0x26; // 15% brightness = 255 *0.15 = 38 (0x26 hexadecimal)
        // Intervals tripled (slower)
        battery_update_ui_interval = 15000;
        datetime_update_ui_interval = 6000;
        battery_sensor_read_interval = 6000;

        screen_timeout_interval = 5000;
      }
    
      if (screen_brightness != last_brightness_value) {
        last_brightness_value = screen_brightness;
        
        // If screen is currently on, update it immediately to reflect the new brightness
        if (screen_state) {
          set_screen_brightness(screen_brightness);
        }
        usb_serial.printf("Brightness percentage: %d\n", screen_brightness);
      }
    }

    if (show_passkey_display != 0) {
      if(!screen_state) {
        usb_serial.println("Screen was off, turned on for ble passkey screen");
        turn_on_screen();
      }

      // Screen will never go to sleep mode if paskey is shown
      previous_millis_screen_timeout = millis();

      update_ble_passkey_display();
    }

    // Process power button hardware interrupt checks
    check_power_button_action();

    // Paint and update screen objects layout only if screen is on
    if (screen_state) {
       unsigned long now = millis();

       if (now - previous_millis_screen_timeout >= screen_timeout_interval) {
          previous_millis_screen_timeout = now;
          if (screen_state) {
            turn_off_screen();
          }
       }

       if(now - previous_millis_battery >= battery_update_ui_interval) {
         previous_millis_battery = now;
         if(active_screen == 0) {
           update_battery_ui();
         }
       }

       if(now - previous_millis_datetime >= datetime_update_ui_interval) {
         previous_millis_datetime = now;
         if(active_screen == 0) {
           update_datetime_ui();
         }
       }
       
       // NO LOCKS HERE: Let the screen_state locks handle it globally!
       uint32_t time_till_next = lv_timer_handler();
      //  if(time_till_next < 5) time_till_next = 5;
      //  if(time_till_next >= 500) time_till_next = 500;
       
      //  vTaskDelay(pdMS_TO_TICKS(time_till_next));  
      vTaskDelay(pdMS_TO_TICKS(5));
    }
    else {
      vTaskDelay(pdMS_TO_TICKS(200)); // 200 ms
    }
  }
}

// ==========================================================
// MAIN SETUP
// ==========================================================
void setup() { 

  usb_serial.begin(115200);
  delay(1000);
  usb_serial.println("Booting LVGL V9 OS...");

  // Configure physical BOOT button (GPIO0) as input with pullup and interrupt
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BOOT_BUTTON_PIN), bootButtonISR, FALLING);

  // ── Automatic light sleep
  esp_pm_config_t pm_config = {
    .max_freq_mhz = 240,   // matches board's normal running clock
    .min_freq_mhz = 40,    // XTAL-derived floor - safe default for automatic light sleep on ESP32-S3
    .light_sleep_enable = true
  };
  esp_err_t pm_ret = esp_pm_configure(&pm_config);
  if (pm_ret != ESP_OK) {
    usb_serial.printf("[PM] esp_pm_configure failed (%d) - check that CONFIG_PM_ENABLE and "
                       "CONFIG_FREERTOS_USE_TICKLESS_IDLE are enabled for this board\n", pm_ret);
  } else {
    usb_serial.println("[PM] Automatic light sleep enabled (BLE-safe)");
  }
  
  // Initialize the Two Power Locks
  esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "sleep_lock", &sleep_lock);
  esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "speed_lock", &speed_lock);

  // Acquire immediately since the screen boots ON
  esp_pm_lock_acquire(sleep_lock);
  esp_pm_lock_acquire(speed_lock);
  usb_serial.println("[PM] Initial locks acquired.");

  i2c_mutex = xSemaphoreCreateMutex();
  usb_serial.println("I2C Mutex initialised");

  Wire.begin(IIC_SDA, IIC_SCL);
  Wire.setClock(100000); 

  power_init();

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

  gfx->setTextSize(2);
  gfx->setTextColor(0xFFFF);
  gfx->setCursor(100, 251);
  gfx->printf("Loading System...");

  ble_passkey = 100000 + (esp_random() % 900000);

  rtc_init();
  fetch_and_sync_time();
  ble_manager_init();

  lvgl_manager_init();

  // Create Background Sensor Monitoring Task, pinned to Core 0
  xTaskCreatePinnedToCore(
    task_background,       
    "TaskBackground",      
    8192,                  
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

  vTaskDelete(NULL);
}

void loop() {}