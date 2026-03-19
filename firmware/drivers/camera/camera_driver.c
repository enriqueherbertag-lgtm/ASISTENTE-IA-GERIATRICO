/**
 * @file camera_driver.c
 * @brief Implementación del driver para cámara OV2640
 * 
 * Este archivo contiene la implementación de las funciones
 * declaradas en camera_driver.h
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
#include "esp_camera.h"
#include "camera_driver.h"
#include "config.h"

static const char *TAG = "CAMERA_DRV";

// Variables internas
static bool camera_initialized = false;
static bool camera_powered = false;
static uint32_t frame_counter = 0;
static camera_config_t current_config;
static camera_fb_t *last_fb = NULL;

/* ==================================================================
 * CONFIGURACIÓN POR DEFECTO
 * ================================================================== */

static const camera_config_t DEFAULT_CONFIG = {
    .resolution = CAM_RES_QVGA,
    .format = CAM_FORMAT_JPEG,
    .jpeg_quality = 12,
    .auto_exposure = true,
    .auto_white_balance = true,
    .brightness = 0,
    .contrast = 0,
    .saturation = 0
};

/* ==================================================================
 * MAPEO DE RESOLUCIONES A FORMATOS ESPRESSIF
 * ================================================================== */

static framesize_t resolution_to_framesize(camera_resolution_t res)
{
    switch (res) {
        case CAM_RES_QQVGA: return FRAMESIZE_QQVGA;
        case CAM_RES_QVGA:  return FRAMESIZE_QVGA;
        case CAM_RES_VGA:   return FRAMESIZE_VGA;
        case CAM_RES_SVGA:  return FRAMESIZE_SVGA;
        case CAM_RES_HD:    return FRAMESIZE_HD;
        case CAM_RES_SXGA:  return FRAMESIZE_SXGA;
        default:            return FRAMESIZE_QVGA;
    }
}

static pixformat_t format_to_pixformat(camera_format_t fmt)
{
    switch (fmt) {
        case CAM_FORMAT_JPEG:      return PIXFORMAT_JPEG;
        case CAM_FORMAT_RGB565:    return PIXFORMAT_RGB565;
        case CAM_FORMAT_GRAYSCALE: return PIXFORMAT_GRAYSCALE;
        default:                   return PIXFORMAT_JPEG;
    }
}

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static esp_err_t camera_apply_config(camera_config_t *config)
{
    sensor_t *s = esp_camera_sensor_get();
    if (!s) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Configurar resolución
    s->set_framesize(s, resolution_to_framesize(config->resolution));
    
    // Configurar exposición
    s->set_exposure_ctrl(s, config->auto_exposure ? 1 : 0);
    
    // Configurar balance de blancos
    s->set_whitebal(s, config->auto_white_balance ? 1 : 0);
    
    // Configurar brillo
    s->set_brightness(s, config->brightness);
    
    // Configurar contraste
    s->set_contrast(s, config->contrast);
    
    // Configurar saturación
    s->set_saturation(s, config->saturation);
    
    return ESP_OK;
}

