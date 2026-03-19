/**
 * @file face_db.h
 * @brief Base de datos de reconocimiento facial
 * 
 * Este módulo gestiona el almacenamiento y consulta de embeddings
 * faciales de familiares para reconocimiento automático.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef FACE_DB_H
#define FACE_DB_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * CONSTANTES
 * ================================================================== */

#define MAX_FAMILY_FACES    10      // Máximo de familiares reconocibles
#define FACE_EMBEDDING_SIZE 128     // Tamaño del embedding facial (bytes)
#define MAX_NAME_LEN        32      // Longitud máxima del nombre
#define MAX_RELATION_LEN    32      // Longitud máxima de la relación

/* ==================================================================
 * TIPOS DE DATOS
 * ================================================================== */

/**
 * @brief Embedding facial de una persona
 */
typedef struct {
    uint8_t data[FACE_EMBEDDING_SIZE];  // Vector de características faciales
    float norm;                           // Norma del vector (para comparación)
} face_embedding_t;

/**
 * @brief Información de un familiar registrado
 */
typedef struct {
    uint8_t id;                           // ID del familiar
    char name[MAX_NAME_LEN];               // Nombre
    char relation[MAX_RELATION_LEN];       // Relación (hijo, hija, etc.)
    face_embedding_t embedding;            // Embedding facial
    uint8_t sample_count;                   // Número de muestras usadas para crear el embedding
    uint32_t last_seen;                      // Timestamp de última vez visto
    uint32_t times_seen;                      // Número de veces visto
    uint8_t priority;                         // Prioridad (0-10, mayor = más prioritario)
} family_face_t;

/**
 * @brief Resultado de reconocimiento facial
 */
typedef struct {
    uint8_t family_id;                       // ID del familiar reconocido
    float confidence;                         // Confianza (0-1)
    uint32_t recognition_time;                 // Timestamp del reconocimiento
} face_recognition_result_t;

/**
 * @brief Estadísticas de la base de datos facial
 */
typedef struct {
    uint32_t total_faces;                      // Total de familiares registrados
    uint32_t total_recognitions;                // Total de reconocimientos exitosos
    uint32_t total_failures;                     // Total de fallos de reconocimiento
    char most_seen[MAX_NAME_LEN];                // Familiar más visto
    uint32_t last_recognition_time;              // Timestamp del último reconocimiento
} face_db_stats_t;

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa la base de datos facial
 * @return ESP_OK en éxito
 */
esp_err_t face_db_init(void);

/**
 * @brief Carga la base de datos desde la microSD
 * @return ESP_OK en éxito
 */
esp_err_t face_db_load(void);

/**
 * @brief Guarda la base de datos en la microSD
 * @return ESP_OK en éxito
 */
esp_err_t face_db_save(void);

/* ==================================================================
 * FUNCIONES DE REGISTRO
 * ================================================================== */

/**
 * @brief Registra un nuevo familiar
 * @param name Nombre del familiar
 * @param relation Relación
 * @param embedding Embedding facial
 * @param priority Prioridad (0-10)
 * @return ID del familiar, o -1 en error
 */
int face_db_add(const char *name, const char *relation, const face_embedding_t *embedding, uint8_t priority);

/**
 * @brief Actualiza el embedding de un familiar
 * @param id ID del familiar
 * @param embedding Nuevo embedding
 * @return ESP_OK en éxito
 */
esp_err_t face_db_update_embedding(uint8_t id, const face_embedding_t *embedding);

/**
 * @brief Elimina un familiar
 * @param id ID del familiar
 * @return ESP_OK en éxito
 */
esp_err_t face_db_delete(uint8_t id);

/* ==================================================================
 * FUNCIONES DE RECONOCIMIENTO
 * ================================================================== */

/**
 * @brief Reconoce un rostro por su embedding
 * @param embedding Embedding a reconocer
 * @param threshold Umbral de confianza mínimo (0-1)
 * @return Resultado del reconocimiento
 */
face_recognition_result_t face_db_recognize(const face_embedding_t *embedding, float threshold);

/**
 * @brief Reconoce un rostro y actualiza estadísticas
 * @param embedding Embedding a reconocer
 * @param threshold Umbral de confianza
 * @return Resultado del reconocimiento
 */
face_recognition_result_t face_db_recognize_and_update(const face_embedding_t *embedding, float threshold);

/**
 * @brief Compara dos embeddings
 * @param emb1 Primer embedding
 * @param emb2 Segundo embedding
 * @return Similitud (0-1, mayor = más similar)
 */
float face_db_compare(const face_embedding_t *emb1, const face_embedding_t *emb2);

/* ==================================================================
 * FUNCIONES DE CONSULTA
 * ================================================================== */

/**
 * @brief Obtiene la información de un familiar
 * @param id ID del familiar
 * @return Puntero a la información, NULL si no existe
 */
const family_face_t* face_db_get(uint8_t id);

/**
 * @brief Obtiene todos los familiares registrados
 * @param faces Buffer para almacenar los familiares
 * @param max_count Máximo número a obtener
 * @return Número de familiares obtenidos
 */
int face_db_get_all(family_face_t *faces, int max_count);

/**
 * @brief Busca un familiar por nombre
 * @param name Nombre a buscar
 * @return ID del familiar, o -1 si no existe
 */
int face_db_find_by_name(const char *name);

/**
 * @brief Verifica si un ID es válido
 * @param id ID a verificar
 * @return true si es válido
 */
bool face_db_id_valid(uint8_t id);

/* ==================================================================
 * FUNCIONES DE ACTUALIZACIÓN
 * ================================================================== */

/**
 * @brief Registra que un familiar fue visto
 * @param id ID del familiar
 */
void face_db_update_last_seen(uint8_t id);

/**
 * @brief Incrementa el contador de fallos de reconocimiento
 */
void face_db_increment_failures(void);

/**
 * @brief Obtiene el número total de familiares registrados
 * @return Número de familiares
 */
uint32_t face_db_get_count(void);

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

/**
 * @brief Obtiene estadísticas de la base de datos facial
 * @param stats Puntero donde almacenar las estadísticas
 */
void face_db_get_stats(face_db_stats_t *stats);

/**
 * @brief Verifica si la base de datos está cargada
 * @return true si está cargada
 */
bool face_db_is_loaded(void);

/**
 * @brief Obtiene el embedding por defecto (para rostros no reconocidos)
 * @return Embedding por defecto
 */
const face_embedding_t* face_db_get_default_embedding(void);

#ifdef __cplusplus
}
#endif

#endif /* FACE_DB_H */
