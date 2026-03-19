/**
 * @file voice_recognition.c
 * @brief Algoritmo de reconocimiento de voz y comandos
 * 
 * Este módulo procesa audio capturado para detectar comandos
 * de voz y activar las funciones correspondientes.
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
#include "voice_recognition.h"
#include "audio_driver.h"
#include "config.h"

static const char *TAG = "VOICE_RECOG";

// Lista de comandos reconocidos
static const voice_command_t commands[] = {
    {CMD_LEER, "leer", "leer etiqueta", 1.0},
    {CMD_RECETA, "receta", "buscar receta", 1.0},
    {CMD_GLUCOSA, "glucosa", "leer glucosa", 1.0},
    {CMD_QUIEN_ES, "quién es", "reconocer persona", 1.0},
    {CMD_QUE_DECIA, "qué decía", "recuperar contexto", 1.0},
    {CMD_AYUDA, "ayuda", "mostrar ayuda", 1.0},
    {CMD_VOLUMEN, "volumen", "ajustar volumen", 1.0},
    {CMD_FECHA, "qué día es", "decir fecha", 1.0},
    {CMD_HORA, "qué hora es", "decir hora", 1.0},
    {CMD_RECORDATORIO, "recordatorio", "gestionar recordatorios", 1.0}
};

#define NUM_COMMANDS (sizeof(commands) / sizeof(commands[0]))

// Variables internas
static bool recognition_initialized = false;
static voice_command_t last_command = {0};
static uint32_t command_count[NUM_COMMANDS] = {0};
static uint32_t total_commands = 0;
static float recognition_confidence = 0;

// Callback para comandos
static command_callback_t command_callback = NULL;
static void *callback_arg = NULL;

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static uint8_t simple_hash(const char *str)
{
    uint8_t hash = 0;
    while (*str) {
        hash = (hash * 31) + *str;
        str++;
    }
    return hash;
}

static float calculate_similarity(const char *s1, const char *s2)
{
    // Implementación muy simple de similitud de Levenshtein
    // En un sistema real, se usarían modelos acústicos y de lenguaje
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    
    if (len1 == 0 || len2 == 0) return 0;
    
    int max_len = (len1 > len2) ? len1 : len2;
    int matches = 0;
    
    for (int i = 0; i < len1 && i < len2; i++) {
        if (s1[i] == s2[i]) {
            matches++;
        }
    }
    
    return (float)matches / max_len;
}

static int find_best_match(const char *text, float *confidence)
{
    int best_index = -1;
    float best_similarity = 0;
    
    for (int i = 0; i < NUM_COMMANDS; i++) {
        float sim = calculate_similarity(text, commands[i].command_str);
        if (sim > best_similarity) {
            best_similarity = sim;
            best_index = i;
        }
    }
    
    *confidence = best_similarity;
    return best_index;
}

static void extract_command_from_text(const char *text, char *command, size_t max_len)
{
    // Buscar palabras clave
    const char *keywords[] = {"leer", "receta", "glucosa", "quién", "qué", "ayuda", "volumen"};
    int num_keywords = sizeof(keywords) / sizeof(keywords[0]);
    
    for (int i = 0; i < num_keywords; i++) {
        if (strstr(text, keywords[i])) {
            strncpy(command, keywords[i], max_len - 1);
            command[max_len - 1] = '\0';
            return;
        }
    }
    
    // Si no hay keywords, usar la primera palabra
    const char *first_space = strchr(text, ' ');
    if (first_space) {
        int len = first_space - text;
        if (len < max_len - 1) {
            strncpy(command, text, len);
            command[len] = '\0';
        } else {
            strncpy(command, text, max_len - 1);
            command[max_len - 1] = '\0';
        }
    } else {
        strncpy(command, text, max_len - 1);
        command[max_len - 1] = '\0';
    }
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

esp_err_t voice_recognition_init(void)
{
    if (recognition_initialized) {
        ESP_LOGW(TAG, "Reconocimiento ya inicializado");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Inicializando reconocimiento de voz");
    ESP_LOGI(TAG, "Comandos disponibles: %d", NUM_COMMANDS);
    
    for (int i = 0; i < NUM_COMMANDS; i++) {
        ESP_LOGD(TAG, "  - %s (%s)", commands[i].command_str, commands[i].description);
    }
    
    recognition_initialized = true;
    
    return ESP_OK;
}

esp_err_t voice_recognition_process(audio_segment_t *segment, voice_command_t *result)
{
    if (!recognition_initialized || !segment || !result) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // En un sistema real, aquí se usaría un modelo acústico y de lenguaje
    // Por ahora, simular reconocimiento basado en el audio
    
    // Calcular RMS como indicador de actividad
    float rms = audio_calculate_rms(segment);
    
    if (rms < SILENCE_THRESHOLD) {
        return ESP_ERR_NOT_FOUND;  // Silencio
    }
    
    // Simular texto reconocido basado en patrón simple
    // En producción, esto sería el resultado de un modelo de speech-to-text
    uint8_t hash = simple_hash((const char*)segment->samples);
    const char *recognized_text = "";
    
    // Patrón basado en hash (solo para simulación)
    switch (hash % 10) {
        case 0: recognized_text = "leer etiqueta"; break;
        case 1: recognized_text = "receta de flan"; break;
        case 2: recognized_text = "glucosa actual"; break;
        case 3: recognized_text = "quién es esa persona"; break;
        case 4: recognized_text = "qué estaba diciendo"; break;
        case 5: recognized_text = "necesito ayuda"; break;
        case 6: recognized_text = "subir volumen"; break;
        case 7: recognized_text = "qué día es hoy"; break;
        case 8: recognized_text = "qué hora es"; break;
        case 9: recognized_text = "recordatorio de pastillas"; break;
    }
    
    // Extraer comando del texto
    char extracted_command[32];
    extract_command_from_text(recognized_text, extracted_command, sizeof(extracted_command));
    
    // Buscar mejor coincidencia
    float confidence;
    int best_index = find_best_match(extracted_command, &confidence);
    
    if (best_index >= 0 && confidence > 0.5) {
        result->id = commands[best_index].id;
        result->command_str = commands[best_index].command_str;
        result->description = commands[best_index].description;
        result->confidence = confidence;
        
        // Actualizar estadísticas
        command_count[best_index]++;
        total_commands++;
        recognition_confidence = confidence;
        last_command = *result;
        
        ESP_LOGI(TAG, "Comando detectado: '%s' (confianza: %.2f)", 
                 result->command_str, confidence);
        
        // Llamar callback si existe
        if (command_callback) {
            command_callback(result, callback_arg);
        }
        
        return ESP_OK;
    }
    
    return ESP_ERR_NOT_FOUND;
}

esp_err_t voice_recognition_recognize(const char *text, voice_command_t *result)
{
    if (!recognition_initialized || !text || !result) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Buscar mejor coincidencia
    float confidence;
    int best_index = find_best_match(text, &confidence);
    
    if (best_index >= 0 && confidence > 0.5) {
        result->id = commands[best_index].id;
        result->command_str = commands[best_index].command_str;
        result->description = commands[best_index].description;
        result->confidence = confidence;
        
        ESP_LOGD(TAG, "Texto '%s' reconocido como comando '%s' (conf: %.2f)",
                 text, result->command_str, confidence);
        
        return ESP_OK;
    }
    
    return ESP_ERR_NOT_FOUND;
}

const voice_command_t* voice_recognition_get_commands(int *count)
{
    if (count) {
        *count = NUM_COMMANDS;
    }
    return commands;
}

const char* voice_recognition_command_to_string(uint8_t command_id)
{
    for (int i = 0; i < NUM_COMMANDS; i++) {
        if (commands[i].id == command_id) {
            return commands[i].command_str;
        }
    }
    return "desconocido";
}

void voice_recognition_set_callback(command_callback_t callback, void *arg)
{
    command_callback = callback;
    callback_arg = arg;
    ESP_LOGI(TAG, "Callback de comandos registrado");
}

void voice_recognition_get_stats(voice_stats_t *stats)
{
    if (stats) {
        stats->total_commands = total_commands;
        stats->last_confidence = recognition_confidence;
        memcpy(stats->command_count, command_count, sizeof(command_count));
    }
}

void voice_recognition_reset_stats(void)
{
    total_commands = 0;
    memset(command_count, 0, sizeof(command_count));
    ESP_LOGI(TAG, "Estadísticas reiniciadas");
}

float voice_recognition_get_confidence(void)
{
    return recognition_confidence;
}

uint32_t voice_recognition_get_command_count(uint8_t command_id)
{
    for (int i = 0; i < NUM_COMMANDS; i++) {
        if (commands[i].id == command_id) {
            return command_count[i];
        }
    }
    return 0;
}

const voice_command_t* voice_recognition_get_last_command(void)
{
    return &last_command;
}
