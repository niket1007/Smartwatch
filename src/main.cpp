#include "Arduino_DriveBus_Library.h"
#include "XPowersLib.h"
#include <lvgl.h>
#include "esp_private/esp_clk.h"
#include "esp_system.h"
#include "esp_pm.h"          // Automatic light sleep + dynamic freq scaling BLE

#include "Common/globals.h"
// #include "manager/lvgl_manager.h"
// #include "manager/ble_manager.h"
// #include "manager/settings_screen_manager.h"
// #include "manager/notification_manager.h"
// #include "manager/weather_manager.h"
// #include "manager/call_screen_manager.h"

static const char *TAG = "APP_MAIN";

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
  LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */, true /* is_shared_interface */);

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

std::unique_ptr<Arduino_IIC> FT3168(new Arduino_FT3x68(
    IIC_Bus, 
    FT3168_DEVICE_ADDRESS,
    DRIVEBUS_DEFAULT_VALUE, TP_INT,
    Arduino_IIC_Touch_Interrupt));

void Arduino_IIC_Touch_Interrupt(void) {
  FT3168->IIC_Interrupt_Flag = true;
}

// ==========================================================
// FT3168 TOUCH DRIVER (Thread-Safe)
// ==========================================================
bool lvgl_get_touch(int16_t &x, int16_t &y) {
  bool is_touched = false;

  // Touch disabled if screen is off
//   if (!gv.is_screen_active) {
//     return false;
//   }

  if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(10))) {

    if (FT3168->IIC_Interrupt_Flag == true) {
      FT3168->IIC_Interrupt_Flag = false;

      x = FT3168->IIC_Read_Device_Value(
        FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
      y = FT3168->IIC_Read_Device_Value(
        FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);
      is_touched = true;
    //   previous_millis_screen_timeout = millis();
    }

    xSemaphoreGive(i2c_mutex);
  }

  return is_touched;
}

// ==========================================================
// PHYSICAL BOOT BUTTON INTERRUPT SERVICE ROUTINE (ISR)
// ==========================================================
void IRAM_ATTR bootButtonISR() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  
  // 200ms software debounce
  if (interrupt_time - last_interrupt_time > 200) {
    usb_serial.println("Power Button Pressed");
    // power_button_pressed = true;

    // BaseType_t taskWoken = pdFALSE;
  
    // if (task_gui_handle != NULL) {
    //   vTaskNotifyGiveFromISR(task_gui_handle, &taskWoken);
    // }
    
    // if (taskWoken) {
    //   portYIELD_FROM_ISR(); // Yield once if any higher-priority task was unblocked
    // }
  }
  last_interrupt_time = interrupt_time;
}

void background_func(void *pvParameters)
{
    esp_err_t ret = wifi_manager.init();
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to init wifi");
    }
    else  
    {
        ret = rtc_manager.sync();
        if(ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to sync rtc");
        }
    }
    ret = wifi_manager.deinit();
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to deinit wifi");
    }

    // Ble init is depended on battery
    battery_manager.refresh();

    while(1)
    {
        uint32_t events = 0;
        bool notified = xTaskNotifyWait(
            0, UINT32_MAX, &events, pdMS_TO_TICKS(2000));
        ESP_LOGI(TAG, "Background Task");
        
        battery_manager.refresh();

        int battery = battery_manager.get_battery_percentage();
        if(
            (battery <= 20) and 
            (bluetooth_manager.is_init()))
        {
            esp_err_t err = bluetooth_manager.deinit();
            if(err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to deinit bluetooth");
            }
        }
        else if (battery >= 25 and 
            (!bluetooth_manager.is_init()))
        {
            esp_err_t err = bluetooth_manager.init();
            if(err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to init bluetooth");
            }
        }

        
    }
    vTaskDelete(nullptr);
}

