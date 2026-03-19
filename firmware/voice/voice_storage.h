/**
 * @file voice_storage.h
 * @brief Gestión de almacenamiento de mensajes de voz
 * 
 * Este módulo maneja la lectura y escritura de archivos de audio
 * en la microSD, incluyendo mensajes pregrabados y voces familiares.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef VOICE_STORAGE_H
#define VOICE_STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * CONSTANTES
 * ================================================================== */

#define MAX_VOICE_MESSAGES  64      // Máximo número de mensajes
#define MAX_VOICE_NAME_LEN   32      // Longitud máxima del nombre
#define MAX_FAMILY_MEMBERS   10      // Máximo número de familiares

/* ==================================================================
 * TIPOS DE DATOS
 * ================================================================== */

/**
 * @brief Metadatos de un mensaje de voz
 */
typedef struct {
    uint8_t id;                      // ID del mensaje (0-63)
    char name[MAX_VOICE_NAME_LEN];   // Nombre descriptivo
    char filename[64];                // Nombre del archivo en microSD
    uint32_t duration_ms;             // Duración en ms
    uint8_t family_member;            // ID del familiar (0-9, 255 = genérico)
    bool is_fallback;                 // true si es mensaje por defecto
} voice_metadata_t;

/**
 * @brief Información de un familiar
 */
typedef struct {
    uint8_t id;                       // ID del familiar
    char name[MAX_VOICE_NAME_LEN];    // Nombre del familiar
    char relation[32];                 // Relación (hijo, hija, nieto, etc.)
    uint32_t voice_count;              // Número de mensajes de este familiar
    uint8_t priority;                  // Prioridad (0-10, mayor = más prioritario)
} family_member_t;

/**
 * @brief Estadísticas de almacenamiento de voz
 */
typedef struct {
    uint32_t total_messages;           // Total de mensajes almacenados
    uint32_t used_space_bytes;         // Espacio usado en bytes
    uint32_t free_space_bytes;         // Espacio libre en bytes
    uint8_t family_count;               // Número de familiares registrados
} voice_storage_stats_t;

/* ==================================================================
 * FUNCIONES DE INICIALIZACIÓN
 * ================================================================== */

/**
 * @brief Inicializa el sistema de almacenamiento de voz
 * @return ESP_OK en éxito, otro en error
 */
esp_err_t voice_storage_init(void);

/**
 * @brief Monta la tarjeta microSD
 * @return ESP_OK en éxito
 */
esp_err_t voice_storage_mount_sd(void);

/**
 * @brief Desmonta la tarjeta microSD
 * @return ESP_OK en éxito
 */
esp_err_t voice_storage_unmount_sd(void);

/* ==================================================================
 * FUNCIONES DE GESTIÓN DE MENSAJES
 * ================================================================== */

/**
 * @brief Carga todos los metadatos de mensajes desde la microSD
 * @return ESP_OK en éxito
 */
esp_err_t voice_storage_load_all(void);

/**
 * @brief Obtiene los metadatos de un mensaje por ID
 * @param id ID del mensaje
 * @return Puntero a metadatos, NULL si no existe
 */
const voice_metadata_t* voice_storage_get_metadata(uint8_t id);

/**
 * @brief Obtiene la ruta completa de un archivo de voz
 * @param id ID del mensaje
 * @param path Buffer para la ruta
 * @param max_len Tamaño del buffer
 * @return ESP_OK en éxito
 */
esp_err_t voice_storage_get_path(uint8_t id, char *path, size_t max_len);

/**
 * @brief Verifica si un mensaje existe
 * @param id ID del mensaje
 * @return true si existe
 */
bool voice_storage_message_exists(uint8_t id);

/**
 * @brief Obtiene el número total de mensajes
 * @return Número de mensajes
 */
uint32_t voice_storage_get_message_count(void);

/* ==================================================================
 * FUNCIONES DE GESTIÓN DE FAMILIARES
 * ================================================================== */

/**
 * @brief Registra un nuevo familiar
 * @param name Nombre del familiar
 * @param relation Relación
 * @param priority Prioridad (0-10)
 * @return ID del familiar, o -1 en error
 */
int voice_storage_add_family_member(const char *name, const char *relation, uint8_t priority);

/**
 * @brief Obtiene información de un familiar por ID
 * @param id ID del familiar
 * @return Puntero a información, NULL si no existe
 */
const family_member_t* voice_storage_get_family_member(uint8_t id);

/**
 * @brief Obtiene todos los familiares registrados
 * @param members Buffer para almacenar los familiares
 * @param max_count Máximo número a obtener
 * @return Número de familiares obtenidos
 */
int voice_storage_get_family_members(family_member_t *members, int max_count);

/**
 * @brief Obtiene los mensajes de un familiar específico
 * @param family_id ID del familiar
 * @param messages Buffer para metadatos
 * @param max_count Máximo número a obtener
 * @return Número de mensajes obtenidos
 */
int voice_storage_get_family_messages(uint8_t family_id, voice_metadata_t *messages, int max_count);

/* ==================================================================
 * FUNCIONES DE REPRODUCCIÓN
 * ================================================================== */

/**
 * @brief Busca el mejor mensaje para una situación
 * @param context Contexto (ej: "saludo", "alerta_caida", "recordatorio")
 * @param family_id ID del familiar preferido (255 = cualquiera)
 * @return ID del mensaje, 0 si no hay
 */
uint8_t voice_storage_find_best_message(const char *context, uint8_t family_id);

/**
 * @brief Obtiene el mensaje de fallback para una situación
 * @param context Contexto
 * @return ID del mensaje de fallback
 */
uint8_t voice_storage_get_fallback_message(const char *context);

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

/**
 * @brief Verifica si la microSD está montada
 * @return true si está montada
 */
bool voice_storage_is_mounted(void);

/**
 * @brief Obtiene estadísticas de almacenamiento
 * @param stats Puntero donde almacenar las estadísticas
 */
void voice_storage_get_stats(voice_storage_stats_t *stats);

/**
 * @brief Escanea la microSD en busca de nuevos archivos
 * @return Número de nuevos mensajes encontrados
 */
int voice_storage_scan_for_new(void);

#ifdef __cplusplus
}
#endif

#endif /* VOICE_STORAGE_H */
