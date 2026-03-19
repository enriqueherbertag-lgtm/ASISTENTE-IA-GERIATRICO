/**
 * @file playback.c
 * @brief Gestión de reproducción de mensajes de voz
 * 
 * Este módulo maneja la reproducción de mensajes de voz,
 * incluyendo colas de reproducción, prioridades y control de volumen.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "playback.h"
#include "voice_storage.h"
#include "audio_driver.h"
#include "config.h"

static const char *TAG = "PLAYBACK";

// Cola de reproducción
#define PLAYBACK_QUEUE_SIZE 16

typedef struct {
    uint8_t message_id;          // ID del mensaje a reproducir
    uint8_t priority;            // Prioridad (0-255, mayor = más prioritario)
    bool interrupt_current;      // true si debe interrumpir lo que está sonando
    uint32_t timestamp;          // Timestamp de la solicitud
} playback_request_t;

static QueueHandle_t playback_queue = NULL;
static TaskHandle_t playback_task_handle = NULL;

static bool is_playing = false;
static uint8_t current_message_id = 0;
static uint32_t playback_start_time = 0;
static uint8_t current_priority = 0;

static playback_callbacks_t user_callbacks = {0};

// Volumen
static uint8_t volume = 80;  // 0-100

// Estadísticas
static playback_stats_t stats = {0};

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static int compare_priority(const void *a, const void *b)
{
    playback_request_t *req_a = (playback_request_t*)a;
    playback_request_t *req_b = (playback_request_t*)b;
    
    // Mayor prioridad primero
    if (req_a->priority != req_b->priority) {
        return req_b->priority - req_a->priority;
    }
    
    // Si igual prioridad, más reciente primero
    return req_b->timestamp - req_a->timestamp;
}

static void sort_queue(void)
{
    if (!playback_queue) return;
    
    playback_request_t queue_copy[PLAYBACK_QUEUE_SIZE];
    int count = 0;
    
    // Vaciar cola en un array
    while (xQueueReceive(playback_queue, &queue_copy[count], 0) == pdTRUE) {
        count++;
    }
    
    if (count == 0) return;
    
    // Ordenar por prioridad
    qsort(queue_copy, count, sizeof(playback_request_t), compare_priority);
    
    // Reinsertar ordenado
    for (int i = 0; i < count; i++) {
        xQueueSend(playback_queue, &queue_copy[i], 0);
    }
}

static void playback_task(void *arg)
{
    playback_request_t request;
    
    while (1) {
        if (xQueueReceive(playback_queue, &request, portMAX_DELAY) == pdTRUE) {
            
            // Verificar si debe interrumpir
            if (request.interrupt_current && is_playing) {
                audio_stop_playback();
                ESP_LOGI(TAG, "Reproducción interrumpida por prioridad %d", request.priority);
                
                // Reinsertar el mensaje actual al inicio de la cola
                playback_request_t current = {
                    .message_id = current_message_id,
                    .priority = current_priority,
                    .interrupt_current = false,
                    .timestamp = playback_start_time
                };
                
                // Ordenar cola
                sort_queue();
                
                // Enviar al frente (usando cola ordenada)
                xQueueSendToFront(playback_queue, &current, 0);
            }
            
            // Reproducir el mensaje
            ESP_LOGI(TAG, "Reproduciendo mensaje ID=%d (prioridad=%d)", 
                     request.message_id, request.priority);
            
            is_playing = true;
            current_message_id = request.message_id;
            current_priority = request.priority;
            playback_start_time = esp_timer_get_time() / 1000;
            
            // Obtener ruta del archivo
            char path[128];
            if (voice_storage_get_path(request.message_id, path, sizeof(path)) == ESP_OK) {
                audio_play_file(path);
            } else {
                ESP_LOGE(TAG, "Error obteniendo ruta del mensaje %d", request.message_id);
            }
            
            // Actualizar estadísticas
            stats.total_playbacks++;
            stats.last_message_id = request.message_id;
            stats.last_playback_time = playback_start_time;
            
            // Notificar a usuario si hay callback
            if (user_callbacks.on_playback_start) {
                user_callbacks.on_playback_start(request.message_id);
            }
            
            // Esperar a que termine (simulado)
            vTaskDelay(pdMS_TO_TICKS(2000));  // En realidad se esperaría un evento
            
            is_playing = false;
            
            if (user_callbacks.on_playback_end) {
                user_callbacks.on_playback_end(request.message_id);
            }
        }
    }
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

esp_err_t playback_init(void)
{
    ESP_LOGI(TAG, "Inicializando sistema de reproducción");
    
    // Crear cola de reproducción
    playback_queue = xQueueCreate(PLAYBACK_QUEUE_SIZE, sizeof(playback_request_t));
    if (!playback_queue) {
        ESP_LOGE(TAG, "Error creando cola de reproducción");
        return ESP_ERR_NO_MEM;
    }
    
    // Crear tarea de reproducción
    xTaskCreate(playback_task, "playback_task", 4096, NULL, 5, &playback_task_handle);
    
    memset(&stats, 0, sizeof(playback_stats_t));
    
    ESP_LOGI(TAG, "Sistema de reproducción inicializado");
    
    return ESP_OK;
}

esp_err_t playback_play(uint8_t message_id, uint8_t priority, bool interrupt)
{
    if (!playback_queue) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!voice_storage_message_exists(message_id)) {
        ESP_LOGW(TAG, "Mensaje ID=%d no existe", message_id);
        return ESP_ERR_NOT_FOUND;
    }
    
    playback_request_t request = {
        .message_id = message_id,
        .priority = priority,
        .interrupt_current = interrupt,
        .timestamp = esp_timer_get_time() / 1000
    };
    
    if (xQueueSend(playback_queue, &request, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Cola de reproducción llena");
        return ESP_ERR_NO_MEM;
    }
    
    ESP_LOGI(TAG, "Mensaje %d encolado (prioridad=%d)", message_id, priority);
    
    return ESP_OK;
}

esp_err_t playback_play_urgent(uint8_t message_id)
{
    return playback_play(message_id, 255, true);  // Máxima prioridad, interrumpe
}

esp_err_t playback_play_background(uint8_t message_id)
{
    return playback_play(message_id, 0, false);   // Mínima prioridad, no interrumpe
}

void playback_stop(void)
{
    audio_stop_playback();
    is_playing = false;
    
    // Vaciar cola
    playback_request_t dummy;
    while (xQueueReceive(playback_queue, &dummy, 0) == pdTRUE) {
        // Descartar
    }
    
    ESP_LOGI(TAG, "Reproducción detenida y cola vaciada");
}

void playback_pause(void)
{
    // TODO: Implementar pausa
    ESP_LOGW(TAG, "playback_pause no implementado");
}

void playback_resume(void)
{
    // TODO: Implementar reanudación
    ESP_LOGW(TAG, "playback_resume no implementado");
}

bool playback_is_playing(void)
{
    return is_playing;
}

uint8_t playback_get_current_message(void)
{
    return current_message_id;
}

void playback_set_volume(uint8_t vol)
{
    if (vol > 100) vol = 100;
    volume = vol;
    audio_set_volume(volume);
    ESP_LOGI(TAG, "Volumen configurado a %d", volume);
}

uint8_t playback_get_volume(void)
{
    return volume;
}

void playback_set_callbacks(playback_callbacks_t *callbacks)
{
    if (callbacks) {
        user_callbacks = *callbacks;
        ESP_LOGI(TAG, "Callbacks de reproducción registrados");
    }
}

uint32_t playback_get_queue_size(void)
{
    if (!playback_queue) return 0;
    
    UBaseType_t count = uxQueueMessagesWaiting(playback_queue);
    return count;
}

void playback_clear_queue(void)
{
    playback_request_t dummy;
    while (xQueueReceive(playback_queue, &dummy, 0) == pdTRUE) {
        // Descartar
    }
    ESP_LOGI(TAG, "Cola de reproducción limpiada");
}

void playback_get_stats(playback_stats_t *stats_out)
{
    if (stats_out) {
        *stats_out = stats;
    }
}

esp_err_t playback_preload(uint8_t message_id)
{
    // TODO: Implementar precarga de mensajes en RAM
    ESP_LOGW(TAG, "playback_preload no implementado");
    return ESP_ERR_NOT_SUPPORTED;
}

bool playback_is_message_playing(uint8_t message_id)
{
    return (is_playing && current_message_id == message_id);
}
