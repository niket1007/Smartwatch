#include "bluetooth_manager.h"
#include "Common/globals.h"

#include "host/ble_store.h"

#include "esp_random.h"

#include "esp_log.h"
#include "esp_check.h"
#include "esp_nimble_hci.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"

#include "host/ble_hs_mbuf.h"
#include "host/ble_store.h"
#include "host/ble_sm.h"

#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// #include <cjson/cJSON.h>
#include "cJSON.h"

#include <cstring>
#include <string>
#include <algorithm>
#include <inttypes.h>

// Custom Data
#include "Common/Custom_Data/Call/call_data.h"
#include "Common/Custom_Data/Weather/weather_data.h"

static constexpr char *TAG = "BLUETOOTH_MANAGER";

extern "C" void ble_store_config_init(void);

const ble_uuid128_t BluetoothManager::SERVICE_UUID =
    BLE_UUID128_INIT(
        0x9e, 0xca, 0xdc, 0x24,
        0x0e, 0xe5, 0xa9, 0xe0,
        0x93, 0xf3, 0xa3, 0xb5,
        0x01, 0x00, 0x40, 0x6e);

const ble_uuid128_t BluetoothManager::RX_UUID =
    BLE_UUID128_INIT(
        0x9e, 0xca, 0xdc, 0x24,
        0x0e, 0xe5, 0xa9, 0xe0,
        0x93, 0xf3, 0xa3, 0xb5,
        0x02, 0x00, 0x40, 0x6e);

const ble_uuid128_t BluetoothManager::TX_UUID =
    BLE_UUID128_INIT(
        0x9e, 0xca, 0xdc, 0x24,
        0x0e, 0xe5, 0xa9, 0xe0,
        0x93, 0xf3, 0xa3, 0xb5,
        0x03, 0x00, 0x40, 0x6e);

char BluetoothManager::rx_buffer[BluetoothManager::RX_BUFFER_SIZE];

size_t BluetoothManager::rx_buffer_length = 0;

uint16_t BluetoothManager::connection_handle =
    BLE_HS_CONN_HANDLE_NONE;

uint16_t BluetoothManager::tx_value_handle = 0;

bool BluetoothManager::tx_notifications_enabled = false;

uint8_t BluetoothManager::own_addr_type =
    BLE_OWN_ADDR_PUBLIC;

uint32_t BluetoothManager::passkey = 0;

// bool BluetoothManager::is_connected = false;

BLE_STATUS BluetoothManager::ble_status = BLE_STATUS::DISCONNECTED;

struct ble_gatt_chr_def BluetoothManager::gatt_characteristics[] = {
    // RX characteristic
    {
        .uuid = &RX_UUID.u,
        .access_cb = BluetoothManager::gatt_access_callback,
        .flags =
            BLE_GATT_CHR_F_WRITE |
            BLE_GATT_CHR_F_WRITE_NO_RSP,
    },

    // TX characteristic:
    {
        .uuid = &TX_UUID.u,
        .access_cb = BluetoothManager::gatt_access_callback,
        .flags = BLE_GATT_CHR_F_NOTIFY,
        .val_handle = &tx_value_handle,
    },

    // A zeroed characteristic marks the end of the array.
    {
        0}};

// One primary Nordic UART service
const struct ble_gatt_svc_def BluetoothManager::gatt_services[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,

        .uuid = &SERVICE_UUID.u,

        .characteristics = gatt_characteristics,
    },

    // A zeroed service marks the end of the array.
    {
        0}};

// GATT registration callback
void BluetoothManager::gatt_register_callback(
    struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char uuid_string[BLE_UUID_STR_LEN];

    switch (ctxt->op)
    {
    case BLE_GATT_REGISTER_OP_SVC:

        // ESP_LOGI(
        //     TAG,
        //     "GATT service registered: %s handle=%d",
        //     ble_uuid_to_str(
        //         ctxt->svc.svc_def->uuid,
        //         uuid_string),
        //     ctxt->svc.handle);

        break;

    case BLE_GATT_REGISTER_OP_CHR:

        // ESP_LOGI(
        //     TAG,
        //     "GATT characteristic registered: %s val_handle=%d",
        //     ble_uuid_to_str(
        //         ctxt->chr.chr_def->uuid,
        //         uuid_string),
        //     ctxt->chr.val_handle);

        break;

    default:
        break;
    }
}

/*
 * --------------------------------------------------------------------------
 * GATT access callback
 * --------------------------------------------------------------------------
 */

