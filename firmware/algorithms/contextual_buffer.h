/**
 * @file contextual_buffer.h
 * @brief Buffer de contexto conversacional
 * 
 * Este módulo mantiene un buffer circular de las últimas
 * conversaciones para permitir preguntas como "¿qué estaba diciendo?"
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef CONTEXTUAL_BUFFER_H
#define CONTEXTUAL_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * ETIQUETAS EMOCIONALES
 * ================================================================== */

#define EMOTION_NEUTRAL    0
#define EMOTION_HAPPY      1
#define EMOTION_ANXIOUS    2
#define EMOTION_CONFUSED   3

/* ==================================================================
 * TIPOS DE DATOS
 * ================================================================== */

/**
 * @brief Estadísticas del buffer contextual
 */
typedef struct {
    uint32_t total_entries;          // Total de entradas desde inicio
    uint32_t valid_entries;          // Entradas válidas actualmente
    uint32_t buffer_size;             // Tamaño máximo del buffer
    uint32_t duration_seconds;        // Duración en segundos
    uint32_t emotional_distribution[4]; // Distribución por emoción
} context_buffer_stats_t;

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa el buffer contextual
 */
void contextual_buffer_init(void);

/**
 * @brief Agrega un fragmento de texto al buffer
 * @param text Texto a agregar
 * @param emotional_tag Etiqueta emocional (0-3)
 */
void contextual_buffer_add(const char *text, uint8_t emotional_tag);

/**
 * @brief Obtiene el último fragmento de texto del buffer
 * @param max_age_seconds Edad máxima en segundos
 * @return Puntero al texto (estático) o NULL si no hay
 */
const char* contextual_buffer_get_last(uint32_t max_age_seconds);

/**
 * @brief Obtiene el último fragmento con su etiqueta emocional
 * @param buffer_out Buffer para el texto
 * @param max_len Tamaño del buffer
 * @param tag Puntero donde almacenar la etiqueta
 * @return true si se encontró
 */
bool contextual_buffer_get_last_with_tag(char *buffer_out, size_t max_len, uint8_t *tag);

/**
 * @brief Busca fragmentos que contengan una palabra clave
 * @param keyword Palabra clave a buscar
 * @param result Buffer para el primer resultado
 * @param max_len Tamaño del buffer
 * @return Número de fragmentos encontrados
 */
int contextual_buffer_search(const char *keyword, char *result, size_t max_len);

/**
 * @brief Obtiene el timestamp de la última entrada
 * @return Timestamp en ms, 0 si no hay
 */
uint32_t contextual_buffer_get_last_time(void);

/**
 * @brief Limpia todo el buffer
 */
void contextual_buffer_clear(void);

/**
 * @brief Obtiene el número de entradas válidas
 * @return Número de entradas
 */
int contextual_buffer_get_count(void);

/**
 * @brief Verifica si hay entradas disponibles
 * @return true si hay entradas
 */
bool contextual_buffer_is_available(void);

/**
 * @brief Elimina la última entrada
 */
void contextual_buffer_remove_last(void);

/**
 * @brief Obtiene estadísticas del buffer
 * @param stats Puntero donde almacenar las estadísticas
 */
void contextual_buffer_get_stats(context_buffer_stats_t *stats);

#ifdef __cplusplus
}
#endif

#endif /* CONTEXTUAL_BUFFER_H */
