/**
 * @file fuel_gauge.c
 * @brief Implementación del driver para fuel gauge MAX17048
 * 
 * Este archivo contiene la implementación de las funciones
 * declaradas en fuel_gauge.h
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
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "fuel_gauge.h"
#include "config.h"

static const char *TAG = "FUEL_GAUGE";

// Dirección I2C del MAX17048
#define MAX17048_ADDR       0x36

// Registros del MAX17048
#define MAX17048_VCELL      0x02    // Voltaje de celda
#define MAX17048_SOC        0x04    // State of Charge
#define MAX17048_MODE       0x06    // Modo
#define MAX17048_VERSION    0x08    // Versión del chip
#define MAX17048_CONFIG     0x0C    // Configuración
#define MAX17048_COMMAND    0xFE    // Comando especial

// Comandos especiales
#define MAX17048_CMD_RESET  0x5400  // Reset
#define MAX17048_CMD_QUICK  0x4000  // Quick start

// Valores por defecto
#define DEFAULT_BATTERY_CAPACITY    250     // mAh
#define DEFAULT_LOW_THRESHOLD       15      // %
#define DEFAULT_CRITICAL_THRESHOLD  5       // %
#define FULL_VOLTAGE                4200    // mV
#define EMPTY_VOLTAGE               3000    // mV

// Variables internas
static bool fuel_gauge_initialized = false;
static fuel_gauge_mode_t current_mode = FUEL_GAUGE_NORMAL;
static uint16_t battery_capacity = DEFAULT_BATTERY_CAPACITY;
static uint8_t low_threshold = DEFAULT_LOW_THRESHOLD;
static uint8_t critical_threshold = DEFAULT_CRITICAL_THRESHOLD;
static uint16_t cycles = 0;

static battery_status_t last_status = {0};
static battery_event_t last_event = BATTERY_EVENT_NONE;
static uint32_t last_read_time = 0;

// Pines de hardware
#define CHARGING_PIN        CHARGING_STATUS_PIN
#define CHARGE_COMPLETE_PIN CHARGE_COMPLETE_PIN
#define ALERT_PIN           FUEL_GAUGE_ALRT

/* ==================================================================
 * FUNCIONES PRIVADAS
 * ================================================================== */

static esp_err_t max17048_write_reg(uint8_t reg, uint16_t value)
{
    uint8_t buf[3] = {reg, (value >> 8) & 0xFF, value & 0xFF};
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MAX17048_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, buf, 3, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_2, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t max17048_read_reg(uint8_t reg, uint16_t *value)
{
    uint8_t buf[2];
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MAX17048_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MAX17048_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, buf, 2, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_2, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    
    if (ret == ESP_OK) {
        *value = (buf[0] << 8) | buf[1];
    }
    
    return ret;
}

static void update_charging_status(battery_status_t *status)
{
    status->charging = (gpio_get_level(CHARGING_PIN) == 0);
    status->charge_complete = (gpio_get_level(CHARGE_COMPLETE_PIN) == 0);
}

static battery_event_t check_thresholds(battery_status_t *status)
{
    if (status->soc <= critical_threshold && last_status.soc > critical_threshold) {
        return BATTERY_EVENT_CRITICAL;
    }
    if (status->soc <= low_threshold && last_status.soc > low_threshold) {
        return BATTERY_EVENT_LOW;
    }
    
    static bool was_charging = false;
    if (status->charging && !was_charging) {
        was_charging = true;
        return BATTERY_EVENT_CHARGING;
    }
    if (!status->charging && was_charging) {
        was_charging = false;
        if (status->charge_complete) {
            return BATTERY_EVENT_CHARGED;
        }
        return BATTERY_EVENT_DISCHARGING;
    }
    
    return BATTERY_EVENT_NONE;
}

/* ==================================================================
 * FUNCIONES PÚBLICAS
 * ================================================================== */

esp_err_t fuel_gauge_init(void)
{
    if (fuel_gauge_initialized) {
        ESP_LOGW(TAG, "Fuel gauge ya inicializado");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Inicializando fuel gauge MAX17048");
    
    // Configurar pines de entrada
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << CHARGING_PIN) | (1ULL << CHARGE_COMPLETE_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    // Verificar presencia del chip
    uint16_t version;
    esp_err_t ret = max17048_read_reg(MAX17048_VERSION, &version);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MAX17048 no detectado");
        return ESP_ERR_NOT_FOUND;
    }
    
    ESP_LOGI(TAG, "MAX17048 versión: 0x%04X", version);
    
    // Quick start para inicializar
    ret = max17048_write_reg(MAX17048_COMMAND, MAX17048_CMD_QUICK);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error en quick start");
        return ret;
    }
    
    // Configurar umbral de alerta
    uint16_t config = (critical_threshold << 8) | 0x00;  // TH=critical, sin alerta por ahora
    ret = max17048_write_reg(MAX17048_CONFIG, config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error configurando umbral");
        return ret;
    }
    
    fuel_gauge_initialized = true;
    ESP_LOGI(TAG, "Fuel gauge inicializado correctamente");
    
    return ESP_OK;
}