int BluetoothManager::gatt_access_callback(
    uint16_t conn_handle,
    uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctxt,
    void *arg)
{
    /*
     * We only expect WRITE operations on the RX characteristic.
     */
    if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR)
    {
        return BLE_ATT_ERR_UNLIKELY;
    }

    /*
     * Determine how many bytes are contained in the mbuf chain.
     */
    const uint16_t data_length =
        OS_MBUF_PKTLEN(ctxt->om);

    if (data_length == 0)
    {
        return 0;
    }

    /*
     * Temporary stack buffer containing this BLE write.
     *
     * BLE writes are normally small because of the negotiated MTU.
     */
    uint8_t data[256];

    if (data_length > sizeof(data))
    {
        ESP_LOGW(
            TAG,
            "BLE write too large: %u",
            data_length);

        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    /*
     * Flatten the NimBLE mbuf chain into a normal byte array.
     */
    int rc = ble_hs_mbuf_to_flat(
        ctxt->om,
        data,
        sizeof(data),
        nullptr);

    if (rc != 0)
    {
        ESP_LOGE(
            TAG,
            "Failed to flatten BLE write: rc=%d",
            rc);

        return BLE_ATT_ERR_UNLIKELY;
    }

    /*
     * Add the bytes to the persistent RX line buffer.
     */
    process_rx_bytes(
        data,
        data_length);

    /*
     * Return zero to tell NimBLE that the GATT write
     * was handled successfully.
     */
    return 0;
}

/*
 * --------------------------------------------------------------------------
 * RX line processing
 * --------------------------------------------------------------------------
 */

void BluetoothManager::reset_rx_buffer()
{
    rx_buffer_length = 0;

    memset(
        rx_buffer,
        0,
        sizeof(rx_buffer));
}

// Add bytes received from Gadgetbridge to the line buffer
void BluetoothManager::process_rx_bytes(
    const uint8_t *data, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        const char c =
            static_cast<char>(data[i]);

        /*
         * Ignore CR.
         *
         * Gadgetbridge normally sends:
         *
         *     \x10GB({...})\r\n
         *
         * or line-oriented variants.
         *
         * We use LF as the frame terminator.
         */
        if (c == '\r')
        {
            continue;
        }

        // LF means that one complete command has arrived
        if (c == '\n')
        {
            rx_buffer[rx_buffer_length] = '\0';

            process_complete_line(rx_buffer);

            reset_rx_buffer();

            continue;
        }

        /*
         * Protect the buffer from overflow.
         *
         * If one command is too large, discard the current
         * command and wait for the next newline.
         */
        if (rx_buffer_length >= RX_BUFFER_SIZE - 1)
        {
            ESP_LOGW(
                TAG,
                "Gadgetbridge command exceeded RX buffer");

            reset_rx_buffer();

            continue;
        }

        /*
         * Store the received character.
         */
        rx_buffer[rx_buffer_length++] = c;
    }
}

// Process one complete Gadgetbridge command
void BluetoothManager::process_complete_line(const char *line)
{
    if (line == nullptr or line[0] == '\0')
    {
        return;
    }

    // ESP_LOGI(TAG, "Gadgetbridge RX: %s", line);

    /*
     * Bangle.js/Gadgetbridge messages normally start with
     * byte 0x10 followed by:
     *
     *     GB({...})
     */
    const char *payload = line;

    if (static_cast<uint8_t>(payload[0]) == 0x10)
    {
        ++payload;
    }

    // Only process the Bangle.js GB(...) protocol
    if (strncmp(payload, "GB(", 3) != 0)
    {
        ESP_LOGD(TAG, "Ignoring non-Gadgetbridge line");
        return;
    }

    const size_t payload_length =
        strlen(payload);

    if (payload_length < 5 or
        payload[payload_length - 1] != ')')
    {
        ESP_LOGW(TAG, "Malformed Gadgetbridge GB command");

        return;
    }

    /*
     * We need a writable JSON buffer because the
     * application handler may want to parse it.
     */
    const char *json = payload + 3;

    const size_t json_length = payload_length - 4;

    std::string json_string(json, json_length);

    /*
     * Application integration point.
     *
     * This should update your common notification/call/
     * weather/navigation data structures and notify the
     * GUI task.
     */
    handle_gadgetbridge_line(json_string.c_str());
}

