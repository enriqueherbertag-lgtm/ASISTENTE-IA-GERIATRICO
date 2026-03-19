# Estudio de Usabilidad - Asistente IA Geriátrico

## Objetivo del Estudio

Evaluar la facilidad de uso, aceptación y efectividad del Asistente IA Geriátrico en un grupo de adultos mayores y sus cuidadores, previo a un estudio clínico más amplio.

---

## Diseño del Estudio

| Parámetro | Descripción |
|:---|:---|
| **Tipo** | Estudio observacional, transversal |
| **Duración** | 4 semanas |
| **Participantes** | 20 adultos mayores + 10 cuidadores |
| **Centros** | 2 centros de día para adultos mayores |
| **Método** | Cuestionarios + entrevistas + observación |

---

## Criterios de Inclusión

### Para adultos mayores
- Edad ≥ 70 años
- Vive solo o con cuidador
- Alguna de estas condiciones:
  - Diabetes tipo 2
  - Deterioro cognitivo leve
  - Retinopatía diabética
  - Riesgo de caídas
- Capacidad de dar consentimiento informado

### Para cuidadores
- Familiar o profesional que cuida al adulto mayor ≥ 6 meses
- Dispuesto a participar en el estudio

---

## Variables Evaluadas

### Usabilidad
| Variable | Método |
|:---|:---|
| Facilidad de colocación | Observación, tiempo |
| Comprensión de instrucciones | Cuestionario |
| Frecuencia de uso | Registro automático |
| Errores de uso | Observación |
| Satisfacción general | Escala 1-10 |

### Funcionalidad
| Variable | Método |
|:---|:---|
| Detección de caídas | Simulación controlada |
| Lectura de etiquetas | Prueba con 5 productos |
| Recordatorios | Cumplimiento registrado |
| Reconocimiento facial | Precisión en 10 fotos |
| Comprensión de voz | Porcentaje de aciertos |

### Aceptación
| Variable | Método |
|:---|:---|
| Uso diario | Registro automático |
| Abandono | Tasa de deserción |
| Recomendación a otros | Pregunta directa |
| Comparación con otras ayudas | Entrevista |

---

## Protocolo

### Semana 0: Selección y entrenamiento
- Evaluación de candidatos
- Firma de consentimiento
- Explicación del dispositivo (30 min)
- Prueba inicial supervisada

### Semana 1-4: Uso en domicilio
- Dispositivo configurado para cada participante
- Registro automático de eventos
- Llamada de seguimiento semanal (15 min)
- Diario de uso para el cuidador

### Semana 4: Evaluación final
- Cuestionario de satisfacción
- Entrevista semiestructurada
- Descarga y análisis de datos
- Entrega de informe individual

---

## Instrumentos de Evaluación

### Cuestionario de Usabilidad (adaptado de SUS)
| Pregunta | Escala (1-5) |
|:---|:---|
| Me gustaría usar este dispositivo con frecuencia | 1-5 |
| El dispositivo es innecesariamente complejo | 1-5 |
| El dispositivo es fácil de usar | 1-5 |
| Necesitaría ayuda técnica para usarlo | 1-5 |
| Las funciones están bien integradas | 1-5 |
| Hay demasiada inconsistencia | 1-5 |
| La mayoría aprendería a usarlo rápidamente | 1-5 |
| El dispositivo es incómodo de usar | 1-5 |
| Me siento seguro usándolo | 1-5 |
| Necesito aprender muchas cosas antes de usarlo | 1-5 |

### Entrevista al cuidador
- ¿Qué fue lo más útil?
- ¿Qué fue lo más difícil?
- ¿Lo recomendaría?
- ¿Qué cambiaría?
- ¿Cómo cambió su rutina?

---

## Resultados Esperados

| Variable | Objetivo |
|:---|:---|
| Tasa de uso diario | > 80% |
| Tiempo de colocación | < 2 minutos |
| Precisión en detección de caídas | > 90% |
| Precisión en lectura de etiquetas | > 95% |
| Satisfacción del usuario | > 8/10 |
| Satisfacción del cuidador | > 8/10 |
| Tasa de abandono | < 10% |

---

## Análisis de Datos

```python
# Pseudocódigo para análisis
import pandas as pd
import matplotlib.pyplot as plt

# Cargar datos
df = pd.read_csv('usabilidad.csv')

# Estadísticas descriptivas
mean_age = df['edad'].mean()
percent_male = (df['genero'] == 'M').mean() * 100

# Usabilidad por grupo
usabilidad = df.groupby('condicion')['puntuacion_sus'].mean()

# Gráfico de satisfacción
plt.bar(df['id'], df['satisfaccion'])
plt.title('Satisfacción por participante')
plt.show()
```

---

## Consideraciones Éticas

- Aprobación por comité de ética institucional
- Consentimiento informado para todos los participantes
- Posibilidad de retiro en cualquier momento
- Confidencialidad de los datos
- No se realizan intervenciones médicas (solo observación)

---

## Cronograma

| Actividad | Duración | Fecha |
|:---|:---|:---|
| Aprobación ética | 2 meses | Ene-Feb 2026 |
| Reclutamiento | 1 mes | Mar 2026 |
| Estudio de campo | 1 mes | Abr 2026 |
| Análisis de datos | 2 semanas | May 2026 |
| Informe final | 2 semanas | Jun 2026 |

---

## Presupuesto Estimado

| Concepto | Costo (USD) |
|:---|:---|
| Dispositivos (20 unidades) | 2,000 |
| Incentivos para participantes | 1,000 |
| Personal (coordinador, asistente) | 3,000 |
| Análisis de datos | 1,000 |
| Informe y publicación | 500 |
| **TOTAL** | **7,500** |

---

## Resultados Preliminares (simulados)

De un piloto anterior con 5 usuarios:

| Usuario | Edad | Condición | Uso diario | Satisfacción |
|:---|:---|:---|:---|:---|
| A1 | 78 | Diabetes | 98% | 9 |
| A2 | 82 | Alzheimer | 85% | 8 |
| A3 | 74 | Retinopatía | 100% | 10 |
| A4 | 80 | Riesgo caídas | 92% | 9 |
| A5 | 76 | Deterioro cognitivo | 88% | 8 |

**Comentarios:**
- "Me gusta que me hable con la voz de mi hija"
- "Al principio me costó acostumbrarme, ahora no me lo saco"
- "Pude leer las etiquetas del supermercado por primera vez en años"

---

## Conclusión

El estudio de usabilidad permitirá ajustar el diseño del dispositivo y su interfaz antes de un ensayo clínico más amplio, asegurando que el producto final sea **aceptado, comprendido y valorado** por sus usuarios finales.

---

*Documento v1.0 - Marzo 2026*
*Asistente IA Geriátrico - Mackiber Labs*
