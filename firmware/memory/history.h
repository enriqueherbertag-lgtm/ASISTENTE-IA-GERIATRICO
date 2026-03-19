/**
 * @file history.h
 * @brief Gestión del historial de eventos del sistema
 * 
 * Este módulo almacena y gestiona el historial de eventos
 * importantes como caídas, lecturas de glucosa, reconocimientos,
 * etc., para consulta posterior y estadísticas.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef HISTORY_H
#define HISTORY_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * CONSTANTES
 * ================================================================== */

#define MAX_HISTORY_ENTRIES 1000     // Máximo número de eventos en historial
#define MAX_EVENT_DESC_LEN  128       // Longitud máxima de descripción

/* ==================================================================
 * TIPOS DE EVENTOS
 * ================================================================== */

typedef enum {
    EVENT_NONE = 0,
    EVENT_SYSTEM_START,               // Sistema iniciado
    EVENT_SYSTEM_SHUTDOWN,            // Sistema apagado
    EVENT_BATTERY_LOW,                 // Batería baja
    EVENT_BATTERY_CRITICAL,            // Batería crítica
    EVENT_BATTERY_CHARGING,            // Comenzó a cargar
    EVENT_BATTERY_CHARGED,             // Carga completa
    
    EVENT_FALL_DETECTED,               // Caída detectada
    EVENT_FALL_CONFIRMED,              // Caída confirmada
    EVENT_FALL_FALSE_POSITIVE,         // Falsa alarma de caída
    
    EVENT_GLUCOSE_READING,              // Lectura de glucosa
    EVENT_GLUCOSE_LOW,                  // Glucosa baja
    EVENT_GLUCOSE_CRITICAL,             // Glucosa crítica
    EVENT_GLUCOSE_HIGH,                  // Glucosa alta
    
    EVENT_FACE_RECOGNIZED,              // Rostro reconocido
    EVENT_FACE_UNKNOWN,                 // Rostro no reconocido
    
    EVENT_VOICE_COMMAND,                 // Comando de voz ejecutado
    EVENT_VOICE_NOT_RECOGNIZED,          // Comando no reconocido
    
    EVENT_RECIPE_QUERIED,                // Receta consultada
    EVENT_RECIPE_COOKED,                  // Receta marcada como cocinada
    
    EVENT_REMINDER_TRIGGERED,             // Recordatorio activado
    
    EVENT_ERROR_GENERAL,                  // Error general
    EVENT_ERROR_SD,                        // Error de tarjeta SD
    EVENT_ERROR_SENSOR,                    // Error de sensor
    EVENT_ERROR_COMMS                       // Error de comunicación
} event_type_t;

/* ==================================================================
 * TIPOS DE DATOS
 * ================================================================== */

/**
 * @brief Entrada del historial
 */
typedef struct {
    uint32_t timestamp;                  // Timestamp del evento (ms)
    event_type_t type;                    // Tipo de evento
    uint8_t severity;                      // Severidad (0-10, mayor = más grave)
    union {
        struct {
            uint16_t glucose;               // Lectura de glucosa (mg/dL)
            uint8_t trend;                   // Tendencia
        } glucose;
        struct {
            uint8_t family_id;                // ID del familiar
            float confidence;                  // Confianza del reconocimiento
        } face;
        struct {
            uint8_t command_id;                // ID del comando
            float confidence;                   // Confianza
        } voice;
        struct {
            uint8_t recipe_id;                  // ID de la receta
        } recipe;
        struct {
            float magnitude;                     // Magnitud de la caída (g)
        } fall;
        struct {
            uint8_t soc;                         // Nivel de batería (%)
        } battery;
    } data;
    char description[MAX_EVENT_DESC_LEN];  // Descripción textual
} history_entry_t;

/**
 * @brief Estadísticas del historial
 */
typedef struct {
    uint32_t total_entries;                 // Total de eventos
    uint32_t events_by_type[64];            // Conteo por tipo de evento
    uint32_t first_event_time;               // Timestamp del primer evento
    uint32_t last_event_time;                 // Timestamp del último evento
    uint32_t last_saved_time;                  // Timestamp del último guardado
} history_stats_t;

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa el sistema de historial
 * @return ESP_OK en éxito
 */
esp_err_t history_init(void);