// NimBLE reset callback
void BluetoothManager::on_reset(int reason)
{
    ESP_LOGE(
        TAG,
        "NimBLE host reset, reason=%d",
        reason);

    connection_handle =
        BLE_HS_CONN_HANDLE_NONE;

    tx_notifications_enabled = false;
}

// NimBLE synchronization callback
void BluetoothManager::on_sync()
{
    /*
     * Ask NimBLE to choose the correct local identity
     * address type.
     */
    int rc =
        ble_hs_id_infer_auto(
            0,
            &own_addr_type);

    if (rc != 0)
    {
        ESP_LOGE(TAG, "ble_hs_id_infer_auto failed: %d", rc);

        return;
    }

    // ESP_LOGI(TAG, "NimBLE host synchronized");

    // Advertising must start only after synchronization
    init_advertising();
}

// Initiate Advertising
esp_err_t BluetoothManager::init_advertising()
{
    // ESP_LOGI(
    //     TAG,
    //     "init_advertising() called");
    struct ble_hs_adv_fields fields;

    memset(&fields, 0, sizeof(fields));

    // BLE_HS_ADV_F_DISC_GEN: This device is available to be discovered
    // BLE_HS_ADV_F_BREDR_UNSUP: Bluetooth Classic is not supported; this device is BLE-only.
    fields.flags =
        BLE_HS_ADV_F_DISC_GEN |
        BLE_HS_ADV_F_BREDR_UNSUP;

    // Advertise the device name
    fields.name = reinterpret_cast<uint8_t *>(const_cast<char *>(DEVICE_NAME));
    fields.name_len = strlen(DEVICE_NAME);
    fields.name_is_complete = 1;

    // Put the Nordic UART service UUID into advertising data
    fields.uuids128 = const_cast<ble_uuid128_t *>(&SERVICE_UUID);
    fields.num_uuids128 = 1;
    fields.uuids128_is_complete = 1;

    /*
     * Write the advertising packet.
     */
    int rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "ble_gap_adv_set_fields failed: %d", rc);
        return ESP_FAIL;
    }

    /*
     * Configure connectable advertising.
     */
    struct ble_gap_adv_params adv_params;

    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;

    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // General Discoverable Mode

    // Start advertising forever
    rc = ble_gap_adv_start(
        own_addr_type,
        nullptr,
        BLE_HS_FOREVER,
        &adv_params,
        gap_event,
        nullptr);

    if (rc != 0)
    {
        ESP_LOGE(TAG, "ble_gap_adv_start failed: %d", rc);
        return ESP_FAIL;
    }

    // ESP_LOGI(TAG, "Advertising started as %s", DEVICE_NAME);

    return ESP_OK;
}

/*
 * --------------------------------------------------------------------------
 * GAP callback
 * --------------------------------------------------------------------------
 */

