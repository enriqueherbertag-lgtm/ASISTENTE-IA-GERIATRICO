/**
 * @file recipe_db.h
 * @brief Base de datos de recetas familiares
 * 
 * Este módulo gestiona el almacenamiento y consulta de recetas
 * familiares, incluyendo ingredientes, pasos y metadatos.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef RECIPE_DB_H
#define RECIPE_DB_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * CONSTANTES
 * ================================================================== */

#define MAX_RECIPES         50      // Máximo número de recetas
#define MAX_NAME_LEN        64      // Longitud máxima del nombre
#define MAX_INGREDIENTS      20      // Máximo número de ingredientes
#define MAX_INGREDIENT_LEN   64      // Longitud de cada ingrediente
#define MAX_STEPS            20      // Máximo número de pasos
#define MAX_STEP_LEN        256      // Longitud de cada paso
#define MAX_CATEGORY_LEN     32      // Longitud de la categoría

/* ==================================================================
 * TIPOS DE DATOS
 * ================================================================== */

/**
 * @brief Ingrediente de una receta
 */
typedef struct {
    char name[MAX_INGREDIENT_LEN];       // Nombre del ingrediente
    char quantity[32];                     // Cantidad (ej: "2 tazas")
    bool optional;                          // true si es opcional
} recipe_ingredient_t;

/**
 * @brief Paso de una receta
 */
typedef struct {
    char description[MAX_STEP_LEN];        // Descripción del paso
    uint16_t duration_seconds;              // Duración estimada (opcional)
} recipe_step_t;

/**
 * @brief Metadatos de una receta
 */
typedef struct {
    uint8_t id;                              // ID de la receta
    char name[MAX_NAME_LEN];                  // Nombre de la receta
    char category[MAX_CATEGORY_LEN];          // Categoría (postre, principal, etc.)
    uint8_t servings;                          // Número de porciones
    uint16_t prep_time_minutes;                // Tiempo de preparación
    uint16_t cook_time_minutes;                // Tiempo de cocción
    uint8_t difficulty;                         // Dificultad (1-5)
    bool favorite;                              // Receta favorita
    uint32_t times_cooked;                      // Veces que se ha cocinado
    uint32_t last_cooked;                        // Timestamp de última vez
} recipe_metadata_t;

/**
 * @brief Receta completa
 */
typedef struct {
    recipe_metadata_t metadata;                 // Metadatos
    recipe_ingredient_t ingredients[MAX_INGREDIENTS]; // Ingredientes
    int ingredient_count;                        // Número de ingredientes
    recipe_step_t steps[MAX_STEPS];              // Pasos
    int step_count;                               // Número de pasos
    char notes[512];                              // Notas adicionales
} recipe_t;

/**
 * @brief Estadísticas de la base de datos de recetas
 */
typedef struct {
    uint32_t total_recipes;                       // Total de recetas
    uint32_t favorite_count;                       // Recetas favoritas
    uint32_t total_cooked;                          // Total de veces cocinadas
    char most_cooked[MAX_NAME_LEN];                 // Receta más cocinada
    char last_cooked[MAX_NAME_LEN];                 // Última receta cocinada
} recipe_stats_t;

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa la base de datos de recetas
 * @return ESP_OK en éxito
 */
esp_err_t recipe_db_init(void);

/**
 * @brief Carga la base de datos desde la microSD
 * @return ESP_OK en éxito
 */
esp_err_t recipe_db_load(void);

/**
 * @brief Guarda la base de datos en la microSD
 * @return ESP_OK en éxito
 */
esp_err_t recipe_db_save(void);

/* ==================================================================
 * FUNCIONES DE BÚSQUEDA
 * ================================================================== */

/**
 * @brief Busca una receta por nombre
 * @param name Nombre a buscar
 * @param results Buffer para resultados
 * @param max_results Máximo número de resultados
 * @return Número de recetas encontradas
 */
