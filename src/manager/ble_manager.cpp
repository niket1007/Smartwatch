#include "ble_manager.h"
#include <NimBLEDevice.h>

// ---------------------------------------------------------------------
// CONFIG
// ---------------------------------------------------------------------
// MUST start with "Bangle.js" - this is literally how Gadgetbridge decides
// which driver to use when it sees the device while scanning.
#define DEVICE_NAME   "Bangle.js 8f3b"
#define SERVICE_UUID  "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_RX_UUID  "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // Gadgetbridge -> watch
#define CHAR_TX_UUID  "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // watch -> Gadgetbridge

// File-scope pointers - these need to be reachable from callbacks and from
// other functions (e.g. ble_is_connected(), future battery/time-sync replies).
// FIX: previously pServer was shadowed by a local variable of the same name
// inside ble_manager_init(), so this global was never actually assigned.
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
        usb_serial.printf("Client connected:\n%s", connInfo.toString().c_str());
        pServer->updateConnParams(connInfo.getConnHandle(), 80, 160, 0, 300);
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        usb_serial.printf("Client disconnected (Reason: %d) - start advertising\n", reason);
        NimBLEDevice::startAdvertising();
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

    // EXTENSION POINT - display rendering.
    // You already have `gfx` initialized (saw the fillScreen test in your
    // onWrite), so this is where a real draw call goes, e.g.:
    //   showNotificationOnScreen(app, title, body);
    gfx->fillScreen(0x07E0);   // keeping your visual "got something" flash for now
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

    pServer = NimBLEDevice::createServer();   // FIX: assigns the global, not a shadow
    pServer->setCallbacks(&serverCallbacks);
    usb_serial.println("BLE: Server Created");

    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    // TX (watch -> phone): battery, HR, etc. later - stored globally so
    // other functions can notify() on it, unlike before.
    pTxCharacteristic = pService->createCharacteristic(
        CHAR_TX_UUID,
        NIMBLE_PROPERTY::NOTIFY
    );

    // RX (phone -> watch): notifications, weather, etc.
    NimBLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
        CHAR_RX_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    pRxCharacteristic->setCallbacks(&ble_callbacks);

    pService->start();
    usb_serial.println("BLE: Service Started");

    ble_start_advertising();
}

void ble_start_advertising() {
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    // FIX: use the same Bangle.js name here - previously this overrode the
    // name set in NimBLEDevice::init() with a non-matching one.
    pAdvertising->setName(DEVICE_NAME);
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->enableScanResponse(true);   // required for some Android phones to see name/services
    pAdvertising->start();

    usb_serial.println("BLE: Advertising Started as " DEVICE_NAME);
}

bool ble_is_connected() {
    if (pServer == nullptr) return false;   // defensive - avoids the earlier crash if called too early
    return pServer->getConnectedCount() > 0;
}