int BluetoothManager::gap_event(
    struct ble_gap_event *event,
    void *arg)
{
    // ESP_LOGI(
    //     TAG,
    //     "GAP event received: type=%d",
    //     event->type);

    switch (event->type)
    {
    // Phone connected
    case BLE_GAP_EVENT_CONNECT:
    {
        ESP_LOGI(TAG, "BLE_GAP_EVENT_CONNECT CALLED");
        if (event->connect.status == 0)
        {
            connection_handle = event->connect.conn_handle;
            // is_connected = true;

            ble_status = BLE_STATUS::CONNECTED;

            struct ble_gap_upd_params params = {
                .itvl_min = 80, // min interval = 80 * 1.25 = 100ms
                .itvl_max = 160, // max Interval = 160 * 1.25 = 200ms
                .latency = 10,
                .supervision_timeout = 600, // 600 * 10 = 6000ms [supervision timeout > 2 × (latency + 1) × max_interval]
                .min_ce_len = 0,
                .max_ce_len = 0,
            };

            int rc = ble_gap_update_params(connection_handle, &params);
            if (rc != 0)
            {
                ESP_LOGE(
                    TAG, "Connection parameter update failed: %d", rc);
            }

            if (gui_task_handle != nullptr)
            {
                xTaskNotify(gui_task_handle, BLE_STATUS_EVENT, eSetBits);
            }

            tx_notifications_enabled = false;
            passkey = generate_passkey();
        }

        return 0;
    }
    // Phone disconnected
    case BLE_GAP_EVENT_DISCONNECT:
    {
        ESP_LOGI(TAG, "BLE_GAP_EVENT_DISCONNECT CALLED");
        // is_connected = false;
        ble_status = BLE_STATUS::DISCONNECTED;

        connection_handle = BLE_HS_CONN_HANDLE_NONE;
        tx_notifications_enabled = false;

        if (gui_task_handle != nullptr)
        {
            xTaskNotify(gui_task_handle, BLE_STATUS_EVENT, eSetBits);
        }

        init_advertising();

        return 0;
    }
    // Encryption changed
    case BLE_GAP_EVENT_ENC_CHANGE:
    {
        ESP_LOGI(TAG, "BLE_GAP_EVENT_ENC_CHANGE CALLED");
        // is_connected = is_connected & (event->enc_change.status == 0);

        if (event->enc_change.status == 0)
        {
            ble_status = BLE_STATUS::PAIRED;
            if (gui_task_handle != nullptr)
            {
                xTaskNotify(gui_task_handle, BLE_STATUS_EVENT, eSetBits);
            }
        }
        else
        {
            ESP_LOGE(
                TAG,
                "BLE encryption failed: %d",
                event->enc_change.status);
        }

        return 0;
    }
    // Pairing/passkey action
    case BLE_GAP_EVENT_PASSKEY_ACTION:
    {
        ESP_LOGI(TAG, "BLE_GAP_EVENT_PASSKEY_ACTION CALLED");
        struct ble_sm_io pkey;

        memset(&pkey, 0, sizeof(pkey));

        if (event->passkey.params.action ==
            BLE_SM_IOACT_DISP)
        {
            pkey.action = BLE_SM_IOACT_DISP;

            pkey.passkey = passkey;
            ble_status = BLE_STATUS::PAIRING;

            if (gui_task_handle != nullptr)
            {
                xTaskNotify(gui_task_handle, BLE_STATUS_EVENT, eSetBits);
            }
            int rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            if (rc != 0)
            {
                ESP_LOGE(TAG, "ble_sm_inject_io failed: %d", rc);
            }
        }

        return 0;
    }
    // Client subscribed to notifications
    case BLE_GAP_EVENT_SUBSCRIBE:
    {
        // Check if TX Characteristic is subescribed
        if (event->subscribe.attr_handle ==
            tx_value_handle)
        {
            tx_notifications_enabled =
                event->subscribe.cur_notify != 0;

            if (tx_notifications_enabled)
            {
                if (background_task_handle != nullptr)
                {
                    xTaskNotify(background_task_handle, SEND_BATTERY_DATA_EVENT, eSetBits);
                }
            }
        }

        return 0;
    }
    // MTU update
    case BLE_GAP_EVENT_MTU:

        ESP_LOGI(
            TAG,
            "MTU updated: conn=%d mtu=%d",
            event->mtu.conn_handle,
            event->mtu.value);

        return 0;

    default:
        return 0;
    }
}

/*
 * --------------------------------------------------------------------------
 * NimBLE host task
 * --------------------------------------------------------------------------
 */

void BluetoothManager::nimble_host_task(void *param)
{
    // ESP_LOGI(TAG, "NimBLE host task started");
    // Run the NimBLE host event loop.
    // This function blocks while BLE is active.
    nimble_port_run();

    // When nimble_port_stop() is eventually called,
    // clean up the FreeRTOS integration.
    nimble_port_freertos_deinit();

    vTaskDelete(nullptr);
}

/*
 * --------------------------------------------------------------------------
 * BLE initialization
 * --------------------------------------------------------------------------
 */

