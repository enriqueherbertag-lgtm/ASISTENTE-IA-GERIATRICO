/**
 * @file bluetooth.c
 * @brief Implementación de comunicación Bluetooth LE
 * 
 * Este archivo contiene la implementación de las funciones
 * declaradas en bluetooth.h
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_device.h"
#include "bluetooth.h"
#include "config.h"

static const char *TAG = "BLUETOOTH";

// Variables internas
static bt_state_t current_state = BT_STATE_DISABLED;
static bt_mode_t current_mode = BT_MODE_OFF;
static bool auto_start = false;
static char device_name[32] = BT_DEVICE_NAME;

static bt_connected_device_t connected_dev = {0};
static bool device_connected = false;

static bt_callbacks_t user_callbacks = {0};

// Parámetros de publicidad
static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .peer_addr          = {0},
    .peer_addr_type     = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// Datos de publicidad
static uint8_t adv_data[32];
static uint8_t adv_data_len = 0;
static uint8_t scan_rsp_data[32];
static uint8_t scan_rsp_len = 0;

// Perfil GATT
#define GATTS_TAG "GATTS"
#define PROFILE_NUM 1
#define PROFILE_APP_IDX 0

static struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t alert_char_handle;
    uint16_t battery_char_handle;
    uint16_t status_char_handle;
} gatts_profile = {
    .gatts_cb = NULL,
    .gatts_if = ESP_GATT_IF_NONE,
};

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static void prepare_adv_data(void)
{
    int idx = 0;
    
    // Flags
    adv_data[idx++] = 0x02;  // Longitud
    adv_data[idx++] = 0x01;  // Tipo: flags
    adv_data[idx++] = 0x06;  // Valor: LE General Discoverable, BR/EDR not supported
    
    // UUID de servicio
    adv_data[idx++] = 0x03;  // Longitud
    adv_data[idx++] = 0x03;  // Tipo: Complete list of 16-bit UUIDs
    adv_data[idx++] = 0x00;  // UUID (parte baja)
    adv_data[idx++] = 0x18;  // UUID (parte alta) - 0x1800
    
    adv_data_len = idx;
    
    // Datos de respuesta al scan
    idx = 0;
    scan_rsp_data[idx++] = strlen(device_name) + 1;
    scan_rsp_data[idx++] = 0x09;  // Tipo: Complete local name
    memcpy(&scan_rsp_data[idx], device_name, strlen(device_name));
    idx += strlen(device_name);
    
    scan_rsp_len = idx;
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
        case ESP_GATTS_REG_EVT:
            ESP_LOGI(GATTS_TAG, "GATTS registrado, app_id=%d, status=%d", 
                     param->reg.app_id, param->reg.status);
            gatts_profile.gatts_if = gatts_if;
            
            // Crear servicio
            esp_ble_gatts_create_service(gatts_if, &gatts_profile.service_id, 8);
            break;
            
        case ESP_GATTS_CREATE_EVT:
            ESP_LOGI(GATTS_TAG, "Servicio creado, handle=%d", param->create.service_handle);
            gatts_profile.service_handle = param->create.service_handle;
            
            // Agregar características
            esp_ble_gatts_add_char(gatts_profile.service_handle,
                                    (uint16_t)0x2A00,  // UUID alerta
                                    ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                    ESP_GATT_CHAR_PROP_BIT_NOTIFY | ESP_GATT_CHAR_PROP_BIT_WRITE,
                                    NULL, NULL);
            break;
            
        case ESP_GATTS_ADD_CHAR_EVT:
            ESP_LOGI(GATTS_TAG, "Característica agregada, handle=%d", param->add_char.attr_handle);
            break;
            
        case ESP_GATTS_CONNECT_EVT:
            ESP_LOGI(GATTS_TAG, "Dispositivo conectado, conn_id=%d", param->connect.conn_id);
            gatts_profile.conn_id = param->connect.conn_id;
            device_connected = true;
            
            memcpy(connected_dev.address, param->connect.remote_bda, 6);
            connected_dev.connected_since = esp_timer_get_time() / 1000;
            connected_dev.bonded = false;
            
            current_state = BT_STATE_CONNECTED;
            
            if (user_callbacks.on_connected) {
                user_callbacks.on_connected(&connected_dev);
            }
            break;
            
        case ESP_GATTS_DISCONNECT_EVT:
            ESP_LOGI(GATTS_TAG, "Dispositivo desconectado");
            device_connected = false;
            memset(&connected_dev, 0, sizeof(connected_dev));
            
            current_state = BT_STATE_ADVERTISING;
            
            if (user_callbacks.on_disconnected) {
                user_callbacks.on_disconnected();
            }
            
            // Reiniciar publicidad
            esp_ble_gap_start_advertising(&adv_params);
            break;
            
        default:
            break;
    }
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            esp_ble_gap_start_advertising(&adv_params);
            break;
            
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            break;
            
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Publicidad iniciada");
                current_state = BT_STATE_ADVERTISING;
            } else {
                ESP_LOGE(TAG, "Error iniciando publicidad: %d", param->adv_start_cmpl.status);
            }
            break;
            
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            ESP_LOGI(TAG, "Publicidad detenida");
            if (current_state == BT_STATE_ADVERTISING) {
                current_state = BT_STATE_DISCONNECTED;
            }
            break;
            
        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            if (param->auth_cmpl.success) {
                ESP_LOGI(TAG, "Autenticación exitosa con %s", param->auth_cmpl.bdname);
                connected_dev.bonded = true;
                if (user_callbacks.on_bonded) {
                    user_callbacks.on_bonded(&connected_dev);
                }
            }
            break;
            
        default:
            break;
    }
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

esp_err_t bluetooth_init(bt_mode_t mode)
{
    ESP_LOGI(TAG, "Inicializando Bluetooth en modo %d", mode);
    
    // Liberar memoria si ya estaba inicializado
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
    
    // Inicializar controlador
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error inicializando controlador: %d", ret);
        return ret;
    }
    
    // Habilitar modo BLE
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error habilitando BLE: %d", ret);
        return ret;
    }
    
    // Inicializar bluedroid
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error inicializando bluedroid: %d", ret);
        return ret;
    }
    
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error habilitando bluedroid: %d", ret);
        return ret;
    }
    
    // Registrar callbacks
    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error registrando callback GATTS: %d", ret);
        return ret;
    }
    
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error registrando callback GAP: %d", ret);
        return ret;
    }
    
    // Registrar aplicación GATT
    ret = esp_ble_gatts_app_register(PROFILE_APP_IDX);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error registrando app GATT: %d", ret);
        return ret;
    }
    
    // Configurar nombre del dispositivo
    esp_ble_gap_set_device_name(device_name);
    
    // Configurar datos de publicidad
    prepare_adv_data();
    
    esp_ble_gap_config_adv_data_raw(adv_data, adv_data_len);
    esp_ble_gap_config_scan_rsp_data_raw(scan_rsp_data, scan_rsp_len);
    
    // Configurar seguridad
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
    esp_ble_io_cap_t iocap = ESP_IO_CAP_OUT;
    uint8_t key_size = 16;
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
    
    current_mode = mode;
    current_state = BT_STATE_DISCONNECTED;
    
    ESP_LOGI(TAG, "Bluetooth inicializado");
    
    return ESP_OK;
}

esp_err_t bluetooth_set_mode(bt_mode_t mode)
{
    if (mode == current_mode) {
        return ESP_OK;
    }
    
    current_mode = mode;
    
    switch (mode) {
        case BT_MODE_OFF:
            bluetooth_enable(false);
            break;
            
        case BT_MODE_ADVERTISING_ONLY:
        case BT_MODE_CONNECTABLE:
            bluetooth_enable(true);
            break;
            
        case BT_MODE_PAIRING:
            bluetooth_enable(true);
            bluetooth_start_pairing();
            break;
    }
    
    return ESP_OK;
}

bt_mode_t bluetooth_get_mode(void)
{
    return current_mode;
}

bt_state_t bluetooth_get_state(void)
{
    return current_state;
}

esp_err_t bluetooth_start_advertising(void)
{
    if (current_state == BT_STATE_DISABLED) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_ble_gap_start_advertising(&adv_params);
    
    return ESP_OK;
}

esp_err_t bluetooth_stop_advertising(void)
{
    if (current_state != BT_STATE_ADVERTISING) {
        return ESP_OK;
    }
    
    esp_ble_gap_stop_advertising();
    
    return ESP_OK;
}

esp_err_t bluetooth_disconnect(void)
{
    if (device_connected) {
        esp_ble_gatts_close(gatts_profile.gatts_if, gatts_profile.conn_id);
    }
    
    return ESP_OK;
}

esp_err_t bluetooth_clear_bonding(void)
{
    // TODO: Implementar eliminación de bonding
    ESP_LOGW(TAG, "bluetooth_clear_bonding no implementado");
    return ESP_ERR_NOT_SUPPORTED;
}

bool bluetooth_get_connected_device(bt_connected_device_t *dev)
{
    if (!device_connected || !dev) {
        return false;
    }
    
    *dev = connected_dev;
    return true;
}

esp_err_t bluetooth_send_alert(uint8_t alert_type, const uint8_t *data, int len)
{
    if (!device_connected) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_ble_gatts_send_indicate(gatts_profile.gatts_if,
                                  gatts_profile.conn_id,
                                  gatts_profile.alert_char_handle,
                                  len, (uint8_t*)data, false);
    
    return ESP_OK;
}

esp_err_t bluetooth_send_battery(uint8_t battery_level)
{
    if (!device_connected) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_ble_gatts_set_attr_value(gatts_profile.battery_char_handle,
                                  1, &battery_level);
    
    return ESP_OK;
}

esp_err_t bluetooth_send_status(uint8_t status)
{
    if (!device_connected) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_ble_gatts_set_attr_value(gatts_profile.status_char_handle,
                                  1, &status);
    
    return ESP_OK;
}

esp_err_t bluetooth_set_name(const char *name)
{
    if (!name) {
        return ESP_ERR_INVALID_ARG;
    }
    
    strncpy(device_name, name, sizeof(device_name) - 1);
    esp_ble_gap_set_device_name(device_name);
    
    prepare_adv_data();
    esp_ble_gap_config_adv_data_raw(adv_data, adv_data_len);
    esp_ble_gap_config_scan_rsp_data_raw(scan_rsp_data, scan_rsp_len);
    
    return ESP_OK;
}

void bluetooth_set_callbacks(bt_callbacks_t *callbacks)
{
    if (callbacks) {
        user_callbacks = *callbacks;
    }
}

void bluetooth_set_auto_start(bool enable)
{
    auto_start = enable;
}

esp_err_t bluetooth_enable(bool enable)
{
    if (enable) {
        if (current_state == BT_STATE_DISABLED) {
            bluetooth_init(current_mode);
        }
        bluetooth_start_advertising();
    } else {
        bluetooth_stop_advertising();
        bluetooth_disconnect();
        esp_bt_controller_disable();
        current_state = BT_STATE_DISABLED;
    }
    
    return ESP_OK;
}

bool bluetooth_is_bonded(const uint8_t *address)
{
    // TODO: Implementar verificación de bonding
    return false;
}

int bluetooth_get_bonded_count(void)
{
    // TODO: Implementar conteo de bonding
    return 0;
}

esp_err_t bluetooth_start_pairing(void)
{
    // TODO: Implementar modo pairing
    ESP_LOGW(TAG, "bluetooth_start_pairing no implementado");
    return ESP_ERR_NOT_SUPPORTED;
}

bool bluetooth_is_enabled(void)
{
    return (current_state != BT_STATE_DISABLED);
}

bool bluetooth_is_connected(void)
{
    return device_connected;
}

int bluetooth_get_rssi(void)
{
    // TODO: Implementar lectura RSSI
    return 0;
}

uint32_t bluetooth_get_version(void)
{
    return 0x0100;  // Versión 1.0
}

uint32_t bluetooth_get_power_consumption(void)
{
    switch (current_state) {
        case BT_STATE_DISABLED:
            return 0;
        case BT_STATE_ADVERTISING:
            return 500;  // 0.5 mA
        case BT_STATE_CONNECTED:
            return 800;  // 0.8 mA
        default:
            return 100;  // 0.1 mA
    }
}

esp_err_t bluetooth_set_advertising_interval(uint32_t interval_ms)
{
    adv_params.adv_int_min = interval_ms * 1000 / 625;  // Convertir a unidades de 0.625ms
    adv_params.adv_int_max = adv_params.adv_int_min * 2;
    
    return ESP_OK;
}
