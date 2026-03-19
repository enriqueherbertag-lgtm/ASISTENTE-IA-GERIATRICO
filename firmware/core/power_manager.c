/**
 * @file power_manager.c
 * @brief Gestión de energía y modos de sueño
 * 
 * Este módulo controla los diferentes estados de energía del dispositivo,
 * optimizando el consumo según la actividad y configurando los wake-up sources.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "power_manager.h"
#include "config.h"

static const char *TAG = "POWER_MGR";

// Variables internas
static PowerMode_t current_mode = POWER_MODE_ACTIVE;
static uint32_t last_activity_time = 0;
static uint32_t activity_timeout_ms = 30000;  // 30 segundos por defecto
static WakeupSource_t wakeup_sources = WAKEUP_NONE;

// Pines de wake-up
#define WAKEUP_PIN_MIC      GPIO_NUM_4   // Micrófono (detección de voz)
#define WAKEUP_PIN_IMU      GPIO_NUM_5   // IMU (movimiento)

/* ------------------------------------------------------------------
 * Inicialización del gestor de energía
 * ------------------------------------------------------------------ */
esp_err_t power_manager_init(void)
{
    ESP_LOGI(TAG, "Inicializando gestor de energía");
    
    // Configurar pines de wake-up
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << WAKEUP_PIN_MIC) | (1ULL << WAKEUP_PIN_IMU),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    // Habilitar wake-up desde GPIO en sleep profundo
    esp_sleep_enable_gpio_wakeup();
    
    // Configurar timer de wake-up por defecto
    esp_sleep_enable_timer_wakeup(ACTIVITY_CHECK_INTERVAL * 1000);
    
    last_activity_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    current_mode = POWER_MODE_ACTIVE;
    
    return ESP_OK;
}

/* ------------------------------------------------------------------
 * Detectar actividad del usuario
 * ------------------------------------------------------------------ */
bool power_manager_detect_activity(void)
{
    // Detectar por IMU (movimiento)
    if (imu_movement_detected()) {
        return true;
    }
    
    // Detectar por micrófono (sonido ambiental)
    if (audio_sound_detected()) {
        return true;
    }
    
    return false;
}

/* ------------------------------------------------------------------
 * Actualizar tiempo de última actividad
 * ------------------------------------------------------------------ */
void power_manager_update_activity(void)
{
    last_activity_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // Si estaba en modo sleep, volver a activo
    if (current_mode != POWER_MODE_ACTIVE) {
        current_mode = POWER_MODE_ACTIVE;
        ESP_LOGI(TAG, "Actividad detectada - Modo ACTIVO");
    }
}

/* ------------------------------------------------------------------
 * Determinar el modo de energía apropiado
 * ------------------------------------------------------------------ */
PowerMode_t power_manager_get_mode(void)
{
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t idle_time = now - last_activity_time;
    
    // Si hay actividad reciente, mantener modo activo
    if (idle_time < activity_timeout_ms) {
        return POWER_MODE_ACTIVE;
    }
    
    // Decidir según tiempo de inactividad
    if (idle_time < (activity_timeout_ms * 2)) {
        return POWER_MODE_LIGHT_SLEEP;
    } else if (idle_time < (activity_timeout_ms * 10)) {
        return POWER_MODE_DEEP_SLEEP;
    } else {
        return POWER_MODE_SHUTDOWN;
    }
}

/* ------------------------------------------------------------------
 * Configurar wake-up sources según el modo
 * ------------------------------------------------------------------ */
void power_manager_configure_wakeup(PowerMode_t next_mode)
{
    // Limpiar wake-up sources anteriores
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    
    switch (next_mode) {
        case POWER_MODE_LIGHT_SLEEP:
            // En light sleep, mantener micrófono y IMU activos
            esp_sleep_enable_gpio_wakeup();
            esp_sleep_enable_timer_wakeup(ACTIVITY_CHECK_INTERVAL * 1000);
            wakeup_sources = WAKEUP_MIC | WAKEUP_IMU | WAKEUP_TIMER;
            break;
            
        case POWER_MODE_DEEP_SLEEP:
            // En deep sleep, solo GPIO puede despertar
            esp_sleep_enable_gpio_wakeup();
            // Desactivar timer para ahorrar energía
            wakeup_sources = WAKEUP_MIC | WAKEUP_IMU;
            break;
            
        case POWER_MODE_SHUTDOWN:
            // En shutdown, solo botón físico (pero no hay botones)
            // Por ahora, timer para reactivación periódica
            esp_sleep_enable_timer_wakeup(3600 * 1000);  // 1 hora
            wakeup_sources = WAKEUP_TIMER;
            break;
            
        default:
            break;
    }
}