/**
 * @brief Agrega un evento al historial
 * @param type Tipo de evento
 * @param severity Severidad (0-10)
 * @param description Descripción del evento
 * @return ID del evento agregado, o -1 en error
 */
int history_add_event(event_type_t type, uint8_t severity, const char *description);

/**
 * @brief Agrega un evento con datos de glucosa
 * @param glucose Valor de glucosa
 * @param trend Tendencia
 * @param severity Severidad
 * @param description Descripción
 * @return ID del evento
 */
int history_add_glucose(uint16_t glucose, uint8_t trend, uint8_t severity, const char *description);

/**
 * @brief Agrega un evento de reconocimiento facial
 * @param family_id ID del familiar
 * @param confidence Confianza
 * @param description Descripción
 * @return ID del evento
 */
int history_add_face_recognition(uint8_t family_id, float confidence, const char *description);

/**
 * @brief Agrega un evento de caída
 * @param magnitude Magnitud de la caída
 * @param confirmed true si fue confirmada
 * @return ID del evento
 */
int history_add_fall(float magnitude, bool confirmed);

/**
 * @brief Agrega un evento de comando de voz
 * @param command_id ID del comando
 * @param confidence Confianza
 * @param description Descripción
 * @return ID del evento
 */
int history_add_voice_command(uint8_t command_id, float confidence, const char *description);

/* ==================================================================
 * FUNCIONES DE CONSULTA
 * ================================================================== */

/**
 * @brief Obtiene una entrada del historial por índice
 * @param index Índice (0 = más reciente)
 * @param entry Puntero donde almacenar la entrada
 * @return ESP_OK en éxito
 */
esp_err_t history_get_entry(int index, history_entry_t *entry);

/**
 * @brief Busca eventos por tipo
 * @param type Tipo de evento a buscar
 * @param results Buffer para resultados
 * @param max_results Máximo número de resultados
 * @return Número de eventos encontrados
 */
int history_find_by_type(event_type_t type, history_entry_t *results, int max_results);

/**
 * @brief Busca eventos en un rango de tiempo
 * @param start_time Tiempo de inicio (ms)
 * @param end_time Tiempo de fin (ms)
 * @param results Buffer para resultados
 * @param max_results Máximo número de resultados
 * @return Número de eventos encontrados
 */
int history_find_by_time(uint32_t start_time, uint32_t end_time, 
                          history_entry_t *results, int max_results);

/**
 * @brief Obtiene los últimos N eventos
 * @param count Número de eventos a obtener
 * @param results Buffer para resultados
 * @return Número de eventos obtenidos
 */
int history_get_last(int count, history_entry_t *results);

/* ==================================================================
 * FUNCIONES DE ESTADÍSTICAS
 * ================================================================== */

/**
 * @brief Obtiene estadísticas del historial
 * @param stats Puntero donde almacenar las estadísticas
 */
void history_get_stats(history_stats_t *stats);

/**
 * @brief Obtiene el número total de eventos
 * @return Número de eventos
 */
uint32_t history_get_count(void);

/**
 * @brief Obtiene el número de eventos de un tipo específico
 * @param type Tipo de evento
 * @return Número de eventos
 */
uint32_t history_get_count_by_type(event_type_t type);

/* ==================================================================
 * FUNCIONES DE PERSISTENCIA
 * ================================================================== */

/**
 * @brief Guarda el historial en la microSD
 * @return ESP_OK en éxito
 */
esp_err_t history_save(void);

/**
 * @brief Carga el historial desde la microSD
 * @return ESP_OK en éxito
 */
esp_err_t history_load(void);

/**
 * @brief Exporta el historial a formato CSV
 * @param filename Nombre del archivo CSV
 * @return ESP_OK en éxito
 */
esp_err_t history_export_csv(const char *filename);

/**
 * @brief Limpia todo el historial
 */
void history_clear(void);

/* ==================================================================
 * FUNCIONES DE UTILIDAD
 * ================================================================== */

/**
 * @brief Convierte tipo de evento a string
 * @param type Tipo de evento
 * @return String descriptivo (estático)
 */
const char* history_event_type_to_string(event_type_t type);

/**
 * @brief Convierte severidad a string
 * @param severity Severidad (0-10)
 * @return String descriptivo (estático)
 */
const char* history_severity_to_string(uint8_t severity);

#ifdef __cplusplus
}
#endif

#endif /* HISTORY_H */
