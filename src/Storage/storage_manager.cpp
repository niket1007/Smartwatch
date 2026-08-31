#include "storage_manager.h"
#include "Common/globals.h"


void StorageManager::store_wifi_credentials()
{
    std::string ssid = wifi_manager.get_ssid();
    std::string password = wifi_manager.get_password();

    preferences.begin("WIFI", false);
    preferences.putString("SSID", ssid.c_str());
    preferences.putString("PASSWORD", password.c_str());
    preferences.end();

    // usb_serial.println("store_wifi_credentials called");
}

std::array<std::string, 2> StorageManager::fetch_wifi_credentials()
{
    preferences.begin("WIFI", true);
    std::string ssid = preferences.getString("SSID", "").c_str();
    std::string password = preferences.getString("PASSWORD", "").c_str();
    preferences.end();

    return {ssid, password};
}