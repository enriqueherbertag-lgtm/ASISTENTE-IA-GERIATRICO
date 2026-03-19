/**
 * @file face_db.c
 * @brief Implementación de base de datos de reconocimiento facial
 * 
 * Este archivo contiene la implementación de las funciones
 * declaradas en face_db.h
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "face_db.h"
#include "config.h"

static const char *TAG = "FACE_DB";

// Base de datos en memoria
static family_face_t faces[MAX_FAMILY_FACES];
static int face_count = 0;
static bool db_loaded = false;

// Estadísticas
static uint32_t total_recognitions = 0;
static uint32_t total_failures = 0;
static uint32_t last_recognition_time = 0;

// Archivo de base de datos
#define FACES_FILE "/sdcard/faces.db"

// Embedding por defecto (para rostros no reconocidos)
static const face_embedding_t default_embedding = {0};

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static int find_face_by_id(uint8_t id)
{
    for (int i = 0; i < face_count; i++) {
        if (faces[i].id == id) {
            return i;
        }
    }
    return -1;
}

static float dot_product(const uint8_t *a, const uint8_t *b, int size)
{
    float sum = 0;
    for (int i = 0; i < size; i++) {
        sum += (float)a[i] * b[i];
    }
    return sum;
}

static void normalize_embedding(face_embedding_t *emb)
{
    float norm = 0;
    for (int i = 0; i < FACE_EMBEDDING_SIZE; i++) {
        norm += emb->data[i] * emb->data[i];
    }
    emb->norm = sqrtf(norm);
}

static esp_err_t save_to_file(void)
{
    FILE *f = fopen(FACES_FILE, "wb");
    if (!f) {
        ESP_LOGE(TAG, "Error abriendo archivo para escritura");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Guardar número de familiares
    fwrite(&face_count, sizeof(int), 1, f);
    
    // Guardar cada familiar
    for (int i = 0; i < face_count; i++) {
        fwrite(&faces[i], sizeof(family_face_t), 1, f);
    }
    
    fclose(f);
    ESP_LOGI(TAG, "%d familiares guardados en %s", face_count, FACES_FILE);
    
    return ESP_OK;
}

static esp_err_t load_from_file(void)
{
    FILE *f = fopen(FACES_FILE, "rb");
    if (!f) {
        ESP_LOGW(TAG, "Archivo de rostros no encontrado");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Leer número de familiares
    fread(&face_count, sizeof(int), 1, f);
    
    if (face_count > MAX_FAMILY_FACES) {
        face_count = MAX_FAMILY_FACES;
    }
    
    // Leer cada familiar
    for (int i = 0; i < face_count; i++) {
        fread(&faces[i], sizeof(family_face_t), 1, f);
    }
    
    fclose(f);
    ESP_LOGI(TAG, "%d familiares cargados desde %s", face_count, FACES_FILE);
    
    return ESP_OK;
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

esp_err_t face_db_init(void)
{
    ESP_LOGI(TAG, "Inicializando base de datos facial");
    
    memset(faces, 0, sizeof(faces));
    face_count = 0;
    total_recognitions = 0;
    total_failures = 0;
    
    return ESP_OK;
}

esp_err_t face_db_load(void)
{
    esp_err_t ret = load_from_file();
    if (ret == ESP_OK) {
        db_loaded = true;
    }
    return ret;
}

esp_err_t face_db_save(void)
{
    return save_to_file();
}

int face_db_add(const char *name, const char *relation, const face_embedding_t *embedding, uint8_t priority)
{
    if (!name || !relation || !embedding || face_count >= MAX_FAMILY_FACES) {
        return -1;
    }
    
    // Verificar si ya existe
    for (int i = 0; i < face_count; i++) {
        if (strcmp(faces[i].name, name) == 0) {
            ESP_LOGW(TAG, "Familiar '%s' ya existe", name);
            return -1;
        }
    }
    
    int new_id = face_count;
    family_face_t *f = &faces[face_count];
    
    f->id = new_id;
    strncpy(f->name, name, MAX_NAME_LEN - 1);
    strncpy(f->relation, relation, MAX_RELATION_LEN - 1);
    f->embedding = *embedding;
    normalize_embedding(&f->embedding);
    f->sample_count = 1;
    f->last_seen = 0;
    f->times_seen = 0;
    f->priority = priority;
    
    face_count++;
    
    save_to_file();
    ESP_LOGI(TAG, "Familiar '%s' (%s) agregado con ID=%d", name, relation, new_id);
    
    return new_id;
}

esp_err_t face_db_update_embedding(uint8_t id, const face_embedding_t *embedding)
{
    int index = find_face_by_id(id);
    if (index < 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    // Promediar con embedding existente
    family_face_t *f = &faces[index];
    
    for (int i = 0; i < FACE_EMBEDDING_SIZE; i++) {
        float avg = (f->embedding.data[i] * f->sample_count + embedding->data[i]) / (f->sample_count + 1);
        f->embedding.data[i] = (uint8_t)avg;
    }
    
    f->sample_count++;
    normalize_embedding(&f->embedding);
    
    save_to_file();
    ESP_LOGI(TAG, "Embedding de '%s' actualizado (ahora %d muestras)", f->name, f->sample_count);
    
    return ESP_OK;
}

esp_err_t face_db_delete(uint8_t id)
{
    int index = find_face_by_id(id);
    if (index < 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    // Desplazar elementos
    for (int i = index; i < face_count - 1; i++) {
        faces[i] = faces[i + 1];
    }
    
    face_count--;
    save_to_file();
    ESP_LOGI(TAG, "Familiar ID=%d eliminado", id);
    
    return ESP_OK;
}

face_recognition_result_t face_db_recognize(const face_embedding_t *embedding, float threshold)
{
    face_recognition_result_t result = {
        .family_id = 255,
        .confidence = 0,
        .recognition_time = 0
    };
    
    if (!embedding || face_count == 0) {
        return result;
    }
    
    // Normalizar embedding de entrada (copia local)
    face_embedding_t normalized = *embedding;
    normalize_embedding(&normalized);
    
    float best_similarity = 0;
    int best_index = -1;
    
    for (int i = 0; i < face_count; i++) {
        float similarity = face_db_compare(&normalized, &faces[i].embedding);
        
        if (similarity > best_similarity) {
            best_similarity = similarity;
            best_index = i;
        }
    }
    
    if (best_index >= 0 && best_similarity >= threshold) {
        result.family_id = faces[best_index].id;
        result.confidence = best_similarity;
        result.recognition_time = esp_timer_get_time() / 1000;
        
        ESP_LOGD(TAG, "Reconocido: %s (confianza=%.2f)", 
                 faces[best_index].name, best_similarity);
    }
    
    return result;
}

face_recognition_result_t face_db_recognize_and_update(const face_embedding_t *embedding, float threshold)
{
    face_recognition_result_t result = face_db_recognize(embedding, threshold);
    
    if (result.family_id < MAX_FAMILY_FACES) {
        // Actualizar estadísticas
        int index = find_face_by_id(result.family_id);
        if (index >= 0) {
            faces[index].last_seen = result.recognition_time;
            faces[index].times_seen++;
        }
        
        total_recognitions++;
        last_recognition_time = result.recognition_time;
    } else {
        total_failures++;
    }
    
    return result;
}

float face_db_compare(const face_embedding_t *emb1, const face_embedding_t *emb2)
{
    float dot = dot_product(emb1->data, emb2->data, FACE_EMBEDDING_SIZE);
    float norm_product = emb1->norm * emb2->norm;
    
    if (norm_product == 0) return 0;
    
    return dot / norm_product;  // Coseno del ángulo (0-1)
}

const family_face_t* face_db_get(uint8_t id)
{
    int index = find_face_by_id(id);
    if (index < 0) {
        return NULL;
    }
    
    return &faces[index];
}

int face_db_get_all(family_face_t *out_faces, int max_count)
{
    int to_copy = (face_count < max_count) ? face_count : max_count;
    
    for (int i = 0; i < to_copy; i++) {
        out_faces[i] = faces[i];
    }
    
    return to_copy;
}

int face_db_find_by_name(const char *name)
{
    if (!name) return -1;
    
    for (int i = 0; i < face_count; i++) {
        if (strcmp(faces[i].name, name) == 0) {
            return faces[i].id;
        }
    }
    
    return -1;
}

bool face_db_id_valid(uint8_t id)
{
    return (find_face_by_id(id) >= 0);
}

void face_db_update_last_seen(uint8_t id)
{
    int index = find_face_by_id(id);
    if (index >= 0) {
        faces[index].last_seen = esp_timer_get_time() / 1000;
        faces[index].times_seen++;
    }
}

void face_db_increment_failures(void)
{
    total_failures++;
}

uint32_t face_db_get_count(void)
{
    return face_count;
}

void face_db_get_stats(face_db_stats_t *stats)
{
    if (!stats) return;
    
    memset(stats, 0, sizeof(face_db_stats_t));
    stats->total_faces = face_count;
    stats->total_recognitions = total_recognitions;
    stats->total_failures = total_failures;
    stats->last_recognition_time = last_recognition_time;
    
    // Encontrar el más visto
    uint32_t max_seen = 0;
    int max_index = -1;
    
    for (int i = 0; i < face_count; i++) {
        if (faces[i].times_seen > max_seen) {
            max_seen = faces[i].times_seen;
            max_index = i;
        }
    }
    
    if (max_index >= 0) {
        strcpy(stats->most_seen, faces[max_index].name);
    }
}

bool face_db_is_loaded(void)
{
    return db_loaded;
}

const face_embedding_t* face_db_get_default_embedding(void)
{
    return &default_embedding;
}
