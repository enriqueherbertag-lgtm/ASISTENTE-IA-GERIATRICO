/**
 * @file nfc_driver.c
 * @brief Implementación del driver para módulo NFC PN532
 * 
 * Este archivo contiene la implementación de las funciones
 * declaradas en nfc_driver.h
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
#include "driver/i2c.h"
#include "nfc_driver.h"
#include "config.h"

static const char *TAG = "NFC_DRV";

// Dirección I2C del PN532
#define PN532_I2C_ADDR      0x48
#define PN532_FIRMWARE_REG  0x02

// Comandos PN532
#define PN532_COMMAND_GET_FW  0x02
#define PN532_COMMAND_SAMCONF 0x14
#define PN532_COMMAND_INLIST  0x4A
#define PN532_COMMAND_INDATA  0x40

// Respuestas esperadas
#define PN532_PREAMBLE        0x00
#define PN532_STARTCODE1      0x00
#define PN532_STARTCODE2      0xFF
#define PN532_POSTAMBLE       0x00

// Comandos específicos FreeStyle Libre
#define FREESTYLE_CMD_AUTH    0x1A
#define FREESTYLE_CMD_READ    0x20
#define FREESTYLE_BLOCKS      24

// Umbrales de glucosa por defecto (mg/dL)
#define DEFAULT_HYPO_LOW      70
#define DEFAULT_HYPO_CRITICAL 54
#define DEFAULT_HYPER         250

// Variables internas
static bool nfc_initialized = false;
static bool nfc_powered = false;
static uint32_t read_counter = 0;
static uint32_t firmware_version = 0;
static sensor_status_t current_sensor_status = SENSOR_NOT_FOUND;

static uint16_t hypo_threshold = DEFAULT_HYPO_LOW;
static uint16_t hypo_critical = DEFAULT_HYPO_CRITICAL;
static uint16_t hyper_threshold = DEFAULT_HYPER;
static bool alerts_enabled = true;
static uint32_t read_interval = GLUCOSE_READ_INTERVAL;

// Última lectura
static glucose_data_t last_reading = {0};
static uint64_t last_read_time = 0;

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static esp_err_t nfc_write_cmd(uint8_t *cmd, size_t len)
{
    i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (PN532_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(i2c_cmd, cmd, len, true);
    i2c_master_stop(i2c_cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_1, i2c_cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(i2c_cmd);
    return ret;
}

static esp_err_t nfc_read_response(uint8_t *buf, size_t len, uint32_t timeout_ms)
{
    uint8_t header[6];
    
    // Esperar a que el PN532 tenga datos
    vTaskDelay(pdMS_TO_TICKS(5));
    
    // Leer header
    i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (PN532_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(i2c_cmd, header, 6, I2C_MASTER_LAST_NACK);
    i2c_master_stop(i2c_cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_1, i2c_cmd, pdMS_TO_TICKS(timeout_ms));
    i2c_cmd_link_delete(i2c_cmd);
    
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Verificar formato de respuesta
    if (header[0] != PN532_PREAMBLE || 
        header[1] != PN532_STARTCODE1 || 
        header[2] != PN532_STARTCODE2) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    
    uint8_t length = header[3];
    uint8_t lcs = header[4];
    
    // Verificar checksum
    if ((uint8_t)(length + lcs) != 0) {
        return ESP_ERR_INVALID_CRC;
    }
    
    // Leer datos
    if (length > len) {
        length = len;
    }
    
    i2c_cmd = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (PN532_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(i2c_cmd, buf, length, I2C_MASTER_LAST_NACK);
    i2c_master_stop(i2c_cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_1, i2c_cmd, pdMS_TO_TICKS(timeout_ms));
    i2c_cmd_link_delete(i2c_cmd);
    
    return ret;
}

static esp_err_t nfc_send_command(uint8_t cmd, uint8_t *params, uint8_t params_len)
{
    uint8_t packet[32];
    int idx = 0;
    
    packet[idx++] = PN532_PREAMBLE;
    packet[idx++] = PN532_STARTCODE1;
    packet[idx++] = PN532_STARTCODE2;
    
    uint8_t len = params_len + 1;
    packet[idx++] = len;
    packet[idx++] = ~len + 1;  // LCS
    
    packet[idx++] = cmd;
    for (int i = 0; i < params_len; i++) {
        packet[idx++] = params[i];
    }
    
    uint8_t dcs = cmd;
    for (int i = 0; i < params_len; i++) {
        dcs += params[i];
    }
    packet[idx++] = ~dcs + 1;
    
    packet[idx++] = PN532_POSTAMBLE;
    
    return nfc_write_cmd(packet, idx);
}

static uint16_t bytes_to_uint16(uint8_t high, uint8_t low)
{
    return (high << 8) | low;
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

esp_err_t nfc_init(void)
{
    if (nfc_initialized) {
        ESP_LOGW(TAG, "NFC ya inicializado");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Inicializando módulo NFC PN532");
    
    // Obtener versión de firmware
    uint8_t fw[4];
    esp_err_t ret = nfc_send_command(PN532_COMMAND_GET_FW, NULL, 0);
    if (ret == ESP_OK) {
        ret = nfc_read_response(fw, 4, 1000);
        if (ret == ESP_OK) {
            firmware_version = (fw[0] << 24) | (fw[1] << 16) | (fw[2] << 8) | fw[3];
            ESP_LOGI(TAG, "Firmware PN532: %d.%d", fw[0], fw[1]);
        }
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error comunicando con PN532");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Configurar modo SAM (Secure Access Module)
    uint8_t sam_params[] = {0x01, 0x14, 0x01};
    ret = nfc_send_command(PN532_COMMAND_SAMCONF, sam_params, 3);
    if (ret == ESP_OK) {
        uint8_t sam_resp[8];
        ret = nfc_read_response(sam_resp, 8, 1000);
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error configurando SAM");
        return ret;
    }
    
    nfc_initialized = true;
    nfc_powered = true;
    ESP_LOGI(TAG, "NFC inicializado correctamente");
    
    return ESP_OK;
}

esp_err_t nfc_read_glucose(glucose_data_t *data)
{
    if (!nfc_initialized || !nfc_powered) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t uid[7];
    uint8_t blocks[24][8];
    
    // 1. Buscar tarjeta (sensor FreeStyle Libre)
    uint8_t inlist_params[] = {0x01, 0x00};
    esp_err_t ret = nfc_send_command(PN532_COMMAND_INLIST, inlist_params, 2);
    if (ret != ESP_OK) {
        current_sensor_status = SENSOR_NOT_FOUND;
        return ESP_ERR_NOT_FOUND;
    }
    
    uint8_t inlist_resp[20];
    ret = nfc_read_response(inlist_resp, 20, 1000);
    if (ret != ESP_OK || inlist_resp[0] != 0x01) {
        current_sensor_status = SENSOR_NOT_FOUND;
        return ESP_ERR_NOT_FOUND;
    }
    
    // Obtener UID (primeros 7 bytes después de los primeros 5)
    memcpy(uid, inlist_resp + 5, 7);
    
    // 2. Autenticar (comando propietario Abbott)
    uint8_t auth_cmd[] = {0x1A, 0x00};
    ret = nfc_send_command(PN532_COMMAND_INDATA, auth_cmd, 2);
    if (ret != ESP_OK) {
        return ESP_ERR_AUTH_FAIL;
    }
    
    uint8_t auth_resp[8];
    ret = nfc_read_response(auth_resp, 8, 1000);
    if (ret != ESP_OK) {
        return ESP_ERR_AUTH_FAIL;
    }
    
    // 3. Leer bloques de datos (FreeStyle Libre tiene 24 bloques)
    for (int i = 0; i < 24; i++) {
        uint8_t read_cmd[] = {FREESTYLE_CMD_READ, (uint8_t)i};
        ret = nfc_send_command(PN532_COMMAND_INDATA, read_cmd, 2);
        if (ret != ESP_OK) {
            return ESP_FAIL;
        }
        
        ret = nfc_read_response(blocks[i], 8, 500);
        if (ret != ESP_OK) {
            return ESP_FAIL;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    // 4. Parsear datos (formato propietario Abbott)
    // Bloque 0 contiene datos actuales
    uint16_t glucose_raw = bytes_to_uint16(blocks[0][0], blocks[0][1]);
    
    // Decodificar glucosa (10 bits de datos)
    data->glucose_mgdl = (glucose_raw >> 2) & 0x3FF;
    
    // Tendencia (bits 4-6 del tercer byte)
    data->trend = (blocks[0][2] >> 4) & 0x07;
    
    // Estado del sensor
    data->sensor_status = blocks[0][3];
    
    // Timestamp
    data->timestamp = esp_timer_get_time() / 1000;  // ms
    
    // Calidad de señal (simulada por ahora)
    data->signal_quality = 90;
    
    // Batería (si disponible, depende del modelo)
    data->battery_status = 100;
    
    // Verificar expiración (14 días)
    // La lógica exacta depende del formato del fabricante
    uint8_t days_used = blocks[0][6];
    if (days_used > 13) {
        current_sensor_status = SENSOR_EXPIRING;
        if (days_used >= 14) {
            current_sensor_status = SENSOR_EXPIRED;
        }
    } else {
        current_sensor_status = SENSOR_OK;
    }
    
    // Guardar última lectura
    memcpy(&last_reading, data, sizeof(glucose_data_t));
    last_read_time = esp_timer_get_time();
    read_counter++;
    
    return ESP_OK;
}

esp_err_t nfc_read_history(glucose_history_t *history)
{
    // TODO: Implementar lectura de historial
    // Requiere leer los 24 bloques y decodificar los históricos
    ESP_LOGW(TAG, "nfc_read_history no implementado completamente");
    history->count = 0;
    return ESP_ERR_NOT_SUPPORTED;
}

sensor_status_t nfc_get_sensor_status(void)
{
    return current_sensor_status;
}

esp_err_t nfc_get_expiration(int *days, int *hours)
{
    if (current_sensor_status == SENSOR_NOT_FOUND) {
        return ESP_ERR_NOT_FOUND;
    }
    
    // TODO: Calcular basado en datos del sensor
    *days = 0;
    *hours = 0;
    
    return ESP_OK;
}

/* ==================================================================
 * FUNCIONES DE INTERPRETACIÓN
 * ================================================================== */

