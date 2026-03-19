/**
 * @file imu_driver.h
 * @brief Driver para IMU MPU-6050 (acelerómetro + giroscopio)
 * 
 * Este módulo gestiona la inicialización, lectura y procesamiento
 * de datos del sensor inercial MPU-6050.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef IMU_DRIVER_H
#define IMU_DRIVER_H

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
 * @brief Modos de energía del IMU
 */
typedef enum {
    IMU_NORMAL = 0,        // Modo normal (mayor consumo)
    IMU_LOW_POWER,         // Modo bajo consumo
    IMU_SLEEP              // Modo sleep (mínimo consumo)
} imu_power_mode_t;

/**
 * @brief Rango del acelerómetro
 */
typedef enum {
    ACCEL_RANGE_2G = 0,    // ±2g
    ACCEL_RANGE_4G,        // ±4g
    ACCEL_RANGE_8G,        // ±8g
    ACCEL_RANGE_16G        // ±16g
} accel_range_t;

/**
 * @brief Rango del giroscopio
 */
typedef enum {
    GYRO_RANGE_250DPS = 0,  // ±250 °/s
    GYRO_RANGE_500DPS,      // ±500 °/s
    GYRO_RANGE_1000DPS,     // ±1000 °/s
    GYRO_RANGE_2000DPS      // ±2000 °/s
} gyro_range_t;

/**
 * @brief Datos del IMU
 */
typedef struct {
    float accel_x;          // Aceleración X (g)
    float accel_y;          // Aceleración Y (g)
    float accel_z;          // Aceleración Z (g)
    float gyro_x;           // Velocidad angular X (°/s)
    float gyro_y;           // Velocidad angular Y (°/s)
    float gyro_z;           // Velocidad angular Z (°/s)
    float temperature;      // Temperatura (°C)
    uint64_t timestamp;     // Timestamp de la lectura
} imu_data_t;

/**
 * @brief Datos crudos del IMU (sin convertir)
 */
typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
    int16_t temperature;
} imu_raw_t;

/* ==================================================================
 * FUNCIONES PRINCIPALES
 * ================================================================== */

/**
 * @brief Inicializa el IMU
 * @return ESP_OK en éxito, otro en error
 */
esp_err_t imu_init(void);

/**
 * @brief Lee datos del IMU
 * @param data Puntero donde almacenar los datos
 * @return ESP_OK en éxito, otro en error
 */
esp_err_t imu_read(imu_data_t *data);

/**
 * @brief Lee datos crudos del IMU
 * @param raw Puntero donde almacenar los datos crudos
 * @return ESP_OK en éxito, otro en error
 */
esp_err_t imu_read_raw(imu_raw_t *raw);

/**
 * @brief Configura el rango del acelerómetro
 * @param range Rango deseado
 * @return ESP_OK en éxito
 */
esp_err_t imu_set_accel_range(accel_range_t range);

/**
 * @brief Configura el rango del giroscopio
 * @param range Rango deseado
 * @return ESP_OK en éxito
 */
esp_err_t imu_set_gyro_range(gyro_range_t range);

/**
 * @brief Configura el modo de energía
 * @param mode Modo deseado
 * @return ESP_OK en éxito
 */
esp_err_t imu_set_power_mode(imu_power_mode_t mode);

/**
 * @brief Calibra el IMU (estima offsets)
 * @param samples Número de muestras para calibrar
 * @return ESP_OK en éxito
 */
esp_err_t imu_calibrate(int samples);

/**
 * @brief Verifica si el IMU está calibrado
 * @return true si está calibrado
 */
bool imu_is_calibrated(void);

/**
 * @brief Obtiene los offsets de calibración
 * @param accel_offset Offset del acelerómetro
 * @param gyro_offset Offset del giroscopio
 */
void imu_get_offsets(float *accel_offset, float *gyro_offset);

/* ==================================================================
 * FUNCIONES DE PROCESAMIENTO
 * ================================================================== */

/**
 * @brief Calcula la magnitud del vector aceleración
 * @param data Datos del IMU
 * @return Magnitud en g
 */
float imu_magnitude(imu_data_t *data);

/**
 * @brief Calcula el ángulo de inclinación (pitch)
 * @param data Datos del IMU
 * @return Ángulo en grados (-90 a 90)
 */
float imu_pitch(imu_data_t *data);

/**
 * @brief Calcula el ángulo de rotación (roll)
 * @param data Datos del IMU
 * @return Ángulo en grados (-180 a 180)
 */
float imu_roll(imu_data_t *data);

/**
 * @brief Calcula la orientación (yaw) integrando giroscopio
 * @param data Datos del IMU
 * @param delta_time Tiempo desde última lectura (segundos)
 * @return Ángulo en grados (0-360)
 */
float imu_yaw(imu_data_t *data, float delta_time);

/**
 * @brief Detecta si hay movimiento significativo
 * @param data Datos del IMU
 * @return true si hay movimiento
 */
bool imu_movement_detected(imu_data_t *data);

/**
 * @brief Aplica filtro pasa-bajos a los datos
 * @param data Datos a filtrar (modificados in-place)
 * @param alpha Factor de suavizado (0-1, menor = más suave)
 */
void imu_lowpass_filter(imu_data_t *data, float alpha);

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

/**
 * @brief Verifica si el IMU está inicializado
 * @return true si está inicializado
 */
bool imu_is_initialized(void);

/**
 * @brief Verifica si el IMU está presente en el bus I2C
 * @return true si está presente
 */
bool imu_is_present(void);

/**
 * @brief Obtiene la tasa de muestreo actual
 * @return Tasa en Hz
 */
uint16_t imu_get_sample_rate(void);

/**
 * @brief Obtiene el contador de lecturas
 * @return Número de lecturas desde el inicio
 */
uint32_t imu_get_read_count(void);

#ifdef __cplusplus
}
#endif

#endif /* IMU_DRIVER_H */
