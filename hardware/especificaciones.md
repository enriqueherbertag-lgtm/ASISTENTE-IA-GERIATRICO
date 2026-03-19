# Especificaciones Técnicas - Asistente IA Geriátrico

## Dimensiones y Diseño Físico

| Parámetro | Especificación |
|:---|:---|
| Formato | Intra-auricular (como audífono) |
| Peso | < 8 gramos |
| Dimensiones | 20 mm × 15 mm × 10 mm |
| Color | Beige / Gris (discreto) |
| Material carcasa | Silicona hipoalergénica + ABS médico |
| Grado de protección | IP54 (resistente a polvo y salpicaduras) |

---

## Procesador y Memoria

| Componente | Especificación |
|:---|:---|
| MCU Principal | ESP32-S3 (o nRF5340) |
| Arquitectura | Dual-core Xtensa LX7 (hasta 240 MHz) |
| RAM | 512 KB |
| Flash | 16 MB (para firmware + datos) |
| Almacenamiento externo | MicroSD (hasta 32 GB, para voces y recetas) |
| NPU (opcional) | Acelerador de redes neuronales (para detección de caídas, emociones) |

---

## Sensores

### Cámara
| Parámetro | Especificación |
|:---|:---|
| Modelo | OV2640 |
| Resolución | 2 megapíxeles (1600×1200) |
| Campo de visión | 60° |
| Enfoque | Fijo (30 cm a infinito) |
| Consumo | 80 mA (activo) / 10 µA (sleep) |
| Función | Lectura de etiquetas, reconocimiento facial básico |

### Giroscopio / Acelerómetro
| Parámetro | Especificación |
|:---|:---|
| Modelo | MPU-6050 (o MPU-9250 con magnetómetro) |
| Ejes | 3 (acelerómetro) + 3 (giroscopio) |
| Rango acelerómetro | ±2g / ±4g / ±8g / ±16g |
| Rango giroscopio | ±250 / ±500 / ±1000 / ±2000 °/s |
| Consumo | 3.5 mA (activo) / 5 µA (sleep) |
| Función | Detección de caídas, orientación, actividad |

### Lector NFC
| Parámetro | Especificación |
|:---|:---|
| Modelo | PN532 (o MFRC522) |
| Frecuencia | 13.56 MHz |
| Protocolos | ISO/IEC 14443 A/B, Mifare, FeliCa |
| Alcance | Hasta 5 cm |
| Consumo | 50 mA (lectura) / 10 µA (sleep) |
| Función | Lectura de sensor FreeStyle Libre 2 |

### Micrófono
| Parámetro | Especificación |
|:---|:---|
| Modelo | SPH0645LM4H (MEMS) |
| SNR | 65 dB |
| Sensibilidad | -26 dBFS |
| Patrón polar | Omnidireccional |
| Consumo | 1 mA (activo) |
| Función | Captura de voz del usuario |

### Altavoz
| Parámetro | Especificación |
|:---|:---|
| Modelo | Receptor balanceado (tipo audífono) |
| Potencia | 50 mW |
| Respuesta frecuencia | 100 Hz - 8 kHz |
| SPL máx | 110 dB |
| Distorsión | < 1% |
| Función | Reproducción de voz familiar y alertas |

---

## Batería y Alimentación

| Parámetro | Especificación |
|:---|:---|
| Tipo | Li-Po (Polímero de litio) |
| Capacidad | 250 mAh |
| Voltaje nominal | 3.7 V |
| Autonomía (uso normal) | 7 días |
| Autonomía (solo espera) | 30 días |
| Tiempo de carga | 2 horas |
| Carga | USB-C (5V / 100 mA) |
| Protecciones | Sobrevoltaje, sobrecorriente, temperatura |

---

## Consumo Energético por Modo

| Modo | Consumo | % tiempo (típico) |
|:---|:---|:---|
| Sleep profundo | 10 µA | 90% |
| Escucha (micrófono activo) | 5 mA | 9% |
| Procesamiento (cámara, detección) | 80 mA | 0.9% |
| Reproducción de voz | 50 mA | 0.09% |
| Transmisión NFC | 50 mA | 0.01% |

**Cálculo de autonomía:**  
(250 mAh) / (5 mA × 9% + 80 mA × 0.9% + ...) ≈ **7 días**

---

## Conectividad

| Tipo | Especificación | Uso |
|:---|:---|:---|
| Bluetooth | 5.2 LE (opcional, desactivado por defecto) | Configuración inicial, alertas a familiar |
| USB | USB-C | Carga + transferencia de voces |
| NFC | 13.56 MHz | Lectura de sensor de glucosa |

**Nota:** El dispositivo **no tiene WiFi**. Opera 100% local para garantizar privacidad.

---

## Almacenamiento de Datos

| Tipo | Capacidad | Contenido |
|:---|:---|:---|
| Flash interna | 16 MB | Firmware, configuraciones |
| MicroSD | Hasta 32 GB | Voces familiares, recetas, rutinas |
| Buffer circular | 60 segundos | Última conversación (en RAM) |

---

## Condiciones Ambientales

| Parámetro | Rango |
|:---|:---|
| Temperatura operación | 0°C a 40°C |
| Temperatura almacenamiento | -20°C a 50°C |
| Humedad | 10% a 90% (sin condensación) |
| Altitud | Hasta 3000 m |

---

## Certificaciones Objetivo

| Norma | Descripción |
|:---|:---|
| IEC 60601-1 | Seguridad eléctrica (equipos médicos) |
| IEC 60601-1-2 | Compatibilidad electromagnética |
| ISO 10993-5 | Biocompatibilidad (citotoxicidad) |
| ISO 10993-10 | Biocompatibilidad (irritación, sensibilización) |
| IP54 | Protección contra polvo y agua |
| RoHS | Libre de sustancias peligrosas |
| CE | Mercado europeo |
| FCC | Mercado estadounidense |

---

## Componentes y Costos

| Componente | Modelo | Costo (USD) |
|:---|:---|:---|
| MCU | ESP32-S3 | 3.50 |
| Cámara | OV2640 | 5.00 |
| Giroscopio | MPU-6050 | 2.00 |
| NFC | PN532 | 4.00 |
| Micrófono | SPH0645LM4H | 1.50 |
| Altavoz | Receptor balanceado | 2.00 |
| Batería | 250 mAh Li-Po | 3.00 |
| MicroSD | 32 GB (clase 10) | 5.00 |
| PCB + componentes pasivos | - | 5.00 |
| Carcasa | Silicona + ABS | 3.00 |
| **Total** | | **34.00** |

**Precio de venta estimado:** USD 99  
**Margen bruto:** 65%

---

## Lo Que No Tiene

- ❌ No tiene pantalla
- ❌ No tiene WiFi
- ❌ No tiene GPS (usa geocerca por tiempo, no por ubicación)
- ❌ No tiene botones físicos
- ❌ No tiene Bluetooth activo por defecto

---

## Conclusión

El Asistente IA Geriátrico es **simple por fuera, complejo por dentro**. Usa componentes estándar, baratos y probados, integrados en una arquitectura que prioriza autonomía, privacidad y facilidad de uso.

Su costo permite un precio accesible (sub-100 USD) sin sacrificar funcionalidad.

---

*Documento v1.0 - Marzo 2026*
*Asistente IA Geriátrico - Mackiber Labs*
