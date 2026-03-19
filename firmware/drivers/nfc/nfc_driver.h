/**
 * @file nfc_driver.h
 * @brief Driver para módulo NFC PN532 (lector FreeStyle Libre)
 * 
 * Este módulo gestiona la comunicación con el lector NFC PN532
 * para leer datos del sensor de glucosa FreeStyle Libre 2.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef NFC_DRIVER_H
#define NFC_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * TIPOS DE DATOS
 * ================================================================== */

/**
 * @brief Datos de glucosa del FreeStyle Libre
 */
typedef struct {
    uint16_t glucose_mgdl;      // Glucosa en mg/dL
    uint8_t trend;               // Tendencia (0-7)
    uint8_t sensor_status;       // Estado del sensor
    uint32_t timestamp;          // Timestamp de la medición
    uint8_t battery_status;      // Batería del sensor (si disponible)
    uint8_t signal_quality;      // Calidad de la señal (0-100)
} glucose_data_t;

/**
 * @brief Datos históricos de glucosa (últimas 8 horas)
 */
typedef struct {
    uint16_t glucose_mgdl[32];   // Cada 15 min → 32 lecturas = 8 horas
    uint8_t trend[32];
    uint32_t timestamps[32];
    uint8_t count;                // Número de lecturas válidas
} glucose_history_t;

/**
 * @brief Estado del sensor FreeStyle Libre
 */
typedef enum {
    SENSOR_OK = 0,
    SENSOR_EXPIRING,              // Quedan menos de 24h
    SENSOR_EXPIRED,                // Más de 14 días
    SENSOR_ERROR,
    SENSOR_NOT_FOUND
} sensor_status_t;

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa el módulo NFC
 * @return ESP_OK en éxito, otro en error
 */
esp_err_t nfc_init(void);

/**
 * @brief Escanea y lee datos del FreeStyle Libre
 * @param data Puntero donde almacenar los datos de glucosa
 * @return ESP_OK en éxito, ESP_ERR_NOT_FOUND si no hay sensor
 */
esp_err_t nfc_read_glucose(glucose_data_t *data);

/**
 * @brief Lee el historial completo del sensor (últimas 8h)
 * @param history Puntero donde almacenar el historial
 * @return ESP_OK en éxito
 */
esp_err_t nfc_read_history(glucose_history_t *history);

/**
 * @brief Obtiene el estado del sensor
 * @return Estado del sensor
 */
sensor_status_t nfc_get_sensor_status(void);

/**
 * @brief Obtiene la fecha de expiración del sensor
 * @param days Puntero para días restantes
 * @param hours Puntero para horas restantes
 * @return ESP_OK en éxito
 */
esp_err_t nfc_get_expiration(int *days, int *hours);

/* ==================================================================
 * FUNCIONES DE INTERPRETACIÓN
 * ================================================================== */

/**
 * @brief Convierte valor de glucosa a texto descriptivo
 * @param glucose_mgdl Valor de glucosa
 * @param buffer Buffer para el texto
 * @param len Longitud del buffer
 */
void nfc_glucose_to_text(uint16_t glucose_mgdl, char *buffer, size_t len);

/**
 * @brief Obtiene descripción de la tendencia
 * @param trend Código de tendencia (0-7)
 * @return String con descripción (estático)
 */
const char* nfc_trend_to_string(uint8_t trend);

/**
 * @brief Determina si hay alerta según valor de glucosa
 * @param glucose_mgdl Valor de glucosa
 * @return Código de alerta (0 = normal, 1 = baja, 2 = crítica, 3 = alta)
 */
uint8_t nfc_get_alert_level(uint16_t glucose_mgdl);

/* ==================================================================
 * FUNCIONES DE CONFIGURACIÓN
 * ================================================================== */

/**
 * @brief Configura umbrales de alerta personalizados
 * @param hypo_low Umbral de hipoglucemia baja (mg/dL)
 * @param hypo_critical Umbral de hipoglucemia crítica
 * @param hyper_threshold Umbral de hiperglucemia
 */
void nfc_set_thresholds(uint16_t hypo_low, uint16_t hypo_critical, uint16_t hyper_threshold);

/**
 * @brief Activa o desactiva las alertas por voz
 * @param enable true para activar
 */
void nfc_enable_alerts(bool enable);

/**
 * @brief Configura el intervalo de lectura automática
 * @param interval_ms Intervalo en milisegundos
 */
void nfc_set_read_interval(uint32_t interval_ms);

/* ==================================================================
 * FUNCIONES DE ENERGÍA
 * ================================================================== */

/**
 * @brief Pone el módulo NFC en modo bajo consumo
 * @return ESP_OK en éxito
 */
esp_err_t nfc_power_off(void);

/**
 * @brief Reactiva el módulo NFC
 * @return ESP_OK en éxito
 */
esp_err_t nfc_power_on(void);

/**
 * @brief Verifica si el módulo está encendido
 * @return true si está encendido
 */
bool nfc_is_powered(void);

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

/**
 * @brief Verifica si el módulo NFC está inicializado
 * @return true si está inicializado
 */
bool nfc_is_initialized(void);

/**
 * @brief Verifica si hay un sensor presente
 * @return true si hay sensor
 */
bool nfc_is_sensor_present(void);

/**
 * @brief Obtiene la versión del firmware del módulo NFC
 * @return Versión del firmware
 */
uint32_t nfc_get_firmware_version(void);

/**
 * @brief Obtiene el contador de lecturas exitosas
 * @return Número de lecturas
 */
uint32_t nfc_get_read_count(void);

#ifdef __cplusplus
}
#endif

#endif /* NFC_DRIVER_H */
