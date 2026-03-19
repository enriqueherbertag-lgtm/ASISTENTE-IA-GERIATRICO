/**
 * @file glucose_alert.h
 * @brief Algoritmo de alertas de glucosa
 * 
 * Este módulo procesa lecturas del sensor FreeStyle Libre
 * y genera alertas según los niveles de glucosa y tendencias.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef GLUCOSE_ALERT_H
#define GLUCOSE_ALERT_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "nfc_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * NIVELES DE ALERTA
 * ================================================================== */

#define GLUCOSE_ALERT_NONE      0
#define GLUCOSE_ALERT_LOW       1   // Hipoglucemia
#define GLUCOSE_ALERT_HIGH      2   // Hiperglucemia
#define GLUCOSE_ALERT_CRITICAL  3   // Hipoglucemia crítica

/* ==================================================================
 * TIPOS DE DATOS
 * ================================================================== */

/**
 * @brief Lectura de glucosa con timestamp
 */
typedef struct {
    uint32_t timestamp;         // Timestamp de la lectura (ms)
    uint16_t glucose;           // Glucosa en mg/dL
    uint8_t trend;              // Tendencia (0-7)
} glucose_reading_t;

/**
 * @brief Estadísticas de glucosa
 */
typedef struct {
    uint32_t total_readings;    // Total de lecturas
    uint32_t hypo_count;        // Eventos de hipoglucemia
    uint32_t hyper_count;       // Eventos de hiperglucemia
    uint32_t alert_count;       // Alertas generadas
    uint16_t last_glucose;       // Última lectura
    uint16_t average_glucose;    // Promedio histórico
    uint16_t min_glucose;        // Mínimo histórico
    uint16_t max_glucose;        // Máximo histórico
} glucose_stats_t;

/**
 * @brief Callback para notificación de alertas
 * @param data Datos de glucosa
 * @param alert_level Nivel de alerta
 * @param arg Argumento definido por el usuario
 */
typedef void (*glucose_callback_t)(glucose_data_t *data, uint8_t alert_level, void *arg);

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa el sistema de alertas de glucosa
 * @return ESP_OK en éxito
 */
esp_err_t glucose_alert_init(void);

/**
 * @brief Procesa una nueva lectura de glucosa y genera alertas
 * @param data Datos de glucosa leídos
 * @return Nivel de alerta generado (0 = ninguna)
 */
uint8_t glucose_alert_check(glucose_data_t *data);

/* ==================================================================
 * FUNCIONES DE CONFIGURACIÓN
 * ================================================================== */

/**
 * @brief Configura umbrales de alerta
 * @param hypo Umbral de hipoglucemia (mg/dL)
 * @param critical Umbral de hipoglucemia crítica (mg/dL)
 * @param hyper Umbral de hiperglucemia (mg/dL)
 */
void glucose_alert_set_thresholds(uint16_t hypo, uint16_t critical, uint16_t hyper);

/**
 * @brief Habilita o deshabilita las alertas
 * @param enable true para habilitar
 */
void glucose_alert_enable(bool enable);

/**
 * @brief Configura callback para notificación de alertas
 * @param callback Función a llamar cuando hay alerta
 * @param arg Argumento para el callback
 */
void glucose_alert_set_callback(glucose_callback_t callback, void *arg);

/* ==================================================================
 * FUNCIONES DE CONSULTA
 * ================================================================== */

/**
 * @brief Obtiene la última lectura
 * @return Última lectura
 */
glucose_data_t glucose_alert_get_last_reading(void);

/**
 * @brief Obtiene el timestamp de la última lectura
 * @return Timestamp en ms
 */
uint32_t glucose_alert_get_last_read_time(void);

/**
 * @brief Obtiene el historial de lecturas
 * @param buffer Buffer para almacenar las lecturas
 * @param count Puntero donde almacenar el número de lecturas
 * @param max Máximo número de lecturas a copiar
 */
void glucose_alert_get_history(glucose_reading_t *buffer, int *count, int max);

/**
 * @brief Obtiene estadísticas de glucosa
 * @return Estadísticas actuales
 */
glucose_stats_t glucose_alert_get_stats(void);

/**
 * @brief Verifica si una tendencia es peligrosa
 * @param trend Código de tendencia (0-7)
 * @return true si es peligrosa
 */
bool glucose_alert_is_trend_dangerous(uint8_t trend);

/**
 * @brief Calcula tiempo estimado para alcanzar un valor objetivo
 * @param current Lectura actual
 * @param target Valor objetivo (mg/dL)
 * @param rate_mgdl_per_min Tasa de cambio (mg/dL por minuto)
 * @return Tiempo estimado en segundos
 */
uint32_t glucose_alert_time_to_target(glucose_data_t *current, uint16_t target, float rate_mgdl_per_min);

/**
 * @brief Reinicia las estadísticas
 */
void glucose_alert_reset_stats(void);

#ifdef __cplusplus
}
#endif

#endif /* GLUCOSE_ALERT_H */
