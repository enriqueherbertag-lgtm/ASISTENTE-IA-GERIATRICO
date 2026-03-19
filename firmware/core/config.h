/**
 * @file config.h
 * @brief Configuración global del Asistente IA Geriátrico
 * 
 * Este archivo contiene todas las constantes de configuración del sistema,
 * incluyendo pines de GPIO, tiempos de muestreo, umbrales y mensajes.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * CONFIGURACIÓN GENERAL DEL SISTEMA
 * ================================================================== */

// Versión del firmware
#define FIRMWARE_VERSION_MAJOR      1
#define FIRMWARE_VERSION_MINOR      0
#define FIRMWARE_VERSION_PATCH      0

// Nombre del dispositivo para Bluetooth
#define DEVICE_NAME                 "AsistenteIA_Geriatico"

// Intervalos de tiempo (en milisegundos)
#define ACTIVITY_CHECK_INTERVAL     100     // 100 ms
#define DEEP_SLEEP_INTERVAL         60000   // 1 minuto
#define SHUTDOWN_INTERVAL           3600000 // 1 hora

// Timeouts de inactividad
#define ACTIVE_TIMEOUT_MS           30000   // 30 segundos
#define LIGHT_SLEEP_TIMEOUT_MS      60000   // 1 minuto
#define DEEP_SLEEP_TIMEOUT_MS       300000  // 5 minutos

/* ==================================================================
 * CONFIGURACIÓN DE PINES GPIO
 * ================================================================== */

// Cámara OV2640
#define CAM_PIN_PWDN                34
#define CAM_PIN_RESET               33
#define CAM_PIN_XCLK                24
#define CAM_PIN_SIOD                20
#define CAM_PIN_SIOC                19
#define CAM_PIN_D7                   32
#define CAM_PIN_D6                   31
#define CAM_PIN_D5                   30
#define CAM_PIN_D4                   29
#define CAM_PIN_D3                   28
#define CAM_PIN_D2                   27
#define CAM_PIN_D1                   26
#define CAM_PIN_D0                   25
#define CAM_PIN_VSYNC                21
#define CAM_PIN_HREF                 22
#define CAM_PIN_PCLK                 23

// IMU MPU-6050 (I2C)
#define I2C_IMU_SCL                  18
#define I2C_IMU_SDA                  17
#define IMU_INT_PIN                  16

// NFC PN532 (I2C)
#define I2C_NFC_SCL                  15
#define I2C_NFC_SDA                  14
#define NFC_IRQ_PIN                  13
#define NFC_RST_PIN                  12

// Audio (micrófono y altavoz)
#define MIC_PIN_DIN                   4
#define MIC_PIN_CLK                   5
#define SPEAKER_PIN                   6

// Batería y fuel gauge
#define FUEL_GAUGE_SCL               37
#define FUEL_GAUGE_SDA               38
#define FUEL_GAUGE_ALRT               39
#define CHARGING_STATUS_PIN           35
#define CHARGE_COMPLETE_PIN           36

// MicroSD (SPI)
#define SD_SPI_CLK                    7
#define SD_SPI_MOSI                   8
#define SD_SPI_MISO                   9
#define SD_SPI_CS                     10

// USB (ya integrado en ESP32-S3)

/* ==================================================================
 * DIRECCIONES I2C
 * ================================================================== */

#define MPU6050_ADDR                 0x68    // Dirección IMU (AD0 a GND)
#define PN532_I2C_ADDR               0x48    // Dirección módulo NFC
#define MAX17048_ADDR                0x36    // Dirección fuel gauge

/* ==================================================================
 * CONFIGURACIÓN DE SENSORES
 * ================================================================== */

// Cámara
#define CAMERA_FRAME_SIZE             FRAMESIZE_QVGA  // 320x240
#define CAMERA_JPEG_QUALITY           12              // 0-63 (menor = mejor)
#define CAMERA_FB_COUNT                1

// IMU
#define IMU_ACCEL_RANGE               ACCE_RANGE_4G
#define IMU_GYRO_RANGE                GYRO_RANGE_500DPS
#define IMU_SAMPLE_RATE               100             // Hz

// NFC (lectura de glucosa)
#define GLUCOSE_READ_INTERVAL         300000          // 5 minutos
#define GLUCOSE_RETRY_COUNT            3

// Audio
#define AUDIO_SAMPLE_RATE              16000          // 16 kHz
#define AUDIO_BITS_PER_SAMPLE           16
#define AUDIO_BUFFER_SECONDS             5

/* ==================================================================
 * UMBRALES DE DETECCIÓN
 * ================================================================== */

// Detección de caídas
#define FALL_THRESHOLD_G               2.5f    // Aceleración > 2.5g
#define FREE_FALL_THRESHOLD_G           0.3f    // Aceleración < 0.3g
#define IMPACT_THRESHOLD_G              3.0f    // Impacto > 3.0g
#define POST_FALL_TIME_MS               3000    // 3 segundos
#define INACTIVITY_TIME_MS              5000    // 5 segundos

