/**
 * @file fall_detection.h
 * @brief Algoritmo de detección de caídas
 * 
 * Este módulo procesa datos del IMU para detectar caídas
 * en tiempo real y gestionar las alertas correspondientes.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef FALL_DETECTION_H
#define FALL_DETECTION_H

#include <stdint.h>
#include <stdbool.h>
#include "imu_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * TIPOS DE DATOS
 * ================================================================== */

/**
 * @brief Estados de la máquina de detección de caídas
 */
typedef enum {
    FALL_STATE_NORMAL = 0,       // Estado normal (sin caída)
    FALL_STATE_FREE_FALL,        // Caída libre detectada
    FALL_STATE_IMPACT,           // Impacto detectado
    FALL_STATE_POST_FALL,        // Post-impacto (esperando confirmación)
    FALL_STATE_FALL_CONFIRMED    // Caída confirmada
} fall_state_t;

/**
 * @brief Estadísticas de detección de caídas
 */
typedef struct {
    uint32_t fall_count;           // Número de caídas detectadas
    uint32_t false_positive_count; // Número de falsos positivos
    fall_state_t current_state;    // Estado actual
    uint32_t last_alert_time;      // Timestamp de última alerta
} fall_stats_t;

/**
 * @brief Callback para notificación de caídas
 * @param arg Argumento definido por el usuario
 */
typedef void (*fall_callback_t)(void *arg);

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa el detector de caídas
 */
void fall_detection_init(void);

/**
 * @brief Procesa una nueva lectura del IMU
 * @param data Datos del IMU
 * @return Estado actual de la detección
 */
fall_state_t fall_detection_process(imu_data_t *data);

/**
 * @brief Reinicia el detector de caídas
 */
void fall_detection_reset(void);

/* ==================================================================
 * FUNCIONES DE CONSULTA
 * ================================================================== */

/**
 * @brief Verifica si se está produciendo una caída
 * @return true si está cayendo
 */
bool fall_detection_is_falling(void);

/**
 * @brief Verifica si se ha producido una caída
 * @return true si hay caída confirmada
 */
bool fall_detection_is_fallen(void);

/**
 * @brief Obtiene el estado actual
 * @return Estado actual
 */
fall_state_t fall_detection_get_state(void);

/**
 * @brief Convierte estado a string
 * @param state Estado a convertir
 * @return String descriptivo
 */
const char* fall_detection_state_to_string(fall_state_t state);

/**
 * @brief Obtiene el contador de caídas
 * @return Número de caídas detectadas
 */
uint32_t fall_detection_get_count(void);

/**
 * @brief Obtiene el contador de falsos positivos
 * @return Número de falsos positivos
 */
uint32_t fall_detection_get_false_positives(void);

/**
 * @brief Obtiene estadísticas completas
 * @param stats Puntero donde almacenar las estadísticas
 */
void fall_detection_get_stats(fall_stats_t *stats);

/**
 * @brief Determina si se debe enviar una alerta
 * @param cooldown_ms Tiempo mínimo entre alertas
 * @return true si se debe alertar
 */
bool fall_detection_should_alert(uint32_t cooldown_ms);

/**
 * @brief Obtiene nivel de confianza de la detección
 * @return Confianza (0-1)
 */
float fall_detection_get_confidence(void);

/* ==================================================================
 * FUNCIONES DE CONFIGURACIÓN
 * ================================================================== */

/**
 * @brief Configura callback para notificación de caídas
 * @param callback Función a llamar cuando se detecta caída
 * @param arg Argumento para el callback
 */
void fall_detection_set_callback(fall_callback_t callback, void *arg);

/**
 * @brief Configura umbrales de detección
 * @param free_fall Umbral de caída libre (g)
 * @param impact Umbral de impacto (g)
 * @param fall Umbral de caída confirmada (g)
 */
void fall_detection_set_thresholds(float free_fall, float impact, float fall);

/**
 * @brief Configura tiempos de detección
 * @param post_fall_ms Tiempo post-impacto (ms)
 * @param inactivity_ms Tiempo de inactividad para confirmar (ms)
 */
void fall_detection_set_timings(uint32_t post_fall_ms, uint32_t inactivity_ms);

#ifdef __cplusplus
}
#endif

#endif /* FALL_DETECTION_H */
