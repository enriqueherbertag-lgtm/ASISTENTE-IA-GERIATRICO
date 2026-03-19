/**
 * @file usb_transfer.c
 * @brief Implementación de transferencia de datos por USB
 * 
 * Este archivo contiene la implementación de las funciones
 * declaradas en usb_transfer.h
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "class/cdc/cdc_device.h"
#include "class/msc/msc_device.h"
#include "usb_transfer.h"
#include "config.h"

static const char *TAG = "USB_TRANSFER";

// Configuración USB por defecto
#define USB_VID              0x303A  // Espressif
#define USB_PID              0x4001
#define USB_MANUFACTURER     "Mackiber Labs"
#define USB_PRODUCT          "Asistente IA Geriatrico"
#define USB_SERIAL           "AIG001"

// Variables internas
static usb_state_t current_state = USB_STATE_DISCONNECTED;
static usb_stats_t stats = {0};
static usb_callbacks_t user_callbacks = {0};

static bool mass_storage_enabled = false;
static bool serial_enabled = false;

static uint32_t current_transfer_total = 0;
static uint32_t current_transfer_progress = 0;
static bool transfer_active = false;
static usb_operation_t current_operation = USB_OP_NONE;
static char current_filename[USB_MAX_FILENAME_LEN] = {0};

// Buffer para transferencias
static uint8_t transfer_buffer[USB_TRANSFER_BLOCK_SIZE];

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static void update_state(usb_state_t new_state)
{
    if (current_state != new_state) {
        usb_state_t old = current_state;
        current_state = new_state;
        
        ESP_LOGI(TAG, "Estado USB: %d -> %d", old, new_state);
        
        if (new_state == USB_STATE_CONNECTED && user_callbacks.on_connected) {
            user_callbacks.on_connected();
        } else if (new_state == USB_STATE_DISCONNECTED && user_callbacks.on_disconnected) {
            user_callbacks.on_disconnected();
        }
    }
}

static void update_progress(uint32_t transferred, uint32_t total)
{
    current_transfer_progress = transferred;
    current_transfer_total = total;
    
    if (user_callbacks.on_transfer_progress) {
        user_callbacks.on_transfer_progress(transferred, total);
    }
    
    ESP_LOGD(TAG, "Progreso: %lu/%lu bytes", transferred, total);
}

static void complete_transfer(esp_err_t result)
{
    transfer_active = false;
    
    if (user_callbacks.on_transfer_complete) {
        user_callbacks.on_transfer_complete(current_operation, 
                                             current_filename, result);
    }
    
    if (result == ESP_OK) {
        if (current_operation == USB_OP_UPLOAD) {
            stats.total_bytes_received += current_transfer_total;
        } else if (current_operation == USB_OP_DOWNLOAD) {
            stats.total_bytes_sent += current_transfer_total;
        }
        stats.transfer_count++;
    } else {
        stats.error_count++;
    }
    
    current_operation = USB_OP_NONE;
    current_transfer_total = 0;
    current_transfer_progress = 0;
    current_filename[0] = '\0';
}

static esp_err_t copy_file(FILE *src, FILE *dst, uint32_t size)
{
    uint32_t remaining = size;
    uint32_t block_size;
    
    while (remaining > 0) {
        block_size = (remaining > USB_TRANSFER_BLOCK_SIZE) ? 
                      USB_TRANSFER_BLOCK_SIZE : remaining;
        
        size_t read = fread(transfer_buffer, 1, block_size, src);
        if (read != block_size) {
            return ESP_FAIL;
        }
        
        size_t written = fwrite(transfer_buffer, 1, block_size, dst);
        if (written != block_size) {
            return ESP_FAIL;
        }
        
        remaining -= block_size;
        update_progress(size - remaining, size);
        
        if (!transfer_active) {
            return ESP_ERR_INVALID_STATE;  // Cancelado
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    return ESP_OK;
}

/* ==================================================================
 * CALLBACKS DE TINYUSB
 * ================================================================== */

static void tusb_device_state_callback(int state)
{
    if (state == TUSB_DEVICE_STATE_SUSPENDED || 
        state == TUSB_DEVICE_STATE_NOT_CONFIGURED) {
        update_state(USB_STATE_DISCONNECTED);
    } else {
        update_state(USB_STATE_CONNECTED);
    }
}

