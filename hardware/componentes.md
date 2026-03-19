# Componentes - Asistente IA Geriátrico

## Lista de Componentes Principales

### Procesador y Memoria
| Componente | Modelo | Fabricante | Función |
|:---|:---|:---|:---|
| MCU Principal | ESP32-S3 | Espressif | Procesamiento central, conectividad |
| Memoria Flash | W25Q128JV | Winbond | 16 MB para firmware |
| MicroSD | Clase 10, 32 GB | Genérico | Almacenamiento de voces, recetas |
| Regulador voltaje | XC6206P332MR | Torex | 3.3V para componentes |

### Sensores
| Componente | Modelo | Fabricante | Función |
|:---|:---|:---|:---|
| Cámara | OV2640 | OmniVision | Captura de imágenes, lectura de texto |
| Giroscopio/Acelerómetro | MPU-6050 | TDK InvenSense | Detección de caídas, orientación |
| Lector NFC | PN532 | NXP | Lectura de sensor FreeStyle Libre 2 |
| Micrófono MEMS | SPH0645LM4H | Knowles | Captura de voz del usuario |
| Altavoz | Receptor balanceado | Knowles / Sonion | Reproducción de voz familiar |

### Batería y Alimentación
| Componente | Modelo | Fabricante | Función |
|:---|:---|:---|:---|
| Batería Li-Po | 250 mAh | Genérico (con protección) | Almacenamiento de energía |
| Cargador USB | TP4056 | NanJing Top Power | Gestión de carga |
| Protección batería | DW01A + 8205 | Genérico | Protección sobrecarga/sobretensión |
| Interruptor de carga | SI2302 | Vishay | Control de encendido/apagado |

### Conectores y Pasivos
| Componente | Especificación | Función |
|:---|:---|:---|
| Conector USB-C | 16 pines | Carga + transferencia de datos |
| Resistores | 0402 (1%) | Varios |
| Capacitores | 0402 (X7R) | Filtrado y desacoplo |
| Inductores | 1 µH, 2.2 µH | Filtros conmutados |
| LEDs | 0603 (verde/rojo) | Indicadores de estado (ocultos) |

---

## Proveedores Recomendados

| Componente | Proveedor | Código | Precio aprox (1-10 uds) |
|:---|:---|:---|:---|
| ESP32-S3 | Mouser / DigiKey | 356-ESP32-S3 | USD 3.50 |
| OV2640 | Aliexpress / LCSC | OV2640 | USD 5.00 |
| MPU-6050 | LCSC | C108985 | USD 2.00 |
| PN532 | LCSC | C12456 | USD 4.00 |
| SPH0645LM4H | Mouser | 721-SPH0645LM4H | USD 1.50 |
| Receptor balanceado | DigiKey / Sonion | Consultar | USD 2.00 |
| Batería 250 mAh | Aliexpress | 602040 | USD 3.00 |
| MicroSD 32GB | Genérico | Clase 10 | USD 5.00 |

---

## Alternativas y Segundas Fuentes

### Procesador
| Alternativa | Ventajas | Desventajas |
|:---|:---|:---|
| nRF5340 | Menor consumo, seguridad | Mayor costo, menos conocido |
| STM32L4 | Muy bajo consumo | Menor capacidad de procesamiento |
| Raspberry Pi RP2040 | Muy económico | Sin conectividad inalámbrica |

### Giroscopio
| Alternativa | Ventajas | Desventajas |
|:---|:---|:---|
| MPU-9250 | Incluye magnetómetro | Mayor consumo |
| ICM-20948 | Más preciso | Más caro |
| LSM6DS3 | Bajo consumo | Menos sensibilidad |

### Lector NFC
| Alternativa | Ventajas | Desventajas |
|:---|:---|:---|
| MFRC522 | Muy económico | Menor alcance |
| PN5180 | Mayor potencia | Más caro, más grande |

---

## Especificaciones de Compra

### Cantidades mínimas (MOQ)
| Componente | MOQ (muestreo) | MOQ (producción) |
|:---|:---|:---|
| ESP32-S3 | 1 | 1000 |
| OV2640 | 1 | 500 |
| MPU-6050 | 1 | 1000 |
| PN532 | 1 | 500 |
| SPH0645LM4H | 1 | 1000 |
| Batería | 1 | 100 |
| MicroSD | 1 | 100 |

### Tiempos de entrega
| Componente | Tiempo (muestreo) | Tiempo (producción) |
|:---|:---|:---|
| Semiconductores | 1-2 semanas | 4-6 semanas |
| Baterías | 1 semana | 3-4 semanas |
| PCB | 1 semana | 2-3 semanas |
| Carcasa | 2-3 semanas | 4-5 semanas |

---

## Costos por Volumen

| Componente | 10 uds | 100 uds | 1000 uds | 10.000 uds |
|:---|:---|:---|:---|:---|
| ESP32-S3 | 3.50 | 2.80 | 2.20 | 1.80 |
| OV2640 | 5.00 | 4.00 | 3.00 | 2.50 |
| MPU-6050 | 2.00 | 1.50 | 1.20 | 1.00 |
| PN532 | 4.00 | 3.00 | 2.50 | 2.00 |
| Micrófono | 1.50 | 1.20 | 1.00 | 0.80 |
| Altavoz | 2.00 | 1.60 | 1.30 | 1.10 |
| Batería | 3.00 | 2.50 | 2.00 | 1.60 |
| MicroSD | 5.00 | 4.00 | 3.00 | 2.50 |
| PCB | 5.00 | 2.00 | 1.00 | 0.50 |
| Carcasa | 3.00 | 2.00 | 1.50 | 1.00 |
| **TOTAL** | **34.00** | **24.60** | **18.70** | **14.80** |

---

## Gestión de Inventario

### Códigos de parte internos
| Componente | Código Mackiber |
|:---|:---|
| ESP32-S3 | AIG-MCU-001 |
| OV2640 | AIG-CAM-001 |
| MPU-6050 | AIG-IMU-001 |
| PN532 | AIG-NFC-001 |
| SPH0645LM4H | AIG-MIC-001 |
| Receptor balanceado | AIG-SPK-001 |
| Batería 250 mAh | AIG-BAT-001 |
| PCB principal | AIG-PCB-001 |

### Trazabilidad
- Cada lote de componentes tiene número de lote trazable
- Se mantiene registro de fecha de compra, proveedor y certificados
- Almacenamiento en condiciones controladas (temperatura, humedad)

---

## Notas Importantes

| Componente | Nota |
|:---|:---|
| Batería | Debe tener protección contra sobrecarga incorporada |
| MicroSD | Formatear en FAT32 antes de uso |
| Cámara | Requiere lente adicional (foco fijo) |
| Altavoz | Necesita caja acústica sellada para buena respuesta |

---

## Proveedores Preferidos

| Región | Proveedor | Tipo |
|:---|:---|:---|
| Global | Mouser Electronics | Semiconductores |
| Global | DigiKey | Semiconductores |
| Asia | LCSC | Componentes chinos |
| Asia | Aliexpress | Baterías, cámaras |
| Local (Chile) | Sosam | Componentes varios |
| Local (Chile) | MercadoLibre | Prototipado rápido |

---

*Documento v1.0 - Marzo 2026*
*Asistente IA Geriátrico - Mackiber Labs*
