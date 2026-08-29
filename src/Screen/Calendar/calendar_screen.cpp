#include "calendar_screen.h"
#include "Screen/Generated/ui.h"
#include "Common/globals.h"

static constexpr const char *TAG = "CALENDAR_SCREEN";


esp_err_t CalendarScreen::on_enter()
{
    RTC_DateTime datetime = rtc_manager.get_current_datetime();
    
    if(objects.calendar_calendar) {
            lv_calendar_set_today_date(
                objects.calendar_calendar, 
                datetime.getYear(), datetime.getMonth(), datetime.getDay());
            lv_calendar_set_month_shown(
                objects.calendar_calendar, datetime.getYear(), datetime.getMonth());
        
    }

    return ESP_OK;
}


esp_err_t CalendarScreen::on_exit()
{
    return ESP_OK;
}


esp_err_t CalendarScreen::handle_events(uint32_t events)
{
    if(events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Menu Screen 1
        return screen_manager.change_screen(SCREEN_ID_MENU_SCREEN_ONE);
    }
    return ESP_OK;
}