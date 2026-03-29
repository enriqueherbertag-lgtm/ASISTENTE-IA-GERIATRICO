# Asistente IA Geriátrico

[![License: CC BY-NC 4.0](https://img.shields.io/badge/License-CC_BY--NC_4.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc/4.0/)

**Dispositivo intra-auricular de asistencia integral para adultos mayores con Alzheimer, demencia senil, retinopatía diabética y diabetes tipo 2.**

No es un audífono. Es un compañero que habla con la voz de tu familia.

---

## ¿Qué Hace?

| Función | Descripción |
|:---|:---|
| **Visión artificial** | Lee etiquetas, fechas de vencimiento, instrucciones de medicamentos en voz alta. |
| **Monitoreo de glucosa** | Se conecta al FreeStyle Libre 2 y alerta si hay riesgo de hipoglucemia. |
| **Detección de caídas** | Giroscopio detecta inclinaciones peligrosas y emite alerta por voz. |
| **Memoria afectiva** | Recuerda recetas, rutinas y conversaciones recientes. |
| **Voz familiar** | Las alertas y recordatorios se reproducen con la voz grabada de un ser querido. |
| **Orientación temporal** | "Son las 3 de la mañana. Es de noche. Mejor sigue durmiendo." |
| **Reconocimiento de personas** | "Es tu hijo Juan. Vino a visitarte." |
| **Geocerca virtual** | Si se aleja de casa, alerta y pregunta si quiere avisar a alguien. |
| **Buffer conversacional** | "¿Qué estaba diciendo?" → "Que querías flan." |

---

## Para Quién

- Personas con **Alzheimer o demencia senil**
- Adultos mayores con **retinopatía diabética** (dificultad para leer)
- Pacientes con **diabetes tipo 2** que necesitan monitoreo continuo
- Familias que cuidan a un ser querido
- Residencias geriátricas que buscan dignidad, no solo control

---

## Por Qué Funciona

| Principio | Aplicación |
|:---|:---|
| **Memoria emocional** | La voz de un hijo activa redes neuronales que ninguna otra voz activa. |
| **Anclajes sensoriales** | Las recetas, rutinas y lugares conocidos son lo último que se pierde. |
| **Reducción de ansiedad** | Escuchar a un ser querido disminuye cortisol y mejora orientación. |
| **Tecnología invisible** | El usuario solo vive. El dispositivo se adapta. |

---

## Lo Que No Hace

-  No requiere internet
-  No envía datos a la nube
-  No necesita que el nieto configure nada
-  No reemplaza al cuidador
-  No cuesta más de 100 USD

---

## Tecnología (Toda Existente)

| Componente | Precio | Función |
|:---|:---|:---|
| Cámara OV2640 | USD 5 | Leer etiquetas, reconocer caras |
| Giroscopio MPU-6050 | USD 2 | Detectar caídas |
| Lector NFC | USD 10 | Conectar a FreeStyle Libre 2 |
| ESP32 / nRF53 | USD 5-8 | Procesamiento local |
| Batería | USD 3 | Autonomía 7 días |
| Altavoz miniatura | USD 2 | Voz clara y cálida |

**Costo total estimado: USD 85**

---

## Estructura del Repositorio

```
Asistente-IA-Geriatrico/
├── README.md
├── LICENSE
├── docs/
│   ├── vision-general.md
│   ├── problema-diseno.md
│   ├── publico-objetivo.md
│   └── guia-para-familias.md
├── diseno/
│   ├── filosofia.md
│   ├── anclajes-emocionales.md
│   ├── voz-familiar.md
│   └── buffer-contextual.md
├── hardware/
│   ├── especificaciones.md
│   ├── componentes.md
│   ├── camara-OV2640.md
│   ├── giroscopio-MPU6050.md
│   ├── nfc-freestyle.md
│   ├── bateria.md
│   └── carcasa-intrauricular.md
├── firmware/
│   ├── README.md
│   ├── core/
│   ├── sensores/
│   ├── memoria/
│   └── salida/
├── software/
│   ├── grabacion-voces/
│   └── configuracion-inicial/
├── memoria-afectiva/
│   ├── recetas-familiares/
│   ├── voces/
│   └── rutinas/
├── casos-de-uso/
│   ├── ana-78-anos.md
│   ├── jose-alzheimer.md
│   └── maria-diabetes.md
├── validacion/
│   ├── estudio-usabilidad.md
│   ├── metricas.md
│   └── testimonios-simulados.md
├── negocio/
│   ├── modelo-comercial.md
│   ├── proyeccion.md
│   ├── alianzas.md
│   └── ruta-regulatoria.md
└── comunidad/
    ├── CONTRIBUTING.md
    └── CODE_OF_CONDUCT.md
```

---

## Autor

**Enrique Aguayo H.**  
Investigador independiente, Mackiber Labs  
Contacto: eaguayo@migst.cl  
ORCID: 0009-0004-4615-6825  
GitHub: [@enriqueherbertag-lgtm](https://github.com/enriqueherbertag-lgtm)

---

> *"No es un dispositivo para viejos. Es un puente hacia lo que nunca deberían perder: su historia, su gente, su voz."*
