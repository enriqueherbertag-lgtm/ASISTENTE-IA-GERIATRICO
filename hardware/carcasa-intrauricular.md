# Carcasa Intrauricular - Asistente IA Geriátrico

## Descripción General

La carcasa del Asistente IA Geriátrico está diseñada para ser **cómoda, discreta y segura**, similar a un audífono de gama alta. Debe alojar todos los componentes electrónicos en un volumen reducido y garantizar su funcionamiento en condiciones cotidianas.

---

## Especificaciones de Diseño

| Parámetro | Especificación |
|:---|:---|
| Formato | Intra-auricular (BTE opcional) |
| Dimensiones | 20 mm × 15 mm × 10 mm |
| Peso total | < 8 gramos |
| Material | Silicona hipoalergénica + ABS médico |
| Grado de protección | IP54 (polvo + salpicaduras) |
| Color | Beige / Gris (piel) |
| Sujeción | Almohadillas intercambiables (3 tamaños) |
| Botones | Ninguno (todo por voz) |
| Indicador LED | Pequeño, oculto tras la rejilla |

---

## Partes de la Carcasa

```
┌─────────────────────────────────────┐
│         REJILLA DE ENTRADA          │ (micrófono)
├─────────────────────────────────────┤
│   CARCASA PRINCIPAL (ABS)           │
│   ┌─────────────────────────────┐   │
│   │       PCB PRINCIPAL          │   │
│   │  • ESP32-S3                  │   │
│   │  • MPU-6050                  │   │
│   │  • Conectores                │   │
│   └─────────────────────────────┘   │
│   ┌─────────────────────────────┐   │
│   │       CÁMARA OV2640          │   │
│   │       (lente al frente)       │   │
│   └─────────────────────────────┘   │
│   ┌─────────────────────────────┐   │
│   │       BATERÍA 250 mAh        │   │
│   │       (en la parte posterior) │   │
│   └─────────────────────────────┘   │
├─────────────────────────────────────┤
│   ALMOHADILLA DE SILICONA           │
│   (intercambiable, 3 tamaños)       │
└─────────────────────────────────────┘
```

---

## Materiales

### Carcasa Principal (ABS médico)
| Propiedad | Valor |
|:---|:---|
| Material | Acrilonitrilo butadieno estireno (grado médico) |
| Biocompatibilidad | ISO 10993-5 (no citotóxico) |
| Resistencia impacto | 20 kJ/m² |
| Temperatura uso | -20°C a 80°C |
| Color | Beige / Gris / Negro |

### Almohadillas (Silicona hipoalergénica)
| Propiedad | Valor |
|:---|:---|
| Material | Silicona de platino (grado médico) |
| Biocompatibilidad | ISO 10993-10 (no irritante) |
| Dureza | Shore A 30 (blanda) |
| Tamaños | S (10 mm), M (12 mm), L (14 mm) |

---

## Distribución de Componentes

| Componente | Ubicación | Observaciones |
|:---|:---|:---|
| PCB principal | Centro de la carcasa | Doble cara, componentes SMD |
| ESP32-S3 | En PCB | Bajo consumo, cerca de antena |
| Cámara OV2640 | Frente | Lente orientada hacia adelante |
| MPU-6050 | En PCB | Cerca del centro de masa |
| Micrófono | Rejilla superior | Exterior, protegido de viento |
| Altavoz | Interior, hacia canal auditivo | Sellado acústico |
| Batería | Parte posterior | Distribuye peso |
| Antena WiFi/BT | Perímetro | Integrada en carcasa |

---

## Gestión Térmica

| Componente | Calor generado | Disipación |
|:---|:---|:---|
| ESP32-S3 | 0.3 W | Por conducción a carcasa |
| Cámara | 0.2 W | Uso intermitente, no acumula |
| Batería | 0.1 W (carga) | Lejos de componentes sensibles |
| Altavoz | 0.1 W | Pulsos cortos |

**Temperatura máxima en piel:** < 41°C (norma IEC 60601-1)

---

## Impermeabilización (IP54)

| Zona | Protección |
|:---|:---|
| Unión carcasa | Junta tórica de silicona |
| Micrófono | Malla hidrofóbica |
| Altavoz | Membrana acústica impermeable |
| Botones | No hay (menos puntos de entrada) |
| USB-C | Tapa de goma sellada |

---

## Fabricación

### Método recomendado
- **Prototipos:** Impresión 3D (resina biocompatible)
- **Series pequeñas:** Moldeo por inyección (ABS médico)
- **Series grandes:** Moldeo por inyección automatizado

### Proveedores sugeridos
| Región | Proveedor | Especialidad |
|:---|:---|:---|
| Chile / LatAm | 3D Printing Chile | Prototipado rápido |
| China | JLCPCB | Moldeo por inyección |
| Global | Protolabs | Series medianas |

---

## Normativas Aplicables

| Norma | Descripción |
|:---|:---|
| ISO 10993-5 | Biocompatibilidad (citotoxicidad) |
| ISO 10993-10 | Biocompatibilidad (irritación) |
| IEC 60601-1 | Seguridad eléctrica |
| IP54 | Protección contra polvo y agua |
| RoHS | Libre de sustancias peligrosas |

---

## Problemas Comunes y Soluciones

| Problema | Causa | Solución |
|:---|:---|:---|
| Molestia al usar | Almohadilla inadecuada | Probar otro tamaño |
| | Puntos de presión | Rediseñar ergonomía |
| Se cae | Poco agarre | Revisar almohadilla o añadir gancho |
| Entra agua | Juntas mal selladas | Revisar diseño IP54 |
| Se ve muy notorio | Color incorrecto | Usar color piel del usuario |
| Acoplamiento acústico | Altavoz mal sellado | Mejorar cámara acústica |

---

## Referencias

1. ISO 10993-5:2022. "Biological evaluation of medical devices — Part 5: Tests for in vitro cytotoxicity".
2. ISO 10993-10:2021. "Tests for skin sensitization".
3. IEC 60529:2023. "Degrees of protection provided by enclosures (IP Code)".
4. ASTM F2020-23. "Standard Practice for Design, Manufacture, and Installation of Medical Devices".

---

*Documento v1.0 - Marzo 2026*
*Asistente IA Geriátrico - Mackiber Labs*
