#include "wifi_manager.h"

const char* SSID     = "Amit 1st Floor";
const char* PASSWORD = "12345689";
const int WIFI_MAX_ATTEMPTS = 20;

bool connect_to_wifi() {
    int attempts = 0;

    usb_serial.println("Initiating Wifi Connection");
    WiFi.begin(SSID, PASSWORD);
    WiFi.persistent(false);
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_MAX_ATTEMPTS) {
        delay(500);
        usb_serial.print(".");
        attempts++;
    }
    return WiFi.status() == WL_CONNECTED;
}

void disconnect_wifi() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    usb_serial.println("Wi-Fi Powered Down.");
}