static int32_t tud_msc_read_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
    // TODO: Implementar lectura de MSC
    return 0;
}

static bool tud_msc_start_read_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint32_t len)
{
    // TODO: Implementar inicio de lectura
    return true;
}

static int32_t tud_msc_write_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    // TODO: Implementar escritura MSC
    return 0;
}

static void tud_cdc_rx_cb(uint8_t itf)
{
    // Datos recibidos por CDC (serie)
    uint8_t buf[64];
    uint32_t count = tud_cdc_read(buf, sizeof(buf));
    
    ESP_LOGI(TAG, "Recibidos %lu bytes por CDC", count);
    
    // TODO: Procesar comandos por CDC
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

esp_err_t usb_transfer_init(void)
{
    ESP_LOGI(TAG, "Inicializando sistema USB");
    
    // Inicializar TinyUSB
    tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };
    
    esp_err_t ret = tinyusb_driver_install(&tusb_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error instalando TinyUSB: %d", ret);
        return ret;
    }
    
    // Registrar callback de estado
    tusb_device_state_register_callback(tusb_device_state_callback);
    
    // Configurar IDs por defecto
    usb_transfer_configure_ids(USB_VID, USB_PID, 
                                 USB_MANUFACTURER, 
                                 USB_PRODUCT, 
                                 USB_SERIAL);
    
    memset(&stats, 0, sizeof(usb_stats_t));
    current_state = USB_STATE_DISCONNECTED;
    
    ESP_LOGI(TAG, "Sistema USB inicializado");
    
    return ESP_OK;
}

bool usb_transfer_is_connected(void)
{
    return (current_state == USB_STATE_CONNECTED);
}

usb_state_t usb_transfer_get_state(void)
{
    return current_state;
}

esp_err_t usb_transfer_upload(const char *source_path, const char *dest_path, uint32_t timeout_ms)
{
    if (!source_path || !dest_path) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (transfer_active) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Subiendo %s -> %s", source_path, dest_path);
    
    FILE *src = fopen(source_path, "rb");
    if (!src) {
        ESP_LOGE(TAG, "Error abriendo archivo origen");
        return ESP_ERR_NOT_FOUND;
    }
    
    FILE *dst = fopen(dest_path, "wb");
    if (!dst) {
        fclose(src);
        ESP_LOGE(TAG, "Error creando archivo destino");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Obtener tamaño
    fseek(src, 0, SEEK_END);
    uint32_t size = ftell(src);
    fseek(src, 0, SEEK_SET);
    
    transfer_active = true;
    current_operation = USB_OP_UPLOAD;
    strncpy(current_filename, source_path, USB_MAX_FILENAME_LEN - 1);
    
    if (user_callbacks.on_transfer_start) {
        user_callbacks.on_transfer_start(USB_OP_UPLOAD, source_path, size);
    }
    
    esp_err_t ret = copy_file(src, dst, size);
    
    fclose(src);
    fclose(dst);
    
    complete_transfer(ret);
    
    return ret;
}

esp_err_t usb_transfer_download(const char *source_path, const char *dest_path, uint32_t timeout_ms)
{
    if (!source_path || !dest_path) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (transfer_active) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Descargando %s -> %s", source_path, dest_path);
    
    FILE *src = fopen(source_path, "rb");
    if (!src) {
        ESP_LOGE(TAG, "Error abriendo archivo origen");
        return ESP_ERR_NOT_FOUND;
    }
    
    FILE *dst = fopen(dest_path, "wb");
    if (!dst) {
        fclose(src);
        ESP_LOGE(TAG, "Error creando archivo destino");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Obtener tamaño
    fseek(src, 0, SEEK_END);
    uint32_t size = ftell(src);
    fseek(src, 0, SEEK_SET);
    
    transfer_active = true;
    current_operation = USB_OP_DOWNLOAD;
    strncpy(current_filename, source_path, USB_MAX_FILENAME_LEN - 1);
    
    if (user_callbacks.on_transfer_start) {
        user_callbacks.on_transfer_start(USB_OP_DOWNLOAD, source_path, size);
    }
    
    esp_err_t ret = copy_file(src, dst, size);
    
    fclose(src);
    fclose(dst);
    
    complete_transfer(ret);
    
    return ret;
}

int usb_transfer_list(const char *path, usb_file_info_t *files, int max_count)
{
    if (!path || !files || max_count <= 0) {
        return 0;
    }
    
    DIR *dir = opendir(path);
    if (!dir) {
        return 0;
    }
    
    int count = 0;
    struct dirent *entry;
    struct stat st;
    char fullpath[256];
    
    while ((entry = readdir(dir)) != NULL && count < max_count) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        
        if (stat(fullpath, &st) == 0) {
            strncpy(files[count].name, entry->d_name, USB_MAX_FILENAME_LEN - 1);
            files[count].size = st.st_size;
            files[count].modified = st.st_mtime;
            files[count].is_directory = (entry->d_type == DT_DIR);
            count++;
        }
    }
    
    closedir(dir);
    return count;
}

esp_err_t usb_transfer_delete(const char *path)
{
    if (!path) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (remove(path) == 0) {
        ESP_LOGI(TAG, "Archivo eliminado: %s", path);
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "Error eliminando archivo: %s", path);
        return ESP_FAIL;
    }
}

esp_err_t usb_transfer_configure_ids(uint16_t vid, uint16_t pid, 
                                      const char *manufacturer,
                                      const char *product,
                                      const char *serial)
{
    // Configurar descriptores USB
    tinyusb_config_t cfg = {
        .vid = vid,
        .pid = pid,
        .product_name = product,
        .manufacturer_name = manufacturer,
        .serial_number = serial,
    };
    
    return tinyusb_set_device_config(&cfg);
}

void usb_transfer_set_callbacks(usb_callbacks_t *callbacks)
{
    if (callbacks) {
        user_callbacks = *callbacks;
    }
}

esp_err_t usb_transfer_enable_mass_storage(bool enable)
{
    if (enable == mass_storage_enabled) {
        return ESP_OK;
    }
    
    if (enable) {
        // Configurar MSC con la microSD
        const tinyusb_msc_config_t msc_cfg = {
            .lun_count = 1,
            .read_cb = tud_msc_read_cb,
            .write_cb = tud_msc_write_cb,
            .start_read_cb = tud_msc_start_read_cb,
        };
        
        esp_err_t ret = tinyusb_msc_register(&msc_cfg);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error registrando MSC");
            return ret;
        }
    }
    
    mass_storage_enabled = enable;
    return ESP_OK;
}

esp_err_t usb_transfer_enable_serial(bool enable)
{
    if (enable == serial_enabled) {
        return ESP_OK;
    }
    
    if (enable) {
        tinyusb_cdcacm_config_t acm_cfg = {
            .usb_dev = TINYUSB_USBDEV_0,
            .cdc_port = TINYUSB_CDC_ACM_0,
            .rx_unread_buf_sz = 64,
            .callback_rx = &tud_cdc_rx_cb,
        };
        
        esp_err_t ret = tinyusb_cdcacm_register(&acm_cfg);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error registrando CDC");
            return ret;
        }
    }
    
    serial_enabled = enable;
    return ESP_OK;
}

esp_err_t usb_transfer_upload_voice_files(const char *folder_path)
{
    // TODO: Implementar carga masiva de archivos de voz
    ESP_LOGW(TAG, "usb_transfer_upload_voice_files no implementado");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t usb_transfer_download_recordings(const char *folder_path)
{
    // TODO: Implementar descarga de grabaciones
    ESP_LOGW(TAG, "usb_transfer_download_recordings no implementado");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t usb_transfer_upload_config(const char *config_path)
{
    if (!config_path) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return usb_transfer_upload(config_path, "/sdcard/config.json", USB_TIMEOUT_MS);
}

void usb_transfer_get_stats(usb_stats_t *stats_out)
{
    if (stats_out) {
        *stats_out = stats;
    }
}

esp_err_t usb_transfer_cancel(void)
{
    if (transfer_active) {
        transfer_active = false;
        ESP_LOGI(TAG, "Transferencia cancelada");
        return ESP_OK;
    }
    return ESP_ERR_INVALID_STATE;
}

bool usb_transfer_get_progress(uint32_t *transferred, uint32_t *total)
{
    if (transfer_active) {
        *transferred = current_transfer_progress;
        *total = current_transfer_total;
        return true;
    }
    return false;
}
