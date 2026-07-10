#include "ble_manager.h"
#include "navigation_manager.h"
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
volatile uint32_t show_passkey_display = 0;

// Accumulates incoming bytes until a full line (ending in '\n') is seen -
// Gadgetbridge can split one message across multiple BLE writes.
static String rxBuffer = "";

// ---------------------------------------------------------------------
// SERVER CALLBACKS
// ---------------------------------------------------------------------
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        usb_serial.printf("onconnect: Client connected:\n%s", connInfo.toString().c_str());
        show_passkey_display = 1;

        if(task_gui_handle != NULL) xTaskNotifyGive(task_gui_handle);
        
        pServer->updateConnParams(connInfo.getConnHandle(), 80, 160, 0, 300);
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        usb_serial.printf("ondisconnect: Client disconnected (Reason: %d) - start advertising\n", reason);
        show_passkey_display = 4;
        NimBLEDevice::startAdvertising();
    }

    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
        // Check that encryption was successful, if not we disconnect the client
        if (!connInfo.isEncrypted()) {
            NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
            show_passkey_display = 3;
            return;
        }
        show_passkey_display = 2;
        usb_serial.printf("Called onAuthenticationComplete %d", show_passkey_display);
    }

} serverCallbacks;

// ---------------------------------------------------------------------
// NOTIFICATION PARSING
// ---------------------------------------------------------------------
// Bangle.js payloads look like:
//   GB({t:"notify",id:1,src:"WhatsApp",title:"Alice",body:"Hey there"})
// Unquoted keys, so not strict JSON - simple string search is enough for now.

static String extractField(const String &payload, const String &key) {
    String pattern = key + ":\"";
    int start = payload.indexOf(pattern);
    if (start == -1) return "";
    start += pattern.length();
    int end = payload.indexOf("\"", start);
    if (end == -1) return "";
    return payload.substring(start, end);
}

static void handleNotification(const String &payload) {
    String app   = extractField(payload, "src");
    String title = extractField(payload, "title");
    String body  = extractField(payload, "body");

    usb_serial.println("---- Notification ----");
    usb_serial.println("App:   " + app);
    usb_serial.println("Title: " + title);
    usb_serial.println("Body:  " + body);
    usb_serial.println("-----------------------");

    // gfx->fillScreen(0x07E0); 
}

// ---------------------------------------------------------------------
// RX/TX CHARACTERISTIC CALLBACKS
// ---------------------------------------------------------------------
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        usb_serial.printf("%s : onRead(), value: %s\n",
                      pCharacteristic->getUUID().toString().c_str(),
                      pCharacteristic->getValue().c_str());
    }

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        std::string value = pCharacteristic->getValue();
        if (value.length() == 0) {
            usb_serial.println("BLE: Received empty data");
            return;
        }

        rxBuffer += value.c_str();

        int newlineIndex;
        while ((newlineIndex = rxBuffer.indexOf('\n')) != -1) {
            String line = rxBuffer.substring(0, newlineIndex);
            rxBuffer = rxBuffer.substring(newlineIndex + 1);
            line.trim();
            if (line.length() == 0) continue;

            usb_serial.printf("[RAW] %s\n", line.c_str());

            if (line.startsWith("GB(")) {
                handleNotification(line);
            }
            else {
                usb_serial.println(line);
            }
            // EXTENSION POINT - other Gadgetbridge commands (time sync,
            // battery query, etc.) go here as additional else-if branches.
        }
    }
} ble_callbacks;

// ---------------------------------------------------------------------
// INIT / ADVERTISING
// ---------------------------------------------------------------------
void ble_manager_init() {
    NimBLEDevice::init(DEVICE_NAME);

    usb_serial.printf("Bluetooth passkey: %d", ble_passkey);
    NimBLEDevice::setSecurityAuth(true, true, false);
    NimBLEDevice::setSecurityPasskey(ble_passkey);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    pServer = NimBLEDevice::createServer();   // FIX: assigns the global, not a shadow
    if(pServer == nullptr) {
        usb_serial.println("Bluetooth not initialised");
        return;
    }
    pServer->setCallbacks(&serverCallbacks);
    usb_serial.println("BLE: Server Created");

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
    pRxCharacteristic->setCallbacks(&ble_callbacks);

    usb_serial.println("BLE: Service Started");

    ble_start_advertising();
}

void ble_start_advertising() {
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName(DEVICE_NAME);
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->enableScanResponse(true);
    
    pAdvertising->setMinInterval(800); // 800 * 0.625 = 500ms
    pAdvertising->setMaxInterval(800); // 800 * 0.625 = 500ms
    
    pAdvertising->start();

    usb_serial.println("BLE: Advertising Started as " DEVICE_NAME);
}

bool ble_is_connected() {
    if (pServer == nullptr) return false;
    return pServer->getConnectedCount() > 0;
}

void ble_manager_deinit() {
    // 1. Deinitialize the NimBLE stack completely (true = free memory)
    NimBLEDevice::deinit(true); 
    
    // 2. Nullify pointers to prevent dangling references
    pServer = nullptr;
    pTxCharacteristic = nullptr;
    
    // 3. Reset state variables
    show_passkey_display = 0;
    
    usb_serial.println("BLE: Stack completely shut down to save battery.");
}


void update_ble_passkey_display() {
    char passkey_str[8];
    snprintf(passkey_str, sizeof(passkey_str), "%d", ble_passkey);

    if(show_passkey_display == 1) {
        if(active_screen != BLE_PASSKEY_SCREEN) {
            usb_serial.println("Load BLE Pass key screen");
            navigate_to_screen(BLE_PASSKEY_SCREEN);
        }
        if (objects.ble_passkey_label) {
            lv_label_set_text(objects.ble_passkey_label, passkey_str);
        }
    }
    else if (show_passkey_display == 2 || show_passkey_display == 3) {
        if (objects.ble_passkey_label) {
            lv_label_set_text(objects.ble_passkey_label,
                               show_passkey_display == 2 ? "Paired!" : "Pairing failed");
        }
        show_passkey_display = 4;
    }
    else if(show_passkey_display == 4) {
        lv_timer_create([](lv_timer_t *timer) {
            show_passkey_display = 0; // Reset global state inside the delayed callback
            navigate_to_screen(HOME_SCREEN);    // Transition back to home
            lv_timer_delete(timer);   // Destroy this one-shot timer
        }, 1000, NULL);
    }
}