esp_err_t BluetoothManager::init()
{
    // Initialize the ESP-IDF NimBLE controller/HCI/host port
    esp_err_t err = nimble_port_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG,
                 "nimble_port_init failed: %s", esp_err_to_name(err));
        return err;
    }

    // Configure NimBLE host callbacks
    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.gatts_register_cb = gatt_register_callback;

    /*
     * --------------------------------------------------------------
     * Security
     * --------------------------------------------------------------
     *
     * Equivalent intent to the old:
     *
     * NimBLEDevice::setSecurityAuth(true, true, false)
     *
     * means:
     *
     * bonding = enabled
     * MITM    = enabled
     * secure connections = disabled
     */
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm = 1;
    ble_hs_cfg.sm_sc = 0;

    /*
     * Display-only I/O capability.
     */
    ble_hs_cfg.sm_io_cap =
        BLE_SM_IO_CAP_DISP_ONLY;

    /*
     * Distribute encryption keys when bonding.
     */
    ble_hs_cfg.sm_our_key_dist |=
        BLE_SM_PAIR_KEY_DIST_ENC;

    ble_hs_cfg.sm_their_key_dist |=
        BLE_SM_PAIR_KEY_DIST_ENC;

    /*
     * Initialize the standard GAP and GATT services.
     */
    ble_svc_gap_init();

    ble_svc_gatt_init();

    /*
     * Register the application GATT database.
     *
     * First NimBLE counts the required attributes.
     */
    int rc =
        ble_gatts_count_cfg(
            gatt_services);

    if (rc != 0)
    {
        ESP_LOGE(
            TAG,
            "ble_gatts_count_cfg failed: %d",
            rc);

        return ESP_FAIL;
    }

    /*
     * Then NimBLE adds the services to its GATT server.
     */
    rc =
        ble_gatts_add_svcs(
            gatt_services);

    if (rc != 0)
    {
        ESP_LOGE(
            TAG,
            "ble_gatts_add_svcs failed: %d",
            rc);

        return ESP_FAIL;
    }

    /*
     * Set the BLE GAP device name.
     */
    rc =
        ble_svc_gap_device_name_set(
            DEVICE_NAME);

    if (rc != 0)
    {
        ESP_LOGE(
            TAG,
            "Failed to set device name: %d",
            rc);

        return ESP_FAIL;
    }

    /*
     * Configure persistent NimBLE security storage.
     *
     * Bonding keys are stored by NimBLE/NVS.
     */
    ble_store_config_init();

    /*
     * Start the NimBLE host task.
     *
     * Advertising itself starts later from on_sync(),
     * because NimBLE must first finish host/controller sync.
     */
    nimble_port_freertos_init(
        nimble_host_task);

    // ESP_LOGI(
    //     TAG,
    //     "BLE manager initialized");

    return ESP_OK;
}

// bool BluetoothManager::is_device_connected()
// {
//     return is_connected;
// }

BLE_STATUS BluetoothManager::get_ble_conn_status()
{
    return ble_status;
}

/*
 * --------------------------------------------------------------------------
 * Send JSON to Gadgetbridge
 * --------------------------------------------------------------------------
 */

bool BluetoothManager::send_to_phone(const std::string &json)
{
    if (ble_status == BLE_STATUS::DISCONNECTED)
    {
        return false;
    }

    if (!tx_notifications_enabled)
    {
        return false;
    }

    if (tx_value_handle == 0)
    {
        return false;
    }

    // Gadgetbridge expects one JSON object per line
    std::string packet = json + "\r\n";

    // Convert the std::string into an mbuf chain
    struct os_mbuf *om =
        ble_hs_mbuf_from_flat(
            packet.data(),
            packet.size());

    if (om == nullptr)
    {
        ESP_LOGE(TAG, "Failed to allocate BLE notification mbuf");
        return false;
    }

    // Send a notification on the TX characteristic
    int rc =
        ble_gatts_notify_custom(
            connection_handle,
            tx_value_handle,
            om);

    if (rc != 0)
    {
        os_mbuf_free_chain(om);
        ESP_LOGE(
            TAG, "ble_gatts_notify_custom failed: %d", rc);

        return false;
    }

    ESP_LOGI(
        TAG, "Gadgetbridge TX: %s", packet.c_str());

    return true;
}

// BLE deinitialization
esp_err_t BluetoothManager::deinit()
{
    int rc = nimble_port_stop();

    if (rc != 0)
    {
        ESP_LOGE(
            TAG,
            "nimble_port_stop failed: %d",
            rc);

        return ESP_FAIL;
    }

    nimble_port_deinit();

    connection_handle = BLE_HS_CONN_HANDLE_NONE;
    tx_value_handle = 0;
    tx_notifications_enabled = false;
    passkey = 0;

    reset_rx_buffer();

    // ESP_LOGI(TAG, "BLE deinitialized");

    return ESP_OK;
}

uint32_t BluetoothManager::generate_passkey()
{
    return 100000 + (esp_random() % 900000);
}

uint32_t BluetoothManager::get_passkey()
{
    return passkey;
}

