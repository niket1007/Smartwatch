#include "Arduino_DriveBus_Library.h"
#include "XPowersLib.h"
#include <lvgl.h>
#include "esp_private/esp_clk.h"
#include "esp_system.h"
#include "esp_pm.h" // Automatic light sleep + dynamic freq scaling BLE


#include "Common/globals.h"

static const char *TAG = "APP_MAIN";

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
    LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */, true /* is_shared_interface */);

Arduino_CO5300 *gfx = new Arduino_CO5300(bus, LCD_RESET /* RST */,
                                      0 /* rotation */, LCD_WIDTH, LCD_HEIGHT,
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

void Arduino_IIC_Touch_Interrupt(void)
{
    FT3168->IIC_Interrupt_Flag = true;
}

// ==========================================================
// FT3168 TOUCH DRIVER (Thread-Safe)
// ==========================================================
bool lvgl_get_touch(int16_t &x, int16_t &y)
{
    bool is_touched = false;

    if (power_saver_manager.is_touch_disabled() or display_manager.is_sleeping())
    {
        return false;
    }

    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(10)))
    {

        if (FT3168->IIC_Interrupt_Flag == true)
        {
            FT3168->IIC_Interrupt_Flag = false;

            x = FT3168->IIC_Read_Device_Value(
                FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
            y = FT3168->IIC_Read_Device_Value(
                FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

            is_touched = true;

            display_manager.reset_screen_timeout_timer();
        }
        xSemaphoreGive(i2c_mutex);
    }
    return is_touched;
}

// ==========================================================
// PHYSICAL BOOT BUTTON INTERRUPT SERVICE ROUTINE (ISR)
// ==========================================================
void IRAM_ATTR bootButtonISR()
{
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();

    // 200ms software debounce
    if (interrupt_time - last_interrupt_time > 200)
    {
        // power_button_pressed = true;

        BaseType_t taskWoken = pdFALSE;

        if (gui_task_handle != NULL)
        {
            xTaskNotifyFromISR(
                gui_task_handle, BOOT_BUTTON_CLICK_EVENT, eSetBits, &taskWoken);
            // usb_serial.println("BOOT BUTTON :: BOOT_BUTTON_CLICK_EVENT sent");
        }

        if (taskWoken)
        {
            portYIELD_FROM_ISR(taskWoken); // Yield once if any higher-priority task was unblocked
            // usb_serial.printf("BOOT BUTTON :: taskWoken %d\n", taskWoken);
        }
    }
    last_interrupt_time = interrupt_time;
}

void background_func(void *pvParameters)
{
    unsigned long now;
    unsigned long update_time_event_timer = 0;

    battery_manager.refresh();

    // esp_err_t ret = wifi_manager.init();
    // if (ret != ESP_OK)
    // {
    //     usb_serial.println("Failed to init wifi");
    // }
    // else
    // {
    //     ret = rtc_manager.sync();
    //     if (ret != ESP_OK)
    //     {
    //         usb_serial.println("Failed to sync rtc");
    //     }
    // }
    // ret = wifi_manager.deinit();
    // if (ret != ESP_OK)
    // {
    //     usb_serial.println("Failed to deinit wifi");
    // }

    while (1)
    {
        uint32_t events = 0;
        bool notified = xTaskNotifyWait(
            0, UINT32_MAX, &events,
            display_manager.is_sleeping() ? pdMS_TO_TICKS(120000) : pdMS_TO_TICKS(2000));

        // usb_serial.printf(
        //     "APP_MAIN :: Background Task %d\n", display_manager.is_sleeping());

        if (notified)
        {
            // usb_serial.println("Notified");

            if (events & BG_WAKE_UP)
            {
                display_manager.reset_screen_timeout_timer();
                // usb_serial.println("Reset time Triggered");
            }

            if(events & WIFI_SAVE_CRED_EVENT)
            {
                storage_manager.store_wifi_credentials();
                if(gui_task_handle != nullptr)
                {
                    xTaskNotify(gui_task_handle, WIFI_CRED_STORED_EVENT, eSetBits);
                }
            }

            // power_saver_manager.handle_ble_event(events);
        }
        else 
        {
            esp_pm_dump_locks(stdout);
        }

        battery_manager.refresh();

        now = millis();

        if (now - update_time_event_timer >= UPDATE_TIME_TIMER)
        {
            update_time_event_timer = now;

            if (gui_task_handle != nullptr and !display_manager.is_sleeping())
            {
                xTaskNotify(
                    gui_task_handle, UPDATE_TIME_EVENT, eSetBits);
            }
        }

        if (!display_manager.is_sleeping() and
            (now - display_manager.get_screen_timeout_timer()) >=
                power_saver_manager.get_screen_timeout())
        {
            display_manager.reset_screen_timeout_timer(now);
            if (gui_task_handle != nullptr)
            {
                // usb_serial.println("SCREEN_OFF_EVENT Triggered");
                xTaskNotify(gui_task_handle, SCREEN_OFF_EVENT, eSetBits);
            }
        }
        // usb_serial.printf(
        //     "Screen Timeout: %d; Subtract Timeout: %d; Saved Timer: %d\n",
        //     power_saver_manager.get_screen_timeout(),
        //     (now - display_manager.get_screen_timeout_timer()),
        //     display_manager.get_screen_timeout_timer()
        // );
    }
}

void gui_func(void *pvParameters)
{
    esp_err_t ret = screen_manager.load_default_screen();
    if (ret != ESP_OK)
    {
        usb_serial.println("APP_MAIN :: Failed to load home screen");
    }

    display_manager.reset_screen_timeout_timer();

    while (true)
    {
        uint32_t events = 0;

        bool notified = xTaskNotifyWait(
            0, UINT32_MAX, &events,
            display_manager.is_sleeping() ? portMAX_DELAY : pdMS_TO_TICKS(10));

        uint32_t display_events =
            events & (BOOT_BUTTON_CLICK_EVENT |
                      SCREEN_OFF_EVENT);

        uint32_t app_events =
            events & ~(
                         BOOT_BUTTON_CLICK_EVENT |
                         SCREEN_OFF_EVENT);

        if (display_events & BOOT_BUTTON_CLICK_EVENT)
        {
            // usb_serial.println("BOOT_BUTTON_CLICK_EVENT");
            if (display_manager.is_sleeping())
            {
                display_manager.wake();
            }
            else
            {
                display_manager.sleep();
            }

            // usb_serial.printf("APP_MAIN :: GUI Task %d\n", display_manager.is_sleeping());
        }
        else if (display_events & SCREEN_OFF_EVENT)
        {
            // usb_serial.println("SCREEN_OFF_EVENT");
            display_manager.sleep();
        }

        if (app_events)
        {
            screen_manager.handle_events(app_events);
            power_saver_manager.handle_events(app_events);
        }

        if (!display_manager.is_sleeping())
        {
            uint32_t wait = lv_timer_handler();

            if (wait < 5)
                wait = 5;
            if (wait > 100)
                wait = 100;

            vTaskDelay(pdMS_TO_TICKS(wait));
        }
    }
}

// ==========================================================
// MAIN SETUP
// ==========================================================
void setup()
{

    usb_serial.begin(115200);
    delay(1000);

    usb_serial.println("Booting NSMARTWATCH...");

    // Configure physical BOOT button (GPIO0) as input with pullup and interrupt
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BOOT_BUTTON_PIN), bootButtonISR, FALLING);

    // ── Automatic light sleep
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 80,// matches board's normal running clock
        .min_freq_mhz = 40,  // XTAL-derived floor - safe default for automatic light sleep on ESP32-S3
        .light_sleep_enable = light_sleep_enable};
    esp_err_t pm_ret = esp_pm_configure(&pm_config);
    if (pm_ret != ESP_OK)
    {
        usb_serial.printf("[PM] esp_pm_configure failed (%d)\n", pm_ret);
    }
    else
    {
        usb_serial.println("[PM] Automatic light sleep enabled (BLE-safe)");
    }

    esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "sleep_lock", &sleep_lock);
    esp_pm_lock_acquire(sleep_lock);

    esp_sleep_enable_gpio_wakeup();
    gpio_wakeup_enable((gpio_num_t)BOOT_BUTTON_PIN, GPIO_INTR_LOW_LEVEL);


    i2c_mutex = xSemaphoreCreateMutex();
    usb_serial.println("I2C Mutex initialised");

    Wire.begin(IIC_SDA, IIC_SCL);
    Wire.setClock(100000);

    esp_err_t ret = battery_manager.init();
    if (ret != ESP_OK)
    {
        usb_serial.printf(
            "Battery manager initialization failed: %s\n",
            esp_err_to_name(ret));
    }
    else
    {
        usb_serial.println("Battery manager initialized successfully");
    }

    if (!gfx->begin())
    {
        usb_serial.println("Screen init failed!");
    }
    gfx->fillScreen(RGB565_BLACK);
    gfx->setTextSize(2);
    gfx->setTextColor(0xFFFF);
    gfx->setCursor(100, 251);
    gfx->printf("Loading System.....");

    while (FT3168->begin() == false)
    {
        usb_serial.println("FT3168 initialization fail");
        delay(2000);
    }
    //   usb_serial.println("FT3168 initialization successfully");

    FT3168->IIC_Write_Device_State(
        FT3168->Arduino_IIC_Touch::Device::TOUCH_POWER_MODE,
        FT3168->Arduino_IIC_Touch::Device_Mode::TOUCH_POWER_MONITOR);

    rtc_manager.init();

    lvgl_manager_init();

    power_saver_manager.init();

    screen_manager.init();

    // Create Background Sensor Monitoring Task, pinned to Core 0
    xTaskCreatePinnedToCore(
        background_func,
        "BACKGROUND",
        12288,
        NULL,
        1,
        &background_task_handle,
        0);

    // Create GUI Task, pinned to Core 1
    xTaskCreatePinnedToCore(
        gui_func,
        "GUI",
        16384,
        NULL,
        2,
        &gui_task_handle,
        1);

    vTaskDelete(NULL);
}

void loop() {}
