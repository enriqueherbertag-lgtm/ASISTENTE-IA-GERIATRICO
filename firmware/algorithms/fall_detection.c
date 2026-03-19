/**
 * @file fall_detection.c
 * @brief Algoritmo de detección de caídas
 * 
 * Este módulo procesa datos del IMU para detectar caídas
 * en tiempo real y gestionar las alertas correspondientes.
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
#include "fall_detection.h"
#include "imu_driver.h"
#include "config.h"

static const char *TAG = "FALL_DETECT";

// Variables de estado
static fall_state_t current_state = FALL_STATE_NORMAL;
static uint32_t state_start_time = 0;
static uint32_t last_alert_time = 0;

// Umbrales (desde config.h)
static float fall_threshold = FALL_THRESHOLD_G;
static float free_fall_threshold = FREE_FALL_THRESHOLD_G;
static float impact_threshold = IMPACT_THRESHOLD_G;
static uint32_t post_fall_time = POST_FALL_TIME_MS;
static uint32_t inactivity_time = INACTIVITY_TIME_MS;

// Historial de lecturas
#define HISTORY_SIZE 50
static float accel_history[HISTORY_SIZE] = {0};
static int history_index = 0;

// Callbacks
static fall_callback_t fall_callback = NULL;
static void *callback_arg = NULL;

// Contadores
static uint32_t fall_count = 0;
static uint32_t false_positive_count = 0;

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static float calculate_magnitude(imu_data_t *data)
{
    return sqrt(data->accel_x * data->accel_x +
                data->accel_y * data->accel_y +
                data->accel_z * data->accel_z);
}

static void update_history(float magnitude)
{
    accel_history[history_index] = magnitude;
    history_index = (history_index + 1) % HISTORY_SIZE;
}

static float get_recent_variance(void)
{
    float mean = 0;
    float variance = 0;
    
    // Calcular media
    for (int i = 0; i < HISTORY_SIZE; i++) {
        mean += accel_history[i];
    }
    mean /= HISTORY_SIZE;
    
    // Calcular varianza
    for (int i = 0; i < HISTORY_SIZE; i++) {
        variance += (accel_history[i] - mean) * (accel_history[i] - mean);
    }
    variance /= HISTORY_SIZE;
    
    return variance;
}

static bool check_inactivity(void)
{
    float variance = get_recent_variance();
    return (variance < 0.01);  // Muy poca variación → inactividad
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

void fall_detection_init(void)
{
    ESP_LOGI(TAG, "Inicializando detector de caídas");
    current_state = FALL_STATE_NORMAL;
    state_start_time = 0;
    fall_count = 0;
    false_positive_count = 0;
}

fall_state_t fall_detection_process(imu_data_t *data)
{
    uint32_t now = esp_timer_get_time() / 1000;  // Convertir a ms
    float magnitude = calculate_magnitude(data);
    
    // Actualizar historial
    update_history(magnitude);
    
    // Máquina de estados
    switch (current_state) {
        case FALL_STATE_NORMAL:
            if (magnitude < free_fall_threshold) {
                current_state = FALL_STATE_FREE_FALL;
                state_start_time = now;
                ESP_LOGD(TAG, "Caída libre detectada (magnitud: %.2f g)", magnitude);
            }
            break;
            
        case FALL_STATE_FREE_FALL:
            if (magnitude > impact_threshold) {
                current_state = FALL_STATE_IMPACT;
                state_start_time = now;
                ESP_LOGI(TAG, "Impacto detectado (magnitud: %.2f g)", magnitude);
            } else if (now - state_start_time > 2000) {
                // Más de 2 segundos en caída libre = falso positivo
                current_state = FALL_STATE_NORMAL;
                false_positive_count++;
                ESP_LOGD(TAG, "Falso positivo: caída libre prolongada");
            }
            break;
            
        case FALL_STATE_IMPACT:
            current_state = FALL_STATE_POST_FALL;
            state_start_time = now;
            break;
            
        case FALL_STATE_POST_FALL:
            // Esperar inactividad post-impacto
            if (check_inactivity()) {
                if (now - state_start_time > post_fall_time) {
                    current_state = FALL_STATE_FALL_CONFIRMED;
                    fall_count++;
                    ESP_LOGW(TAG, "¡CAÍDA CONFIRMADA! (contador: %lu)", fall_count);
                    
                    // Llamar callback si está registrado
                    if (fall_callback) {
                        fall_callback(callback_arg);
                    }
                }
            } else {
                // Se movió después del impacto → no fue caída
                current_state = FALL_STATE_NORMAL;
                false_positive_count++;
                ESP_LOGD(TAG, "Falso positivo: movimiento post-impacto");
            }
            break;
            
        case FALL_STATE_FALL_CONFIRMED:
            // Salir del estado después de un tiempo
            if (now - state_start_time > 10000) {  // 10 segundos mostrando alerta
                current_state = FALL_STATE_NORMAL;
                ESP_LOGI(TAG, "Volviendo a estado normal");
            }
            break;
    }
    
    return current_state;
}

void fall_detection_reset(void)
{
    current_state = FALL_STATE_NORMAL;
    state_start_time = 0;
    ESP_LOGI(TAG, "Detector de caídas reiniciado");
}

bool fall_detection_is_falling(void)
{
    return (current_state == FALL_STATE_FREE_FALL ||
            current_state == FALL_STATE_IMPACT);
}

bool fall_detection_is_fallen(void)
{
    return (current_state == FALL_STATE_FALL_CONFIRMED);
}

void fall_detection_set_callback(fall_callback_t callback, void *arg)
{
    fall_callback = callback;
    callback_arg = arg;
    ESP_LOGI(TAG, "Callback registrado");
}

void fall_detection_set_thresholds(float free_fall, float impact, float fall)
{
    free_fall_threshold = free_fall;
    impact_threshold = impact;
    fall_threshold = fall;
    ESP_LOGI(TAG, "Umbrales actualizados: FF=%.1f g, IMP=%.1f g, FALL=%.1f g",
             free_fall, impact, fall);
}

void fall_detection_set_timings(uint32_t post_fall_ms, uint32_t inactivity_ms)
{
    post_fall_time = post_fall_ms;
    inactivity_time = inactivity_ms;
    ESP_LOGI(TAG, "Tiempos actualizados: post-fall=%lu ms, inactividad=%lu ms",
             post_fall_ms, inactivity_ms);
}

fall_state_t fall_detection_get_state(void)
{
    return current_state;
}

const char* fall_detection_state_to_string(fall_state_t state)
{
    switch (state) {
        case FALL_STATE_NORMAL:        return "Normal";
        case FALL_STATE_FREE_FALL:     return "Caída libre";
        case FALL_STATE_IMPACT:        return "Impacto";
        case FALL_STATE_POST_FALL:     return "Post-impacto";
        case FALL_STATE_FALL_CONFIRMED: return "Caída confirmada";
        default:                        return "Desconocido";
    }
}

uint32_t fall_detection_get_count(void)
{
    return fall_count;
}

uint32_t fall_detection_get_false_positives(void)
{
    return false_positive_count;
}

void fall_detection_get_stats(fall_stats_t *stats)
{
    stats->fall_count = fall_count;
    stats->false_positive_count = false_positive_count;
    stats->current_state = current_state;
    stats->last_alert_time = last_alert_time;
}

bool fall_detection_should_alert(uint32_t cooldown_ms)
{
    uint32_t now = esp_timer_get_time() / 1000;
    
    if (current_state == FALL_STATE_FALL_CONFIRMED) {
        if (now - last_alert_time > cooldown_ms) {
            last_alert_time = now;
            return true;
        }
    }
    
    return false;
}

float fall_detection_get_confidence(void)
{
    // Calcular confianza basada en la claridad de la detección
    switch (current_state) {
        case FALL_STATE_FALL_CONFIRMED:
            return 0.95;
        case FALL_STATE_POST_FALL:
            return 0.70;
        case FALL_STATE_IMPACT:
            return 0.50;
        case FALL_STATE_FREE_FALL:
            return 0.30;
        default:
            return 0.0;
    }
}
