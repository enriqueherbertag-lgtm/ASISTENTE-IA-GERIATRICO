/**
 * @file camera_driver.h
 * @brief Driver para cámara OV2640
 * 
 * Este módulo gestiona la inicialización, captura y procesamiento
 * básico de imágenes de la cámara OV2640.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef CAMERA_DRIVER_H
#define CAMERA_DRIVER_H

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
 * @brief Resolución de la cámara
 */
typedef enum {
    CAM_RES_QQVGA = 0,   // 160x120
    CAM_RES_QVGA,        // 320x240
    CAM_RES_VGA,         // 640x480
    CAM_RES_SVGA,        // 800x600
    CAM_RES_HD,          // 1280x720
    CAM_RES_SXGA         // 1600x1200
} camera_resolution_t;

/**
 * @brief Formato de imagen
 */
typedef enum {
    CAM_FORMAT_JPEG = 0,
    CAM_FORMAT_RGB565,
    CAM_FORMAT_GRAYSCALE
} camera_format_t;

/**
 * @brief Estructura de frame capturado
 */
typedef struct {
    uint8_t *data;           // Datos de la imagen
    size_t len;              // Longitud de los datos
    uint16_t width;          // Ancho en píxeles
    uint16_t height;         // Alto en píxeles
    camera_format_t format;   // Formato
    uint64_t timestamp;       // Timestamp de captura
} camera_frame_t;

/**
 * @brief Configuración de la cámara
 */
typedef struct {
    camera_resolution_t resolution;
    camera_format_t format;
    uint8_t jpeg_quality;     // 0-63 (menor = mejor calidad)
    bool auto_exposure;
    bool auto_white_balance;
    uint8_t brightness;       // -2 a 2
    uint8_t contrast;         // -2 a 2
    uint8_t saturation;       // -2 a 2
} camera_config_t;

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa la cámara con configuración por defecto
 * @return ESP_OK en éxito, otro en error
 */
esp_err_t camera_init(void);

/**
 * @brief Inicializa la cámara con configuración personalizada
 * @param config Configuración deseada
 * @return ESP_OK en éxito, otro en error
 */
esp_err_t camera_init_with_config(camera_config_t *config);

/**
 * @brief Captura un frame
 * @param frame Puntero donde se almacenará el frame (debe liberarse con camera_free_frame)
 * @return ESP_OK en éxito, otro en error
 */
esp_err_t camera_capture_frame(camera_frame_t **frame);

/**
 * @brief Libera un frame previamente capturado
 * @param frame Frame a liberar
 */
void camera_free_frame(camera_frame_t *frame);

/**
 * @brief Activa el modo de bajo consumo de la cámara
 * @return ESP_OK en éxito
 */
esp_err_t camera_power_off(void);

/**
 * @brief Reactiva la cámara desde modo de bajo consumo
 * @return ESP_OK en éxito
 */
esp_err_t camera_power_on(void);

/* ==================================================================
 * FUNCIONES DE PROCESAMIENTO
 * ================================================================== */

/**
 * @brief Detecta y recorta texto en una imagen
 * @param frame Frame de entrada
 * @param regions Array donde se almacenarán las regiones de texto
 * @param max_regions Máximo número de regiones a detectar
 * @return Número de regiones detectadas
 */
int camera_detect_text_regions(camera_frame_t *frame, void *regions, int max_regions);

/**
 * @brief Realiza OCR en una región de la imagen
 * @param frame Frame de entrada
 * @param x, y, width, height Región a procesar
 * @param text Buffer para el texto extraído
 * @param max_len Tamaño del buffer
 * @return ESP_OK en éxito
 */
esp_err_t camera_ocr_region(camera_frame_t *frame, int x, int y, int width, int height, char *text, size_t max_len);

/**
 * @brief Captura y realiza OCR directamente
 * @param text Buffer para el texto extraído
 * @param max_len Tamaño del buffer
 * @return ESP_OK en éxito, ESP_ERR_NOT_FOUND si no se detecta texto
 */
esp_err_t camera_capture_and_ocr(char *text, size_t max_len);

/* ==================================================================
 * FUNCIONES DE RECONOCIMIENTO FACIAL
 * ================================================================== */

/**
 * @brief Extrae embedding facial de un frame
 * @param frame Frame con rostro
 * @param embedding Buffer de 128 bytes para el embedding
 * @return ESP_OK en éxito, ESP_ERR_NOT_FOUND si no se detecta rostro
 */
esp_err_t camera_extract_face_embedding(camera_frame_t *frame, uint8_t *embedding);

/**
 * @brief Compara dos embeddings faciales
 * @param emb1 Primer embedding
 * @param emb2 Segundo embedding
 * @return Similitud (0-1, mayor = más similar)
 */
float camera_compare_embeddings(uint8_t *emb1, uint8_t *emb2);

/**
 * @brief Detecta rostros en un frame
 * @param frame Frame de entrada
 * @param count Puntero donde se almacenará el número de rostros detectados
 * @return Array con coordenadas de rostros (debe liberarse)
 */
void* camera_detect_faces(camera_frame_t *frame, int *count);

/* ==================================================================
 * FUNCIONES DE CONFIGURACIÓN
 * ================================================================== */

/**
 * @brief Cambia la resolución de la cámara
 * @param resolution Nueva resolución
 * @return ESP_OK en éxito
 */
esp_err_t camera_set_resolution(camera_resolution_t resolution);

/**
 * @brief Cambia la calidad JPEG
 * @param quality 0-63 (menor = mejor calidad)
 * @return ESP_OK en éxito
 */
esp_err_t camera_set_jpeg_quality(uint8_t quality);

/**
 * @brief Ajusta el brillo
 * @param value -2 a 2
 * @return ESP_OK en éxito
 */
esp_err_t camera_set_brightness(int8_t value);

/**
 * @brief Ajusta el contraste
 * @param value -2 a 2
 * @return ESP_OK en éxito
 */
esp_err_t camera_set_contrast(int8_t value);

/**
 * @brief Obtiene la configuración actual
 * @return Configuración actual
 */
camera_config_t camera_get_config(void);

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

/**
 * @brief Verifica si la cámara está inicializada
 * @return true si está inicializada
 */
bool camera_is_initialized(void);

/**
 * @brief Verifica si la cámara está encendida
 * @return true si está encendida
 */
bool camera_is_powered(void);

/**
 * @brief Obtiene el contador de frames capturados
 * @return Número de frames capturados desde el inicio
 */
uint32_t camera_get_frame_count(void);

#ifdef __cplusplus
}
#endif

#endif /* CAMERA_DRIVER_H */
