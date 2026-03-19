# Giroscopio MPU-6050 - Asistente IA Geriátrico

## Descripción General

El **MPU-6050** es un sensor inercial de 6 ejes (acelerómetro + giroscopio) fabricado por TDK InvenSense. En el Asistente IA Geriátrico se utiliza para:

- **Detección de caídas** (evento crítico)
- **Monitorización de actividad** (caminar, estar sentado, acostado)
- **Detección de deambulación nocturna** (movimiento en horas de sueño)
- **Orientación espacial** (inclinación, posición)

---

## Especificaciones Técnicas

### Acelerómetro
| Parámetro | Valor |
|:---|:---|
| Rango | ±2g, ±4g, ±8g, ±16g (seleccionable) |
| Resolución | 16 bits (65,536 LSB/g a ±2g) |
| Sensibilidad | 16384 LSB/g (a ±2g) |
| Ruido | 400 µg/√Hz |
| Frecuencia muestreo | 4 Hz - 1 kHz |

### Giroscopio
| Parámetro | Valor |
|:---|:---|
| Rango | ±250, ±500, ±1000, ±2000 °/s |
| Resolución | 16 bits |
| Sensibilidad | 131 LSB/°/s (a ±250 °/s) |
| Ruido | 0.005 °/s/√Hz |
| Frecuencia muestreo | 4 Hz - 8 kHz |

### Generales
| Parámetro | Valor |
|:---|:---|
| Modelo | MPU-6050 |
| Fabricante | TDK InvenSense |
| Interfaz | I2C (hasta 400 kHz) |
| Dirección I2C | 0x68 (por defecto) / 0x69 |
| Voltaje operación | 2.375V - 3.46V |
| Consumo | 3.5 mA (activo) / 5 µA (sleep) |
| FIFO | 1024 bytes |
| Temperatura operación | -40°C a +85°C |

---

## Conexión con ESP32-S3

| Pin MPU-6050 | Pin ESP32-S3 | Función |
|:---|:---|:---|
| VCC | 3.3V | Alimentación |
| GND | GND | Tierra |
| SCL | GPIO 18 | I2C Clock |
| SDA | GPIO 17 | I2C Data |
| AD0 | GND | Dirección I2C (0x68) |
| INT | GPIO 16 | Interrupción (opcional) |

---

## Configuración en Firmware

### Inicialización

```c
#include "mpu6050.h"

#define I2C_MASTER_SCL_IO   18
#define I2C_MASTER_SDA_IO   17
#define MPU6050_ADDR        0x68

mpu6050_dev_t mpu_dev;

esp_err_t mpu6050_init(void)
{
    // Inicializar I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };
    
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    
    // Inicializar MPU6050
    memset(&mpu_dev, 0, sizeof(mpu6050_dev_t));
    mpu_dev.i2c_port = I2C_NUM_0;
    mpu_dev.dev_addr = MPU6050_ADDR;
    
    esp_err_t ret = mpu6050_register(&mpu_dev);
    if (ret != ESP_OK) {
        printf("Error inicializando MPU6050\n");
        return ret;
    }
    
    // Configurar rangos
    mpu6050_config(&mpu_dev, ACCE_RANGE_4G, GYRO_RANGE_500DPS);
    
    // Activar
    mpu6050_wake_up(&mpu_dev);
    
    return ESP_OK;
}
```

### Lectura de Datos

```c
typedef struct {
    float ax, ay, az;      // Aceleración (g)
    float gx, gy, gz;      // Rotación (°/s)
    float temperature;      // Temperatura (°C)
} imu_data_t;

imu_data_t read_imu(void)
{
    imu_data_t data;
    mpu6050_acce_value_t acce;
    mpu6050_gyro_value_t gyro;
    float temp;
    
    // Leer acelerómetro
    mpu6050_get_acce(&mpu_dev, &acce);
    data.ax = acce.acce_x / 1000.0;  // Convertir mg a g
    data.ay = acce.acce_y / 1000.0;
    data.az = acce.acce_z / 1000.0;
    
    // Leer giroscopio
    mpu6050_get_gyro(&mpu_dev, &gyro);
    data.gx = gyro.gyro_x / 100.0;   // Convertir cientos de °/s a °/s
    data.gy = gyro.gyro_y / 100.0;
    data.gz = gyro.gyro_z / 100.0;
    
    // Leer temperatura
    mpu6050_get_temp(&mpu_dev, &temp);
    data.temperature = temp / 100.0;
    
    return data;
}
```

---

## Algoritmo de Detección de Caídas

### Lógica de Detección

