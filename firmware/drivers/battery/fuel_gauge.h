/**
 * @file fuel_gauge.h
 * @brief Driver para fuel gauge MAX17048 (gestión de batería)
 * 
 * Este módulo gestiona la monitorización del estado de la batería,
 * incluyendo nivel de carga, voltaje, corriente y alertas.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef FUEL_GAUGE_H
#define FUEL_GAUGE_H

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
 * @brief Estado de la batería
 */
typedef struct {
    uint8_t soc;                // State of Charge (0-100%)
    float voltage;              // Voltaje (V)
    float current;              // Corriente (mA, positivo = carga)
    float temperature;          // Temperatura (°C)
    bool charging;              // true si está cargando
    bool battery_low;           // true si batería baja (<15%)
    bool battery_critical;      // true si batería crítica (<5%)
    bool charge_complete;       // true si carga completa
    uint32_t time_remaining;    // Tiempo restante estimado (segundos)
} battery_status_t;

/**
 * @brief Modos de energía del fuel gauge
 */
typedef enum {
    FUEL_GAUGE_NORMAL = 0,      // Modo normal
    FUEL_GAUGE_SLEEP,           // Modo sleep (bajo consumo)
    FUEL_GAUGE_DEEPSLEEP        // Modo deep sleep (mínimo consumo)
} fuel_gauge_mode_t;

/**
 * @brief Eventos de batería
 */
typedef enum {
    BATTERY_EVENT_NONE = 0,
    BATTERY_EVENT_LOW,           // Batería baja (15%)
    BATTERY_EVENT_CRITICAL,      // Batería crítica (5%)
    BATTERY_EVENT_CHARGING,      // Comenzó a cargar
    BATTERY_EVENT_CHARGED,       // Carga completa
    BATTERY_EVENT_DISCHARGING    // Dejó de cargar
} battery_event_t;

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa el fuel gauge
 * @return ESP_OK en éxito, otro en error
 */
esp_err_t fuel_gauge_init(void);

/**
 * @brief Lee el estado actual de la batería
 * @param status Puntero donde almacenar el estado
 * @return ESP_OK en éxito
 */
esp_err_t fuel_gauge_read(battery_status_t *status);

/**
 * @brief Configura el modo de operación
 * @param mode Modo deseado
 * @return ESP_OK en éxito
 */
esp_err_t fuel_gauge_set_mode(fuel_gauge_mode_t mode);

/**
 * @brief Obtiene el modo actual
 * @return Modo actual
 */
fuel_gauge_mode_t fuel_gauge_get_mode(void);

/* ==================================================================
 * FUNCIONES DE LECTURA INDIVIDUAL
 * ================================================================== */

/**
 * @brief Lee el nivel de carga actual (State of Charge)
 * @return Porcentaje de batería (0-100)
 */
uint8_t fuel_gauge_get_soc(void);

/**
 * @brief Lee el voltaje actual de la batería
 * @return Voltaje en voltios
 */
float fuel_gauge_get_voltage(void);

/**
 * @brief Lee la corriente actual
 * @return Corriente en mA (positivo = carga)
 */
float fuel_gauge_get_current(void);

/**
 * @brief Lee la temperatura de la batería
 * @return Temperatura en °C
 */
float fuel_gauge_get_temperature(void);

/**
 * @brief Verifica si la batería está en carga
 * @return true si está cargando
 */
bool fuel_gauge_is_charging(void);

/**
 * @brief Verifica si la carga está completa
 * @return true si carga completa
 */
bool fuel_gauge_is_charge_complete(void);

/**
 * @brief Obtiene el tiempo restante estimado
 * @return Tiempo en segundos
 */
uint32_t fuel_gauge_get_time_remaining(void);

/* ==================================================================
 * FUNCIONES DE CONFIGURACIÓN
 * ================================================================== */

/**
 * @brief Configura umbrales de alerta
 * @param low Umbral de batería baja (%)
 * @param critical Umbral de batería crítica (%)
 */
void fuel_gauge_set_thresholds(uint8_t low, uint8_t critical);

/**
 * @brief Configura la capacidad de la batería
 * @param capacity_mah Capacidad en mAh
 */
void fuel_gauge_set_battery_capacity(uint16_t capacity_mah);

/**
 * @brief Calibra el fuel gauge
 * @param full_voltage Voltaje a plena carga (mV)
 * @param empty_voltage Voltaje a carga agotada (mV)
 * @return ESP_OK en éxito
 */
esp_err_t fuel_gauge_calibrate(uint16_t full_voltage, uint16_t empty_voltage);

/**
 * @brief Reinicia el contador de ciclos de carga
 */
void fuel_gauge_reset_cycles(void);

/* ==================================================================
 * FUNCIONES DE ALERTAS
 * ================================================================== */

/**
 * @brief Verifica si hay un evento de batería pendiente
 * @return Evento detectado
 */
battery_event_t fuel_gauge_check_event(void);

/**
 * @brief Habilita las interrupciones por alerta
 * @param enable true para habilitar
 */
void fuel_gauge_enable_interrupt(bool enable);

/**
 * @brief Obtiene el último evento registrado
 * @return Último evento
 */
battery_event_t fuel_gauge_get_last_event(void);

/* ==================================================================
 * FUNCIONES DE ENERGÍA
 * ================================================================== */

/**
 * @brief Pone el fuel gauge en bajo consumo
 * @return ESP_OK en éxito
 */
esp_err_t fuel_gauge_sleep(void);

/**
 * @brief Despierta el fuel gauge
 * @return ESP_OK en éxito
 */
esp_err_t fuel_gauge_wake(void);

/**
 * @brief Obtiene el consumo estimado del fuel gauge
 * @return Consumo en µA
 */
uint32_t fuel_gauge_get_self_consumption(void);

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

/**
 * @brief Verifica si el fuel gauge está inicializado
 * @return true si está inicializado
 */
bool fuel_gauge_is_initialized(void);

/**
 * @brief Verifica si el fuel gauge está presente en el bus I2C
 * @return true si está presente
 */
bool fuel_gauge_is_present(void);

/**
 * @brief Obtiene la versión del chip
 * @return Versión
 */
uint16_t fuel_gauge_get_version(void);

/**
 * @brief Obtiene el número de ciclos de carga completados
 * @return Número de ciclos
 */
uint16_t fuel_gauge_get_cycles(void);

/**
 * @brief Obtiene la salud estimada de la batería
 * @return Porcentaje de salud (0-100)
 */
uint8_t fuel_gauge_get_health(void);

#ifdef __cplusplus
}
#endif

#endif /* FUEL_GAUGE_H */