// Glucosa (mg/dL)
#define GLUCOSE_HYPO_SEVERE              54
#define GLUCOSE_HYPO                      70
#define GLUCOSE_NORMAL_MIN                70
#define GLUCOSE_NORMAL_MAX               180
#define GLUCOSE_HYPER                     250

// Batería (%)
#define BATTERY_LOW                       15
#define BATTERY_CRITICAL                    5

// Reconocimiento facial
#define FACE_RECOGNITION_THRESHOLD       0.6f    // Similitud mínima
#define MAX_FAMILY_FACES                    10

/* ==================================================================
 * MENSAJES DE VOZ (IDs)
 * ================================================================== */

// Mensajes generales
#define MSG_STARTUP                        0
#define MSG_SHUTDOWN                        1
#define MSG_NO_ENTENDI                      2
#define MSG_BATTERY_LOW                      3
#define MSG_BATTERY_CRITICAL                  4
#define MSG_BATTERY_FULL                      5

// Caídas
#define MSG_FALL_DETECTED                    10
#define MSG_FALL_CONFIRMED                    11
#define MSG_FALL_ASK_STATUS                    12

// Glucosa
#define MSG_GLUCOSE_CRITICAL                  20
#define MSG_GLUCOSE_LOW                        21
#define MSG_GLUCOSE_HIGH                        22
#define MSG_GLUCOSE_NORMAL                      23
#define MSG_GLUCOSE_READING                      24

// Recordatorios
#define MSG_REMINDER_MEDICINE                  30
#define MSG_REMINDER_WATER                      31
#define MSG_REMINDER_MEAL                        32

// Familia
#define MSG_FAMILY_GREETING                      40
#define MSG_FAMILY_RECOGNITION                    41
#define MSG_FAMILY_VISIT                          42

// Recetas
#define MSG_RECIPE_INGREDIENTS                    50
#define MSG_RECIPE_STEPS                          51
#define MSG_RECIPE_MISSING                         52

// Orientación
#define MSG_ORIENTATION_NIGHT                      60
#define MSG_ORIENTATION_DAY                        61
#define MSG_ORIENTATION_LOCATION                    62

/* ==================================================================
 * CONFIGURACIÓN DE ALMACENAMIENTO
 * ================================================================== */

// MicroSD
#define SD_MOUNT_POINT                    "/sdcard"
#define VOICE_FOLDER                      "/sdcard/voices"
#define RECIPES_FILE                       "/sdcard/recipes.csv"
#define FACES_FILE                         "/sdcard/faces.bin"
#define HISTORY_FILE                       "/sdcard/history.db"

// Memoria flash (NVS)
#define NVS_NAMESPACE                      "aig"

// Tamaños de buffer
#define CONTEXTUAL_BUFFER_SECONDS           60
#define MAX_RECIPES                         50
#define MAX_HISTORY_ENTRIES                  1000

/* ==================================================================
 * CONFIGURACIÓN DE BLUETOOTH
 * ================================================================== */

#define BT_ENABLED                           0   // Desactivado por defecto
#define BT_ALERT_SERVICE_UUID           "A1B2C3D4"
#define BT_ALERT_CHAR_UUID              "E5F6A7B8"

/* ==================================================================
 * ESTRUCTURAS DE CONFIGURACIÓN
 * ================================================================== */

typedef struct {
    uint8_t firmware_version[3];
    uint32_t device_id;
    
    // Configuración de sensores
    uint16_t glucose_interval_ms;
    uint16_t imu_sample_rate_hz;
    
    // Umbrales personalizables
    float fall_threshold_g;
    uint16_t hypo_threshold_mgdl;
    uint16_t hyper_threshold_mgdl;
    
    // Preferencias del usuario
    uint8_t voice_volume;          // 0-100
    uint8_t bedtime_hour;           // 0-23
    uint8_t wakeup_hour;            // 0-23
    
    // Familiares registrados
    uint8_t family_members_count;
    char family_members[10][32];    // Nombres
} SystemConfig_t;

/* ==================================================================
 * DECLARACIONES DE FUNCIONES (implementadas en config.c)
 * ================================================================== */

/**
 * @brief Carga la configuración desde NVS
 */
esp_err_t config_load(SystemConfig_t *config);

/**
 * @brief Guarda la configuración en NVS
 */
esp_err_t config_save(SystemConfig_t *config);

/**
 * @brief Restaura configuración de fábrica
 */
esp_err_t config_reset_to_defaults(SystemConfig_t *config);

/**
 * @brief Imprime configuración actual en log
 */
void config_print(SystemConfig_t *config);

/* ==================================================================
 * MACROS DE UTILIDAD
 * ================================================================== */

#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */
