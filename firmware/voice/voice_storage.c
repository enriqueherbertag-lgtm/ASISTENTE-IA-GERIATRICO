/**
 * @file voice_storage.c
 * @brief Implementación de gestión de almacenamiento de voz
 * 
 * Este archivo contiene la implementación de las funciones
 * declaradas en voice_storage.h
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
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include "voice_storage.h"
#include "config.h"

static const char *TAG = "VOICE_STORAGE";

// Variables internas
static bool sd_mounted = false;
static sdmmc_card_t *sd_card = NULL;

static voice_metadata_t messages[MAX_VOICE_MESSAGES];
static int message_count = 0;

static family_member_t family[MAX_FAMILY_MEMBERS];
static int family_count = 0;

static voice_storage_stats_t current_stats = {0};

// Directorios
#define VOICE_DIR      "/sdcard/voices"
#define FAMILY_DIR     "/sdcard/family"
#define CONFIG_FILE    "/sdcard/voice_index.csv"

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static esp_err_t mount_sd(void)
{
    if (sd_mounted) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Montando tarjeta microSD");
    
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &sd_card);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error montando microSD: %d", ret);
        return ret;
    }
    
    sd_mounted = true;
    
    // Obtener información de la tarjeta
    sdmmc_card_print_info(stdout, sd_card);
    
    return ESP_OK;
}

static void unmount_sd(void)
{
    if (sd_mounted) {
        esp_vfs_fat_sdcard_unmount("/sdcard", sd_card);
        sd_mounted = false;
        ESP_LOGI(TAG, "MicroSD desmontada");
    }
}

static void ensure_directories(void)
{
    struct stat st;
    
    if (stat(VOICE_DIR, &st) != 0) {
        mkdir(VOICE_DIR, 0755);
        ESP_LOGI(TAG, "Directorio %s creado", VOICE_DIR);
    }
    
    if (stat(FAMILY_DIR, &st) != 0) {
        mkdir(FAMILY_DIR, 0755);
        ESP_LOGI(TAG, "Directorio %s creado", FAMILY_DIR);
    }
}

static int parse_index_file(void)
{
    FILE *f = fopen(CONFIG_FILE, "r");
    if (!f) {
        ESP_LOGW(TAG, "Archivo de índice no encontrado");
        return 0;
    }
    
    char line[256];
    int count = 0;
    
    while (fgets(line, sizeof(line), f) && count < MAX_VOICE_MESSAGES) {
        // Formato: id,nombre,archivo,duracion,familiar,fallback
        voice_metadata_t *msg = &messages[count];
        
        int fallback;
        sscanf(line, "%hhu,%63[^,],%63[^,],%lu,%hhu,%d",
               &msg->id, msg->name, msg->filename,
               &msg->duration_ms, &msg->family_member, &fallback);
        
        msg->is_fallback = (fallback != 0);
        count++;
    }
    
    fclose(f);
    return count;
}

static void save_index_file(void)
{
    FILE *f = fopen(CONFIG_FILE, "w");
    if (!f) {
        ESP_LOGE(TAG, "Error guardando archivo de índice");
        return;
    }
    
    for (int i = 0; i < message_count; i++) {
        voice_metadata_t *msg = &messages[i];
        fprintf(f, "%d,%s,%s,%lu,%d,%d\n",
                msg->id, msg->name, msg->filename,
                msg->duration_ms, msg->family_member,
                msg->is_fallback ? 1 : 0);
    }
    
    fclose(f);
    ESP_LOGI(TAG, "Índice guardado (%d mensajes)", message_count);
}

static void scan_voice_files(void)
{
    DIR *dir = opendir(VOICE_DIR);
    if (!dir) {
        ESP_LOGE(TAG, "Error abriendo directorio %s", VOICE_DIR);
        return;
    }
    
    struct dirent *entry;
    int new_count = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".wav") || strstr(entry->d_name, ".mp3")) {
            // Verificar si ya está en el índice
            bool found = false;
            for (int i = 0; i < message_count; i++) {
                if (strcmp(messages[i].filename, entry->d_name) == 0) {
                    found = true;
                    break;
                }
            }
            
            if (!found && message_count < MAX_VOICE_MESSAGES) {
                // Agregar al índice
                voice_metadata_t *msg = &messages[message_count];
                msg->id = message_count;
                strncpy(msg->filename, entry->d_name, sizeof(msg->filename) - 1);
                msg->filename[sizeof(msg->filename) - 1] = '\0';
                
                // Nombre por defecto (sin extensión)
                char *dot = strrchr(msg->filename, '.');
                if (dot) {
                    int len = dot - msg->filename;
                    if (len < sizeof(msg->name)) {
                        strncpy(msg->name, msg->filename, len);
                        msg->name[len] = '\0';
                    }
                } else {
                    strncpy(msg->name, msg->filename, sizeof(msg->name) - 1);
                }
                
                msg->duration_ms = 0; // Se puede calcular después
                msg->family_member = 255; // Genérico
                msg->is_fallback = false;
                
                message_count++;
                new_count++;
            }
        }
    }
    
    closedir(dir);
    
    if (new_count > 0) {
        ESP_LOGI(TAG, "%d nuevos archivos de voz encontrados", new_count);
        save_index_file();
    }
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

esp_err_t voice_storage_init(void)
{
    ESP_LOGI(TAG, "Inicializando sistema de almacenamiento de voz");
    
    // Montar microSD
    esp_err_t ret = mount_sd();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Crear directorios si no existen
    ensure_directories();
    
    // Cargar índice
    message_count = parse_index_file();
    
    // Escanear archivos nuevos
    scan_voice_files();
    
    // Inicializar familiares por defecto
    family_count = 0;
    
    ESP_LOGI(TAG, "Sistema de voz inicializado: %d mensajes", message_count);
    
    return ESP_OK;
}

esp_err_t voice_storage_mount_sd(void)
{
    return mount_sd();
}

esp_err_t voice_storage_unmount_sd(void)
{
    unmount_sd();
    return ESP_OK;
}

esp_err_t voice_storage_load_all(void)
{
    return voice_storage_init();  // Recarga todo
}

const voice_metadata_t* voice_storage_get_metadata(uint8_t id)
{
    if (id >= message_count) {
        return NULL;
    }
    
    return &messages[id];
}

esp_err_t voice_storage_get_path(uint8_t id, char *path, size_t max_len)
{
    if (id >= message_count || !path) {
        return ESP_ERR_INVALID_ARG;
    }
    
    snprintf(path, max_len, "%s/%s", VOICE_DIR, messages[id].filename);
    return ESP_OK;
}

bool voice_storage_message_exists(uint8_t id)
{
    return (id < message_count);
}

uint32_t voice_storage_get_message_count(void)
{
    return message_count;
}

int voice_storage_add_family_member(const char *name, const char *relation, uint8_t priority)
{
    if (family_count >= MAX_FAMILY_MEMBERS || !name || !relation) {
        return -1;
    }
    
    family_member_t *fm = &family[family_count];
    fm->id = family_count;
    strncpy(fm->name, name, sizeof(fm->name) - 1);
    strncpy(fm->relation, relation, sizeof(fm->relation) - 1);
    fm->voice_count = 0;
    fm->priority = priority;
    
    family_count++;
    
    ESP_LOGI(TAG, "Familiar agregado: %s (%s)", name, relation);
    
    return fm->id;
}

const family_member_t* voice_storage_get_family_member(uint8_t id)
{
    if (id >= family_count) {
        return NULL;
    }
    
    return &family[id];
}

int voice_storage_get_family_members(family_member_t *members, int max_count)
{
    int to_copy = (family_count < max_count) ? family_count : max_count;
    
    for (int i = 0; i < to_copy; i++) {
        members[i] = family[i];
    }
    
    return to_copy;
}

int voice_storage_get_family_messages(uint8_t family_id, voice_metadata_t *msg_buffer, int max_count)
{
    if (family_id >= family_count) {
        return 0;
    }
    
    int found = 0;
    for (int i = 0; i < message_count && found < max_count; i++) {
        if (messages[i].family_member == family_id) {
            msg_buffer[found++] = messages[i];
        }
    }
    
    return found;
}

uint8_t voice_storage_find_best_message(const char *context, uint8_t family_id)
{
    // Buscar mensajes que coincidan con el contexto
    for (int i = 0; i < message_count; i++) {
        if (strstr(messages[i].name, context)) {
            if (family_id == 255 || messages[i].family_member == family_id) {
                return messages[i].id;
            }
        }
    }
    
    // Buscar mensajes de fallback
    for (int i = 0; i < message_count; i++) {
        if (messages[i].is_fallback && strstr(messages[i].name, context)) {
            return messages[i].id;
        }
    }
    
    return 0;  // No encontrado
}

uint8_t voice_storage_get_fallback_message(const char *context)
{
    for (int i = 0; i < message_count; i++) {
        if (messages[i].is_fallback && strstr(messages[i].name, context)) {
            return messages[i].id;
        }
    }
    
    return 0;
}

bool voice_storage_is_mounted(void)
{
    return sd_mounted;
}

void voice_storage_get_stats(voice_storage_stats_t *stats)
{
    if (!stats) return;
    
    stats->total_messages = message_count;
    stats->family_count = family_count;
    
    // Calcular espacio usado/libre
    if (sd_card) {
        stats->used_space_bytes = sd_card->csd.capacity * sd_card->csd.sector_size;
        stats->free_space_bytes = stats->used_space_bytes;  // Simplificado
    } else {
        stats->used_space_bytes = 0;
        stats->free_space_bytes = 0;
    }
}

int voice_storage_scan_for_new(void)
{
    int old_count = message_count;
    scan_voice_files();
    return message_count - old_count;
}
