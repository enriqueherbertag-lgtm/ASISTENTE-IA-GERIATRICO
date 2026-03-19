# Firmware - Asistente IA Geriátrico

## Descripción General

El firmware del Asistente IA Geriátrico gestiona todos los sensores, el procesamiento local, la reproducción de voz y la gestión de energía del dispositivo. Está optimizado para **bajo consumo** y **respuesta en tiempo real**.

---

## Estructura del Firmware

```
firmware/
├── README.md
├── core/
│   ├── main.c
│   ├── power_manager.c
│   └── config.h
├── drivers/
│   ├── camera/
│   │   ├── camera_driver.c
│   │   └── camera_driver.h
│   ├── imu/
│   │   ├── imu_driver.c
│   │   └── imu_driver.h
│   ├── nfc/
│   │   ├── nfc_driver.c
│   │   └── nfc_driver.h
│   ├── audio/
│   │   ├── mic_driver.c
│   │   ├── speaker_driver.c
│   │   └── audio_driver.h
│   └── battery/
│       ├── fuel_gauge.c
│       └── fuel_gauge.h
├── algorithms/
│   ├── fall_detection.c
│   ├── voice_recognition.c
│   ├── glucose_alert.c
│   └── contextual_buffer.c
├── voice/
│   ├── voice_storage.c
│   ├── playback.c
│   └── messages.h
├── memory/
│   ├── recipe_db.c
│   ├── face_db.c
│   └── history.c
└── communication/
    ├── bluetooth.c
    └── usb_transfer.c
```

---

## Plataforma de Desarrollo

### Hardware Objetivo
| Componente | Modelo |
|:---|:---|
| MCU | ESP32-S3 (Xtensa LX7 dual-core) |
| Flash | 16 MB |
| RAM | 512 KB |
| Almacenamiento externo | MicroSD (SPI) |

### Entorno de Desarrollo
| Herramienta | Versión | Uso |
|:---|:---|:---|
| ESP-IDF | v5.2 | SDK oficial de Espressif |
| Toolchain | GCC 13.2 | Compilador |
| CMake | 3.25 | Sistema de construcción |
| VS Code | 1.95 | IDE recomendado |
| Python | 3.10 | Scripts auxiliares |

---

## Módulos Principales

### core/
Punto de entrada y gestión central del sistema.

| Archivo | Función |
|:---|:---|
| `main.c` | Bucle principal, inicialización de subsistemas |
| `power_manager.c` | Gestión de modos de sueño, wake-up |
| `config.h` | Constantes de configuración global |

### drivers/
Controladores de bajo nivel para periféricos.

| Módulo | Descripción |
|:---|:---|
| `camera/` | Driver para OV2640 (captura, formatos) |
| `imu/` | Driver para MPU-6050 (acelerómetro + giroscopio) |
| `nfc/` | Driver para PN532 (lectura FreeStyle Libre) |
| `audio/` | Micrófono + altavoz (grabación, reproducción) |
| `battery/` | Fuel gauge MAX17048, gestión de carga |

### algorithms/
Algoritmos de procesamiento específicos.

| Archivo | Descripción |
|:---|:---|
| `fall_detection.c` | Detección de caídas en tiempo real |
| `voice_recognition.c` | Reconocimiento básico de comandos |
| `glucose_alert.c` | Interpretación de datos de glucosa |
| `contextual_buffer.c` | Buffer de conversación (60 seg) |

### voice/
Gestión de mensajes de voz familiares.

| Archivo | Descripción |
|:---|:---|
| `voice_storage.c` | Lectura/escritura de archivos de audio en microSD |
| `playback.c` | Reproducción con prioridades |
| `messages.h` | Índice de mensajes disponibles |

### memory/
Bases de datos locales.

| Archivo | Descripción |
|:---|:---|
| `recipe_db.c` | Recetas familiares (nombre + ingredientes) |
| `face_db.c` | Embeddings faciales de familiares |
| `history.c` | Registro de eventos (caídas, glucosa) |

### communication/
Comunicación externa (limitada por privacidad).

| Archivo | Descripción |
|:---|:---|
| `bluetooth.c` | Bluetooth LE (solo para alertas, desactivado por defecto) |
| `usb_transfer.c` | Carga de voces y configuración por USB |

---

## Flujo Principal del Sistema

```c
void app_main(void)
{
    // 1. Inicializar hardware
    init_power_manager();
    init_drivers();
    
    // 2. Cargar configuración y datos desde microSD
    load_voice_messages();
    load_recipe_db();
    load_face_db();
    
    // 3. Entrar en modo escucha de bajo consumo
    while (1) {
        // Escuchar comando de voz (bajo consumo)
        if (detect_wake_word()) {
            process_voice_command();
        }
        
        // Leer sensores periódicamente
        if (time_for_sensor_read()) {
            read_imu();
            check_for_falls();
            
            if (time_for_glucose_read()) {
                read_glucose();
                check_glucose_alerts();
            }
        }
        
        // Verificar batería
        check_battery_level();
        
        // Gestión de energía
        go_to_sleep_until_next_event();
    }
}
```

---

## Modos de Energía

| Modo | Consumo | Descripción |
|:---|:---|:---|
| **Deep Sleep** | 10 µA | Todo apagado excepto RTC |
| **Light Sleep** | 100 µA | Micrófono activo (detección de palabra) |
| **Escucha** | 5 mA | Procesamiento de voz |
| **Activo** | 50-150 mA | Cámara, NFC, procesamiento intenso |

---

## Compilación y Programación

### Requisitos previos
```bash
# Instalar ESP-IDF v5.2
git clone -b v5.2 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
source export.sh
```

### Compilar
```bash
cd firmware
idf.py set-target esp32s3
idf.py menuconfig  # Configurar opciones
idf.py build
```

### Programar
```bash
# Por USB
idf.py -p /dev/ttyUSB0 flash

# Por USB (con monitoreo serial)
idf.py -p /dev/ttyUSB0 flash monitor
```

---

## Pruebas Unitarias

```bash
# Compilar y ejecutar pruebas en host
idf.py tests
```

---

## Referencias

1. Espressif. (2025). "ESP32-S3 Technical Reference Manual".
2. Espressif. (2025). "ESP-IDF Programming Guide".
3. FreeRTOS. (2024). "FreeRTOS Kernel Documentation".
4. IEEE 802.15.6-2024. "Wireless Body Area Networks".

---

*Documento v1.0 - Marzo 2026*
*Asistente IA Geriátrico - Mackiber Labs*
