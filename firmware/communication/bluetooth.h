/**
 * @file bluetooth.h
 * @brief Gestión de comunicación Bluetooth LE
 * 
 * Este módulo maneja la comunicación Bluetooth Low Energy
 * del dispositivo, permitiendo la conexión con dispositivos
 * externos para configuración y alertas.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * CONSTANTES
 * ================================================================== */

#define BT_DEVICE_NAME          "AsistenteIA-Geriatrico"
#define BT_MANUFACTURER_ID      0xFFFF

// Servicios y características UUIDs
#define BT_SERVICE_ALERT        "00001800-0000-1000-8000-00805F9B34FB"
#define BT_CHAR_ALERT            "00002A00-0000-1000-8000-00805F9B34FB"
#define BT_CHAR_BATTERY          "00002A19-0000-1000-8000-00805F9B34FB"
#define BT_CHAR_STATUS           "00002A01-0000-1000-8000-00805F9B34FB"

/* ==================================================================
 * TIPOS DE DATOS
 * ================================================================== */

/**
 * @brief Estado de la conexión Bluetooth
 */
typedef enum {
    BT_STATE_DISABLED = 0,       // Bluetooth desactivado
    BT_STATE_ADVERTISING,        // Publicitando (visible)
    BT_STATE_CONNECTED,          // Conectado a dispositivo
    BT_STATE_DISCONNECTED        // Desconectado pero activo
} bt_state_t;

/**
 * @brief Modos de operación Bluetooth
 */
typedef enum {
    BT_MODE_OFF = 0,              // Completamente apagado
    BT_MODE_ADVERTISING_ONLY,     // Solo publicitando (bajo consumo)
    BT_MODE_CONNECTABLE,          // Acepta conexiones
    BT_MODE_PAIRING               // Modo de emparejamiento
} bt_mode_t;

/**
 * @brief Información del dispositivo conectado
 */
typedef struct {
    char name[32];                  // Nombre del dispositivo
    uint8_t address[6];              // Dirección MAC
    bool bonded;                      // true si está vinculado
    uint32_t connected_since;         // Timestamp de conexión
} bt_connected_device_t;

/**
 * @brief Callbacks para eventos Bluetooth
 */
typedef struct {
    void (*on_connected)(bt_connected_device_t *dev);
    void (*on_disconnected)(void);
    void (*on_bonded)(bt_connected_device_t *dev);
    void (*on_alert_received)(uint8_t alert_type, const uint8_t *data, int len);
} bt_callbacks_t;

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa el sistema Bluetooth
 * @param mode Modo inicial de operación
 * @return ESP_OK en éxito
 */
esp_err_t bluetooth_init(bt_mode_t mode);

/**
 * @brief Configura el modo de operación
 * @param mode Nuevo modo
 * @return ESP_OK en éxito
 */
esp_err_t bluetooth_set_mode(bt_mode_t mode);

/**
 * @brief Obtiene el modo actual
 * @return Modo actual
 */
bt_mode_t bluetooth_get_mode(void);

/**
 * @brief Obtiene el estado actual
 * @return Estado actual
 */
bt_state_t bluetooth_get_state(void);

/* ==================================================================
 * FUNCIONES DE CONEXIÓN
 * ================================================================== */

/**
 * @brief Inicia publicidad (hace visible el dispositivo)
 * @return ESP_OK en éxito
 */
esp_err_t bluetooth_start_advertising(void);

/**
 * @brief Detiene la publicidad
 * @return ESP_OK en éxito
 */
esp_err_t bluetooth_stop_advertising(void);

/**
 * @brief Desconecta el dispositivo actual
 * @return ESP_OK en éxito
 */
esp_err_t bluetooth_disconnect(void);

/**
 * @brief Elimina todos los dispositivos vinculados
 * @return ESP_OK en éxito
 */
esp_err_t bluetooth_clear_bonding(void);

/**
 * @brief Obtiene información del dispositivo conectado
 * @param dev Puntero donde almacenar la información
 * @return true si hay dispositivo conectado
 */
bool bluetooth_get_connected_device(bt_connected_device_t *dev);

/* ==================================================================
 * FUNCIONES DE ENVÍO
 * ================================================================== */

/**
 * @brief Envía una alerta a través de Bluetooth
 * @param alert_type Tipo de alerta
 * @param data Datos adicionales
 * @param len Longitud de los datos
 * @return ESP_OK en éxito
 */
esp_err_t bluetooth_send_alert(uint8_t alert_type, const uint8_t *data, int len);

/**
 * @brief Envía el nivel de batería
 * @param battery_level Nivel de batería (0-100)
 * @return ESP_OK en éxito
 */
esp_err_t bluetooth_send_battery(uint8_t battery_level);

/**
 * @brief Envía el estado actual del dispositivo
 * @param status Código de estado
 * @return ESP_OK en éxito
 */
esp_err_t bluetooth_send_status(uint8_t status);

/* ==================================================================
 * FUNCIONES DE CONFIGURACIÓN
 * ================================================================== */

/**
 * @brief Configura el nombre del dispositivo
 * @param name Nuevo nombre
 * @return ESP_OK en éxito
 */
esp_err_t bluetooth_set_name(const char *name);

/**
 * @brief Configura callbacks para eventos
 * @param callbacks Puntero a estructura de callbacks
 */
void bluetooth_set_callbacks(bt_callbacks_t *callbacks);

/**
 * @brief Configura si el Bluetooth se activa al inicio
 * @param enable true para activar al inicio
 */
void bluetooth_set_auto_start(bool enable);

/**
 * @brief Habilita o deshabilita el Bluetooth (ahorro de energía)
 * @param enable true para habilitar
 * @return ESP_OK en éxito
 */
esp_err_t bluetooth_enable(bool enable);

/* ==================================================================
 * FUNCIONES DE SEGURIDAD
 * ================================================================== */

/**
 * @brief Verifica si el dispositivo está vinculado
 * @param address Dirección MAC a verificar
 * @return true si está vinculado
 */
bool bluetooth_is_bonded(const uint8_t *address);

/**
 * @brief Obtiene el número de dispositivos vinculados
 * @return Número de dispositivos
 */
int bluetooth_get_bonded_count(void);

/**
 * @brief Inicia el modo de emparejamiento
 * @return ESP_OK en éxito
 */
esp_err_t bluetooth_start_pairing(void);

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

/**
 * @brief Verifica si el Bluetooth está habilitado
 * @return true si está habilitado
 */
bool bluetooth_is_enabled(void);

/**
 * @brief Verifica si hay un dispositivo conectado
 * @return true si hay conexión
 */
bool bluetooth_is_connected(void);

/**
 * @brief Obtiene la intensidad de la señal (RSSI)
 * @return RSSI en dBm, 0 si no hay conexión
 */
int bluetooth_get_rssi(void);

/**
 * @brief Obtiene la versión del stack Bluetooth
 * @return Versión del stack
 */
uint32_t bluetooth_get_version(void);

/* ==================================================================
 * FUNCIONES DE ENERGÍA
 * ================================================================== */

/**
 * @brief Obtiene el consumo estimado del módulo Bluetooth
 * @return Consumo en µA
 */
uint32_t bluetooth_get_power_consumption(void);

/**
 * @brief Configura el intervalo de publicidad (afecta consumo)
 * @param interval_ms Intervalo en milisegundos
 * @return ESP_OK en éxito
 */
esp_err_t bluetooth_set_advertising_interval(uint32_t interval_ms);

#ifdef __cplusplus
}
#endif

#endif /* BLUETOOTH_H */
