/**
 * @file contextual_buffer.c
 * @brief Buffer de contexto conversacional
 * 
 * Este módulo mantiene un buffer circular de las últimas
 * conversaciones para permitir preguntas como "¿qué estaba diciendo?"
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "contextual_buffer.h"
#include "config.h"

static const char *TAG = "CONTEXT_BUF";

// Configuración del buffer
#define BUFFER_DURATION_SEC  CONTEXTUAL_BUFFER_SECONDS
#define SAMPLE_RATE          10   // Muestras por segundo (aproximado)
#define BUFFER_SIZE          (BUFFER_DURATION_SEC * SAMPLE_RATE)

// Entrada del buffer
typedef struct {
    char text[128];                 // Fragmento de texto
    uint32_t timestamp;              // Timestamp de la captura
    uint8_t emotional_tag;           // 0=neutro, 1=alegría, 2=angustia, 3=confusión
    bool valid;                      // true si la entrada es válida
} buffer_entry_t;

// Buffer circular
static buffer_entry_t buffer[BUFFER_SIZE];
static int buffer_index = 0;
static int buffer_count = 0;

// Último contexto recuperado
static char last_context[256] = {0};
static uint32_t last_context_time = 0;

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static uint32_t get_current_time_ms(void)
{
    return esp_timer_get_time() / 1000;
}

static int find_recent_entry(uint32_t max_age_ms)
{
    uint32_t now = get_current_time_ms();
    int search_index = (buffer_index - 1 + BUFFER_SIZE) % BUFFER_SIZE;
    
    // Buscar hacia atrás hasta encontrar una entrada reciente
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer[search_index].valid) {
            if (now - buffer[search_index].timestamp <= max_age_ms) {
                return search_index;
            }
        }
        search_index = (search_index - 1 + BUFFER_SIZE) % BUFFER_SIZE;
    }
    
    return -1;
}

static void cleanup_old_entries(void)
{
    uint32_t now = get_current_time_ms();
    uint32_t max_age_ms = BUFFER_DURATION_SEC * 1000;
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer[i].valid) {
            if (now - buffer[i].timestamp > max_age_ms) {
                buffer[i].valid = false;
            }
        }
    }
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

void contextual_buffer_init(void)
{
    ESP_LOGI(TAG, "Inicializando buffer contextual (%d segundos)", BUFFER_DURATION_SEC);
    
    memset(buffer, 0, sizeof(buffer));
    buffer_index = 0;
    buffer_count = 0;
    
    ESP_LOGI(TAG, "Buffer contextual inicializado");
}

void contextual_buffer_add(const char *text, uint8_t emotional_tag)
{
    if (!text) return;
    
    // Limpiar entradas viejas
    cleanup_old_entries();
    
    // Crear nueva entrada
    buffer_entry_t *entry = &buffer[buffer_index];
    strncpy(entry->text, text, sizeof(entry->text) - 1);
    entry->text[sizeof(entry->text) - 1] = '\0';
    entry->timestamp = get_current_time_ms();
    entry->emotional_tag = emotional_tag;
    entry->valid = true;
    
    ESP_LOGD(TAG, "Buffer +: '%s' (tag=%d)", text, emotional_tag);
    
    // Avanzar índice
    buffer_index = (buffer_index + 1) % BUFFER_SIZE;
    if (buffer_count < BUFFER_SIZE) {
        buffer_count++;
    }
}

const char* contextual_buffer_get_last(uint32_t max_age_seconds)
{
    uint32_t max_age_ms = max_age_seconds * 1000;
    int index = find_recent_entry(max_age_ms);
    
    if (index >= 0) {
        last_context_time = buffer[index].timestamp;
        strncpy(last_context, buffer[index].text, sizeof(last_context) - 1);
        last_context[sizeof(last_context) - 1] = '\0';
        
        ESP_LOGD(TAG, "Buffer get: '%s'", last_context);
        return last_context;
    }
    
    return NULL;
}

bool contextual_buffer_get_last_with_tag(char *buffer_out, size_t max_len, uint8_t *tag)
{
    if (!buffer_out || max_len == 0) return false;
    
    int index = find_recent_entry(BUFFER_DURATION_SEC * 1000);
    
    if (index >= 0) {
        strncpy(buffer_out, buffer[index].text, max_len - 1);
        buffer_out[max_len - 1] = '\0';
        if (tag) *tag = buffer[index].emotional_tag;
        return true;
    }
    
    return false;
}

int contextual_buffer_search(const char *keyword, char *result, size_t max_len)
{
    if (!keyword || !result || max_len == 0) return 0;
    
    int found = 0;
    uint32_t now = get_current_time_ms();
    uint32_t max_age_ms = BUFFER_DURATION_SEC * 1000;
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer[i].valid) {
            if (now - buffer[i].timestamp <= max_age_ms) {
                if (strstr(buffer[i].text, keyword)) {
                    if (found == 0) {
                        // Primer resultado: copiar
                        strncpy(result, buffer[i].text, max_len - 1);
                        result[max_len - 1] = '\0';
                    }
                    found++;
                }
            }
        }
    }
    
    return found;
}

uint32_t contextual_buffer_get_last_time(void)
{
    int index = find_recent_entry(BUFFER_DURATION_SEC * 1000);
    
    if (index >= 0) {
        return buffer[index].timestamp;
    }
    
    return 0;
}

void contextual_buffer_clear(void)
{
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i].valid = false;
    }
    buffer_index = 0;
    buffer_count = 0;
    ESP_LOGI(TAG, "Buffer limpiado");
}

int contextual_buffer_get_count(void)
{
    cleanup_old_entries();
    
    int count = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer[i].valid) {
            count++;
        }
    }
    return count;
}

void contextual_buffer_get_stats(context_buffer_stats_t *stats)
{
    if (!stats) return;
    
    cleanup_old_entries();
    
    stats->total_entries = buffer_count;
    stats->valid_entries = contextual_buffer_get_count();
    stats->buffer_size = BUFFER_SIZE;
    stats->duration_seconds = BUFFER_DURATION_SEC;
    
    // Calcular distribución emocional
    memset(stats->emotional_distribution, 0, sizeof(stats->emotional_distribution));
    
    uint32_t now = get_current_time_ms();
    uint32_t max_age_ms = BUFFER_DURATION_SEC * 1000;
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer[i].valid) {
            if (now - buffer[i].timestamp <= max_age_ms) {
                if (buffer[i].emotional_tag < 4) {
                    stats->emotional_distribution[buffer[i].emotional_tag]++;
                }
            }
        }
    }
}

bool contextual_buffer_is_available(void)
{
    return (contextual_buffer_get_count() > 0);
}

void contextual_buffer_remove_last(void)
{
    int index = (buffer_index - 1 + BUFFER_SIZE) % BUFFER_SIZE;
    
    if (buffer[index].valid) {
        buffer[index].valid = false;
        ESP_LOGD(TAG, "Última entrada eliminada");
    }
}