void gui_func(void *pvParameters)
{

    while(1)
    {
        // ESP_LOGI(TAG, "GUI Task");
        uint32_t wait = lv_timer_handler();
        
        if (wait < 5) wait = 5;
        if (wait > 100) wait = 100;

        vTaskDelay(pdMS_TO_TICKS(wait));
    }
    vTaskDelete(nullptr);
}


// ==========================================================
// MAIN SETUP
// ==========================================================
void setup() { 

    esp_reset_reason_t reason = esp_reset_reason();
    ESP_LOGW(
        "BOOT",
        "Reset reason = %d",
        reason
    );
    ESP_LOGW(
        "SYSTEM",
        "Free heap: %u",
        ESP.getFreeHeap()
    );
    usb_serial.begin(115200);
    delay(1000);
    ESP_LOGI(TAG, "Booting NSMARTWATCH...");

    // Configure physical BOOT button (GPIO0) as input with pullup and interrupt
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BOOT_BUTTON_PIN), bootButtonISR, FALLING);

    esp_sleep_enable_gpio_wakeup();
    gpio_wakeup_enable((gpio_num_t)BOOT_BUTTON_PIN, GPIO_INTR_LOW_LEVEL);

    // ── Automatic light sleep
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 240,   // matches board's normal running clock
        .min_freq_mhz = 40,    // XTAL-derived floor - safe default for automatic light sleep on ESP32-S3
        .light_sleep_enable = false
    };
    esp_err_t pm_ret = esp_pm_configure(&pm_config);
    if (pm_ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "[PM] esp_pm_configure failed (%d)", pm_ret);
    } else 
    {
        ESP_LOGE(TAG, "[PM] Automatic light sleep enabled (BLE-safe)");
    }
  
  // Initialize the Two Power Locks
//   esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "sleep_lock", &sleep_lock);

//   // Acquire immediately since the screen boots ON
//   esp_pm_lock_acquire(sleep_lock);

    i2c_mutex = xSemaphoreCreateMutex();
    ESP_LOGI(TAG, "I2C Mutex initialised");

    Wire.begin(IIC_SDA, IIC_SCL);
    Wire.setClock(100000); 

    #ifdef GFX_EXTRA_PRE_INIT
        GFX_EXTRA_PRE_INIT();
    #endif

    if (!gfx->begin()) {
        ESP_LOGE(TAG, "Screen init failed!");
    }
    gfx->fillScreen(RGB565_BLACK);

    gfx->setTextSize(2);
    gfx->setTextColor(0xFFFF);
    gfx->setCursor(100, 251);
    gfx->printf("Loading System.....");

    esp_err_t ret = battery_manager.init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Battery manager initialization failed: %s",
            esp_err_to_name(ret)
        );
    }
    else
    {
        ESP_LOGI(
            TAG,
            "Battery manager initialized successfully"
        );
    }

    while (FT3168->begin() == false) 
    {
        ESP_LOGE(TAG, "FT3168 initialization fail");
        delay(2000);
    }
//   usb_serial.println("FT3168 initialization successfully");

    FT3168->IIC_Write_Device_State(
        FT3168->Arduino_IIC_Touch::Device::TOUCH_POWER_MODE,
        FT3168->Arduino_IIC_Touch::Device_Mode::TOUCH_POWER_MONITOR);


//   gv.ble_passkey = 100000 + (esp_random() % 900000);

    rtc_manager.init();

    lvgl_manager_init();

    // Create Background Sensor Monitoring Task, pinned to Core 0
    xTaskCreatePinnedToCore(
        background_func,       
        "BACKGROUND",      
        12288,                  
        NULL,                  
        1,                     
        &background_task_handle, 
        0                      
    );

    // Create GUI Task, pinned to Core 1
    xTaskCreatePinnedToCore(
        gui_func,              
        "GUI",             
        16384,                 
        NULL,                  
        2,         
        &gui_task_handle,        
        1                    
    );

    vTaskDelete(NULL);
}

void loop() {}