static esp_err_t camera_init_hw(void)
{
    // Configuración de pines (definidos en config.h)
    camera_config_t esp_config = {
        .pin_pwdn  = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sscb_sda = CAM_PIN_SIOD,
        .pin_sscb_scl = CAM_PIN_SIOC,
        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,
        
        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = format_to_pixformat(current_config.format),
        .frame_size = resolution_to_framesize(current_config.resolution),
        .jpeg_quality = current_config.jpeg_quality,
        .fb_count = CAMERA_FB_COUNT
    };
    
    esp_err_t err = esp_camera_init(&esp_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error inicializando cámara: 0x%x", err);
        return err;
    }
    
    return ESP_OK;
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

esp_err_t camera_init(void)
{
    return camera_init_with_config((camera_config_t*)&DEFAULT_CONFIG);
}

esp_err_t camera_init_with_config(camera_config_t *config)
{
    if (camera_initialized) {
        ESP_LOGW(TAG, "Cámara ya inicializada");
        return ESP_OK;
    }
    
    // Copiar configuración
    memcpy(&current_config, config, sizeof(camera_config_t));
    
    // Inicializar hardware
    esp_err_t err = camera_init_hw();
    if (err != ESP_OK) {
        return err;
    }
    
    // Aplicar configuración adicional
    err = camera_apply_config(config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error aplicando configuración");
        return err;
    }
    
    camera_initialized = true;
    camera_powered = true;
    frame_counter = 0;
    
    ESP_LOGI(TAG, "Cámara inicializada correctamente");
    return ESP_OK;
}

esp_err_t camera_capture_frame(camera_frame_t **out_frame)
{
    if (!camera_initialized || !camera_powered) {
        ESP_LOGE(TAG, "Cámara no inicializada o apagada");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Capturar frame
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Error capturando frame");
        return ESP_FAIL;
    }
    
    // Asignar memoria para el frame de salida
    camera_frame_t *frame = (camera_frame_t*)malloc(sizeof(camera_frame_t));
    if (!frame) {
        esp_camera_fb_return(fb);
        return ESP_ERR_NO_MEM;
    }
    
    // Copiar datos (si es JPEG, usamos los datos directamente)
    if (fb->format == PIXFORMAT_JPEG) {
        frame->data = (uint8_t*)malloc(fb->len);
        if (!frame->data) {
            free(frame);
            esp_camera_fb_return(fb);
            return ESP_ERR_NO_MEM;
        }
        memcpy(frame->data, fb->buf, fb->len);
        frame->len = fb->len;
    } else {
        // Para otros formatos, implementar conversión si es necesario
        // Por simplicidad, usamos el mismo buffer
        frame->data = (uint8_t*)malloc(fb->len);
        if (!frame->data) {
            free(frame);
            esp_camera_fb_return(fb);
            return ESP_ERR_NO_MEM;
        }
        memcpy(frame->data, fb->buf, fb->len);
        frame->len = fb->len;
    }
    
    frame->width = fb->width;
    frame->height = fb->height;
    frame->format = current_config.format;
    frame->timestamp = esp_timer_get_time();
    
    frame_counter++;
    
    // Liberar buffer de la cámara
    esp_camera_fb_return(fb);
    
    *out_frame = frame;
    return ESP_OK;
}

void camera_free_frame(camera_frame_t *frame)
{
    if (frame) {
        if (frame->data) {
            free(frame->data);
        }
        free(frame);
    }
}

esp_err_t camera_power_off(void)
{
    if (!camera_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Poner en modo sleep
    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        s->set_power_down(s, 1);
    }
    
    camera_powered = false;
    ESP_LOGI(TAG, "Cámara apagada");
    return ESP_OK;
}

esp_err_t camera_power_on(void)
{
    if (!camera_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Salir de modo sleep
    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        s->set_power_down(s, 0);
    }
    
    // Reaplicar configuración
    camera_apply_config(&current_config);
    
    camera_powered = true;
    ESP_LOGI(TAG, "Cámara encendida");
    return ESP_OK;
}

/* ==================================================================
 * FUNCIONES DE PROCESAMIENTO (STUBS - REQUIEREN INTEGRACIÓN CON LIBRERÍAS EXTERNAS)
 * ================================================================== */

int camera_detect_text_regions(camera_frame_t *frame, void *regions, int max_regions)
{
    // TODO: Implementar detección de texto
    // Por ahora, stub que simula detección
    ESP_LOGW(TAG, "camera_detect_text_regions no implementado");
    return 0;
}

esp_err_t camera_ocr_region(camera_frame_t *frame, int x, int y, int width, int height, char *text, size_t max_len)
{
    // TODO: Implementar OCR
    // Por ahora, stub
    ESP_LOGW(TAG, "camera_ocr_region no implementado");
    if (max_len > 0) {
        text[0] = '\0';
    }
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t camera_capture_and_ocr(char *text, size_t max_len)
{
    camera_frame_t *frame = NULL;
    esp_err_t err = camera_capture_frame(&frame);
    if (err != ESP_OK) {
        return err;
    }
    
    // TODO: Procesar imagen para OCR
    err = camera_ocr_region(frame, 0, 0, frame->width, frame->height, text, max_len);
    
    camera_free_frame(frame);
    return err;
}

esp_err_t camera_extract_face_embedding(camera_frame_t *frame, uint8_t *embedding)
{
    // TODO: Implementar extracción de embeddings faciales
    ESP_LOGW(TAG, "camera_extract_face_embedding no implementado");
    return ESP_ERR_NOT_SUPPORTED;
}

float camera_compare_embeddings(uint8_t *emb1, uint8_t *emb2)
{
    // TODO: Implementar comparación
    ESP_LOGW(TAG, "camera_compare_embeddings no implementado");
    return 0.0f;
}

void* camera_detect_faces(camera_frame_t *frame, int *count)
{
    // TODO: Implementar detección de rostros
    ESP_LOGW(TAG, "camera_detect_faces no implementado");
    *count = 0;
    return NULL;
}

/* ==================================================================
 * FUNCIONES DE CONFIGURACIÓN
 * ================================================================== */

esp_err_t camera_set_resolution(camera_resolution_t resolution)
{
    if (!camera_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    current_config.resolution = resolution;
    
    sensor_t *s = esp_camera_sensor_get();
    if (!s) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t err = s->set_framesize(s, resolution_to_framesize(resolution));
    if (err != 0) {
        ESP_LOGE(TAG, "Error cambiando resolución");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t camera_set_jpeg_quality(uint8_t quality)
{
    if (!camera_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (quality > 63) quality = 63;
    current_config.jpeg_quality = quality;
    
    sensor_t *s = esp_camera_sensor_get();
    if (!s) {
        return ESP_ERR_INVALID_STATE;
    }
    
    s->set_quality(s, quality);
    
    return ESP_OK;
}

esp_err_t camera_set_brightness(int8_t value)
{
    if (!camera_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (value < -2) value = -2;
    if (value > 2) value = 2;
    current_config.brightness = value;
    
    sensor_t *s = esp_camera_sensor_get();
    if (!s) {
        return ESP_ERR_INVALID_STATE;
    }
    
    s->set_brightness(s, value);
    
    return ESP_OK;
}

esp_err_t camera_set_contrast(int8_t value)
{
    if (!camera_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (value < -2) value = -2;
    if (value > 2) value = 2;
    current_config.contrast = value;
    
    sensor_t *s = esp_camera_sensor_get();
    if (!s) {
        return ESP_ERR_INVALID_STATE;
    }
    
    s->set_contrast(s, value);
    
    return ESP_OK;
}

camera_config_t camera_get_config(void)
{
    return current_config;
}

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

bool camera_is_initialized(void)
{
    return camera_initialized;
}

bool camera_is_powered(void)
{
    return camera_powered;
}

uint32_t camera_get_frame_count(void)
{
    return frame_counter;
}
