#include "bluetooth_manager.h"
#include "Common/globals.h"

static const char* TAG = "BLUETOOTH_MANAGER";

String BluetoothManager::rxBuffer = "";
std::string BluetoothManager::connected_device_name = "";
BLE_STATUS BluetoothManager::ble_status = BLE_STATUS::NOTHING;

// =====================================================================
// BLE SERVER CALLBACKS
// =====================================================================

class ServerCallbacks : public NimBLEServerCallbacks
{
public:

    void onConnect(
        NimBLEServer* pServer,
        NimBLEConnInfo& connInfo) override
    {
        usb_serial.printf(
            "BLE CONNECTED addr=%s handle=%d bonded=%d encrypted=%d\n",
            connInfo.getAddress().toString().c_str(),
            connInfo.getConnHandle(),
            connInfo.isBonded(),
            connInfo.isEncrypted());

        BluetoothManager::connected_device_name = connInfo.getAddress().toString();
        
        pServer->updateConnParams(
            connInfo.getConnHandle(),
            160,     // 160 * 1.25 = 200 ms (min)
            320,     // 320 * 1.25 = 400 ms (max)
            1,      // Latency = 1 for stability during security handshakes
            1000     // 1000 * 10 = 10000 ms timeout
        );
        BluetoothManager::ble_status = BLE_STATUS::PAIRING;
        if(gui_task_handle != nullptr)
        {
            xTaskNotify(gui_task_handle, BLE_STATUS_EVENT, eSetBits);
        }
    }

    void onDisconnect(
        NimBLEServer* pServer,
        NimBLEConnInfo& connInfo,
        int reason) override
    {
        usb_serial.printf(
            "BLE DISCONNECTED addr=%s reason=%d (%s)\n",
            connInfo.getAddress().toString().c_str(),
            reason,
            NimBLEUtils::returnCodeToString(reason));

        NimBLEDevice::whiteListAdd(connInfo.getAddress());
        
        BluetoothManager::connected_device_name.clear();
        BluetoothManager::rxBuffer.clear();

        NimBLEDevice::startAdvertising();        
        
        BluetoothManager::ble_status = BLE_STATUS::DISCONNECTED;
        if(gui_task_handle != nullptr)
        {
            xTaskNotify(gui_task_handle, BLE_STATUS_EVENT, eSetBits);
        }
    }

    void onAuthenticationComplete(
        NimBLEConnInfo& connInfo) override
    {
        if (!connInfo.isEncrypted())
        {
            usb_serial.println("BLE authentication failed");
            
            NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
            
            BluetoothManager::ble_status = BLE_STATUS::PAIRTING_FAILED;
        }
        else 
        {
            BluetoothManager::ble_status = BLE_STATUS::PAIRED;
            usb_serial.println("BLE authentication successful");
        }
        
        if(gui_task_handle != nullptr)
        {
            xTaskNotify(gui_task_handle, BLE_STATUS_EVENT, eSetBits);
        }
        
        return;
    }
};

class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
public:
    void onWrite(
        NimBLECharacteristic* pCharacteristic,
        NimBLEConnInfo& connInfo) override
    {
        std::string value = pCharacteristic->getValue();
        if (value.empty()) return;

        BluetoothManager::rxBuffer += value.c_str();

        int newlineIndex;
        while ((newlineIndex = BluetoothManager::rxBuffer.indexOf('\n')) != -1) {
            String line = BluetoothManager::rxBuffer.substring(0, newlineIndex);
            BluetoothManager::rxBuffer = BluetoothManager::rxBuffer.substring(newlineIndex + 1);
            line.trim();
            if (line.length() == 0) continue;

            if (line.startsWith("GB(")) 
            {
                usb_serial.printf("%s\n", line.c_str());
            }
        }
    }

    void onSubscribe(
        NimBLECharacteristic* pCharacteristic,
        NimBLEConnInfo& connInfo,
        uint16_t subValue) override
    {
        usb_serial.printf(
            "Subscription changed: device=%s value=%u\n",
            connInfo.getAddress().toString().c_str(),
            subValue
        );
    }
};

static ServerCallbacks server_callbacks;
static CharacteristicCallbacks characteristic_callbacks;

bool BluetoothManager::is_init() const
{
    return is_initialised;
}

