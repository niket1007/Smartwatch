#include "bluetooth_manager.h"
#include "esp_log.h"

static const char* TAG = "BLUETOOTH_MANAGER";

String BluetoothManager::rxBuffer = "";
std::string BluetoothManager::connected_device_name = "";

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
        ESP_LOGI(
            TAG,
            "BLE CONNECTED addr=%s handle=%d bonded=%d encrypted=%d",
            connInfo.getAddress().toString().c_str(),
            connInfo.getConnHandle(),
            connInfo.isBonded(),
            connInfo.isEncrypted());

        BluetoothManager::connected_device_name = connInfo.getAddress().toString();

        // FIX: Modern Android and Gadgetbridge need latency at 0 during initialization 
        // to complete handshakes seamlessly. Relax the bounds slightly.
        pServer->updateConnParams(
            connInfo.getConnHandle(),
            24,     // 24 * 1.25 = 30 ms (min)
            40,     // 40 * 1.25 = 50 ms (max)
            0,      // Latency = 0 for stability during security handshakes
            600     // 600 * 10 = 6000 ms timeout
        );
    }

    void onDisconnect(
        NimBLEServer* pServer,
        NimBLEConnInfo& connInfo,
        int reason) override
    {
        ESP_LOGW(
            TAG,
            "BLE DISCONNECTED addr=%s reason=%d (%s)",
            connInfo.getAddress().toString().c_str(),
            reason,
            NimBLEUtils::returnCodeToString(reason));

        NimBLEDevice::whiteListAdd(connInfo.getAddress());
        
        BluetoothManager::connected_device_name.clear();
        BluetoothManager::rxBuffer.clear();

        // FIX: Manually restart advertising safely rather than using automated loops
        NimBLEDevice::startAdvertising();
    }

    void onAuthenticationComplete(
        NimBLEConnInfo& connInfo) override
    {
        if (!connInfo.isEncrypted())
        {
            ESP_LOGE(TAG, "BLE authentication failed");
            NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
            return;
        }
        ESP_LOGI(TAG, "BLE authentication successful");
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
                ESP_LOGI(TAG, "%s", line.c_str());
            }
        }
    }

    void onSubscribe(
        NimBLECharacteristic* pCharacteristic,
        NimBLEConnInfo& connInfo,
        uint16_t subValue) override
    {
        ESP_LOGI(
            TAG,
            "Subscription changed: device=%s value=%u",
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
        ESP_LOGI(TAG, "Bluetooth already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing Bluetooth");

    NimBLEDevice::init(BLE_DEVICE_NAME);

    NimBLEDevice::setSecurityAuth(
        true,   // bonding
        true,   // MITM
        false   // secure connections
    );

    NimBLEDevice::setSecurityPasskey(101010);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    pServer = NimBLEDevice::createServer();
    if (pServer == nullptr)
    {
        ESP_LOGE(TAG, "Failed to create BLE server");
        return ESP_FAIL;
    }

    pServer->setCallbacks(&server_callbacks);

    NimBLEService* pService = pServer->createService(BLE_SERVICE_UUID);
    if (pService == nullptr)
    {
        ESP_LOGE(TAG, "Failed to create BLE service");
        return ESP_FAIL;
    }

    pTxCharacteristic = pService->createCharacteristic(
        BLE_CHAR_TX_UUID, NIMBLE_PROPERTY::NOTIFY);

    if (pTxCharacteristic == nullptr)
    {
        ESP_LOGE(TAG, "Failed to create TX characteristic");
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
        ESP_LOGE(TAG, "Failed to create RX characteristic");
        return ESP_FAIL;
    }
    pRxCharacteristic->setCallbacks(&characteristic_callbacks);

    pService->start();

    esp_err_t result = init_advertising();
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start BLE advertising");
        return result;
    }

    is_initialised = true;
    ESP_LOGI(TAG,"Bluetooth initialized successfully");

    return ESP_OK;
}

esp_err_t BluetoothManager::init_advertising()
{
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();

    if (pAdvertising == nullptr)
    {
        ESP_LOGE(TAG, "BLE advertising object is null");
        return ESP_FAIL;
    }

    pAdvertising->setName(BLE_DEVICE_NAME);
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->enableScanResponse(true);

    pAdvertising->setScanFilter(false, false);

    if (!pAdvertising->start())
    {
        ESP_LOGE(TAG, "Failed to start BLE advertising");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "BLE advertising started as %s", BLE_DEVICE_NAME);
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
        ESP_LOGW(TAG, "Cannot send BLE data: no client connected");
        return ESP_ERR_INVALID_STATE;
    }

    if (pTxCharacteristic == nullptr)
    {
        ESP_LOGE(TAG, "TX characteristic is null");
        return ESP_FAIL;
    }

    String payload;
    serializeJson(json, payload);
    payload += "\r\n";

    ESP_LOGI(TAG, "Sending BLE data: %s", payload.c_str());

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

    ESP_LOGI(TAG, "Deinitializing Bluetooth");
    NimBLEDevice::deinit(true);

    pServer = nullptr;
    pTxCharacteristic = nullptr;
    connected_device_name.clear();
    rxBuffer.clear();
    is_initialised = false;

    ESP_LOGI(TAG, "Bluetooth deinitialized");
    return ESP_OK;
}
