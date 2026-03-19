# Cámara OV2640 - Asistente IA Geriátrico

## Descripción General

La **OV2640** es un sensor de imagen CMOS de baja potencia fabricado por OmniVision. En el Asistente IA Geriátrico se utiliza para:

- **Lectura de etiquetas** (medicamentos, alimentos, fechas de vencimiento)
- **Reconocimiento facial básico** de familiares
- **Detección de obstáculos** para prevención de caídas

---

## Especificaciones Técnicas

| Parámetro | Valor |
|:---|:---|
| Modelo | OV2640 |
| Fabricante | OmniVision |
| Resolución máx | 2 MP (1600×1200) |
| Tamaño de píxel | 2.2 µm × 2.2 µm |
| Formato óptico | 1/4" |
| Ángulo de visión | 60° (con lente incluida) |
| Interfaz | SCCB (similar a I2C) + DVP (paralelo) |
| Voltaje operación | 1.8V - 3.3V |
| Consumo (activo) | 40 mA (QXGA) |
| Consumo (sleep) | 20 µA |
| Temperatura operación | -30°C a +70°C |

---

## Módulo Recomendado

Para facilitar la integración, se recomienda usar el módulo **OV2640 con interfaz FIFO**:

| Característica | Valor |
|:---|:---|
| Módulo | OV2640 Camera Module (2MP) |
| Dimensiones | 20 mm × 20 mm × 5 mm |
| Interfaz | SPI (a través de FIFO AL422B) |
| Voltaje | 3.3V |
| Consumo | 80 mA (con FIFO activa) |
| Lente incluida | 60° FOV, foco fijo |

---

## Conexión con ESP32-S3

| Pin OV2640 | Pin ESP32-S3 | Función |
|:---|:---|:---|
| SIOC | GPIO 19 | I2C Clock |
| SIOD | GPIO 20 | I2C Data |
| VSYNC | GPIO 21 | Sincronía vertical |
| HREF | GPIO 22 | Referencia horizontal |
| PCLK | GPIO 23 | Pixel Clock |
| XCLK | GPIO 24 | Clock externo |
| D7-D0 | GPIO 25-32 | Datos paralelos |
| RESET | GPIO 33 | Reset |
| PWDN | GPIO 34 | Power Down |

---

## Configuración en Firmware

### Inicialización

```c
#include "esp_camera.h"

// Configuración de pines
static camera_config_t camera_config = {
    .pin_pwdn  = GPIO_NUM_34,
    .pin_reset = GPIO_NUM_33,
    .pin_xclk = GPIO_NUM_24,
    .pin_sscb_sda = GPIO_NUM_20,
    .pin_sscb_scl = GPIO_NUM_19,
    .pin_d7 = GPIO_NUM_32,
    .pin_d6 = GPIO_NUM_31,
    .pin_d5 = GPIO_NUM_30,
    .pin_d4 = GPIO_NUM_29,
    .pin_d3 = GPIO_NUM_28,
    .pin_d2 = GPIO_NUM_27,
    .pin_d1 = GPIO_NUM_26,
    .pin_d0 = GPIO_NUM_25,
    .pin_vsync = GPIO_NUM_21,
    .pin_href = GPIO_NUM_22,
    .pin_pclk = GPIO_NUM_23,
    
    .xclk_freq_hz = 20000000,      // 20 MHz
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG, // JPEG para ahorrar memoria
    .frame_size = FRAMESIZE_QVGA,   // 320x240
    .jpeg_quality = 12,              // 0-63 (menor = mejor calidad)
    .fb_count = 1                     // 1 frame buffer
};

esp_err_t camera_init(void)
{
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        printf("Error inicializando cámara: %x\n", err);
        return err;
    }
    return ESP_OK;
}
```

### Captura de Imagen

```c
camera_fb_t * capture_image(void)
{
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        printf("Error capturando imagen\n");
        return NULL;
    }
    return fb;
}

void release_image(camera_fb_t * fb)
{
    esp_camera_fb_return(fb);
}
```

---

## Modos de Operación

### Modo 1: Lectura de Etiquetas (Alta Resolución)

