/**
 * @file recipe_db.c
 * @brief Implementación de base de datos de recetas familiares
 * 
 * Este archivo contiene la implementación de las funciones
 * declaradas en recipe_db.h
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "recipe_db.h"
#include "voice_storage.h"
#include "config.h"

static const char *TAG = "RECIPE_DB";

// Base de datos en memoria
static recipe_t recipes[MAX_RECIPES];
static int recipe_count = 0;
static bool db_loaded = false;

// Archivo de base de datos
#define RECIPES_FILE "/sdcard/recipes.db"

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static int find_recipe_by_name(const char *name)
{
    for (int i = 0; i < recipe_count; i++) {
        if (strcasecmp(recipes[i].metadata.name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int find_recipe_by_id(uint8_t id)
{
    for (int i = 0; i < recipe_count; i++) {
        if (recipes[i].metadata.id == id) {
            return i;
        }
    }
    return -1;
}

static void load_default_recipes(void)
{
    ESP_LOGI(TAG, "Cargando recetas por defecto");
    
    // Receta 1: Flan
    recipe_t flan = {
        .metadata = {
            .id = 0,
            .name = "Flan casero",
            .category = "postre",
            .servings = 6,
            .prep_time_minutes = 15,
            .cook_time_minutes = 45,
            .difficulty = 2,
            .favorite = true,
            .times_cooked = 0,
            .last_cooked = 0
        },
        .ingredient_count = 5,
        .step_count = 4,
        .notes = "El flan de la abuela"
    };
    strcpy(flan.ingredients[0].name, "Huevos");
    strcpy(flan.ingredients[0].quantity, "4");
    strcpy(flan.ingredients[1].name, "Leche");
    strcpy(flan.ingredients[1].quantity, "500 ml");
    strcpy(flan.ingredients[2].name, "Azúcar");
    strcpy(flan.ingredients[2].quantity, "150 g");
    strcpy(flan.ingredients[3].name, "Esencia de vainilla");
    strcpy(flan.ingredients[3].quantity, "1 cucharadita");
    strcpy(flan.ingredients[4].name, "Caramelo");
    strcpy(flan.ingredients[4].quantity, "para el molde");
    
    strcpy(flan.steps[0].description, "Precalentar el horno a 180°C");
    strcpy(flan.steps[1].description, "Mezclar todos los ingredientes en una licuadora");
    strcpy(flan.steps[2].description, "Verter en molde acaramelado");
    strcpy(flan.steps[3].description, "Hornear a baño María por 45 minutos");
    
    recipes[recipe_count++] = flan;
    
    // Receta 2: Torta de chocolate
    recipe_t torta = {
        .metadata = {
            .id = 1,
            .name = "Torta de chocolate",
            .category = "postre",
            .servings = 8,
            .prep_time_minutes = 20,
            .cook_time_minutes = 35,
            .difficulty = 3,
            .favorite = false,
            .times_cooked = 0,
            .last_cooked = 0
        },
        .ingredient_count = 7,
        .step_count = 5,
        .notes = "Torta húmeda de chocolate"
    };
    
    recipes[recipe_count++] = torta;
}

static esp_err_t save_to_file(void)
{
    FILE *f = fopen(RECIPES_FILE, "wb");
    if (!f) {
        ESP_LOGE(TAG, "Error abriendo archivo para escritura");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Guardar número de recetas
    fwrite(&recipe_count, sizeof(int), 1, f);
    
    // Guardar cada receta
    for (int i = 0; i < recipe_count; i++) {
        fwrite(&recipes[i], sizeof(recipe_t), 1, f);
    }
    
    fclose(f);
    ESP_LOGI(TAG, "%d recetas guardadas en %s", recipe_count, RECIPES_FILE);
    
    return ESP_OK;
}

static esp_err_t load_from_file(void)
{
    FILE *f = fopen(RECIPES_FILE, "rb");
    if (!f) {
        ESP_LOGW(TAG, "Archivo de recetas no encontrado, cargando por defecto");
        load_default_recipes();
        save_to_file();
        return ESP_OK;
    }
    
    // Leer número de recetas
    fread(&recipe_count, sizeof(int), 1, f);
    
    if (recipe_count > MAX_RECIPES) {
        recipe_count = MAX_RECIPES;
    }
    
    // Leer cada receta
    for (int i = 0; i < recipe_count; i++) {
        fread(&recipes[i], sizeof(recipe_t), 1, f);
    }
    
    fclose(f);
    ESP_LOGI(TAG, "%d recetas cargadas desde %s", recipe_count, RECIPES_FILE);
    
    return ESP_OK;
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

esp_err_t recipe_db_init(void)
{
    ESP_LOGI(TAG, "Inicializando base de datos de recetas");
    
    memset(recipes, 0, sizeof(recipes));
    recipe_count = 0;
    
    return ESP_OK;
}

esp_err_t recipe_db_load(void)
{
    esp_err_t ret = load_from_file();
    if (ret == ESP_OK) {
        db_loaded = true;
    }
    return ret;
}

esp_err_t recipe_db_save(void)
{
    return save_to_file();
}

int recipe_db_search_by_name(const char *name, recipe_metadata_t *results, int max_results)
{
    if (!name || !results || max_results <= 0) return 0;
    
    int found = 0;
    
    for (int i = 0; i < recipe_count && found < max_results; i++) {
        if (strstr(recipes[i].metadata.name, name) != NULL) {
            results[found++] = recipes[i].metadata;
        }
    }
    
    return found;
}

int recipe_db_search_by_category(const char *category, recipe_metadata_t *results, int max_results)
{
    if (!category || !results || max_results <= 0) return 0;
    
    int found = 0;
    
    for (int i = 0; i < recipe_count && found < max_results; i++) {
        if (strcmp(recipes[i].metadata.category, category) == 0) {
            results[found++] = recipes[i].metadata;
        }
    }
    
    return found;
}

int recipe_db_search_by_ingredient(const char *ingredient, recipe_metadata_t *results, int max_results)
{
    if (!ingredient || !results || max_results <= 0) return 0;
    
    int found = 0;
    
    for (int i = 0; i < recipe_count && found < max_results; i++) {
        for (int j = 0; j < recipes[i].ingredient_count; j++) {
            if (strstr(recipes[i].ingredients[j].name, ingredient) != NULL) {
                results[found++] = recipes[i].metadata;
                break;
            }
        }
    }
    
    return found;
}

int recipe_db_get_favorites(recipe_metadata_t *results, int max_results)
{
    if (!results || max_results <= 0) return 0;
    
    int found = 0;
    
    for (int i = 0; i < recipe_count && found < max_results; i++) {
        if (recipes[i].metadata.favorite) {
            results[found++] = recipes[i].metadata;
        }
    }
    
    return found;
}

int recipe_db_add(const recipe_t *recipe)
{
    if (!recipe || recipe_count >= MAX_RECIPES) {
        return -1;
    }
    
    // Verificar si ya existe
    if (find_recipe_by_name(recipe->metadata.name) >= 0) {
        ESP_LOGW(TAG, "Receta '%s' ya existe", recipe->metadata.name);
        return -1;
    }
    
    int new_id = recipe_count;
    recipes[recipe_count] = *recipe;
    recipes[recipe_count].metadata.id = new_id;
    recipe_count++;
    
    save_to_file();
    ESP_LOGI(TAG, "Receta '%s' agregada con ID=%d", recipe->metadata.name, new_id);
    
    return new_id;
}

esp_err_t recipe_db_get(uint8_t id, recipe_t *recipe)
{
    int index = find_recipe_by_id(id);
    if (index < 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    *recipe = recipes[index];
    return ESP_OK;
}

esp_err_t recipe_db_update(uint8_t id, const recipe_t *recipe)
{
    int index = find_recipe_by_id(id);
    if (index < 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    // Mantener el mismo ID
    recipes[index] = *recipe;
    recipes[index].metadata.id = id;
    
    save_to_file();
    ESP_LOGI(TAG, "Receta ID=%d actualizada", id);
    
    return ESP_OK;
}

esp_err_t recipe_db_delete(uint8_t id)
{
    int index = find_recipe_by_id(id);
    if (index < 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    // Desplazar elementos
    for (int i = index; i < recipe_count - 1; i++) {
        recipes[i] = recipes[i + 1];
    }
    
    recipe_count--;
    save_to_file();
    ESP_LOGI(TAG, "Receta ID=%d eliminada", id);
    
    return ESP_OK;
}

esp_err_t recipe_db_set_favorite(uint8_t id, bool favorite)
{
    int index = find_recipe_by_id(id);
    if (index < 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    recipes[index].metadata.favorite = favorite;
    save_to_file();
    ESP_LOGI(TAG, "Receta ID=%d favorita=%d", id, favorite);
    
    return ESP_OK;
}

esp_err_t recipe_db_increment_cooked(uint8_t id)
{
    int index = find_recipe_by_id(id);
    if (index < 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    recipes[index].metadata.times_cooked++;
    recipes[index].metadata.last_cooked = esp_timer_get_time() / 1000;
    
    save_to_file();
    ESP_LOGI(TAG, "Receta ID=%d cocinada %d veces", id, recipes[index].metadata.times_cooked);
    
    return ESP_OK;
}

int recipe_db_check_ingredients(uint8_t id, char available[][MAX_INGREDIENT_LEN], 
                                 int available_count, char missing[][MAX_INGREDIENT_LEN], 
                                 int max_missing)
{
    int index = find_recipe_by_id(id);
    if (index < 0) {
        return 0;
    }
    
    int missing_count = 0;
    recipe_t *recipe = &recipes[index];
    
    for (int i = 0; i < recipe->ingredient_count && missing_count < max_missing; i++) {
        if (recipe->ingredients[i].optional) continue;
        
        bool found = false;
        for (int j = 0; j < available_count; j++) {
            if (strcasecmp(recipe->ingredients[i].name, available[j]) == 0) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            strcpy(missing[missing_count], recipe->ingredients[i].name);
            missing_count++;
        }
    }
    
    return missing_count;
}

esp_err_t recipe_db_get_voice_text(uint8_t id, char *text, size_t max_len)
{
    int index = find_recipe_by_id(id);
    if (index < 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    recipe_t *r = &recipes[index];
    char buffer[1024];
    int pos = 0;
    
    pos += snprintf(buffer + pos, sizeof(buffer) - pos, 
                    "%s. ", r->metadata.name);
    pos += snprintf(buffer + pos, sizeof(buffer) - pos, 
                    "Ingredientes: ");
    
    for (int i = 0; i < r->ingredient_count; i++) {
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, 
                        "%s %s%s", 
                        r->ingredients[i].quantity,
                        r->ingredients[i].name,
                        (i < r->ingredient_count - 1) ? ", " : ". ");
    }
    
    pos += snprintf(buffer + pos, sizeof(buffer) - pos, 
                    "Preparación: ");
    for (int i = 0; i < r->step_count; i++) {
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, 
                        "%s. %s ", 
                        r->steps[i].description,
                        (i < r->step_count - 1) ? "Luego " : "");
    }
    
    strncpy(text, buffer, max_len - 1);
    text[max_len - 1] = '\0';
    
    return ESP_OK;
}

void recipe_db_get_stats(recipe_stats_t *stats)
{
    if (!stats) return;
    
    memset(stats, 0, sizeof(recipe_stats_t));
    stats->total_recipes = recipe_count;
    
    uint32_t max_cooked = 0;
    int most_cooked_index = -1;
    int last_cooked_index = -1;
    uint32_t latest_time = 0;
    
    for (int i = 0; i < recipe_count; i++) {
        if (recipes[i].metadata.favorite) {
            stats->favorite_count++;
        }
        
        stats->total_cooked += recipes[i].metadata.times_cooked;
        
        if (recipes[i].metadata.times_cooked > max_cooked) {
            max_cooked = recipes[i].metadata.times_cooked;
            most_cooked_index = i;
        }
        
        if (recipes[i].metadata.last_cooked > latest_time) {
            latest_time = recipes[i].metadata.last_cooked;
            last_cooked_index = i;
        }
    }
    
    if (most_cooked_index >= 0) {
        strcpy(stats->most_cooked, recipes[most_cooked_index].metadata.name);
    }
    
    if (last_cooked_index >= 0) {
        strcpy(stats->last_cooked, recipes[last_cooked_index].metadata.name);
    }
}

uint32_t recipe_db_get_count(void)
{
    return recipe_count;
}

bool recipe_db_is_loaded(void)
{
    return db_loaded;
}
