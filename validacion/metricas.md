# Métricas de Validación - Asistente IA Geriátrico

## Descripción

Este documento define las métricas clave utilizadas para validar el rendimiento, efectividad y aceptación del Asistente IA Geriátrico en estudios clínicos y de usabilidad.

---

## Categorías de Métricas

### 1. Métricas de Rendimiento Técnico
### 2. Métricas de Efectividad Clínica
### 3. Métricas de Usabilidad
### 4. Métricas de Aceptación
### 5. Métricas de Impacto en Cuidadores

---

## 1. Métricas de Rendimiento Técnico

### 1.1 Detección de Caídas
| Métrica | Definición | Objetivo | Método |
|:---|:---|:---|:---|
| **Sensibilidad** | % de caídas reales detectadas | > 95% | Simulación controlada |
| **Especificidad** | % de no-caídas correctamente clasificadas | > 98% | Registro continuo |
| **Tiempo de detección** | Tiempo entre caída y alerta | < 5 seg | Simulación |
| **Falsos positivos** | Alertas sin caída real | < 1 por semana | Registro |

### 1.2 Lectura de Glucosa
| Métrica | Definición | Objetivo | Método |
|:---|:---|:---|:---|
| **Tasa de éxito de lectura** | % de intentos exitosos | > 95% | Pruebas con sensor |
| **Precisión** | Diferencia vs glucómetro de referencia | < 10 mg/dL | Comparación |
| **Tiempo de lectura** | Tiempo desde acercamiento hasta resultado | < 5 seg | Medición directa |
| **Alerta de hipoglucemia** | % de eventos <70 mg/dL alertados | 100% | Registro |

### 1.3 Reconocimiento Facial
| Métrica | Definición | Objetivo | Método |
|:---|:---|:---|:---|
| **Precisión** | % de reconocimientos correctos | > 90% | Prueba con 10 fotos |
| **Tasa de falsos positivos** | Reconocer a alguien que no es | < 5% | Prueba cruzada |
| **Tiempo de reconocimiento** | Desde captura hasta resultado | < 3 seg | Medición directa |

### 1.4 Comprensión de Voz
| Métrica | Definición | Objetivo | Método |
|:---|:---|:---|:---|
| **Tasa de acierto** | % de comandos correctamente interpretados | > 90% | Prueba con 20 comandos |
| **Tasa de falsos positivos** | Activar sin comando | < 1 por día | Registro |
| **Tiempo de respuesta** | Desde comando hasta acción | < 2 seg | Medición directa |

### 1.5 Autonomía de Batería
| Métrica | Definición | Objetivo | Método |
|:---|:---|:---|:---|
| **Autonomía normal** | Días con uso normal | > 7 días | Prueba de campo |
| **Autonomía modo crítico** | Días con funciones mínimas | > 30 días | Simulación |
| **Tiempo de carga** | Desde 0% a 100% | < 2 horas | Medición directa |

---

## 2. Métricas de Efectividad Clínica

### 2.1 Reducción de Caídas
| Métrica | Definición | Objetivo | Método |
|:---|:---|:---|:---|
| **Tasa de caídas** | Caídas por paciente-mes | Reducción > 30% | Comparación pre/post |
| **Caídas con lesión** | Caídas que requieren atención médica | Reducción > 40% | Registro |
| **Tiempo entre caídas** | Días promedio sin caídas | Aumento > 20% | Análisis de supervivencia |

### 2.2 Control de Glucosa
| Métrica | Definición | Objetivo | Método |
|:---|:---|:---|:---|
| **Tiempo en rango** | % del día con glucosa 70-180 mg/dL | Aumento > 15% | Registro continuo |
| **Hipoglucemias** | Eventos <70 mg/dL por semana | Reducción > 30% | Registro |
| **Hipoglucemias graves** | Eventos <54 mg/dL por mes | Reducción > 50% | Registro |
| **HbA1c** | Hemoglobina glicosilada (3 meses) | Reducción > 0.5% | Análisis de sangre |

### 2.3 Adherencia a Medicación
| Métrica | Definición | Objetivo | Método |
|:---|:---|:---|:---|
| **Tasa de cumplimiento** | % de dosis tomadas correctamente | > 90% | Registro + confirmación |
| **Olvidos** | Dosis omitidas por semana | Reducción > 50% | Comparación pre/post |
| **Duplicaciones** | Dosis tomadas dos veces | Reducción > 80% | Registro |

### 2.4 Calidad de Vida
| Métrica | Definición | Objetivo | Método |
|:---|:---|:---|:---|
| **EQ-5D-5L** | Escala de calidad de vida | Mejoría > 0.1 | Cuestionario |
| **WHOQOL-OLD** | Calidad de vida en adultos mayores | Mejoría > 10% | Cuestionario |
| **Autonomía percibida** | Escala 1-10 | Mejoría > 2 puntos | Entrevista |

