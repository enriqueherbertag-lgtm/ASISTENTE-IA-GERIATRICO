/**
 * @file imu_driver.c
 * @brief Implementación del driver para IMU MPU-6050
 * 
 * Este archivo contiene la implementación de las funciones
 * declaradas en imu_driver.h
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
#include "esp_log.h"
#include "driver/i2c.h"
#include "imu_driver.h"
#include "config.h"

static const char *TAG = "IMU_DRV";

// Dirección I2C del MPU-6050
#define MPU6050_ADDR        0x68
#define MPU6050_WHO_AM_I    0x75
#define MPU6050_PWR_MGMT_1  0x6B
#define MPU6050_CONFIG       0x1A
#define MPU6050_GYRO_CONFIG  0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_TEMP_OUT_H   0x41
#define MPU6050_GYRO_XOUT_H  0x43

// Valores esperados
#define MPU6050_WHO_AM_I_VAL 0x68

// Factores de conversión
#define ACCEL_SENSITIVITY_2G  16384.0f
#define ACCEL_SENSITIVITY_4G  8192.0f
#define ACCEL_SENSITIVITY_8G  4096.0f
#define ACCEL_SENSITIVITY_16G 2048.0f

#define GYRO_SENSITIVITY_250DPS  131.0f
#define GYRO_SENSITIVITY_500DPS  65.5f
#define GYRO_SENSITIVITY_1000DPS 32.8f
#define GYRO_SENSITIVITY_2000DPS 16.4f

#define TEMP_SENSITIVITY  340.0f
#define TEMP_OFFSET       36.53f

// Variables internas
static bool imu_initialized = false;
static bool imu_calibrated = false;
static uint32_t read_counter = 0;
static uint16_t sample_rate = 100;  // Hz

static accel_range_t current_accel_range = ACCEL_RANGE_2G;
static gyro_range_t current_gyro_range = GYRO_RANGE_250DPS;

static float accel_offset[3] = {0, 0, 0};
static float gyro_offset[3] = {0, 0, 0};
static float last_yaw = 0;
static uint64_t last_yaw_time = 0;

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static esp_err_t imu_write_reg(uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t imu_read_reg(uint8_t reg, uint8_t *value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, value, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t imu_read_multi(uint8_t reg, uint8_t *buf, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
    if (len > 1) {
        i2c_master_read(cmd, buf, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, buf + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

static float get_accel_sensitivity(void)
{
    switch (current_accel_range) {
        case ACCEL_RANGE_2G:  return ACCEL_SENSITIVITY_2G;
        case ACCEL_RANGE_4G:  return ACCEL_SENSITIVITY_4G;
        case ACCEL_RANGE_8G:  return ACCEL_SENSITIVITY_8G;
        case ACCEL_RANGE_16G: return ACCEL_SENSITIVITY_16G;
        default:               return ACCEL_SENSITIVITY_2G;
    }
}

static float get_gyro_sensitivity(void)
{
    switch (current_gyro_range) {
        case GYRO_RANGE_250DPS:  return GYRO_SENSITIVITY_250DPS;
        case GYRO_RANGE_500DPS:  return GYRO_SENSITIVITY_500DPS;
        case GYRO_RANGE_1000DPS: return GYRO_SENSITIVITY_1000DPS;
        case GYRO_RANGE_2000DPS: return GYRO_SENSITIVITY_2000DPS;
        default:                  return GYRO_SENSITIVITY_250DPS;
    }
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

esp_err_t imu_init(void)
{
    if (imu_initialized) {
        ESP_LOGW(TAG, "IMU ya inicializado");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Inicializando IMU MPU-6050");
    
    // Verificar presencia del dispositivo
    uint8_t who_am_i = 0;
    esp_err_t ret = imu_read_reg(MPU6050_WHO_AM_I, &who_am_i);
    if (ret != ESP_OK || who_am_i != MPU6050_WHO_AM_I_VAL) {
        ESP_LOGE(TAG, "IMU no detectado (ID: 0x%02x)", who_am_i);
        return ESP_ERR_NOT_FOUND;
    }
    
    // Salir del modo sleep
    ret = imu_write_reg(MPU6050_PWR_MGMT_1, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error despertando IMU");
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Configurar rangos por defecto
    imu_set_accel_range(ACCEL_RANGE_2G);
    imu_set_gyro_range(GYRO_RANGE_250DPS);
    
    // Configurar filtro pasa-bajos (DLPF)
    imu_write_reg(MPU6050_CONFIG, 0x03);  // 44 Hz
    
    imu_initialized = true;
    ESP_LOGI(TAG, "IMU inicializado correctamente");
    
    return ESP_OK;
}

esp_err_t imu_read(imu_data_t *data)
{
    if (!imu_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t buf[14];
    esp_err_t ret = imu_read_multi(MPU6050_ACCEL_XOUT_H, buf, 14);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Convertir datos crudos
    int16_t accel_x = (buf[0] << 8) | buf[1];
    int16_t accel_y = (buf[2] << 8) | buf[3];
    int16_t accel_z = (buf[4] << 8) | buf[5];
    int16_t temp = (buf[6] << 8) | buf[7];
    int16_t gyro_x = (buf[8] << 8) | buf[9];
    int16_t gyro_y = (buf[10] << 8) | buf[11];
    int16_t gyro_z = (buf[12] << 8) | buf[13];
    
    float accel_sens = get_accel_sensitivity();
    float gyro_sens = get_gyro_sensitivity();
    
    // Aplicar offsets y convertir a unidades físicas
    data->accel_x = (accel_x / accel_sens) - accel_offset[0];
    data->accel_y = (accel_y / accel_sens) - accel_offset[1];
    data->accel_z = (accel_z / accel_sens) - accel_offset[2];
    
    data->gyro_x = (gyro_x / gyro_sens) - gyro_offset[0];
    data->gyro_y = (gyro_y / gyro_sens) - gyro_offset[1];
    data->gyro_z = (gyro_z / gyro_sens) - gyro_offset[2];
    
    data->temperature = (temp / TEMP_SENSITIVITY) + TEMP_OFFSET;
    data->timestamp = esp_timer_get_time();
    
    read_counter++;
    
    return ESP_OK;
}

esp_err_t imu_read_raw(imu_raw_t *raw)
{
    if (!imu_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t buf[14];
    esp_err_t ret = imu_read_multi(MPU6050_ACCEL_XOUT_H, buf, 14);
    if (ret != ESP_OK) {
        return ret;
    }
    
    raw->accel_x = (buf[0] << 8) | buf[1];
    raw->accel_y = (buf[2] << 8) | buf[3];
    raw->accel_z = (buf[4] << 8) | buf[5];
    raw->temperature = (buf[6] << 8) | buf[7];
    raw->gyro_x = (buf[8] << 8) | buf[9];
    raw->gyro_y = (buf[10] << 8) | buf[11];
    raw->gyro_z = (buf[12] << 8) | buf[13];
    
    return ESP_OK;
}

esp_err_t imu_set_accel_range(accel_range_t range)
{
    uint8_t value = 0;
    switch (range) {
        case ACCEL_RANGE_2G:  value = 0x00; break;
        case ACCEL_RANGE_4G:  value = 0x08; break;
        case ACCEL_RANGE_8G:  value = 0x10; break;
        case ACCEL_RANGE_16G: value = 0x18; break;
        default: return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = imu_write_reg(MPU6050_ACCEL_CONFIG, value);
    if (ret == ESP_OK) {
        current_accel_range = range;
    }
    return ret;
}

esp_err_t imu_set_gyro_range(gyro_range_t range)
{
    uint8_t value = 0;
    switch (range) {
        case GYRO_RANGE_250DPS:  value = 0x00; break;
        case GYRO_RANGE_500DPS:  value = 0x08; break;
        case GYRO_RANGE_1000DPS: value = 0x10; break;
        case GYRO_RANGE_2000DPS: value = 0x18; break;
        default: return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = imu_write_reg(MPU6050_GYRO_CONFIG, value);
    if (ret == ESP_OK) {
        current_gyro_range = range;
    }
    return ret;
}

esp_err_t imu_set_power_mode(imu_power_mode_t mode)
{
    uint8_t value;
    esp_err_t ret = imu_read_reg(MPU6050_PWR_MGMT_1, &value);
    if (ret != ESP_OK) {
        return ret;
    }
    
    switch (mode) {
        case IMU_NORMAL:
            value &= ~0x40;  // Limpiar bit SLEEP
            break;
        case IMU_LOW_POWER:
            value |= 0x40;   // Activar SLEEP
            // Configurar ciclo de trabajo (6.25 Hz)
            break;
        case IMU_SLEEP:
            value |= 0x40;   // Activar SLEEP
            break;
        default:
            return ESP_ERR_INVALID_ARG;
    }
    
    return imu_write_reg(MPU6050_PWR_MGMT_1, value);
}

esp_err_t imu_calibrate(int samples)
{
    ESP_LOGI(TAG, "Calibrando IMU con %d muestras...", samples);
    
    float accel_sum[3] = {0, 0, 0};
    float gyro_sum[3] = {0, 0, 0};
    
    for (int i = 0; i < samples; i++) {
        imu_data_t data;
        if (imu_read(&data) == ESP_OK) {
            accel_sum[0] += data.accel_x;
            accel_sum[1] += data.accel_y;
            accel_sum[2] += data.accel_z;
            gyro_sum[0] += data.gyro_x;
            gyro_sum[1] += data.gyro_y;
            gyro_sum[2] += data.gyro_z;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    // Calcular promedios
    for (int i = 0; i < 3; i++) {
        accel_offset[i] = accel_sum[i] / samples;
        gyro_offset[i] = gyro_sum[i] / samples;
    }
    
    // El acelerómetro en reposo debería tener solo gravedad en Z
    // Ajustar offset de Z para que muestre 1g
    accel_offset[2] -= 1.0;
    
    imu_calibrated = true;
    ESP_LOGI(TAG, "Calibración completada");
    
    return ESP_OK;
}

bool imu_is_calibrated(void)
{
    return imu_calibrated;
}

void imu_get_offsets(float *accel_offset_out, float *gyro_offset_out)
{
    memcpy(accel_offset_out, accel_offset, 3 * sizeof(float));
    memcpy(gyro_offset_out, gyro_offset, 3 * sizeof(float));
}

/* ==================================================================
 * FUNCIONES DE PROCESAMIENTO
 * ================================================================== */

