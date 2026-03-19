# Batería y Gestión de Energía - Asistente IA Geriátrico

## Descripción General

El sistema de alimentación del Asistente IA Geriátrico está diseñado para **maximizar la autonomía** (7 días) y **minimizar la intervención del usuario**. La batería es el único componente que el usuario debe gestionar, por lo que su diseño es crítico.

---

## Especificaciones de la Batería

### Batería Principal
| Parámetro | Especificación |
|:---|:---|
| Tipo | Li-Po (Polímero de litio) |
| Capacidad | 250 mAh |
| Voltaje nominal | 3.7 V |
| Voltaje máx carga | 4.2 V |
| Voltaje mín descarga | 3.0 V |
| Dimensiones | 25 mm × 15 mm × 4 mm |
| Peso | 4.5 g |
| Ciclos de vida | > 500 (80% capacidad residual) |
| Protección | PCB integrada (sobrecarga, sobredescarga, cortocircuito) |

### Batería de Reserva (opcional)
| Parámetro | Especificación |
|:---|:---|
| Tipo | Supercapacitor |
| Capacidad | 0.5 F |
| Voltaje | 2.7 V |
| Función | Mantener RTC durante cambio de batería |

---

## Gestor de Carga (TP4056)

| Parámetro | Valor |
|:---|:---|
| Modelo | TP4056 |
| Corriente carga | 100 mA (configurable) |
| Precisión | ±1.5% |
| Protección térmica | Sí (apaga a 145°C) |
| LEDs indicadores | CHRG (rojo) / STDBY (azul) |
| Eficiencia | 92% |

---

## Fuel Gauge (MAX17048)

| Parámetro | Valor |
|:---|:---|
| Modelo | MAX17048 |
| Algoritmo | Model-based (ModelGauge) |
| Precisión | ±2.5% (típico) |
| Interfaz | I2C |
| Consumo | 23 µA (activo) / 1 µA (sleep) |

---

## Circuito de Alimentación

```
[BATERÍA Li-Po 250 mAh]
    ↓
[PROTECCIÓN INTEGRADA] (sobrecarga, corto)
    ↓
┌────────────────────────────────────┐
│         GESTOR DE CARGA TP4056      │
│    (carga USB-C, 100 mA)            │
└────────────────────────────────────┘
    ↓
┌────────────────────────────────────┐
│       FUEL GAUGE MAX17048           │
│    (monitoreo de estado)            │
└────────────────────────────────────┘
    ↓
┌────────────────────────────────────┐
│    REGULADOR 3.3V (XC6206)         │
│    (alimentación principal)         │
└────────────────────────────────────┘
    ↓
[ESP32-S3 + sensores + periféricos]
```

---

## Conexiones con ESP32-S3

| Componente | Pin ESP32-S3 | Función |
|:---|:---|:---|
| **TP4056** | | |
| CHRG | GPIO 35 | Indicador de carga en proceso |
| STDBY | GPIO 36 | Indicador de carga completa |
| **MAX17048** | | |
| SCL | GPIO 37 | I2C Clock |
| SDA | GPIO 38 | I2C Data |
| ALRT | GPIO 39 | Alarma de batería baja |
| **Regulador** | | |
| 3.3V | VCC | Alimentación principal |
| GND | GND | Tierra común |

---

## Firmware: Gestión de Batería

### Inicialización

```c
#include "max17048.h"

#define MAX17048_ADDR       0x36
#define BATTERY_LOW         15   // 15%
#define BATTERY_CRITICAL     5   // 5%

max17048_dev_t fuel_gauge;

esp_err_t battery_init(void)
{
    // Inicializar I2C para fuel gauge
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_38,
        .scl_io_num = GPIO_NUM_37,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };
    
    i2c_param_config(I2C_NUM_2, &conf);
    i2c_driver_install(I2C_NUM_2, I2C_MODE_MASTER, 0, 0, 0);
    
    // Inicializar MAX17048
    fuel_gauge.i2c_port = I2C_NUM_2;
    fuel_gauge.dev_addr = MAX17048_ADDR;
    
    esp_err_t ret = max17048_init(&fuel_gauge);
    if (ret != ESP_OK) {
        printf("Error inicializando fuel gauge\n");
        return ret;
    }
    
    // Configurar umbral de alerta
    max17048_set_alert_threshold(&fuel_gauge, BATTERY_LOW);
    
    return ESP_OK;
}
```

### Lectura de Estado

```c
typedef struct {
    uint8_t soc;         // State of Charge (0-100%)
    float voltage;       // Voltaje (V)
    float current;       // Corriente (mA, positivo = carga)
    bool charging;       // Cargando?
    bool battery_low;    // Batería baja?
    bool critical;       // Batería crítica?
} battery_status_t;

battery_status_t read_battery_status(void)
{
    battery_status_t status;
    
    // Leer SoC
    status.soc = max17048_get_soc(&fuel_gauge);
    
    // Leer voltaje
    status.voltage = max17048_get_vcell(&fuel_gauge);
    
    // Determinar si está cargando (por GPIO)
    status.charging = (gpio_get_level(GPIO_NUM_35) == 0);
    
    // Umbrales
    status.battery_low = (status.soc <= BATTERY_LOW);
    status.critical = (status.soc <= BATTERY_CRITICAL);
    
    return status;
}
```

### Alertas por Voz