esp_err_t BluetoothManager::init()
{
    if (is_initialised)
    {
        usb_serial.println("Bluetooth already initialized");
        return ESP_OK;
    }

    usb_serial.println("Initializing Bluetooth");

    NimBLEDevice::init(BLE_DEVICE_NAME);

    NimBLEDevice::setSecurityAuth(
        true,   // bonding
        true,   // MITM
        false   // secure connections
    );

    passkey = 100000 + (esp_random() % 900000);
    usb_serial.printf("PassKey: %d\n", passkey);

    NimBLEDevice::setSecurityPasskey(passkey);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    pServer = NimBLEDevice::createServer();
    if (pServer == nullptr)
    {
        init_errored = true;
        usb_serial.println("Failed to create BLE server");
        return ESP_FAIL;
    }

    pServer->setCallbacks(&server_callbacks);

    NimBLEService* pService = pServer->createService(BLE_SERVICE_UUID);
    if (pService == nullptr)
    {
        init_errored = true;
        usb_serial.println("Failed to create BLE service");
        return ESP_FAIL;
    }

    pTxCharacteristic = pService->createCharacteristic(
        BLE_CHAR_TX_UUID, NIMBLE_PROPERTY::NOTIFY);

    if (pTxCharacteristic == nullptr)
    {
        init_errored = true;
        usb_serial.println("Failed to create TX characteristic");
        return ESP_FAIL;
    }
    pTxCharacteristic->setCallbacks(&characteristic_callbacks);

    NimBLECharacteristic* pRxCharacteristic =
        pService->createCharacteristic(
            BLE_CHAR_RX_UUID,
            NIMBLE_PROPERTY::WRITE |
            NIMBLE_PROPERTY::WRITE_NR
        );

    if (pRxCharacteristic == nullptr)
    {
        init_errored = true;
        usb_serial.println("Failed to create RX characteristic");
        return ESP_FAIL;
    }
    pRxCharacteristic->setCallbacks(&characteristic_callbacks);

    pService->start();

    esp_err_t result = init_advertising();
    if (result != ESP_OK)
    {
        init_errored = true;
        usb_serial.println("Failed to start BLE advertising");
        return result;
    }

    is_initialised = true;
    usb_serial.println("Bluetooth initialized successfully");

    init_errored = false;
    return ESP_OK;
}

esp_err_t BluetoothManager::init_advertising()
{
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();

    if (pAdvertising == nullptr)
    {
        init_errored = true;
        usb_serial.println("BLE advertising object is null");
        return ESP_FAIL;
    }

    pAdvertising->setName(BLE_DEVICE_NAME);
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->enableScanResponse(true);

    pAdvertising->setScanFilter(false, false);

    if (!pAdvertising->start())
    {
        init_errored = true;
        usb_serial.println("Failed to start BLE advertising");
        return ESP_FAIL;
    }

    usb_serial.printf("BLE advertising started as %s\n", BLE_DEVICE_NAME);
    init_errored = false;
    return ESP_OK;
}

bool BluetoothManager::is_connected() const
{
    if (pServer == nullptr)
    {
        return false;
    }
    return pServer->getConnectedCount() > 0;
}

std::string BluetoothManager::get_connected_device_name() const
{
    return connected_device_name;
}

int BluetoothManager::get_passkey()
{
    return passkey;
}

BLE_STATUS BluetoothManager::get_ble_status()
{
    return ble_status;
}

esp_err_t BluetoothManager::handle_events()
{
    if (!is_initialised)
    {
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}

esp_err_t BluetoothManager::send_to_phone(const JsonDocument& json)
{
    if (!is_connected())
    {
        usb_serial.println("Cannot send BLE data: no client connected");
        return ESP_ERR_INVALID_STATE;
    }

    if (pTxCharacteristic == nullptr)
    {
        usb_serial.println("TX characteristic is null");
        return ESP_FAIL;
    }

    String payload;
    serializeJson(json, payload);
    payload += "\r\n";

    usb_serial.printf("Sending BLE data: %s\n", payload.c_str());

    pTxCharacteristic->setValue(payload.c_str());
    pTxCharacteristic->notify();

    return ESP_OK;
}

esp_err_t BluetoothManager::deinit()
{
    if (!is_initialised)
    {
        return ESP_OK;
    }

    usb_serial.println("Deinitializing Bluetooth");
    
    bool ret = NimBLEDevice::deinit(true);
    if(!ret)
    {
        deinit_errored = true;
    }
    else
    {
        deinit_errored = false;
    }

    pServer = nullptr;
    pTxCharacteristic = nullptr;
    connected_device_name.clear();
    rxBuffer.clear();
    is_initialised = false;

    usb_serial.println("Bluetooth deinitialized");
    return ESP_OK;
}