void BluetoothManager::handle_gadgetbridge_line(const char *line)
{
    ESP_LOGI(TAG, "Gadgetbridge JSON: %s", line);

    cJSON *json = cJSON_Parse(line);

    if (json == nullptr)
    {
        const char *error_ptr = cJSON_GetErrorPtr();

        if (error_ptr != nullptr)
        {
            ESP_LOGE(TAG, "JSON parse error near: %s", error_ptr);
        }

        return;
    }

    cJSON *event = cJSON_GetObjectItemCaseSensitive(json, "t");

    if (!cJSON_IsString(event) || event->valuestring == nullptr)
    {
        ESP_LOGW(TAG, "Missing or invalid 't' field");
        cJSON_Delete(json);
        return;
    }

    ESP_LOGI(TAG, "Event: %s", event->valuestring);

    if (strcmp(event->valuestring, "call") == 0)
    {
        cJSON *cmd = cJSON_GetObjectItemCaseSensitive(json, "cmd");
        cJSON *name_item = cJSON_GetObjectItemCaseSensitive(json, "name");
        cJSON *number_item = cJSON_GetObjectItemCaseSensitive(json, "number");

        std::string status = cJSON_IsString(cmd) ? cmd->valuestring : "";

        std::string name = cJSON_IsString(name_item) ? name_item->valuestring : "UNKNOWN";
        name = name.empty() ? "UNKNOWN" : name;

        std::string number = cJSON_IsString(number_item) ? number_item->valuestring : "";

        call_data.update(name, number, status);

        if (gui_task_handle != nullptr)
        {
            xTaskNotify(
                gui_task_handle, CALL_SCREEN_EVENT, eSetBits);
        }
    }

    if (strcmp(event->valuestring, "weather") == 0)
    {
        cJSON *temp_item = cJSON_GetObjectItemCaseSensitive(json, "temp");
        cJSON *high_item = cJSON_GetObjectItemCaseSensitive(json, "hi");
        cJSON *low_item = cJSON_GetObjectItemCaseSensitive(json, "lo");
        cJSON *humidity_item = cJSON_GetObjectItemCaseSensitive(json, "hum");
        cJSON *rain_item = cJSON_GetObjectItemCaseSensitive(json, "rain");
        cJSON *wind_item = cJSON_GetObjectItemCaseSensitive(json, "wind");
        cJSON *text_item = cJSON_GetObjectItemCaseSensitive(json, "txt");
        cJSON *location_item = cJSON_GetObjectItemCaseSensitive(json, "loc");

        float temp = cJSON_IsNumber(temp_item) ? temp_item->valuedouble - 273.15f : 0;
        float high = cJSON_IsNumber(high_item) ? high_item->valuedouble - 273.15f : 0;
        float low = cJSON_IsNumber(low_item) ? low_item->valuedouble - 273.15f : 0;
        float humidity = cJSON_IsNumber(humidity_item) ? humidity_item->valuedouble : 0;
        float rain = cJSON_IsNumber(rain_item) ? rain_item->valuedouble : 0;
        float wind = cJSON_IsNumber(wind_item) ? wind_item->valuedouble : 0;

        std::string text =
            cJSON_IsString(text_item) ? text_item->valuestring : "Weather";

        std::string location =
            cJSON_IsString(location_item) ? location_item->valuestring : "Location";

        weather_data.update(
            temp,
            high,
            low,
            humidity,
            rain,
            wind,
            text,
            location);

        if (gui_task_handle != nullptr)
        {
            xTaskNotify(
                gui_task_handle, WEATHER_UI_UPDATE_EVENT, eSetBits);
        }
    }

    cJSON_Delete(json);
}

esp_err_t BluetoothManager::handle_events(uint32_t events)
{
    std::string data;
    if (events & ACCEPT_CALL_EVENT)
    {
        data = R"({"t":"call","n":"ACCEPT"})";
        bool status = send_to_phone(data);
        if (!status)
        {
            return ESP_FAIL;
        }
    }

    if (events & REJECT_CALL_EVENT)
    {
        data = R"({"t":"call","n":"REJECT"})";
        bool status = send_to_phone(data);
        if (!status)
        {
            return ESP_FAIL;
        }
    }

    if (events & SEND_BATTERY_DATA_EVENT)
    {
        ESP_LOGI(TAG, "SEND_BATTERY_DATA_EVENT triggered");
        vTaskDelay(pdMS_TO_TICKS(1000));

        data = R"({"t":"status","bat":90,"chg":0})";
        bool status = send_to_phone(data);
        if (!status)
            return ESP_FAIL;
    }

    if (events & INIT_WEATHER_FETCH_EVENT)
    {
        vTaskDelay(pdMS_TO_TICKS(200));

        // Weather related data fetch init call
        data = R"({"t":"weather","v":1})";
        bool status = send_to_phone(data);
        if (!status)
            return ESP_FAIL;
    }
    return ESP_OK;
}