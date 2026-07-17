#include "ble_manager.h"
#include "navigation_manager.h"
#include "notification_manager.h"
#include "weather_manager.h"
#include "call_screen_manager.h"
#include <NimBLEDevice.h>

// ---------------------------------------------------------------------
// CONFIG
// ---------------------------------------------------------------------
#define DEVICE_NAME   "NSmartWatch"
#define SERVICE_UUID  "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_RX_UUID  "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // Gadgetbridge -> watch
#define CHAR_TX_UUID  "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // watch -> Gadgetbridge

static NimBLEServer* pServer = nullptr;
static NimBLECharacteristic* pTxCharacteristic = nullptr;

// Accumulates incoming bytes until a full line (ending in '\n') is seen -
// Gadgetbridge can split one message across multiple BLE writes.
static String rxBuffer = "";

// ---------------------------------------------------------------------
// SERVER CALLBACKS
// ---------------------------------------------------------------------
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        // usb_serial.printf("onconnect: Client connected:\n%s", connInfo.toString().c_str());
        ts_var.show_passkey_display = 1;

        if(task_gui_handle != NULL) xTaskNotifyGive(task_gui_handle);

        pServer->updateConnParams(
            connInfo.getConnHandle(), 
            80, // min interval = 80 * 1.25 = 100ms
            160, // max Interval = 160 * 1.25 = 200ms
            10, // Latency
            400 // 400 * 10 = 4000ms [Connection Timeout]
        );
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        // usb_serial.printf("ondisconnect: Client disconnected (Reason: %d) - start advertising\n", reason);
        ts_var.show_passkey_display = 4;
        NimBLEDevice::startAdvertising();
    }

    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
        // Check that encryption was successful, if not we disconnect the client
        if (!connInfo.isEncrypted()) {
            NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
            ts_var.show_passkey_display = 3;
            return;
        }
        ts_var.show_passkey_display = 2;
        // usb_serial.printf("Called onAuthenticationComplete %d", ts_var.show_passkey_display);
    }

};

// ---------------------------------------------------------------------
// NOTIFICATION PARSING
// ---------------------------------------------------------------------
static void handleNotification(String payload) {
    if (payload.startsWith("GB(") && payload.endsWith(")")) {
        payload = payload.substring(4, payload.length() - 1);
    }
    usb_serial.printf("[RAW] %s\n", payload.c_str());

    JsonDocument json_doc;
    DeserializationError error = deserializeJson(json_doc, payload);
    if(error) {
        usb_serial.print(F("deserializeJson() failed: "));
        usb_serial.println(error.f_str());
        return;
    }

    if(!json_doc.isNull() ) {
        String type = json_doc["t"] | "";
        if(type == "musicinfo") {
            /*
                GadgetBridge Payload: 
                {"t":"musicinfo","artist":"Ali & Shjr & Haider Ali","album":"Heer","track":"Heer","dur":250,"c":-1,"n":-1}
            */
            int duration = json_doc["dur"] | 0;
            update_music_notification_details(
                String(json_doc["track"]),
                String(json_doc["artist"]),
                duration
            );
        }
        else if(type == "call") {
            /*
                GadgetBridge Payload
                {"t":"call","cmd":"incoming","name":"XXXX","number":"XXXXXXXXXX"}
                {"t":"call","cmd":"end","name":"XXXX","number":"XXXXXXXXXX"}
                {"t":"call","cmd":"outgoing","name":"XXXX","number":"XXXXXXXXXX"}
            */
            usb_serial.println("---- Call Notification ----");
            usb_serial.println("Type: " + String(json_doc["cmd"]));
            usb_serial.println("Name:   " + String(json_doc["name"]));
            usb_serial.println("Number:  " + String(json_doc["number"]));
            usb_serial.println("-----------------------");

            update_call_notification_details(
                String(json_doc["cmd"]),
                String(json_doc["name"]),
                String(json_doc["number"])
            );
        }
        else if(type == "weather") {
            /*
                GadgetBridge Payload
                {"t":"weather","v":1,"temp":310,"hi":311,"lo":302,"hum":39,
                "rain":20,"uv":4,"code":803,"txt":"Partly cloudy",
                "wind":13.68,"wdir":295,"loc":"Gurgaon, Sushant Lok 2"}
            */
            float temp = json_doc["temp"].as<float>() - 273.15f;
            float hi = json_doc["hi"].as<float>() - 273.15f;
            float low = json_doc["lo"].as<float>() - 273.15f;
            update_weather_details(
                String(temp, 2),// temp provided is in Kelvin and C = K - 273.15
                String(hi, 2),
                String(low, 2),
                String(json_doc["hum"]),
                String(json_doc["rain"]),
                String(json_doc["wind"]),
                String(json_doc["txt"]),
                String(json_doc["loc"])
            );
            
        }
    }

}