void nfc_glucose_to_text(uint16_t glucose_mgdl, char *buffer, size_t len)
{
    if (glucose_mgdl < hypo_critical) {
        snprintf(buffer, len, "Glucosa crítica: %d mg/dL", glucose_mgdl);
    } else if (glucose_mgdl < hypo_threshold) {
        snprintf(buffer, len, "Glucosa baja: %d mg/dL", glucose_mgdl);
    } else if (glucose_mgdl > hyper_threshold) {
        snprintf(buffer, len, "Glucosa alta: %d mg/dL", glucose_mgdl);
    } else {
        snprintf(buffer, len, "Glucosa normal: %d mg/dL", glucose_mgdl);
    }
}

const char* nfc_trend_to_string(uint8_t trend)
{
    static const char *trend_strings[] = {
        "estable",
        "subiendo lentamente",
        "subiendo",
        "subiendo rápido",
        "bajando lentamente",
        "bajando",
        "bajando rápido",
        "desconocida"
    };
    
    if (trend > 7) trend = 7;
    return trend_strings[trend];
}

uint8_t nfc_get_alert_level(uint16_t glucose_mgdl)
{
    if (glucose_mgdl < hypo_critical) {
        return 2;  // Crítica
    } else if (glucose_mgdl < hypo_threshold) {
        return 1;  // Baja
    } else if (glucose_mgdl > hyper_threshold) {
        return 3;  // Alta
    }
    return 0;  // Normal
}