```c
#define FALL_THRESHOLD     2.5  // Aceleración > 2.5g (caída)
#define FREE_FALL_THRESHOLD 0.3  // Aceleración < 0.3g (caída libre)
#define IMPACT_THRESHOLD    3.0  // Impacto > 3.0g
#define POST_FALL_TIME      3000 // 3 segundos post-impacto
#define INACTIVITY_TIME     5000 // 5 segundos sin movimiento

typedef enum {
    STATE_NORMAL,
    STATE_FREE_FALL,
    STATE_IMPACT,
    STATE_POST_FALL,
    STATE_FALL_CONFIRMED
} fall_state_t;

fall_state_t detect_fall(imu_data_t *data, uint32_t timestamp)
{
    static fall_state_t state = STATE_NORMAL;
    static uint32_t last_event_time = 0;
    
    float total_accel = sqrt(data->ax*data->ax + data->ay*data->ay + data->az*data->az);
    
    switch(state) {
        case STATE_NORMAL:
            if (total_accel < FREE_FALL_THRESHOLD) {
                state = STATE_FREE_FALL;
                last_event_time = timestamp;
            }
            break;
            
        case STATE_FREE_FALL:
            if (total_accel > IMPACT_THRESHOLD) {
                state = STATE_IMPACT;
                last_event_time = timestamp;
            } else if (timestamp - last_event_time > 2000) {
                state = STATE_NORMAL;  // Falso positivo
            }
            break;
            
        case STATE_IMPACT:
            state = STATE_POST_FALL;
            last_event_time = timestamp;
            break;
            
        case STATE_POST_FALL:
            // Verificar inactividad post-impacto
            if (total_accel < 0.1) {  // Sin movimiento
                if (timestamp - last_event_time > INACTIVITY_TIME) {
                    state = STATE_FALL_CONFIRMED;
                }
            } else {
                state = STATE_NORMAL;  // Se levantó
            }
            break;
            
        case STATE_FALL_CONFIRMED:
            // Ya se detectó, resetear después de alertar
            break;
    }
    
    return state;
}
```

---

## Detección de Actividad

### Clasificación de Actividades

```c
typedef enum {
    ACTIVITY_RESTING,
    ACTIVITY_WALKING,
    ACTIVITY_FIDGETING,
    ACTIVITY_FALL_RISK
} activity_type_t;

activity_type_t classify_activity(imu_data_t *data, float* variance_buffer, int buf_len)
{
    float total_accel = sqrt(data->ax*data->ax + data->ay*data->ay + data->az*data->az);
    
    // Calcular varianza del movimiento (agitación)
    float variance = 0;
    for (int i = 0; i < buf_len; i++) {
        variance += pow(variance_buffer[i] - total_accel, 2);
    }
    variance /= buf_len;
    
    // Clasificar
    if (total_accel < 0.1 && variance < 0.01) {
        return ACTIVITY_RESTING;      // Reposo
    } else if (total_accel > 0.8 && total_accel < 1.2 && variance > 0.1) {
        return ACTIVITY_WALKING;      // Caminando
    } else if (variance > 0.5) {
        return ACTIVITY_FIDGETING;    // Agitación
    } else if (total_accel > 1.5) {
        return ACTIVITY_FALL_RISK;    // Movimiento brusco
    }
    
    return ACTIVITY_RESTING;
}
```

---

## Detección de Deambulación Nocturna

```c
bool detect_night_wandering(imu_data_t *data, uint8_t hour)
{
    static uint32_t last_movement = 0;
    static float total_distance = 0;
    
    // Solo entre 11 PM y 7 AM
    if (hour < 23 && hour > 7) {
        return false;
    }
    
    // Detectar movimiento
    float movement = fabs(data->ax) + fabs(data->ay) + fabs(data->az);
    
    if (movement > 0.5) {
        uint32_t now = millis();
        
        if (last_movement > 0) {
            // Acumular tiempo de movimiento
            total_distance += movement * (now - last_movement) / 1000.0;
        }
        last_movement = now;
    }
    
    // Si ha caminado más de 10 metros en la noche, alertar
    if (total_distance > 10.0) {
        total_distance = 0;
        return true;  // Deambulación significativa
    }
    
    return false;
}
```

---

## Consumo Energético

| Modo | Consumo | Tiempo activo/día | Energía/día |
|:---|:---|:---|:---|
| Sleep | 5 µA | 22 h | 0.11 mAh |
| Lectura (1 Hz) | 500 µA | 2 h | 1.00 mAh |
| Procesamiento caídas | 3.5 mA | 10 min | 0.58 mAh |
| **TOTAL** | | | **1.69 mAh** |

---

## Problemas Comunes y Soluciones

| Problema | Causa | Solución |
|:---|:---|:---|
| Datos erráticos | Interferencia de vibraciones | Usar filtro pasa-bajos |
| | Alimentación inestable | Agregar capacitor de 10 µF |
| No comunica | Dirección I2C incorrecta | Verificar AD0 |
| | Pull-ups faltantes | Agregar resistores 4.7k |
| Caídas no detectadas | Umbral muy alto | Ajustar thresholds |
| | Sensor mal orientado | Verificar montaje |
| Falsas caídas | Movimientos bruscos | Ajustar tiempo post-impacto |

---

## Referencias

1. TDK InvenSense. (2023). "MPU-6050 Product Specification".
2. TDK InvenSense. (2022). "MPU-6000/MPU-6050 Register Map".
3. Espressif. (2025). "ESP32-S3 Technical Reference Manual".
4. Bourke, A. et al. (2023). "Fall detection algorithms for wearable devices".
5. IEEE Standard for Fall Detection Systems. (2024). "IEEE 802.15.6-2024".

---

*Documento v1.0 - Marzo 2026*
*Asistente IA Geriátrico - Mackiber Labs*
