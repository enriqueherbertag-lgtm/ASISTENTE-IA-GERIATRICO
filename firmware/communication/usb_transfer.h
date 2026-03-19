/**
 * @file usb_transfer.h
 * @brief Gestión de transferencia de datos por USB
 * 
 * Este módulo maneja la comunicación por USB para carga de
 * archivos de voz, configuración inicial y actualizaciones.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef USB_TRANSFER_H
#define USB_TRANSFER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * CONSTANTES
 * ================================================================== */

#define USB_MAX_FILENAME_LEN    64
#define USB_TRANSFER_BLOCK_SIZE 4096
#define USB_TIMEOUT_MS          5000

/* ==================================================================
 * TIPOS DE DATOS
 * ================================================================== */

/**
 * @brief Tipo de operación de transferencia
 */
typedef enum {
    USB_OP_NONE = 0,
    USB_OP_UPLOAD,               // Subir archivo al dispositivo
    USB_OP_DOWNLOAD,             // Descargar archivo del dispositivo
    USB_OP_LIST,                 // Listar archivos
    USB_OP_DELETE,               // Eliminar archivo
    USB_OP_CONFIG                // Configuración
} usb_operation_t;

/**
 * @brief Estado de la conexión USB
 */
typedef enum {
    USB_STATE_DISCONNECTED = 0,
    USB_STATE_CONNECTED,
    USB_STATE_TRANSFERRING,
    USB_STATE_ERROR
} usb_state_t;

/**
 * @brief Información de un archivo
 */
typedef struct {
    char name[USB_MAX_FILENAME_LEN];
    uint32_t size;
    uint32_t modified;
    bool is_directory;
} usb_file_info_t;

/**
 * @brief Estadísticas de transferencia
 */
typedef struct {
    uint32_t total_bytes_sent;
    uint32_t total_bytes_received;
    uint32_t transfer_count;
    uint32_t error_count;
    usb_state_t current_state;
} usb_stats_t;

/**
 * @brief Callbacks para eventos USB
 */
typedef struct {
    void (*on_connected)(void);
    void (*on_disconnected)(void);
    void (*on_transfer_start)(usb_operation_t op, const char *filename, uint32_t size);
    void (*on_transfer_progress)(uint32_t transferred, uint32_t total);
    void (*on_transfer_complete)(usb_operation_t op, const char *filename, esp_err_t result);
} usb_callbacks_t;

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa el sistema de transferencia USB
 * @return ESP_OK en éxito
 */
esp_err_t usb_transfer_init(void);

/**
 * @brief Verifica si hay un dispositivo conectado por USB
 * @return true si hay conexión
 */
bool usb_transfer_is_connected(void);

/**
 * @brief Obtiene el estado actual de la conexión USB
 * @return Estado actual
 */
usb_state_t usb_transfer_get_state(void);

/* ==================================================================
 * FUNCIONES DE TRANSFERENCIA
 * ================================================================== */

/**
 * @brief Sube un archivo al dispositivo
 * @param source_path Ruta en el PC
 * @param dest_path Ruta en el dispositivo (microSD)
 * @param timeout_ms Timeout en milisegundos
 * @return ESP_OK en éxito
 */
esp_err_t usb_transfer_upload(const char *source_path, const char *dest_path, uint32_t timeout_ms);

/**
 * @brief Descarga un archivo del dispositivo
 * @param source_path Ruta en el dispositivo (microSD)
 * @param dest_path Ruta en el PC
 * @param timeout_ms Timeout en milisegundos
 * @return ESP_OK en éxito
 */
esp_err_t usb_transfer_download(const char *source_path, const char *dest_path, uint32_t timeout_ms);

/**
 * @brief Lista archivos en un directorio
 * @param path Ruta en el dispositivo
 * @param files Buffer para almacenar la lista de archivos
 * @param max_count Máximo número de archivos
 * @return Número de archivos encontrados
 */
int usb_transfer_list(const char *path, usb_file_info_t *files, int max_count);

/**
 * @brief Elimina un archivo
 * @param path Ruta del archivo a eliminar
 * @return ESP_OK en éxito
 */
esp_err_t usb_transfer_delete(const char *path);

/* ==================================================================
 * FUNCIONES DE CONFIGURACIÓN
 * ================================================================== */

/**
 * @brief Configura el dispositivo para ser detectado por el PC
 * @param vid Vendor ID
 * @param pid Product ID
 * @param manufacturer Nombre del fabricante
 * @param product Nombre del producto
 * @param serial Número de serie
 * @return ESP_OK en éxito
 */
esp_err_t usb_transfer_configure_ids(uint16_t vid, uint16_t pid, 
                                      const char *manufacturer,
                                      const char *product,
                                      const char *serial);

/**
 * @brief Configura callbacks para eventos USB
 * @param callbacks Puntero a estructura de callbacks
 */
void usb_transfer_set_callbacks(usb_callbacks_t *callbacks);

/**
 * @brief Habilita o deshabilita el modo de almacenamiento masivo
 * @param enable true para habilitar
 * @return ESP_OK en éxito
 */
esp_err_t usb_transfer_enable_mass_storage(bool enable);

/**
 * @brief Habilita o deshabilita el modo de comunicación serie (CDC)
 * @param enable true para habilitar
 * @return ESP_OK en éxito
 */
esp_err_t usb_transfer_enable_serial(bool enable);

/* ==================================================================
 * FUNCIONES DE TRANSFERENCIA DE VOZ
 * ================================================================== */

/**
 * @brief Carga archivos de voz desde el PC
 * @param folder_path Carpeta en el PC con los archivos de voz
 * @return ESP_OK en éxito
 */
esp_err_t usb_transfer_upload_voice_files(const char *folder_path);

/**
 * @brief Descarga grabaciones de voz del usuario
 * @param folder_path Carpeta en el PC donde guardar
 * @return ESP_OK en éxito
 */
esp_err_t usb_transfer_download_recordings(const char *folder_path);

/**
 * @brief Carga configuración inicial desde el PC
 * @param config_path Ruta del archivo de configuración
 * @return ESP_OK en éxito
 */
esp_err_t usb_transfer_upload_config(const char *config_path);

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

/**
 * @brief Obtiene estadísticas de transferencia
 * @param stats Puntero donde almacenar las estadísticas
 */
void usb_transfer_get_stats(usb_stats_t *stats);

/**
 * @brief Cancela la transferencia en curso
 * @return ESP_OK en éxito
 */
esp_err_t usb_transfer_cancel(void);

/**
 * @brief Obtiene el progreso de la transferencia actual
 * @param transferred Puntero para bytes transferidos
 * @param total Puntero para total de bytes
 * @return true si hay transferencia activa
 */
bool usb_transfer_get_progress(uint32_t *transferred, uint32_t *total);

#ifdef __cplusplus
}
#endif

#endif /* USB_TRANSFER_H */
