/**
 * @file voice_recognition.h
 * @brief Algoritmo de reconocimiento de voz y comandos
 * 
 * Este módulo procesa audio capturado para detectar comandos
 * de voz y activar las funciones correspondientes.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef VOICE_RECOGNITION_H
#define VOICE_RECOGNITION_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "audio_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * IDENTIFICADORES DE COMANDOS
 * ================================================================== */

#define CMD_LEER             0x01
#define CMD_RECETA           0x02
#define CMD_GLUCOSA          0x03
#define CMD_QUIEN_ES         0x04
#define CMD_QUE_DECIA        0x05
#define CMD_AYUDA            0x06
#define CMD_VOLUMEN          0x07
#define CMD_FECHA            0x08
#define CMD_HORA             0x09
#define CMD_RECORDATORIO     0x0A

/* ==================================================================
 * TIPOS DE DATOS
 * ================================================================== */

/**
 * @brief Estructura de un comando de voz
 */
typedef struct {
    uint8_t id;                 // Identificador del comando
    const char *command_str;     // String del comando (ej: "leer")
    const char *description;     // Descripción para ayuda
    float confidence;            // Confianza de detección (0-1)
} voice_command_t;

/**
 * @brief Estadísticas de reconocimiento
 */
typedef struct {
    uint32_t total_commands;     // Total de comandos detectados
    uint32_t command_count[16];  // Contador por comando (máximo 16)
    float last_confidence;       // Confianza del último comando
} voice_stats_t;

/**
 * @brief Callback para notificación de comandos
 * @param command Comando detectado
 * @param arg Argumento definido por el usuario
 */
typedef void (*command_callback_t)(voice_command_t *command, void *arg);

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa el reconocimiento de voz
 * @return ESP_OK en éxito, otro en error
 */
esp_err_t voice_recognition_init(void);

/**
 * @brief Procesa un segmento de audio y reconoce comandos
 * @param segment Segmento de audio capturado
 * @param result Puntero donde almacenar el comando reconocido
 * @return ESP_OK si se reconoció un comando, ESP_ERR_NOT_FOUND si no
 */
esp_err_t voice_recognition_process(audio_segment_t *segment, voice_command_t *result);

/**
 * @brief Reconoce un comando a partir de texto
 * @param text Texto a reconocer
 * @param result Puntero donde almacenar el comando
 * @return ESP_OK si se reconoció, ESP_ERR_NOT_FOUND si no
 */
esp_err_t voice_recognition_recognize(const char *text, voice_command_t *result);

/* ==================================================================
 * FUNCIONES DE CONSULTA
 * ================================================================== */

/**
 * @brief Obtiene la lista de comandos disponibles
 * @param count Puntero donde almacenar el número de comandos
 * @return Array de comandos (estático, no liberar)
 */
const voice_command_t* voice_recognition_get_commands(int *count);

/**
 * @brief Convierte ID de comando a string
 * @param command_id ID del comando
 * @return String del comando
 */
const char* voice_recognition_command_to_string(uint8_t command_id);

/**
 * @brief Obtiene el último comando detectado
 * @return Puntero al último comando (estático)
 */
const voice_command_t* voice_recognition_get_last_command(void);

/**
 * @brief Obtiene la confianza del último reconocimiento
 * @return Confianza (0-1)
 */
float voice_recognition_get_confidence(void);

/**
 * @brief Obtiene el contador de un comando específico
 * @param command_id ID del comando
 * @return Número de veces detectado
 */
uint32_t voice_recognition_get_command_count(uint8_t command_id);

/* ==================================================================
 * FUNCIONES DE CONFIGURACIÓN
 * ================================================================== */

/**
 * @brief Configura callback para notificación de comandos
 * @param callback Función a llamar cuando se detecta un comando
 * @param arg Argumento para el callback
 */
void voice_recognition_set_callback(command_callback_t callback, void *arg);

/**
 * @brief Obtiene estadísticas de reconocimiento
 * @param stats Puntero donde almacenar las estadísticas
 */
void voice_recognition_get_stats(voice_stats_t *stats);

/**
 * @brief Reinicia las estadísticas de reconocimiento
 */
void voice_recognition_reset_stats(void);

#ifdef __cplusplus
}
#endif

#endif /* VOICE_RECOGNITION_H */
