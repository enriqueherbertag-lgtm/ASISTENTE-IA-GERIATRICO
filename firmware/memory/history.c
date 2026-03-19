/**
 * @file history.c
 * @brief Implementación del historial de eventos del sistema
 * 
 * Este archivo contiene la implementación de las funciones
 * declaradas en history.h
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "history.h"
#include "voice_storage.h"
#include "config.h"

static const char *TAG = "HISTORY";

// Buffer circular de historial
static history_entry_t history[MAX_HISTORY_ENTRIES];
static int history_index = 0;
static int history_count = 0;
static uint32_t next_id = 0;

// Estadísticas
static history_stats_t current_stats = {0};

// Archivo de historial
#define HISTORY_FILE "/sdcard/history.db"
#define HISTORY_CSV   "/sdcard/history.csv"

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static uint32_t get_timestamp(void)
{
    return esp_timer_get_time() / 1000;  // Convertir a ms
}

static void update_stats_with_entry(const history_entry_t *entry)
{
    current_stats.total_entries++;
    
    if (entry->type < 64) {
        current_stats.events_by_type[entry->type]++;
    }
    
    if (current_stats.first_event_time == 0) {
        current_stats.first_event_time = entry->timestamp;
    }
    
    current_stats.last_event_time = entry->timestamp;
}

static int add_entry(history_entry_t *entry)
{
    entry->timestamp = get_timestamp();
    
    // Guardar en buffer circular
    history[history_index] = *entry;
    
    // Actualizar estadísticas
    update_stats_with_entry(entry);
    
    int id = history_index;
    history_index = (history_index + 1) % MAX_HISTORY_ENTRIES;
    
    if (history_count < MAX_HISTORY_ENTRIES) {
        history_count++;
    }
    
    ESP_LOGD(TAG, "Evento agregado: %s", entry->description);
    
    return id;
}

static esp_err_t save_to_file(void)
{
    FILE *f = fopen(HISTORY_FILE, "wb");
    if (!f) {
        ESP_LOGE(TAG, "Error abriendo archivo de historial");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Guardar metadata
    fwrite(&history_count, sizeof(int), 1, f);
    fwrite(&history_index, sizeof(int), 1, f);
    fwrite(&current_stats, sizeof(history_stats_t), 1, f);
    
    // Guardar entradas
    fwrite(history, sizeof(history_entry_t), MAX_HISTORY_ENTRIES, f);
    
    fclose(f);
    
    current_stats.last_saved_time = get_timestamp();
    ESP_LOGI(TAG, "Historial guardado (%d entradas)", history_count);
    
    return ESP_OK;
}

static esp_err_t load_from_file(void)
{
    FILE *f = fopen(HISTORY_FILE, "rb");
    if (!f) {
        ESP_LOGW(TAG, "Archivo de historial no encontrado");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Cargar metadata
    fread(&history_count, sizeof(int), 1, f);
    fread(&history_index, sizeof(int), 1, f);
    fread(&current_stats, sizeof(history_stats_t), 1, f);
    
    // Cargar entradas
    fread(history, sizeof(history_entry_t), MAX_HISTORY_ENTRIES, f);
    
    fclose(f);
    ESP_LOGI(TAG, "Historial cargado (%d entradas)", history_count);
    
    return ESP_OK;
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

esp_err_t history_init(void)
{
    ESP_LOGI(TAG, "Inicializando sistema de historial");
    
    memset(history, 0, sizeof(history));
    history_index = 0;
    history_count = 0;
    memset(&current_stats, 0, sizeof(history_stats_t));
    
    // Intentar cargar historial existente
    load_from_file();
    
    return ESP_OK;
}

int history_add_event(event_type_t type, uint8_t severity, const char *description)
{
    history_entry_t entry;
    memset(&entry, 0, sizeof(history_entry_t));
    
    entry.type = type;
    entry.severity = severity;
    if (description) {
        strncpy(entry.description, description, MAX_EVENT_DESC_LEN - 1);
    } else {
        snprintf(entry.description, MAX_EVENT_DESC_LEN - 1, 
                 "Evento tipo %d", type);
    }
    
    return add_entry(&entry);
}

int history_add_glucose(uint16_t glucose, uint8_t trend, uint8_t severity, const char *description)
{
    history_entry_t entry;
    memset(&entry, 0, sizeof(history_entry_t));
    
    entry.type = EVENT_GLUCOSE_READING;
    entry.severity = severity;
    entry.data.glucose.glucose = glucose;
    entry.data.glucose.trend = trend;
    
    if (description) {
        strncpy(entry.description, description, MAX_EVENT_DESC_LEN - 1);
    } else {
        snprintf(entry.description, MAX_EVENT_DESC_LEN - 1, 
                 "Glucosa: %d mg/dL, tendencia %d", glucose, trend);
    }
    
    return add_entry(&entry);
}

int history_add_face_recognition(uint8_t family_id, float confidence, const char *description)
{
    history_entry_t entry;
    memset(&entry, 0, sizeof(history_entry_t));
    
    entry.type = EVENT_FACE_RECOGNIZED;
    entry.severity = 1;  // Baja severidad
    entry.data.face.family_id = family_id;
    entry.data.face.confidence = confidence;
    
    if (description) {
        strncpy(entry.description, description, MAX_EVENT_DESC_LEN - 1);
    } else {
        snprintf(entry.description, MAX_EVENT_DESC_LEN - 1, 
                 "Rostro reconocido ID=%d conf=%.2f", family_id, confidence);
    }
    
    return add_entry(&entry);
}

int history_add_fall(float magnitude, bool confirmed)
{
    history_entry_t entry;
    memset(&entry, 0, sizeof(history_entry_t));
    
    entry.type = confirmed ? EVENT_FALL_CONFIRMED : EVENT_FALL_DETECTED;
    entry.severity = confirmed ? 10 : 8;  // Alta severidad
    entry.data.fall.magnitude = magnitude;
    
    snprintf(entry.description, MAX_EVENT_DESC_LEN - 1, 
             "Caída %s (magnitud=%.2f g)", 
             confirmed ? "CONFIRMADA" : "DETECTADA", magnitude);
    
    return add_entry(&entry);
}

int history_add_voice_command(uint8_t command_id, float confidence, const char *description)
{
    history_entry_t entry;
    memset(&entry, 0, sizeof(history_entry_t));
    
    entry.type = EVENT_VOICE_COMMAND;
    entry.severity = 2;  // Severidad baja-media
    entry.data.voice.command_id = command_id;
    entry.data.voice.confidence = confidence;
    
    if (description) {
        strncpy(entry.description, description, MAX_EVENT_DESC_LEN - 1);
    } else {
        snprintf(entry.description, MAX_EVENT_DESC_LEN - 1, 
                 "Comando ID=%d conf=%.2f", command_id, confidence);
    }
    
    return add_entry(&entry);
}

esp_err_t history_get_entry(int index, history_entry_t *entry)
{
    if (index < 0 || index >= history_count || !entry) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Calcular posición en buffer circular (los más recientes al final)
    int pos = (history_index - 1 - index + MAX_HISTORY_ENTRIES) % MAX_HISTORY_ENTRIES;
    *entry = history[pos];
    
    return ESP_OK;
}

int history_find_by_type(event_type_t type, history_entry_t *results, int max_results)
{
    if (!results || max_results <= 0) return 0;
    
    int found = 0;
    int start = (history_index - 1 + MAX_HISTORY_ENTRIES) % MAX_HISTORY_ENTRIES;
    
    for (int i = 0; i < history_count && found < max_results; i++) {
        int pos = (start - i + MAX_HISTORY_ENTRIES) % MAX_HISTORY_ENTRIES;
        if (history[pos].type == type) {
            results[found++] = history[pos];
        }
    }
    
    return found;
}

int history_find_by_time(uint32_t start_time, uint32_t end_time, 
                          history_entry_t *results, int max_results)
{
    if (!results || max_results <= 0) return 0;
    
    int found = 0;
    
    for (int i = 0; i < history_count && found < max_results; i++) {
        int pos = (history_index - 1 - i + MAX_HISTORY_ENTRIES) % MAX_HISTORY_ENTRIES;
        if (history[pos].timestamp >= start_time && 
            history[pos].timestamp <= end_time) {
            results[found++] = history[pos];
        }
    }
    
    return found;
}

int history_get_last(int count, history_entry_t *results)
{
    if (!results || count <= 0) return 0;
    
    int to_copy = (count < history_count) ? count : history_count;
    int start = (history_index - 1 + MAX_HISTORY_ENTRIES) % MAX_HISTORY_ENTRIES;
    
    for (int i = 0; i < to_copy; i++) {
        int pos = (start - i + MAX_HISTORY_ENTRIES) % MAX_HISTORY_ENTRIES;
        results[i] = history[pos];
    }
    
    return to_copy;
}

void history_get_stats(history_stats_t *stats)
{
    if (stats) {
        *stats = current_stats;
    }
}

uint32_t history_get_count(void)
{
    return history_count;
}

uint32_t history_get_count_by_type(event_type_t type)
{
    if (type < 64) {
        return current_stats.events_by_type[type];
    }
    return 0;
}

esp_err_t history_save(void)
{
    return save_to_file();
}

esp_err_t history_load(void)
{
    return load_from_file();
}

esp_err_t history_export_csv(const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f) {
        ESP_LOGE(TAG, "Error creando archivo CSV");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Escribir cabecera
    fprintf(f, "Timestamp,Tipo,Severidad,Descripción\n");
    
    // Escribir entradas (más recientes primero)
    int start = (history_index - 1 + MAX_HISTORY_ENTRIES) % MAX_HISTORY_ENTRIES;
    
    for (int i = 0; i < history_count; i++) {
        int pos = (start - i + MAX_HISTORY_ENTRIES) % MAX_HISTORY_ENTRIES;
        fprintf(f, "%u,%d,%d,\"%s\"\n",
                history[pos].timestamp,
                history[pos].type,
                history[pos].severity,
                history[pos].description);
    }
    
    fclose(f);
    ESP_LOGI(TAG, "Historial exportado a %s", filename);
    
    return ESP_OK;
}

void history_clear(void)
{
    memset(history, 0, sizeof(history));
    history_index = 0;
    history_count = 0;
    memset(&current_stats, 0, sizeof(history_stats_t));
    
    save_to_file();
    ESP_LOGI(TAG, "Historial borrado");
}

const char* history_event_type_to_string(event_type_t type)
{
    static const char *strings[] = {
        "Ninguno",
        "Sistema iniciado",
        "Sistema apagado",
        "Batería baja",
        "Batería crítica",
        "Cargando",
        "Carga completa",
        "Caída detectada",
        "Caída confirmada",
        "Falsa alarma",
        "Lectura glucosa",
        "Glucosa baja",
        "Glucosa crítica",
        "Glucosa alta",
        "Rostro reconocido",
        "Rostro desconocido",
        "Comando de voz",
        "Comando no reconocido",
        "Receta consultada",
        "Receta cocinada",
        "Recordatorio",
        "Error general",
        "Error SD",
        "Error sensor",
        "Error comunicación"
    };
    
    if (type >= 0 && type < sizeof(strings)/sizeof(strings[0])) {
        return strings[type];
    }
    
    return "Desconocido";
}

const char* history_severity_to_string(uint8_t severity)
{
    if (severity >= 8) return "CRÍTICO";
    if (severity >= 5) return "ALTO";
    if (severity >= 3) return "MEDIO";
    if (severity >= 1) return "BAJO";
    return "INFO";
}