int recipe_db_search_by_name(const char *name, recipe_metadata_t *results, int max_results);

/**
 * @brief Busca recetas por categoría
 * @param category Categoría a buscar
 * @param results Buffer para resultados
 * @param max_results Máximo número de resultados
 * @return Número de recetas encontradas
 */
int recipe_db_search_by_category(const char *category, recipe_metadata_t *results, int max_results);

/**
 * @brief Busca recetas por ingrediente
 * @param ingredient Ingrediente a buscar
 * @param results Buffer para resultados
 * @param max_results Máximo número de resultados
 * @return Número de recetas encontradas
 */
int recipe_db_search_by_ingredient(const char *ingredient, recipe_metadata_t *results, int max_results);

/**
 * @brief Obtiene las recetas favoritas
 * @param results Buffer para resultados
 * @param max_results Máximo número de resultados
 * @return Número de recetas encontradas
 */
int recipe_db_get_favorites(recipe_metadata_t *results, int max_results);

/* ==================================================================
 * FUNCIONES DE CRUD
 * ================================================================== */

/**
 * @brief Agrega una nueva receta a la base de datos
 * @param recipe Receta a agregar
 * @return ID de la nueva receta, o -1 en error
 */
int recipe_db_add(const recipe_t *recipe);

/**
 * @brief Obtiene una receta por su ID
 * @param id ID de la receta
 * @param recipe Puntero donde almacenar la receta
 * @return ESP_OK en éxito
 */
esp_err_t recipe_db_get(uint8_t id, recipe_t *recipe);

/**
 * @brief Actualiza una receta existente
 * @param id ID de la receta
 * @param recipe Nuevos datos de la receta
 * @return ESP_OK en éxito
 */
esp_err_t recipe_db_update(uint8_t id, const recipe_t *recipe);

/**
 * @brief Elimina una receta
 * @param id ID de la receta
 * @return ESP_OK en éxito
 */
esp_err_t recipe_db_delete(uint8_t id);

/* ==================================================================
 * FUNCIONES DE UTILIDAD
 * ================================================================== */

/**
 * @brief Marca una receta como favorita
 * @param id ID de la receta
 * @param favorite true para marcar como favorita
 * @return ESP_OK en éxito
 */
esp_err_t recipe_db_set_favorite(uint8_t id, bool favorite);

/**
 * @brief Incrementa el contador de veces cocinada
 * @param id ID de la receta
 * @return ESP_OK en éxito
 */
esp_err_t recipe_db_increment_cooked(uint8_t id);

/**
 * @brief Obtiene una lista de ingredientes faltantes
 * @param id ID de la receta
 * @param available Ingredientes disponibles (array de strings)
 * @param available_count Número de ingredientes disponibles
 * @param missing Buffer para ingredientes faltantes
 * @param max_missing Máximo número de faltantes a devolver
 * @return Número de ingredientes faltantes
 */
int recipe_db_check_ingredients(uint8_t id, char available[][MAX_INGREDIENT_LEN], 
                                 int available_count, char missing[][MAX_INGREDIENT_LEN], 
                                 int max_missing);

/**
 * @brief Obtiene el texto de voz para una receta
 * @param id ID de la receta
 * @param text Buffer para el texto
 * @param max_len Tamaño del buffer
 * @return ESP_OK en éxito
 */
esp_err_t recipe_db_get_voice_text(uint8_t id, char *text, size_t max_len);

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

/**
 * @brief Obtiene estadísticas de la base de datos
 * @param stats Puntero donde almacenar las estadísticas
 */
void recipe_db_get_stats(recipe_stats_t *stats);

/**
 * @brief Obtiene el número total de recetas
 * @return Número de recetas
 */
uint32_t recipe_db_get_count(void);

/**
 * @brief Verifica si la base de datos está cargada
 * @return true si está cargada
 */
bool recipe_db_is_loaded(void);

#ifdef __cplusplus
}
#endif

#endif /* RECIPE_DB_H */
