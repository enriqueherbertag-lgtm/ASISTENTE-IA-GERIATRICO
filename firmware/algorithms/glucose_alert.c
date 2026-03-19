/**
 * @file glucose_alert.c
 * @brief Algoritmo de alertas de glucosa
 * 
 * Este módulo procesa lecturas del sensor FreeStyle Libre
 * y genera alertas según los niveles de glucosa y tendencias.
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
#include "glucose_alert.h"
#include "nfc_driver.h"
#include "audio_driver.h"
#include "config.h"

static const char *TAG = "GLUCOSE_ALERT";

// Umbrales por defecto
static uint16_t hypo_threshold = DEFAULT_HYPO_LOW;
static uint16_t hypo_critical = DEFAULT_HYPO_CRITICAL;
static uint16_t hyper_threshold = DEFAULT_HYPER;

// Historial de lecturas
#define MAX_HISTORY 100
static glucose_reading_t history[MAX_HISTORY];
static int history_count = 0;
static int history_index = 0;

// Últimas lecturas
static glucose_data_t last_reading = {0};
static uint32_t last_read_time = 0;

// Alertas
static bool alerts_enabled = true;
static uint32_t last_alert_time = 0;
static uint8_t last_alert_level = 0;

// Callbacks
static glucose_callback_t glucose_callback = NULL;
static void *callback_arg = NULL;

// Estadísticas
static glucose_stats_t stats = {0};

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static void add_to_history(glucose_data_t *data)
{
    glucose_reading_t reading;
    reading.timestamp = data->timestamp;
    reading.glucose = data->glucose_mgdl;
    reading.trend = data->trend;
    
    history[history_index] = reading;
    history_index = (history_index + 1) % MAX_HISTORY;
    
    if (history_count < MAX_HISTORY) {
        history_count++;
    }
}

static uint8_t get_alert_level(glucose_data_t *data)
{
    if (data->glucose_mgdl < hypo_critical) {
        return GLUCOSE_ALERT_CRITICAL;
    } else if (data->glucose_mgdl < hypo_threshold) {
        return GLUCOSE_ALERT_LOW;
    } else if (data->glucose_mgdl > hyper_threshold) {
        return GLUCOSE_ALERT_HIGH;
    }
    return GLUCOSE_ALERT_NONE;
}

static bool should_alert(glucose_data_t *data)
{
    uint8_t level = get_alert_level(data);
    
    if (level == GLUCOSE_ALERT_NONE) {
        return false;
    }
    
    // Si es la primera alerta de este nivel, alertar
    if (level != last_alert_level) {
        return true;
    }
    
    // Si es el mismo nivel, verificar tiempo desde última alerta
    uint32_t now = esp_timer_get_time() / 1000;
    uint32_t time_since_last = now - last_alert_time;
    
    // Alertar cada 30 minutos para niveles bajos/altos
    // Alertar cada 5 minutos para nivel crítico
    uint32_t cooldown = (level == GLUCOSE_ALERT_CRITICAL) ? 300000 : 1800000; // 5min / 30min
    
    return (time_since_last > cooldown);
}

static void update_stats(glucose_data_t *data, uint8_t alert_level)
{
    stats.total_readings++;
    stats.last_glucose = data->glucose_mgdl;
    
    if (data->glucose_mgdl < hypo_threshold) {
        stats.hypo_count++;
    }
    if (data->glucose_mgdl > hyper_threshold) {
        stats.hyper_count++;
    }
    
    if (alert_level != GLUCOSE_ALERT_NONE) {
        stats.alert_count++;
    }
    
    // Actualizar promedio
    if (stats.total_readings > 1) {
        stats.average_glucose = (stats.average_glucose * (stats.total_readings - 1) + 
                                 data->glucose_mgdl) / stats.total_readings;
    } else {
        stats.average_glucose = data->glucose_mgdl;
    }
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

esp_err_t glucose_alert_init(void)
{
    ESP_LOGI(TAG, "Inicializando sistema de alertas de glucosa");
    ESP_LOGI(TAG, "Umbrales: hipo=%d mg/dL, crítica=%d mg/dL, hiper=%d mg/dL",
             hypo_threshold, hypo_critical, hyper_threshold);
    
    memset(&stats, 0, sizeof(glucose_stats_t));
    history_count = 0;
    history_index = 0;
    
    return ESP_OK;
}

uint8_t glucose_alert_check(glucose_data_t *data)
{
    if (!data) {
        return GLUCOSE_ALERT_NONE;
    }
    
    uint8_t alert_level = get_alert_level(data);
    
    // Actualizar historial
    add_to_history(data);
    
    // Actualizar estadísticas
    update_stats(data, alert_level);
    
    // Verificar si debe alertar
    if (alerts_enabled && should_alert(data)) {
        last_alert_time = esp_timer_get_time() / 1000;
        last_alert_level = alert_level;
        last_reading = *data;
        
        ESP_LOGI(TAG, "ALERTA nivel %d: glucosa=%d mg/dL, tendencia=%d",
                 alert_level, data->glucose_mgdl, data->trend);
        
        // Llamar callback si existe
        if (glucose_callback) {
            glucose_callback(data, alert_level, callback_arg);
        }
    }
    
    return alert_level;
}

void glucose_alert_set_thresholds(uint16_t hypo, uint16_t critical, uint16_t hyper)
{
    hypo_threshold = hypo;
    hypo_critical = critical;
    hyper_threshold = hyper;
    
    ESP_LOGI(TAG, "Umbrales actualizados: hipo=%d, crítica=%d, hiper=%d",
             hypo, critical, hyper);
}

void glucose_alert_enable(bool enable)
{
    alerts_enabled = enable;
    ESP_LOGI(TAG, "Alertas %s", enable ? "habilitadas" : "deshabilitadas");
}

void glucose_alert_set_callback(glucose_callback_t callback, void *arg)
{
    glucose_callback = callback;
    callback_arg = arg;
    ESP_LOGI(TAG, "Callback de glucosa registrado");
}

glucose_data_t glucose_alert_get_last_reading(void)
{
    return last_reading;
}

uint32_t glucose_alert_get_last_read_time(void)
{
    return last_read_time;
}

void glucose_alert_get_history(glucose_reading_t *buffer, int *count, int max)
{
    int to_copy = (history_count < max) ? history_count : max;
    
    if (to_copy > 0) {
        // Copiar desde el índice más antiguo hasta el más reciente
        int start = (history_index - history_count + MAX_HISTORY) % MAX_HISTORY;
        
        for (int i = 0; i < to_copy; i++) {
            int idx = (start + i) % MAX_HISTORY;
            buffer[i] = history[idx];
        }
    }
    
    *count = to_copy;
}

glucose_stats_t glucose_alert_get_stats(void)
{
    return stats;
}

bool glucose_alert_is_trend_dangerous(uint8_t trend)
{
    // Tendencias peligrosas: bajando rápido o subiendo rápido
    return (trend == 6 || trend == 3);  // 6 = bajando rápido, 3 = subiendo rápido
}

uint32_t glucose_alert_time_to_target(glucose_data_t *current, uint16_t target, float rate_mgdl_per_min)
{
    if (rate_mgdl_per_min == 0) return 0;
    
    int16_t diff = (int16_t)target - (int16_t)current->glucose_mgdl;
    float minutes = diff / rate_mgdl_per_min;
    
    if (minutes < 0) minutes = 0;
    
    return (uint32_t)(minutes * 60);  // segundos
}

void glucose_alert_reset_stats(void)
{
    memset(&stats, 0, sizeof(glucose_stats_t));
    ESP_LOGI(TAG, "Estadísticas reiniciadas");
}
