# Lector NFC para FreeStyle Libre - Asistente IA Geriátrico

## Descripción General

El **módulo NFC PN532** se utiliza para leer el sensor **FreeStyle Libre 2** de Abbott, un dispositivo de monitoreo continuo de glucosa (CGM) ampliamente utilizado por personas con diabetes.

En el Asistente IA Geriátrico, esta integración permite:

- **Lectura automática de glucosa** sin intervención del usuario
- **Alertas de hipoglucemia** (glucosa baja)
- **Alertas de hiperglucemia** (glucosa alta)
- **Registro histórico** para el médico tratante
- **Tranquilidad para el cuidador**

---

## El Sensor FreeStyle Libre 2

### Especificaciones del Sensor
| Parámetro | Valor |
|:---|:---|
| Fabricante | Abbott |
| Tipo | Sensor flash de glucosa |
| Duración | 14 días |
| Medición | Cada minuto |
| Lectura | Por NFC (cercanía < 4 cm) |
| Almacenamiento | 8 horas de datos |
| Precisión | MARD 9.3% |

### Datos que Entrega
| Dato | Rango | Precisión |
|:---|:---|:---|
| Glucosa actual | 40-500 mg/dL | ±10 mg/dL |
| Tendencia | Flecha (6 direcciones) | - |
| Historial | Últimas 8 horas | Cada 15 min |
| Temperatura sensor | - | - |

---

## Módulo NFC Recomendado: PN532

| Parámetro | Valor |
|:---|:---|
| Modelo | PN532 |
| Fabricante | NXP |
| Interfaz | I2C / SPI / HSU |
| Frecuencia | 13.56 MHz |
| Protocolos | ISO/IEC 14443 A/B, Mifare, FeliCa |
| Alcance | Hasta 5 cm |
| Consumo (lectura) | 50 mA |
| Consumo (sleep) | 10 µA |
| Voltaje | 3.3V - 5V |
| Dimensiones módulo | 40 mm × 20 mm × 5 mm |

---

## Conexión con ESP32-S3 (I2C)

| Pin PN532 | Pin ESP32-S3 | Función |
|:---|:---|:---|
| VCC | 3.3V | Alimentación |
| GND | GND | Tierra |
| SCL | GPIO 15 | I2C Clock |
| SDA | GPIO 14 | I2C Data |
| IRQ | GPIO 13 | Interrupción (tarjeta detectada) |
| RSTO | GPIO 12 | Reset (opcional) |

---

## Configuración en Firmware

### Inicialización

```c
#include "pn532.h"

#define PN532_I2C_ADDRESS  0x48  // Dirección típica del módulo
#define PN532_IRQ_GPIO     13

PN532 pn532;

esp_err_t nfc_init(void)
{
    // Inicializar I2C para NFC
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_14,
        .scl_io_num = GPIO_NUM_15,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };
    
    i2c_param_config(I2C_NUM_1, &conf);
    i2c_driver_install(I2C_NUM_1, I2C_MODE_MASTER, 0, 0, 0);
    
    // Inicializar PN532
    pn532.begin(I2C_NUM_1, PN532_I2C_ADDRESS);
    
    // Configurar
    uint32_t version = pn532.getFirmwareVersion();
    if (!version) {
        printf("Error: PN532 no detectado\n");
        return ESP_FAIL;
    }
    
    printf("PN532 versión: %d.%d\n", (version >> 16) & 0xFF, (version >> 8) & 0xFF);
    
    // Configurar modo NFC
    pn532.SAMConfig();  // Configuración de acceso seguro
    
    return ESP_OK;
}
```

---

## Protocolo de Lectura FreeStyle Libre

### Secuencia de Comunicación

1. **Detección de sensor** (campo NFC)
2. **Autenticación** (comando específico Abbott)
3. **Lectura de datos** (bloques de memoria)
4. **Parseo** de datos de glucosa
5. **Almacenamiento** local
6. **Alerta** si corresponde

### Implementación

```c
// Comandos específicos para FreeStyle Libre
#define CMD_GET_UID          0x00
#define CMD_READ_BLOCK       0x20
#define CMD_AUTHENTICATE     0x1A

typedef struct {
    uint16_t glucose_raw;      // Valor crudo de glucosa
    uint8_t trend;             // Dirección de tendencia (0-7)
    uint32_t timestamp;        // Timestamp de la medición
    uint8_t sensor_status;     // Estado del sensor
} libre_data_t;

libre_data_t current_glucose;

esp_err_t read_freestyle_libre(libre_data_t *data)
{
    uint8_t uid[7];
    uint8_t blocks[24][8];  // 24 bloques de 8 bytes
    
    // 1. Detectar y obtener UID
    if (!pn532.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, 2000)) {
        return ESP_ERR_NOT_FOUND;  // No hay sensor
    }
    
    // 2. Autenticar (comando propietario Abbott)
    uint8_t auth_cmd[] = {0x1A, 0x00};  // Comando de autenticación
    if (!pn532.inDataExchange(auth_cmd, sizeof(auth_cmd), NULL, NULL)) {
        return ESP_ERR_AUTH_FAIL;
    }
    
    // 3. Leer bloques de datos (direcciones específicas Libre)
    for (int i = 0; i < 24; i++) {
        uint8_t read_cmd[] = {CMD_READ_BLOCK, i};
        pn532.inDataExchange(read_cmd, sizeof(read_cmd), blocks[i], NULL);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    
    // 4. Parsear datos (formato propietario Abbott)
    data->glucose_raw = (blocks[0][0] << 8) | blocks[0][1];
    data->trend = (blocks[0][2] >> 4) & 0x07;
    data->sensor_status = blocks[0][3];
    
    // 5. Convertir a mg/dL
    uint16_t glucose_mgdl = (data->glucose_raw >> 2) & 0x3FF;  // 10 bits de datos
    glucose_mgdl = glucose_mgdl * 1.0;  // Factor de conversión
    
    data->glucose_raw = glucose_mgdl;
    
    return ESP_OK;
}
```

