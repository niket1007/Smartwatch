#include "display_manager.h"
#include "Common/globals.h"

void DisplayManager::init()
{
    set_brightness(70);
    reset_screen_timeout_timer();
}

void DisplayManager::set_brightness(uint32_t brightness_percent) {
    // Send command 0x51 (brightness register) followed by the brightness value
    uint32_t brightness_val = (brightness_percent * 255)/100;
    brightness_val = brightness_val > 255u ? 255u : brightness_val;

    bus->beginWrite();
    bus->writeC8D8(0x51, brightness_val);
    bus->endWrite();
}

esp_err_t DisplayManager::sleep()
{
    if (current_sleep_status) return ESP_OK;

    set_brightness(0); 

    bus->beginWrite();
    bus->writeCommand(0x28);   // Display OFF
    bus->endWrite();

    vTaskDelay(pdMS_TO_TICKS(20));

    bus->beginWrite();
    bus->writeCommand(0x10);   // Sleep In
    bus->endWrite();

    current_sleep_status = true;

    esp_pm_lock_release(sleep_lock);

    return ESP_OK;
}

esp_err_t DisplayManager::wake()
{
    if (!current_sleep_status) return ESP_OK;
    
    esp_pm_lock_acquire(sleep_lock);

    bus->beginWrite();
    bus->writeCommand(0x11); // Amoled Sleep Out
    bus->endWrite();
    
    vTaskDelay(pdMS_TO_TICKS(150)); // Give some time for display wake up

    bus->beginWrite();
    bus->writeCommand(0x29); // Amoled Display On
    bus->endWrite();

    set_brightness(power_saver_manager.get_brightness_percentage());
    
    current_sleep_status = false;

    vTaskDelay(pdMS_TO_TICKS(150)); 

    if (background_task_handle != nullptr)
    {
        xTaskNotify(
            background_task_handle, BG_WAKE_UP, eSetBits);
    }

    if (gui_task_handle != nullptr)
    {
        xTaskNotify(
            gui_task_handle, 
            UPDATE_TIME_EVENT | BAT_PERCENT_CHG_EVENT | BAT_CHARGE_CHG_EVENT, eSetBits);
    }
    return ESP_OK;
}


bool DisplayManager::is_sleeping()
{
    return current_sleep_status;
}
unsigned long DisplayManager::get_screen_timeout_timer()
{
    return screen_timeout_timer_;
}

void DisplayManager::reset_screen_timeout_timer(unsigned long timer)
{
    if (timer == 0)
        timer = millis();

    screen_timeout_timer_ = timer;
}