/* ==================================================================
 * FUNCIONES DE CONFIGURACIÓN
 * ================================================================== */

void nfc_set_thresholds(uint16_t hypo_low, uint16_t hypo_critical_val, uint16_t hyper)
{
    hypo_threshold = hypo_low;
    hypo_critical = hypo_critical_val;
    hyper_threshold = hyper;
}

void nfc_enable_alerts(bool enable)
{
    alerts_enabled = enable;
}

void nfc_set_read_interval(uint32_t interval_ms)
{
    read_interval = interval_ms;
}

/* ==================================================================
 * FUNCIONES DE ENERGÍA
 * ================================================================== */

esp_err_t nfc_power_off(void)
{
    if (!nfc_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // El PN532 no tiene modo sleep específico en I2C
    // Se puede implementar quitando alimentación externa
    nfc_powered = false;
    ESP_LOGI(TAG, "NFC apagado");
    return ESP_OK;
}

esp_err_t nfc_power_on(void)
{
    if (!nfc_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    nfc_powered = true;
    ESP_LOGI(TAG, "NFC encendido");
    return ESP_OK;
}

bool nfc_is_powered(void)
{
    return nfc_powered;
}

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

bool nfc_is_initialized(void)
{
    return nfc_initialized;
}

bool nfc_is_sensor_present(void)
{
    return (current_sensor_status != SENSOR_NOT_FOUND);
}

uint32_t nfc_get_firmware_version(void)
{
    return firmware_version;
}

uint32_t nfc_get_read_count(void)
{
    return read_counter;
}
