#include "navigation_screen.h"
#include "Screen/generated/ui.h"
#include "Screen/generated/images.h"
#include "Common/globals.h"
#include "Common/Custom_Data/Navigation/nav_data.h"

#include "esp_log.h"
#include <cctype>

static constexpr const char *TAG = "NAVIGATION_SCREEN";

esp_err_t NavigationScreen::on_enter()
{
    {
        std::string action = nav_data.get_directions();

        lv_label_set_text(
            objects.direction_label, action.c_str());
        
        lv_image_set_src(objects.direction_icon, get_direction_icon(action));
    }

    {
        std::string dist = std::to_string(nav_data.get_distance()) + " meter";
        lv_label_set_text(
            objects.distance_label, dist.c_str());
    }

    {
        lv_label_set_text(
            objects.direction_instr_label, nav_data.get_instructions().c_str());
    }

    {
        lv_label_set_text(
            objects.eta_label, nav_data.get_eta().c_str());
    }

    return ESP_OK;
}

const lv_image_dsc_t *NavigationScreen::get_direction_icon(std::string action)
{
    if (action == "continue")
    {
        return &img_arrow_up;
    }
    else if (action == "left")
    {
        return &img_arrow_left;
    }
    else if (action == "left_slight")
    {
        return &img_slight_left;
    }
    else if (action == "left_sharp")
    {
        return &img_sharp_left;
    }
    else if (action == "right")
    {
        return &img_arrow_right;
    }
    else if (action == "right_slight")
    {
        return &img_slight_right;
    }
    else if (action == "right_sharp")
    {
        return &img_sharp_right;
    }
    else if (action == "keep_left")
    {
        return &img_keep_left;
    }
    else if (action == "keep_right")
    {
        return &img_keep_right;
    }
    else if (action == "uturn_left")
    {
        return &img_u_turn_left;
    }
    else if (action == "uturn_right")
    {
        return &img_u_turn_right;
    }
    else if (action == "offroute")
    {
        return &img_offroute;
    }
    else if (action == "roundabout_left")
    {
        return &img_roundabout_left;
    }
    else if (action == "roundabout_right")
    {
        return &img_roundabout_right;
    }
    else if (action == "roundabout_straight")
    {
        return &img_roundabout_straight;
    }
    else if (action == "roundabout_uturn")
    {
        return &img_roundabout_uturn;
    }
    else if (action == "finish")
    {
        return &img_nav_finish;
    }
    else
    {
        return &img_navigation;
    }
}

esp_err_t NavigationScreen::update()
{
    if (nav_data.is_distance_diirty())
    {
        std::string dist = std::to_string(nav_data.get_distance()) + " meter";
        lv_label_set_text(
            objects.distance_label, dist.c_str());
    }

    if (nav_data.is_eta_diirty())
    {
        lv_label_set_text(
            objects.eta_label, nav_data.get_eta().c_str());
    }

    if (nav_data.is_instr_dirty())
    {
        lv_label_set_text(
            objects.direction_instr_label, nav_data.get_instructions().c_str());
    }

    if (nav_data.is_direction_diirty())
    {
        std::string action = nav_data.get_directions();

        lv_label_set_text(
            objects.direction_label, action.c_str());
        
        lv_image_set_src(objects.direction_icon, get_direction_icon(action));
    }
    return ESP_OK;
}

esp_err_t NavigationScreen::on_exit()
{
    // ESP_LOGI(TAG, "on_exit called");

    return ESP_OK;
}

esp_err_t NavigationScreen::handle_events(uint32_t events)
{
    if (events & (SWIPE_LEFT_EVENT | SWIPE_RIGHT_EVENT))
    {
        // Menu Screen 1
        return screen_manager.change_screen(SCREEN_ID_MENU_SCREEN_ONE);
    }

    if (events & NAV_SCREEN_UDPATE_EVENT)
    {
        return update();
    }
    return ESP_OK;
}