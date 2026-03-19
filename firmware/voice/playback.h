/**
 * @file playback.h
 * @brief Gestión de reproducción de mensajes de voz
 * 
 * Este módulo maneja la reproducción de mensajes de voz,
 * incluyendo colas de reproducción, prioridades y control de volumen.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * CONSTANTES
 * ================================================================== */

#define PRIORITY_LOW        0
#define PRIORITY_NORMAL     128
#define PRIORITY_HIGH       192
#define PRIORITY_URGENT     255

/* ==================================================================
 * TIPOS DE DATOS
 * ================================================================== */

/**
 * @brief Estadísticas de reproducción
 */
typedef struct {
    uint32_t total_playbacks;        // Total de reproducciones
    uint32_t interrupted_count;      // Reproducciones interrumpidas
    uint32_t last_message_id;         // Último mensaje reproducido
    uint32_t last_playback_time;      // Timestamp de última reproducción
} playback_stats_t;

/**
 * @brief Callbacks para eventos de reproducción
 */
typedef struct {
    void (*on_playback_start)(uint8_t message_id);  // Al iniciar reproducción
    void (*on_playback_end)(uint8_t message_id);    // Al terminar reproducción
    void (*on_playback_interrupted)(uint8_t message_id); // Al ser interrumpido
} playback_callbacks_t;

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa el sistema de reproducción
 * @return ESP_OK en éxito
 */
esp_err_t playback_init(void);

/**
 * @brief Reproduce un mensaje de voz
 * @param message_id ID del mensaje
 * @param priority Prioridad (0-255, mayor = más prioritario)
 * @param interrupt true para interrumpir reproducción actual
 * @return ESP_OK en éxito
 */
esp_err_t playback_play(uint8_t message_id, uint8_t priority, bool interrupt);

/**
 * @brief Reproduce un mensaje urgente (prioridad máxima, interrumpe)
 * @param message_id ID del mensaje
 * @return ESP_OK en éxito
 */
esp_err_t playback_play_urgent(uint8_t message_id);

/**
 * @brief Reproduce un mensaje de fondo (baja prioridad)
 * @param message_id ID del mensaje
 * @return ESP_OK en éxito
 */
esp_err_t playback_play_background(uint8_t message_id);

/**
 * @brief Detiene la reproducción actual y vacía la cola
 */
void playback_stop(void);

/**
 * @brief Pausa la reproducción actual
 */
void playback_pause(void);

/**
 * @brief Reanuda la reproducción pausada
 */
void playback_resume(void);

/* ==================================================================
 * FUNCIONES DE CONSULTA
 * ================================================================== */

/**
 * @brief Verifica si hay reproducción activa
 * @return true si está reproduciendo
 */
bool playback_is_playing(void);

/**
 * @brief Obtiene el ID del mensaje actual
 * @return ID del mensaje, 0 si no hay
 */
uint8_t playback_get_current_message(void);

/**
 * @brief Obtiene el volumen actual
 * @return Volumen (0-100)
 */
uint8_t playback_get_volume(void);

/**
 * @brief Obtiene el tamaño de la cola de reproducción
 * @return Número de mensajes en cola
 */
uint32_t playback_get_queue_size(void);

/**
 * @brief Verifica si un mensaje específico está sonando
 * @param message_id ID del mensaje
 * @return true si está sonando
 */
bool playback_is_message_playing(uint8_t message_id);

/* ==================================================================
 * FUNCIONES DE CONFIGURACIÓN
 * ================================================================== */

/**
 * @brief Configura el volumen de reproducción
 * @param volume Volumen (0-100)
 */
void playback_set_volume(uint8_t volume);

/**
 * @brief Configura callbacks para eventos de reproducción
 * @param callbacks Puntero a estructura de callbacks
 */
void playback_set_callbacks(playback_callbacks_t *callbacks);

/**
 * @brief Limpia la cola de reproducción
 */
void playback_clear_queue(void);

/**
 * @brief Obtiene estadísticas de reproducción
 * @param stats Puntero donde almacenar las estadísticas
 */
void playback_get_stats(playback_stats_t *stats);

/**
 * @brief Precarga un mensaje en RAM para reproducción inmediata
 * @param message_id ID del mensaje
 * @return ESP_OK en éxito
 */
esp_err_t playback_preload(uint8_t message_id);

#ifdef __cplusplus
}
#endif

#endif /* PLAYBACK_H */