/* ------------------------------------------------------------------
 * Ejecutar la transición a un modo de energía
 * ------------------------------------------------------------------ */
esp_err_t power_manager_enter_mode(PowerMode_t mode)
{
    esp_err_t ret = ESP_OK;
    
    ESP_LOGI(TAG, "Transición a modo %d", mode);
    
    // Guardar modo actual
    current_mode = mode;
    
    // Configurar periféricos según el modo
    switch (mode) {
        case POWER_MODE_ACTIVE:
            // Reactivar todos los periféricos
            camera_power_on();
            nfc_power_on();
            imu_set_power_mode(IMU_NORMAL);
            audio_set_power_mode(AUDIO_NORMAL);
            break;
            
        case POWER_MODE_LIGHT_SLEEP:
            // Apagar periféricos de alto consumo
            camera_power_off();
            nfc_power_off();
            imu_set_power_mode(IMU_LOW_POWER);
            audio_set_power_mode(AUDIO_LOW_POWER);
            
            // Configurar wake-up
            power_manager_configure_wakeup(mode);
            
            // Entrar en light sleep
            ESP_LOGI(TAG, "Entrando en light sleep");
            ret = esp_light_sleep_start();
            break;
            
        case POWER_MODE_DEEP_SLEEP:
            // Apagar casi todo
            camera_power_off();
            nfc_power_off();
            imu_set_power_mode(IMU_SLEEP);
            audio_set_power_mode(AUDIO_SLEEP);
            
            // Configurar wake-up
            power_manager_configure_wakeup(mode);
            
            // Entrar en deep sleep
            ESP_LOGI(TAG, "Entrando en deep sleep");
            ret = esp_deep_sleep_start();
            break;
            
        case POWER_MODE_SHUTDOWN:
            // Apagar todo (solo RTC)
            camera_power_off();
            nfc_power_off();
            imu_set_power_mode(IMU_SLEEP);
            audio_set_power_mode(AUDIO_SLEEP);
            
            // Configurar wake-up
            power_manager_configure_wakeup(mode);
            
            // Entrar en deep sleep prolongado
            ESP_LOGI(TAG, "Entrando en shutdown");
            ret = esp_deep_sleep_start();
            break;
    }
    
    // Si llegamos aquí, es porque despertamos
    ESP_LOGI(TAG, "Sistema despertado");
    
    // Identificar fuente de wake-up
    power_manager_identify_wakeup_source();
    
    return ret;
}

/* ------------------------------------------------------------------
 * Identificar qué causó el wake-up
 * ------------------------------------------------------------------ */
WakeupSource_t power_manager_get_wakeup_source(void)
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    WakeupSource_t source = WAKEUP_NONE;
    
    switch (cause) {
        case ESP_SLEEP_WAKEUP_TIMER:
            source = WAKEUP_TIMER;
            ESP_LOGI(TAG, "Wake-up por TIMER");
            break;
            
        case ESP_SLEEP_WAKEUP_GPIO:
            source = WAKEUP_GPIO;
            ESP_LOGI(TAG, "Wake-up por GPIO");
            break;
            
        case ESP_SLEEP_WAKEUP_UART:
            source = WAKEUP_UART;
            ESP_LOGI(TAG, "Wake-up por UART");
            break;
            
        default:
            source = WAKEUP_NONE;
            ESP_LOGI(TAG, "Wake-up por REINICIO");
            break;
    }
    
    return source;
}

/* ------------------------------------------------------------------
 * Obtener el tiempo de sleep recomendado
 * ------------------------------------------------------------------ */