esp_err_t fuel_gauge_read(battery_status_t *status)
{
    if (!fuel_gauge_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint16_t vcell, soc_raw;
    
    // Leer voltaje
    esp_err_t ret = max17048_read_reg(MAX17048_VCELL, &vcell);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Leer estado de carga
    ret = max17048_read_reg(MAX17048_SOC, &soc_raw);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Convertir voltaje (1.25mV por LSB)
    status->voltage = (vcell >> 4) * 0.00125;
    
    // Convertir SoC (1/256% por LSB)
    status->soc = (soc_raw >> 8) & 0xFF;
    
    // Valores por defecto (el MAX17048 no mide corriente directamente)
    status->current = 0;
    status->temperature = 25.0;
    status->time_remaining = 0;
    
    // Actualizar estado de carga
    update_charging_status(status);
    
    // Calcular batería baja/crítica
    status->battery_low = (status->soc <= low_threshold);
    status->battery_critical = (status->soc <= critical_threshold);
    
    // Estimar tiempo restante (muy aproximado)
    if (!status->charging && status->soc > 0) {
        // Asumir consumo promedio de 20mA
        float capacity_remaining = battery_capacity * status->soc / 100.0;
        status->time_remaining = (capacity_remaining / 0.02) * 3600;  // segundos
    }
    
    // Detectar eventos
    battery_event_t event = check_thresholds(status);
    if (event != BATTERY_EVENT_NONE) {
        last_event = event;
    }
    
    // Guardar última lectura
    memcpy(&last_status, status, sizeof(battery_status_t));
    last_read_time = esp_timer_get_time();
    
    return ESP_OK;
}

esp_err_t fuel_gauge_set_mode(fuel_gauge_mode_t mode)
{
    current_mode = mode;
    return ESP_OK;
}

fuel_gauge_mode_t fuel_gauge_get_mode(void)
{
    return current_mode;
}

/* ==================================================================
 * FUNCIONES DE LECTURA INDIVIDUAL
 * ================================================================== */

uint8_t fuel_gauge_get_soc(void)
{
    battery_status_t status;
    if (fuel_gauge_read(&status) == ESP_OK) {
        return status.soc;
    }
    return 0;
}

float fuel_gauge_get_voltage(void)
{
    battery_status_t status;
    if (fuel_gauge_read(&status) == ESP_OK) {
        return status.voltage;
    }
    return 0;
}

float fuel_gauge_get_current(void)
{
    battery_status_t status;
    if (fuel_gauge_read(&status) == ESP_OK) {
        return status.current;
    }
    return 0;
}

float fuel_gauge_get_temperature(void)
{
    battery_status_t status;
    if (fuel_gauge_read(&status) == ESP_OK) {
        return status.temperature;
    }
    return 25.0;
}

bool fuel_gauge_is_charging(void)
{
    return (gpio_get_level(CHARGING_PIN) == 0);
}

bool fuel_gauge_is_charge_complete(void)
{
    return (gpio_get_level(CHARGE_COMPLETE_PIN) == 0);
}

uint32_t fuel_gauge_get_time_remaining(void)
{
    battery_status_t status;
    if (fuel_gauge_read(&status) == ESP_OK) {
        return status.time_remaining;
    }
    return 0;
}

/* ==================================================================
 * FUNCIONES DE CONFIGURACIÓN
 * ================================================================== */

void fuel_gauge_set_thresholds(uint8_t low, uint8_t critical)
{
    low_threshold = low;
    critical_threshold = critical;
    
    // Configurar alerta en el chip
    uint16_t config = (critical << 8) | 0x00;
    max17048_write_reg(MAX17048_CONFIG, config);
}

void fuel_gauge_set_battery_capacity(uint16_t capacity_mah)
{
    battery_capacity = capacity_mah;
}

esp_err_t fuel_gauge_calibrate(uint16_t full_voltage, uint16_t empty_voltage)
{
    // El MAX17048 no requiere calibración explícita
    return ESP_OK;
}

void fuel_gauge_reset_cycles(void)
{
    cycles = 0;
}

/* ==================================================================
 * FUNCIONES DE ALERTAS
 * ================================================================== */

battery_event_t fuel_gauge_check_event(void)
{
    battery_event_t event = last_event;
    last_event = BATTERY_EVENT_NONE;
    return event;
}

void fuel_gauge_enable_interrupt(bool enable)
{
    if (enable) {
        gpio_set_intr_type(ALERT_PIN, GPIO_INTR_NEGEDGE);
        gpio_intr_enable(ALERT_PIN);
    } else {
        gpio_intr_disable(ALERT_PIN);
    }
}

battery_event_t fuel_gauge_get_last_event(void)
{
    return last_event;
}

/* ==================================================================
 * FUNCIONES DE ENERGÍA
 * ================================================================== */

esp_err_t fuel_gauge_sleep(void)
{
    // El MAX17048 entra en sleep automáticamente
    return ESP_OK;
}

esp_err_t fuel_gauge_wake(void)
{
    // Despertar con quick start
    return max17048_write_reg(MAX17048_COMMAND, MAX17048_CMD_QUICK);
}

uint32_t fuel_gauge_get_self_consumption(void)
{
    return 23;  // 23 µA típico
}

/* ==================================================================
 * FUNCIONES DE ESTADO
 * ================================================================== */

bool fuel_gauge_is_initialized(void)
{
    return fuel_gauge_initialized;
}

bool fuel_gauge_is_present(void)
{
    uint16_t version;
    return (max17048_read_reg(MAX17048_VERSION, &version) == ESP_OK);
}

uint16_t fuel_gauge_get_version(void)
{
    uint16_t version;
    max17048_read_reg(MAX17048_VERSION, &version);
    return version;
}

uint16_t fuel_gauge_get_cycles(void)
{
    return cycles;
}

uint8_t fuel_gauge_get_health(void)
{
    // Estimar salud basada en voltaje a plena carga
    battery_status_t status;
    if (fuel_gauge_read(&status) == ESP_OK && status.charge_complete) {
        if (status.voltage >= 4.15) return 100;
        if (status.voltage >= 4.10) return 90;
        if (status.voltage >= 4.00) return 80;
        if (status.voltage >= 3.90) return 70;
        if (status.voltage >= 3.80) return 60;
        return 50;
    }
    return 100;
}
