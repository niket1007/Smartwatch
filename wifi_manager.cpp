#include "wifi_manager.h"

const char* ssid     = "Amit 1st Floor";
const char* password = "12345689";

bool connect_to_wifi() {
    int attempts = 0;

    USBSerial.println("Initiating Wifi Connection");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        USBSerial.print(".");
        attempts++;
    }
    return WiFi.status() == WL_CONNECTED;
}

void disconnect_wifi() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    USBSerial.println("Wi-Fi Powered Down.");
}