uint32_t power_manager_get_sleep_duration(void)
{
    PowerMode_t next_mode = power_manager_get_mode();
    
    switch (next_mode) {
        case POWER_MODE_ACTIVE:
            return 0;  // No dormir
            
        case POWER_MODE_LIGHT_SLEEP:
            return ACTIVITY_CHECK_INTERVAL;
            
        case POWER_MODE_DEEP_SLEEP:
            return DEEP_SLEEP_INTERVAL;
            
        case POWER_MODE_SHUTDOWN:
            return SHUTDOWN_INTERVAL;
            
        default:
            return ACTIVITY_CHECK_INTERVAL;
    }
}

/* ------------------------------------------------------------------
 * Obtener el modo actual
 * ------------------------------------------------------------------ */
PowerMode_t power_manager_get_current_mode(void)
{
    return current_mode;
}

/* ------------------------------------------------------------------
 * Forzar entrada en modo de bajo consumo
 * ------------------------------------------------------------------ */
void power_manager_force_sleep(PowerMode_t mode)
{
    if (mode > current_mode) {
        power_manager_enter_mode(mode);
    }
}

/* ------------------------------------------------------------------
 * Verificar si el sistema está en modo activo
 * ------------------------------------------------------------------ */
bool power_manager_is_active(void)
{
    return (current_mode == POWER_MODE_ACTIVE);
}

/* ------------------------------------------------------------------
 * Calcular consumo estimado
 * ------------------------------------------------------------------ */
uint32_t power_manager_get_estimated_consumption(void)
{
    switch (current_mode) {
        case POWER_MODE_ACTIVE:
            return 50000;  // 50 mA (promedio con procesamiento)
        case POWER_MODE_LIGHT_SLEEP:
            return 5000;   // 5 mA
        case POWER_MODE_DEEP_SLEEP:
            return 100;    // 100 µA
        case POWER_MODE_SHUTDOWN:
            return 10;     // 10 µA
        default:
            return 0;
    }
}

/* ------------------------------------------------------------------
 * Configurar timeout de actividad
 * ------------------------------------------------------------------ */
void power_manager_set_activity_timeout(uint32_t timeout_ms)
{
    activity_timeout_ms = timeout_ms;
    ESP_LOGI(TAG, "Timeout de actividad configurado a %lu ms", timeout_ms);
}

/* ------------------------------------------------------------------
 * Obtener estadísticas de energía
 * ------------------------------------------------------------------ */
void power_manager_get_stats(PowerStats_t *stats)
{
    stats->current_mode = current_mode;
    stats->estimated_consumption_ua = power_manager_get_estimated_consumption();
    stats->wakeup_sources = wakeup_sources;
    stats->last_activity_ms = last_activity_time;
    stats->uptime_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
}

/* ------------------------------------------------------------------
 * Manejar evento de wake-up por GPIO
 * ------------------------------------------------------------------ */
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    
    // Solo registrar el evento, el procesamiento se hace en la tarea principal
    if (gpio_num == WAKEUP_PIN_MIC) {
        wakeup_sources |= WAKEUP_MIC;
    } else if (gpio_num == WAKEUP_PIN_IMU) {
        wakeup_sources |= WAKEUP_IMU;
    }
}

/* ------------------------------------------------------------------
 * Identificar fuente de wake-up
 * ------------------------------------------------------------------ */
void power_manager_identify_wakeup_source(void)
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    
    switch (cause) {
        case ESP_SLEEP_WAKEUP_TIMER:
            ESP_LOGI(TAG, "Despertado por TIMER");
            wakeup_sources = WAKEUP_TIMER;
            break;
            
        case ESP_SLEEP_WAKEUP_GPIO:
            ESP_LOGI(TAG, "Despertado por GPIO");
            wakeup_sources = WAKEUP_GPIO;
            
            // Identificar qué pin
            if (gpio_get_level(WAKEUP_PIN_MIC)) {
                wakeup_sources |= WAKEUP_MIC;
                ESP_LOGI(TAG, "  → Micrófono detectó sonido");
            }
            if (gpio_get_level(WAKEUP_PIN_IMU)) {
                wakeup_sources |= WAKEUP_IMU;
                ESP_LOGI(TAG, "  → IMU detectó movimiento");
            }
            break;
            
        default:
            wakeup_sources = WAKEUP_NONE;
            break;
    }
}