| Parámetro | Valor |
|:---|:---|
| Resolución | SVGA (800×600) |
| Formato | JPEG |
| Consumo | 80 mA |
| Tiempo captura | 300 ms |
| Frecuencia | Bajo demanda (cuando se activa por voz) |

### Modo 2: Reconocimiento Facial (Media Resolución)

| Parámetro | Valor |
|:---|:---|
| Resolución | QVGA (320×240) |
| Formato | JPEG |
| Consumo | 60 mA |
| Tiempo captura | 150 ms |
| Frecuencia | Cada 5 segundos (si hay movimiento) |

### Modo 3: Detección de Obstáculos (Baja Resolución)

| Parámetro | Valor |
|:---|:---|
| Resolución | QQVGA (160×120) |
| Formato | JPEG |
| Consumo | 40 mA |
| Tiempo captura | 100 ms |
| Frecuencia | Cada 2 segundos (al caminar) |

---

## Procesamiento de Imagen

### Lectura de Texto (OCR)

```c
#include "tesseract.h"

char* ocr_from_image(uint8_t *jpeg_data, size_t len)
{
    // Decodificar JPEG
    img_t img = jpeg_decode(jpeg_data, len);
    
    // Aplicar preprocesamiento (binarización, etc.)
    img_preprocess(&img);
    
    // Ejecutar OCR local (Tesseract liviano)
    char *text = tesseract_ocr(&img);
    
    return text;
}
```

### Reconocimiento Facial (Simplificado)

```c
typedef struct {
    char name[32];
    uint8_t face_embedding[128];  // Embedding facial
} FaceData_t;

FaceData_t family_faces[10];  // Hasta 10 familiares

int recognize_face(uint8_t *jpeg_data, size_t len)
{
    // Extraer embedding de la imagen capturada
    uint8_t current_embedding[128];
    extract_face_embedding(jpeg_data, len, current_embedding);
    
    // Comparar con base de datos familiar
    int best_match = -1;
    float best_distance = 1000;
    
    for (int i = 0; i < family_count; i++) {
        float dist = compare_embeddings(current_embedding, family_faces[i].face_embedding);
        if (dist < 0.6 && dist < best_distance) {  // Umbral de similitud
            best_distance = dist;
            best_match = i;
        }
    }
    
    return best_match;  // -1 si no reconoce
}
```

---

## Consumo Energético Detallado

| Modo | Resolución | Formato | Consumo | Tiempo activo/día | Energía/día |
|:---|:---|:---|:---|:---|:---|
| Sleep | - | - | 20 µA | 23 h | 0.46 mAh |
| Lectura etiquetas | SVGA | JPEG | 80 mA | 2 min | 2.67 mAh |
| Reconocimiento facial | QVGA | JPEG | 60 mA | 30 min | 30 mAh |
| Detección obstáculos | QQVGA | JPEG | 40 mA | 60 min | 40 mAh |
| **TOTAL** | | | | | **73.13 mAh** |

---

## Problemas Comunes y Soluciones

| Problema | Causa | Solución |
|:---|:---|:---|
| Imagen borrosa | Lente sucia | Limpiar con paño de microfibra |
| | Foco incorrecto | Ajustar distancia (30 cm mínimo) |
| Imagen oscura | Poca luz | Activar LED auxiliar |
| | Exposición automática lenta | Esperar 2-3 segundos |
| No captura | Pines mal conectados | Verificar conexiones |
| | Voltaje insuficiente | Asegurar 3.3V estables |
| Consumo alto | Resolución muy alta | Reducir a QVGA para uso continuo |
| | Modo siempre activo | Usar detección de movimiento |

---

## Referencias

1. OmniVision. (2023). "OV2640 Datasheet: 2-Megapixel CMOS Image Sensor".
2. Espressif. (2025). "ESP32-CAM Camera Development Board Documentation".
3. Espressif. (2025). "esp32-camera Library Documentation".
4. Tesseract OCR. (2024). "Tesseract Open Source OCR Engine".
5. OpenCV. (2025). "Face Recognition with OpenCV".

---

*Documento v1.0 - Marzo 2026*
*Asistente IA Geriátrico - Mackiber Labs*