---

## Interpretación de Datos

### Valores de Glucosa
| Rango (mg/dL) | Interpretación | Acción |
|:---|:---|:---|
| < 54 | Hipoglucemia severa | Alerta inmediata + emergencia |
| 54 - 70 | Hipoglucemia | Alerta al usuario + recomendación |
| 70 - 180 | Rango normal | Registro silencioso |
| 180 - 250 | Hiperglucemia leve | Alerta + sugerencia |
| > 250 | Hiperglucemia severa | Alerta + recomendar consulta |

### Tendencia (Flechas)

| Código | Dirección | Significado |
|:---|:---|:---|
| 0 | → | Estable |
| 1 | ↗ | Subiendo lentamente |
| 2 | ↑ | Subiendo |
| 3 | ↑↑ | Subiendo rápido |
| 4 | ↘ | Bajando lentamente |
| 5 | ↓ | Bajando |
| 6 | ↓↓ | Bajando rápido |
| 7 | - | No computable |

---

## Alertas de Glucosa

```c
void check_glucose_alert(libre_data_t *data)
{
    // Hipoglucemia severa
    if (data->glucose_raw < 54) {
        play_voice_message("alerta_glucosa_critica_hijo");
        send_alert_to_caregiver(GLUCOSE_CRITICAL);
        return;
    }
    
    // Hipoglucemia
    if (data->glucose_raw < 70) {
        play_voice_message("alerta_glucosa_baja_hija");
        
        if (data->trend >= 5) {  // Bajando rápido
            play_voice_message("recomendacion_azucar");
        }
        return;
    }
    
    // Hiperglucemia severa
    if (data->glucose_raw > 250) {
        play_voice_message("alerta_glucosa_alta_hijo");
        return;
    }
    
    // Registro normal (sin alerta)
    log_glucose_reading(data);
}
```

---

## Almacenamiento Local

```c
#define MAX_HISTORY 1008  // 14 días × 72 lecturas/día (cada 20 min)

glucose_reading_t glucose_history[MAX_HISTORY];
uint16_t history_index = 0;

void log_glucose_reading(libre_data_t *data)
{
    glucose_history[history_index].timestamp = get_epoch_time();
    glucose_history[history_index].glucose = data->glucose_raw;
    glucose_history[history_index].trend = data->trend;
    
    history_index = (history_index + 1) % MAX_HISTORY;
    
    // Guardar en Flash cada 10 lecturas
    if (history_index % 10 == 0) {
        save_glucose_history();
    }
}

void save_glucose_history(void)
{
    // Escribir en flash (SPIFFS o LittleFS)
    FILE *f = fopen("/spiffs/glucose.dat", "wb");
    fwrite(glucose_history, sizeof(glucose_reading_t), MAX_HISTORY, f);
    fclose(f);
}
```

---

## Consumo Energético

| Modo | Consumo | Tiempo activo/día | Energía/día |
|:---|:---|:---|:---|
| Sleep | 10 µA | 23.5 h | 0.235 mAh |
| Detección (cada 5 min) | 50 mA | 0.5 h | 25 mAh |
| Lectura completa | 50 mA | 0.2 h | 10 mAh |
| **TOTAL** | | | **35.235 mAh** |

---

## Problemas Comunes y Soluciones

| Problema | Causa | Solución |
|:---|:---|:---|
| No detecta sensor | Distancia > 4 cm | Acercar el dispositivo |
| | Sensor mal orientado | Rotar el dispositivo |
| | Batería del sensor baja | Reemplazar sensor |
| Lectura falla | Interferencia metálica | Alejar de objetos metálicos |
| | Sensor expirado | Verificar fecha (14 días) |
| Datos erráticos | Protocolo incorrecto | Actualizar firmware |
| | Múltiples sensores | Alejar otros sensores NFC |

---

## Referencias

1. NXP. (2023). "PN532 User Manual".
2. Abbott. (2024). "FreeStyle Libre 2 User's Manual".
3. Abbott. (2024). "FreeStyle Libre NFC Communication Protocol" (NDA required).
4. IEEE 802.15.6-2024. "Wireless Body Area Networks".
5. Diabetes Technology Society. (2025). "Standards for CGM Interoperability".

---

*Documento v1.0 - Marzo 2026*
*Asistente IA Geriátrico - Mackiber Labs*
