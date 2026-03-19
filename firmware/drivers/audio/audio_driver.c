/**
 * @file audio_driver.c
 * @brief Implementación del driver para audio (micrófono y altavoz)
 * 
 * Este archivo contiene la implementación de las funciones
 * declaradas en audio_driver.h
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
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "audio_driver.h"
#include "config.h"

static const char *TAG = "AUDIO_DRV";

// Configuración I2S para micrófono
#define I2S_MIC_PORT        I2S_NUM_0
#define I2S_MIC_SAMPLE_RATE 16000
#define I2S_MIC_BITS        16
#define I2S_MIC_CHANNELS    1

// Configuración I2S para altavoz
#define I2S_SPEAKER_PORT    I2S_NUM_1
#define I2S_SPEAKER_SAMPLE_RATE 16000
#define I2S_SPEAKER_BITS    16
#define I2S_SPEAKER_CHANNELS 1

// Buffers
#define AUDIO_BUFFER_SIZE   512   // Muestras
#define AUDIO_RINGBUFFER_SIZE (AUDIO_BUFFER_SIZE * 8)

// Umbrales
#define SILENCE_THRESHOLD   500    // RMS para considerar silencio
#define WAKE_WORD_THRESHOLD 80     // Confianza para palabra de activación

// Palabras de activación (simuladas por ahora)
#define WAKE_WORD_COUNT      3
static const char *wake_words[] = {"hola", "ayuda", "escucha"};

// Variables internas
static bool audio_initialized = false;
static audio_power_mode_t current_power_mode = AUDIO_NORMAL;
static voice_state_t current_voice_state = VOICE_IDLE;

static uint32_t play_count = 0;
static uint32_t capture_count = 0;
static uint8_t current_volume = 80;  // 0-100

static QueueHandle_t audio_queue = NULL;
static TaskHandle_t audio_task_handle = NULL;

// I2S configuraciones
static i2s_config_t i2s_mic_config = {
    .mode = I2S_MODE_MASTER | I2S_MODE_RX,
    .sample_rate = I2S_MIC_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = AUDIO_BUFFER_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
};

static i2s_config_t i2s_speaker_config = {
    .mode = I2S_MODE_MASTER | I2S_MODE_TX,
    .sample_rate = I2S_SPEAKER_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = AUDIO_BUFFER_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
};

static i2s_pin_config_t mic_pins = {
    .bck_io_num = MIC_PIN_CLK,
    .ws_io_num = MIC_PIN_CLK,  // WS compartido con CLK en algunos MEMS
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = MIC_PIN_DIN
};

static i2s_pin_config_t speaker_pins = {
    .bck_io_num = -1,  // No usado en modo simple
    .ws_io_num = -1,
    .data_out_num = SPEAKER_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
};

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static void audio_task(void *arg)
{
    int16_t buffer[AUDIO_BUFFER_SIZE];
    size_t bytes_read;
    
    while (1) {
        if (current_voice_state == VOICE_LISTENING) {
            // Leer datos del micrófono
            esp_err_t err = i2s_read(I2S_MIC_PORT, buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);
            
            if (err == ESP_OK && bytes_read > 0) {
                int samples = bytes_read / sizeof(int16_t);
                
                // Calcular RMS para detección de actividad
                int32_t sum_sq = 0;
                for (int i = 0; i < samples; i++) {
                    sum_sq += buffer[i] * buffer[i];
                }
                float rms = sqrt(sum_sq / samples);
                
                // Enviar a cola para procesamiento si hay actividad
                if (rms > SILENCE_THRESHOLD) {
                    // Simular detección de palabra (aleatoria por ahora)
                    if (rand() % 100 < 5) {  // 5% de probabilidad
                        wake_word_result_t result = {
                            .detected = true,
                            .timestamp = esp_timer_get_time(),
                            .confidence = 70 + rand() % 30
                        };
                        xQueueSend(audio_queue, &result, 0);
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

esp_err_t audio_init(void)
{
    if (audio_initialized) {
        ESP_LOGW(TAG, "Audio ya inicializado");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Inicializando sistema de audio");
    
    // Inicializar I2S para micrófono
    esp_err_t err = i2s_driver_install(I2S_MIC_PORT, &i2s_mic_config, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error instalando driver I2S micrófono: %d", err);
        return err;
    }
    
    err = i2s_set_pin(I2S_MIC_PORT, &mic_pins);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error configurando pines micrófono");
        i2s_driver_uninstall(I2S_MIC_PORT);
        return err;
    }
    
    // Inicializar I2S para altavoz
    err = i2s_driver_install(I2S_SPEAKER_PORT, &i2s_speaker_config, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error instalando driver I2S altavoz");
        i2s_driver_uninstall(I2S_MIC_PORT);
        return err;
    }
    
    err = i2s_set_pin(I2S_SPEAKER_PORT, &speaker_pins);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error configurando pines altavoz");
        i2s_driver_uninstall(I2S_MIC_PORT);
        i2s_driver_uninstall(I2S_SPEAKER_PORT);
        return err;
    }
    
    // Crear cola para eventos de audio
    audio_queue = xQueueCreate(10, sizeof(wake_word_result_t));
    
    // Crear tarea de procesamiento de audio
    xTaskCreate(audio_task, "audio_task", 4096, NULL, 5, &audio_task_handle);
    
    audio_initialized = true;
    current_power_mode = AUDIO_NORMAL;
    current_voice_state = VOICE_IDLE;
    
    ESP_LOGI(TAG, "Audio inicializado correctamente");
    
    return ESP_OK;
}

esp_err_t audio_set_power_mode(audio_power_mode_t mode)
{
    current_power_mode = mode;
    
    switch (mode) {
        case AUDIO_NORMAL:
            ESP_LOGI(TAG, "Audio modo NORMAL");
            i2s_start(I2S_MIC_PORT);
            i2s_start(I2S_SPEAKER_PORT);
            break;
            
        case AUDIO_LOW_POWER:
            ESP_LOGI(TAG, "Audio modo LOW POWER");
            i2s_stop(I2S_MIC_PORT);
            i2s_stop(I2S_SPEAKER_PORT);
            break;
            
        case AUDIO_SLEEP:
            ESP_LOGI(TAG, "Audio modo SLEEP");
            i2s_stop(I2S_MIC_PORT);
            i2s_stop(I2S_SPEAKER_PORT);
            break;
    }
    
    return ESP_OK;
}

audio_power_mode_t audio_get_power_mode(void)
{
    return current_power_mode;
}

/* ==================================================================
 * FUNCIONES DE ENTRADA (MICRÓFONO)
 * ================================================================== */

esp_err_t audio_start_listening(void)
{
    if (!audio_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    current_voice_state = VOICE_LISTENING;
    i2s_start(I2S_MIC_PORT);
    ESP_LOGI(TAG, "Escucha iniciada");
    
    return ESP_OK;
}

esp_err_t audio_stop_listening(void)
{
    if (!audio_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    current_voice_state = VOICE_IDLE;
    i2s_stop(I2S_MIC_PORT);
    ESP_LOGI(TAG, "Escucha detenida");
    
    return ESP_OK;
}

wake_word_result_t audio_check_wake_word(void)
{
    wake_word_result_t result = {0};
    
    if (audio_queue && xQueueReceive(audio_queue, &result, 0) == pdTRUE) {
        return result;
    }
    
    return result;
}

bool audio_wait_for_wake_word(uint32_t timeout_ms)
{
    if (!audio_initialized || !audio_queue) {
        return false;
    }
    
    current_voice_state = VOICE_LISTENING;
    
    wake_word_result_t result;
    TickType_t ticks = (timeout_ms == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    
    if (xQueueReceive(audio_queue, &result, ticks) == pdTRUE) {
        current_voice_state = VOICE_PROCESSING;
        return result.detected && result.confidence > WAKE_WORD_THRESHOLD;
    }
    
    current_voice_state = VOICE_IDLE;
    return false;
}

audio_segment_t* audio_capture(uint32_t duration_ms)
{
    if (!audio_initialized) {
        return NULL;
    }
    
    uint32_t sample_count = (I2S_MIC_SAMPLE_RATE * duration_ms) / 1000;
    audio_segment_t *segment = malloc(sizeof(audio_segment_t));
    if (!segment) {
        return NULL;
    }
    
    segment->samples = malloc(sample_count * sizeof(int16_t));
    if (!segment->samples) {
        free(segment);
        return NULL;
    }
    
    segment->sample_count = sample_count;
    segment->sample_rate = I2S_MIC_SAMPLE_RATE;
    segment->timestamp = esp_timer_get_time();
    
    size_t bytes_read;
    esp_err_t err = i2s_read(I2S_MIC_PORT, segment->samples, 
                              sample_count * sizeof(int16_t), 
                              &bytes_read, pdMS_TO_TICKS(duration_ms + 100));
    
    if (err != ESP_OK || bytes_read < sample_count * sizeof(int16_t)) {
        free(segment->samples);
        free(segment);
        return NULL;
    }
    
    capture_count++;
    return segment;
}

audio_segment_t* audio_capture_until_silence(uint32_t max_duration_ms, uint16_t silence_threshold)
{
    // TODO: Implementar captura hasta silencio
    ESP_LOGW(TAG, "audio_capture_until_silence no implementado");
    return audio_capture(max_duration_ms);
}

void audio_free_segment(audio_segment_t *segment)
{
    if (segment) {
        if (segment->samples) {
            free(segment->samples);
        }
        free(segment);
    }
}

bool audio_sound_detected(void)
{
    int16_t buffer[64];
    size_t bytes_read;
    
    esp_err_t err = i2s_read(I2S_MIC_PORT, buffer, sizeof(buffer), &bytes_read, 0);
    if (err != ESP_OK || bytes_read == 0) {
        return false;
    }
    
    int samples = bytes_read / sizeof(int16_t);
    int32_t sum_sq = 0;
    for (int i = 0; i < samples; i++) {
        sum_sq += buffer[i] * buffer[i];
    }
    float rms = sqrt(sum_sq / samples);
    
    return (rms > SILENCE_THRESHOLD);
}

/* ==================================================================
 * FUNCIONES DE SALIDA (ALTAVOZ)
 * ================================================================== */

esp_err_t audio_play_file(const char *filename)
{
    // TODO: Implementar reproducción desde microSD
    ESP_LOGW(TAG, "audio_play_file no implementado");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t audio_play_message(uint8_t message_id)
{
    if (!audio_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // TODO: Cargar mensaje desde microSD
    // Por ahora, generar un tono simple
    uint8_t buffer[256];
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i * current_volume / 100;
    }
    
    size_t bytes_written;
    esp_err_t err = i2s_write(I2S_SPEAKER_PORT, buffer, sizeof(buffer), &bytes_written, portMAX_DELAY);
    
    if (err == ESP_OK) {
        play_count++;
        ESP_LOGI(TAG, "Mensaje %d reproducido", message_id);
    }
    
    return err;
}

esp_err_t audio_speak(const char *text)
{
    // TODO: Implementar síntesis de voz
    ESP_LOGW(TAG, "audio_speak no implementado para texto: %s", text);
    return ESP_ERR_NOT_SUPPORTED;
}

void audio_set_volume(uint8_t volume)
{
    if (volume > 100) volume = 100;
    current_volume = volume;
    // Ajustar ganancia del altavoz
    i2s_set_vol(I2S_SPEAKER_PORT, volume);
}

uint8_t audio_get_volume(void)
{
    return current_volume;
}

void audio_stop_playback(void)
{
    i2s_zero_dma_buffer(I2S_SPEAKER_PORT);
}

bool audio_is_playing(void)
{
    // TODO: Verificar si hay reproducción activa
    return false;
}

/* ==================================================================
 * FUNCIONES DE RECONOCIMIENTO DE VOZ
 * ================================================================== */

esp_err_t audio_recognize_command(audio_segment_t *segment, char *command, size_t max_len)
{
    // TODO: Implementar reconocimiento de comandos
    // Por ahora, simular reconocimiento
    if (segment && command && max_len > 0) {
        strcpy(command, "leer");
        return ESP_OK;
    }
    return ESP_ERR_NOT_SUPPORTED;
}

voice_state_t audio_get_voice_state(void)
{
    return current_voice_state;
}

const char* audio_get_command_name(uint8_t command_id)
{
    static const char *commands[] = {
        "leer", "receta", "glucosa", "quién es", 
        "qué estaba diciendo", "ayuda", "volumen"
    };
    
    if (command_id < sizeof(commands) / sizeof(commands[0])) {
        return commands[command_id];
    }
    return "desconocido";
}

/* ==================================================================
 * FUNCIONES DE PROCESAMIENTO
 * ================================================================== */

float audio_calculate_rms(audio_segment_t *segment)
{
    if (!segment || !segment->samples || segment->sample_count == 0) {
        return 0;
    }
    
    int64_t sum_sq = 0;
    for (size_t i = 0; i < segment->sample_count; i++) {
        sum_sq += segment->samples[i] * segment->samples[i];
    }
    
    return sqrt(sum_sq / segment->sample_count);
}

bool audio_is_silence(audio_segment_t *segment, float threshold)
{
    float rms = audio_calculate_rms(segment);
    return rms < threshold;
}

int audio_extract_features(audio_segment_t *segment, float *features, int max_features)
{
    // TODO: Implementar extracción de características MFCC o similares
    return 0;
}

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

bool audio_is_initialized(void)
{
    return audio_initialized;
}

uint32_t audio_get_play_count(void)
{
    return play_count;
}

uint32_t audio_get_capture_count(void)
{
    return capture_count;
}