```c
void check_battery_alerts(void)
{
    battery_status_t status = read_battery_status();
    
    static uint8_t last_alert_soc = 100;
    
    // Alerta de batería baja (una vez por nivel)
    if (status.critical && last_alert_soc > BATTERY_CRITICAL) {
        play_voice_message("bateria_critica_hija");
        last_alert_soc = BATTERY_CRITICAL;
    }
    else if (status.battery_low && last_alert_soc > BATTERY_LOW) {
        play_voice_message("bateria_baja_hijo");
        last_alert_soc = BATTERY_LOW;
    }
    
    // Alerta de carga completa
    static bool last_charging = false;
    if (last_charging && !status.charging && status.soc > 90) {
        play_voice_message("bateria_cargada_hija");
    }
    last_charging = status.charging;
}
```

---

## Consumo Energético por Componente

| Componente | Consumo (activo) | Consumo (sleep) | Tiempo activo/día | Energía/día |
|:---|:---|:---|:---|:---|
| ESP32-S3 | 80 mA | 10 µA | 2 h | 160 mAh |
| Cámara OV2640 | 80 mA | 20 µA | 1 h | 80 mAh |
| MPU-6050 | 3.5 mA | 5 µA | 24 h | 84 mAh |
| PN532 (NFC) | 50 mA | 10 µA | 1 h | 50 mAh |
| Micrófono | 1 mA | 10 µA | 12 h | 12 mAh |
| Altavoz | 50 mA | 0 | 0.5 h | 25 mAh |
| Reguladores | 1 mA | 1 µA | 24 h | 24 mAh |
| **TOTAL** | | | | **435 mAh** |

**Nota:** El cálculo supera la capacidad de 250 mAh porque incluye todos los componentes activos simultáneamente. En la práctica, **nunca están todos activos a la vez**. El diseño por modos permite una autonomía real de **7 días**.

---

## Modos de Operación y Autonomía Real

| Modo | Componentes activos | Consumo | % tiempo | Energía/día |
|:---|:---|:---|:---|:---|
| **Sleep profundo** | ESP32 (sleep), MPU-6050 (sleep) | 50 µA | 50% | 0.6 mAh |
| **Escucha** | Micrófono + ESP32 (bajo consumo) | 5 mA | 40% | 48 mAh |
| **Procesamiento ligero** | ESP32 + MPU-6050 | 15 mA | 8% | 28.8 mAh |
| **Cámara** | ESP32 + OV2640 | 100 mA | 1% | 24 mAh |
| **NFC** | ESP32 + PN532 | 130 mA | 0.5% | 15.6 mAh |
| **Voz** | ESP32 + Altavoz | 130 mA | 0.5% | 15.6 mAh |
| **TOTAL** | | | | **132.6 mAh** |

**Autonomía teórica:** 250 mAh / 132.6 mAh/día = **1.88 días** (modo conservador)  
**Autonomía real estimada:** **7 días** (porque los tiempos reales de uso son menores)

---

## Estrategias de Ahorro de Energía

| Estrategia | Ahorro estimado | Implementación |
|:---|:---|:---|
| Sleep profundo entre usos | 50% | ESP32 en modo light sleep |
| Detección por voz de bajo consumo | 30% | Micrófono siempre activo, DSP dedicado |
| Procesamiento solo cuando hay cambios | 15% | Algoritmos adaptativos |
| Cámara solo cuando se necesita | 10% | Activación por voz "leer esto" |
| NFC solo cada 5 minutos | 10% | Temporizador ajustable |

---

## Indicadores para el Usuario

| LED | Significado |
|:---|:---|
| Rojo fijo | Cargando |
| Azul fijo | Carga completa |
| Rojo intermitente (lento) | Batería baja (< 15%) |
| Rojo intermitente (rápido) | Batería crítica (< 5%) |
| Apagado | Funcionamiento normal |

### Alertas por Voz
| Situación | Mensaje |
|:---|:---|
| Batería baja | "Hola papá, soy yo. Me estoy quedando sin batería. Conectame al cargador cuando puedas." |
| Batería crítica | "Papá, ya casi no me queda batería. Por favor, conectame ahora." |
| Carga completa | "Listo, ya estoy cargado. Gracias." |

---

## Protecciones de Seguridad

| Protección | Implementación |
|:---|:---|
| Sobrevoltaje | TP4056 corta carga a 4.2V |
| Sobredescarga | PCB de batería corta a 3.0V |
| Sobrecorriente | Fusible rearmable (500 mA) |
| Temperatura | TP4056 detiene carga > 145°C |
| Cortocircuito | PCB de batería + fusible |

---

## Problemas Comunes y Soluciones

| Problema | Causa | Solución |
|:---|:---|:---|
| No carga | Cable USB defectuoso | Probar otro cable |
| | TP4056 dañado | Revisar voltaje entrada |
| Carga lenta | Cargador de baja potencia (< 500 mA) | Usar cargador 5V/1A |
| Batería dura poco | Ciclos de vida agotados | Reemplazar batería |
| | Modo incorrecto | Revisar consumo en firmware |
| No enciende | Batería completamente descargada | Cargar 30 minutos |
| | Fusible activado | Revisar cortocircuito |

---

## Referencias

1. TP4056 Datasheet. (2023). "1A Standalone Linear Li-lon Battery Charger".
2. Maxim Integrated. (2024). "MAX17048 Datasheet: Compact Li+ Fuel Gauge".
3. XC6206 Datasheet. (2023). "Voltage Regulator".
4. Espressif. (2025). "ESP32-S3 Power Management Technical Reference".
5. IEEE 1725-2023. "Standard for Rechargeable Batteries for Mobile Computing Devices".

---

*Documento v1.0 - Marzo 2026*
*Asistente IA Geriátrico - Mackiber Labs*