// ---------------------------------------------------------------------
// RX/TX CHARACTERISTIC CALLBACKS
// ---------------------------------------------------------------------
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        // usb_serial.printf("%s : onRead(), value: %s\n",
                    //   pCharacteristic->getUUID().toString().c_str(),
                    //   pCharacteristic->getValue().c_str());
    }

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        std::string value = pCharacteristic->getValue();
        if (value.length() == 0) {
            // usb_serial.println("BLE: Received empty data");
            return;
        }
        // usb_serial.printf("Notification: %s\n", value.c_str());

        rxBuffer += value.c_str();

        int newlineIndex;
        while ((newlineIndex = rxBuffer.indexOf('\n')) != -1) {
            String line = rxBuffer.substring(0, newlineIndex);
            rxBuffer = rxBuffer.substring(newlineIndex + 1);
            line.trim();
            if (line.length() == 0) continue;

            if (line.startsWith("GB(")) {
                handleNotification(line);
            }
        }
    }
};

// ---------------------------------------------------------------------
// INIT / ADVERTISING
// ---------------------------------------------------------------------
void ble_manager_init() {
    NimBLEDevice::init(DEVICE_NAME);

    // usb_serial.printf("Bluetooth passkey: %d", gv.ble_passkey);
    NimBLEDevice::setSecurityAuth(true, true, false);
    NimBLEDevice::setSecurityPasskey(gv.ble_passkey);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    pServer = NimBLEDevice::createServer();   // FIX: assigns the global, not a shadow
    if(pServer == nullptr) {
        // usb_serial.println("Bluetooth not initialised");
        return;
    }
    pServer->setCallbacks(new ServerCallbacks());
    // usb_serial.println("BLE: Server Created");

    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    // TX (watch -> Gadgetbridge): battery, HR, etc. later - stored globally so
    // other functions can notify() on it, unlike before.
    pTxCharacteristic = pService->createCharacteristic(
        CHAR_TX_UUID,
        NIMBLE_PROPERTY::NOTIFY
    );

    // RX (Gadgetbridge -> watch): notifications, weather, etc.
    NimBLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
        CHAR_RX_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    pRxCharacteristic->setCallbacks(new CharacteristicCallbacks());

    // usb_serial.println("BLE: Service Started");

    ble_init_advertising();
}

void ble_init_advertising() {
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName(DEVICE_NAME);
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->enableScanResponse(true);
    
    // pAdvertising->setMinInterval(800); // 800 * 0.625 = 500ms
    // pAdvertising->setMaxInterval(800); // 800 * 0.625 = 500ms
    
    pAdvertising->start();

    // usb_serial.println("BLE: Advertising Started as " DEVICE_NAME);
    
    gv.ble_is_powered_on = true;
}

bool ble_is_connected() {
    if (pServer == nullptr) return false;
    if (pTxCharacteristic == nullptr) return false;
    return pServer->getConnectedCount() > 0;
}

void ble_manager_deinit() {
    // 1. Deinitialize the NimBLE stack completely (true = free memory)
    NimBLEDevice::deinit(true); 
    
    // 2. Nullify pointers to prevent dangling references
    pServer = nullptr;
    pTxCharacteristic = nullptr;
    
    // 3. Reset state variables
    ts_var.show_passkey_display = 0;

    gv.ble_is_powered_on = false;
    
    // usb_serial.println("BLE: Stack completely shut down to save battery.");
}


void update_ble_passkey_display() {
    char passkey_str[8];
    snprintf(passkey_str, sizeof(passkey_str), "%d", gv.ble_passkey);

    if(ts_var.show_passkey_display == 1) {
        if(gv.active_screen_id != BLE_PASSKEY_SCREEN) {
            // usb_serial.println("Load BLE Pass key screen");
            navigate_to_screen(BLE_PASSKEY_SCREEN);
        }
        if (objects.ble_passkey_label) {
            lv_label_set_text(objects.ble_passkey_label, passkey_str);
        }
    }
    else if (ts_var.show_passkey_display == 2 || ts_var.show_passkey_display == 3) {
        if (objects.ble_passkey_label) {
            lv_label_set_text(objects.ble_passkey_label,
                               ts_var.show_passkey_display == 2 ? "Paired!" : "Pairing failed");
        }

        if(ts_var.show_passkey_display == 2) {
            ts_var.bluetooth_battery_weather_init = 1;
        }
        ts_var.show_passkey_display = 4;
    }
    else if(ts_var.show_passkey_display == 4) {
        ts_var.show_passkey_display = 0;
        lv_timer_create([](lv_timer_t *timer) {
            navigate_to_screen(HOME_SCREEN);    // Transition back to home
            lv_timer_delete(timer);   // Destroy this one-shot timer
        }, 1000, NULL);
    }
}

bool ble_send_to_phone(JsonDocument json) {
    if(ble_is_connected()) {
        String stringified;
        serializeJson(json, stringified);
        stringified += "\r\n";

        usb_serial.printf("Stringified JSON %s\n", stringified.c_str());
        
        pTxCharacteristic->setValue(stringified.c_str());
        pTxCharacteristic->notify();
        usb_serial.println("notification sent");
        return true;
    }
    return false;
}