---

## 3. Métricas de Usabilidad

### 3.1 Facilidad de Uso
| Métrica | Definición | Objetivo | Método |
|:---|:---|:---|:---|
| **SUS Score** | System Usability Scale | > 70 | Cuestionario |
| **Tiempo de aprendizaje** | Días hasta uso autónomo | < 7 días | Observación |
| **Tasa de errores** | Errores por semana | < 2 | Registro |
| **Necesidad de ayuda** | Solicitudes de asistencia por semana | < 3 | Registro |

### 3.2 Confort
| Métrica | Definición | Objetivo | Método |
|:---|:---|:---|:---|
| **Tiempo de uso diario** | Horas con el dispositivo puesto | > 12 h | Registro |
| **Molestias reportadas** | Eventos de incomodidad | < 1 por semana | Cuestionario |
| **Aceptación cosmética** | Escala 1-10 | > 7 | Entrevista |

---

## 4. Métricas de Aceptación

### 4.1 Aceptación por el Usuario
| Métrica | Definición | Objetivo | Método |
|:---|:---|:---|:---|
| **Tasa de uso** | % de días que lo usa | > 80% | Registro |
| **Tasa de abandono** | % que deja de usarlo | < 10% | Registro |
| **Recomendación** | % que lo recomendaría | > 80% | Encuesta |
| **Net Promoter Score** | NPS (0-10) | > 50 | Encuesta |

### 4.2 Aceptación por el Cuidador
| Métrica | Definición | Objetivo | Método |
|:---|:---|:---|:---|
| **Satisfacción** | Escala 1-10 | > 8 | Encuesta |
| **Reducción de carga** | Escala de Zarit | Reducción > 20% | Cuestionario |
| **Tranquilidad percibida** | Escala 1-10 | Mejoría > 3 puntos | Entrevista |

---

## 5. Métricas de Impacto en Cuidadores

| Métrica | Definición | Objetivo | Método |
|:---|:---|:---|:---|
| **Horas de cuidado diario** | Tiempo dedicado al paciente | Reducción > 2 h | Registro |
| **Calidad del sueño** | Escala de Pittsburgh | Mejoría > 2 puntos | Cuestionario |
| **Estrés percibido** | Escala PSS | Reducción > 20% | Cuestionario |
| **Ausencias laborales** | Días perdidos por mes | Reducción > 50% | Registro |

---

## Tabla Resumen de Objetivos

| Categoría | Métrica Clave | Objetivo |
|:---|:---|:---|
| Técnica | Sensibilidad caídas | > 95% |
| Técnica | Precisión glucosa | < 10 mg/dL |
| Técnica | Autonomía batería | > 7 días |
| Clínica | Reducción caídas | > 30% |
| Clínica | Reducción hipoglucemias | > 30% |
| Clínica | Adherencia medicación | > 90% |
| Usabilidad | SUS Score | > 70 |
| Usabilidad | Tasa de uso | > 80% |
| Aceptación | NPS | > 50 |
| Cuidador | Reducción carga (Zarit) | > 20% |

---

## Recolección de Datos

| Métrica | Frecuencia | Método |
|:---|:---|:---|
| Datos técnicos | Continua | Registro automático |
| Eventos clínicos | Diaria | App + confirmación |
| Cuestionarios | Mensual | App / papel |
| Entrevistas | Trimestral | Presencial |

---

## Análisis Estadístico

```python
# Pseudocódigo para análisis principal
import pandas as pd
import scipy.stats as stats

# Cargar datos
df = pd.read_csv('estudio.csv')

# Comparación pre-post (variables continuas)
t_stat, p_value = stats.ttest_rel(df['caidas_pre'], df['caidas_post'])

# Comparación de proporciones (variables categóricas)
tabla = pd.crosstab(df['grupo'], df['mejoria'])
chi2, p, dof, expected = stats.chi2_contingency(tabla)

# Tamaño del efecto
cohen_d = (df['caidas_pre'].mean() - df['caidas_post'].mean()) / \
           ((df['caidas_pre'].std() + df['caidas_post'].std()) / 2)
```

---

## Reporte de Resultados

Los resultados se presentarán en:

- Tablas de características basales
- Gráficos de evolución temporal
- Diagramas de caja para comparaciones
- Matrices de confusión para clasificadores
- Análisis de subgrupos (por edad, condición, etc.)

---

*Documento v1.0 - Marzo 2026*
*Asistente IA Geriátrico - Mackiber Labs*