float imu_magnitude(imu_data_t *data)
{
    return sqrt(data->accel_x * data->accel_x +
                data->accel_y * data->accel_y +
                data->accel_z * data->accel_z);
}

float imu_pitch(imu_data_t *data)
{
    return atan2(-data->accel_x, 
                 sqrt(data->accel_y * data->accel_y + 
                      data->accel_z * data->accel_z)) * 180 / M_PI;
}

float imu_roll(imu_data_t *data)
{
    return atan2(data->accel_y, data->accel_z) * 180 / M_PI;
}

float imu_yaw(imu_data_t *data, float delta_time)
{
    // Integración simple del giroscopio (asume orientación inicial conocida)
    last_yaw += data->gyro_z * delta_time;
    
    // Mantener en rango 0-360
    if (last_yaw < 0) last_yaw += 360;
    if (last_yaw >= 360) last_yaw -= 360;
    
    return last_yaw;
}

bool imu_movement_detected(imu_data_t *data)
{
    // Detectar si la magnitud de la aceleración se desvía de 1g
    float mag = imu_magnitude(data);
    return (mag < 0.8f || mag > 1.2f);
}

void imu_lowpass_filter(imu_data_t *data, float alpha)
{
    static imu_data_t filtered = {0};
    
    if (filtered.timestamp == 0) {
        // Primera lectura
        filtered = *data;
    } else {
        filtered.accel_x = alpha * data->accel_x + (1 - alpha) * filtered.accel_x;
        filtered.accel_y = alpha * data->accel_y + (1 - alpha) * filtered.accel_y;
        filtered.accel_z = alpha * data->accel_z + (1 - alpha) * filtered.accel_z;
        filtered.gyro_x = alpha * data->gyro_x + (1 - alpha) * filtered.gyro_x;
        filtered.gyro_y = alpha * data->gyro_y + (1 - alpha) * filtered.gyro_y;
        filtered.gyro_z = alpha * data->gyro_z + (1 - alpha) * filtered.gyro_z;
        filtered.temperature = alpha * data->temperature + (1 - alpha) * filtered.temperature;
        filtered.timestamp = data->timestamp;
    }
    
    *data = filtered;
}

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

bool imu_is_initialized(void)
{
    return imu_initialized;
}

bool imu_is_present(void)
{
    uint8_t who_am_i = 0;
    imu_read_reg(MPU6050_WHO_AM_I, &who_am_i);
    return (who_am_i == MPU6050_WHO_AM_I_VAL);
}

uint16_t imu_get_sample_rate(void)
{
    return sample_rate;
}

uint32_t imu_get_read_count(void)
{
    return read_counter;
}
