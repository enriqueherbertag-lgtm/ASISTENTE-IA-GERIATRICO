/**
 * @file audio_driver.h
 * @brief Driver para audio (micrófono y altavoz)
 * 
 * Este módulo gestiona la captura de audio del micrófono
 * y la reproducción de voz a través del altavoz.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef AUDIO_DRIVER_H
#define AUDIO_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * TIPOS DE DATOS
 * ================================================================== */

/**
 * @brief Modos de energía del audio
 */
typedef enum {
    AUDIO_NORMAL = 0,      // Modo normal
    AUDIO_LOW_POWER,       // Modo bajo consumo (detección de palabra)
    AUDIO_SLEEP            // Modo sleep (mínimo consumo)
} audio_power_mode_t;

/**
 * @brief Estado de detección de voz
 */
typedef enum {
    VOICE_IDLE = 0,
    VOICE_LISTENING,
    VOICE_PROCESSING,
    VOICE_SPEAKING
} voice_state_t;

/**
 * @brief Resultado de detección de palabra de activación
 */
typedef struct {
    bool detected;          // true si se detectó
    uint32_t timestamp;     // Timestamp de la detección
    uint8_t confidence;      // Confianza (0-100)
} wake_word_result_t;

/**
 * @brief Segmento de audio capturado
 */
typedef struct {
    int16_t *samples;        // Muestras PCM (16 bits)
    size_t sample_count;      // Número de muestras
    uint32_t sample_rate;     // Tasa de muestreo (Hz)
    uint64_t timestamp;       // Timestamp de inicio
} audio_segment_t;

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa el sistema de audio
 * @return ESP_OK en éxito, otro en error
 */
esp_err_t audio_init(void);

/**
 * @brief Configura el modo de energía
 * @param mode Modo deseado
 * @return ESP_OK en éxito
 */
esp_err_t audio_set_power_mode(audio_power_mode_t mode);

/**
 * @brief Obtiene el modo de energía actual
 * @return Modo actual
 */
audio_power_mode_t audio_get_power_mode(void);

/* ==================================================================
 * FUNCIONES DE ENTRADA (MICRÓFONO)
 * ================================================================== */

/**
 * @brief Inicia la escucha continua de audio (bajo consumo)
 * @return ESP_OK en éxito
 */
esp_err_t audio_start_listening(void);

/**
 * @brief Detiene la escucha continua
 * @return ESP_OK en éxito
 */
esp_err_t audio_stop_listening(void);

/**
 * @brief Verifica si se detectó la palabra de activación
 * @return Resultado de la detección
 */
wake_word_result_t audio_check_wake_word(void);

/**
 * @brief Espera hasta detectar la palabra de activación
 * @param timeout_ms Timeout en milisegundos (0 = infinito)
 * @return true si se detectó
 */
bool audio_wait_for_wake_word(uint32_t timeout_ms);

/**
 * @brief Captura un segmento de audio
 * @param duration_ms Duración en milisegundos
 * @return Segmento de audio (debe liberarse con audio_free_segment)
 */
audio_segment_t* audio_capture(uint32_t duration_ms);

/**
 * @brief Captura audio hasta silencio
 * @param max_duration_ms Duración máxima
 * @param silence_threshold Umbral de silencio
 * @return Segmento de audio
 */
audio_segment_t* audio_capture_until_silence(uint32_t max_duration_ms, uint16_t silence_threshold);

/**
 * @brief Libera un segmento de audio previamente capturado
 * @param segment Segmento a liberar
 */
void audio_free_segment(audio_segment_t *segment);

/**
 * @brief Detecta si hay sonido ambiental
 * @return true si hay sonido
 */
bool audio_sound_detected(void);

/* ==================================================================
 * FUNCIONES DE SALIDA (ALTAVOZ)
 * ================================================================== */

/**
 * @brief Reproduce un archivo de audio desde la microSD
 * @param filename Nombre del archivo
 * @return ESP_OK en éxito
 */
esp_err_t audio_play_file(const char *filename);

/**
 * @brief Reproduce un mensaje de voz por su ID
 * @param message_id ID del mensaje (definido en config.h)
 * @return ESP_OK en éxito
 */
esp_err_t audio_play_message(uint8_t message_id);

/**
 * @brief Reproduce texto usando síntesis de voz (si está disponible)
 * @param text Texto a reproducir
 * @return ESP_OK en éxito
 */
esp_err_t audio_speak(const char *text);

/**
 * @brief Ajusta el volumen de reproducción
 * @param volume 0-100
 */
void audio_set_volume(uint8_t volume);

/**
 * @brief Obtiene el volumen actual
 * @return Volumen (0-100)
 */
uint8_t audio_get_volume(void);

/**
 * @brief Detiene la reproducción actual
 */
void audio_stop_playback(void);

/**
 * @brief Verifica si está reproduciendo
 * @return true si está reproduciendo
 */
bool audio_is_playing(void);

/* ==================================================================
 * FUNCIONES DE RECONOCIMIENTO DE VOZ
 * ================================================================== */

/**
 * @brief Reconoce un comando de voz
 * @param segment Segmento de audio capturado
 * @param command Buffer para el comando reconocido
 * @param max_len Tamaño del buffer
 * @return ESP_OK en éxito, ESP_ERR_NOT_FOUND si no se reconoce
 */
esp_err_t audio_recognize_command(audio_segment_t *segment, char *command, size_t max_len);

/**
 * @brief Obtiene el estado actual del sistema de voz
 * @return Estado actual
 */
voice_state_t audio_get_voice_state(void);

/**
 * @brief Obtiene el nombre del comando por su ID
 * @param command_id ID del comando
 * @return String con el nombre
 */
const char* audio_get_command_name(uint8_t command_id);

/* ==================================================================
 * FUNCIONES DE PROCESAMIENTO
 * ================================================================== */

/**
 * @brief Calcula el nivel RMS de un segmento de audio
 * @param segment Segmento de audio
 * @return Nivel RMS
 */
float audio_calculate_rms(audio_segment_t *segment);

/**
 * @brief Detecta si un segmento es silencio
 * @param segment Segmento de audio
 * @param threshold Umbral RMS
 * @return true si es silencio
 */
bool audio_is_silence(audio_segment_t *segment, float threshold);

/**
 * @brief Extrae características de audio para reconocimiento
 * @param segment Segmento de audio
 * @param features Buffer para características
 * @param max_features Número máximo de características
 * @return Número de características extraídas
 */
int audio_extract_features(audio_segment_t *segment, float *features, int max_features);

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

/**
 * @brief Verifica si el audio está inicializado
 * @return true si está inicializado
 */
bool audio_is_initialized(void);

/**
 * @brief Obtiene el contador de reproducciones
 * @return Número de reproducciones desde el inicio
 */
uint32_t audio_get_play_count(void);

/**
 * @brief Obtiene el contador de capturas
 * @return Número de capturas desde el inicio
 */
uint32_t audio_get_capture_count(void);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_DRIVER_H */
