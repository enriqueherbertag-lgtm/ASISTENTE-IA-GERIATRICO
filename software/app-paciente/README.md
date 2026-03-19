# App Paciente / Cuidador - Asistente IA Geriátrico

## Descripción General

La App Paciente (y su versión para cuidador) es una aplicación móvil sencilla diseñada para que el adulto mayor o su cuidador puedan consultar información básica del dispositivo, recibir recordatorios y mantener comunicación con el médico.

La interfaz está diseñada para ser **extremadamente simple**, con letra grande, contrastes altos y navegación mínima.

---

## Funcionalidades Principales

### Para el Paciente (Modo Sencillo)
| Función | Descripción |
|:---|:---|
| **Estado del dispositivo** | Batería, conectividad, última alerta |
| **Recordatorios** | Medicación, agua, comidas (con voz) |
| **Mensajes del médico** | Reproducción de mensajes de voz recibidos |
| **Llamada de emergencia** | Botón grande para contactar al cuidador/familiar |
| **Mi información** | Datos básicos (nombre, médico, contacto) |

### Para el Cuidador (Modo Completo)
| Función | Descripción |
|:---|:---|
| **Monitoreo básico** | Estado del dispositivo, batería, alertas recientes |
| **Historial de eventos** | Caídas, lecturas de glucosa, comandos de voz |
| **Configuración simple** | Ajuste de recordatorios, verificación de funcionamiento |
| **Contacto con médico** | Envío de mensajes, solicitud de ayuda |

---

## Pantallas Principales

```
App Paciente
├── Pantalla de Inicio
│   ├── Estado del dispositivo (batería, conexión)
│   ├── Última alerta / recordatorio
│   └── Botón grande de emergencia
├── Recordatorios
│   ├── Lista de recordatorios del día
│   └── Opción de marcar como cumplidos
├── Mensajes
│   ├── Lista de mensajes del médico
│   └── Reproducción de audio
├── Mi Perfil
│   ├── Información personal
│   ├── Contacto del médico
│   └── Configuración básica
└── (Modo Cuidador) Panel de Control
    ├── Estado detallado
    ├── Historial
    └── Configuración avanzada
```

---

## Tecnologías Utilizadas

| Componente | Tecnología |
|:---|:---|
| Framework | React Native (iOS/Android) |
| Estado | Context API + hooks |
| Navegación | React Navigation (stack simple) |
| Notificaciones | Firebase Cloud Messaging |
| Base de datos local | AsyncStorage |
| Audio | React Native Audio |
| Accesibilidad | VoiceOver / TalkBack |

---

## Estructura del Proyecto

```
app-paciente/
├── src/
│   ├── assets/               # Imágenes, fuentes (grandes)
│   ├── components/           # Componentes simples
│   │   ├── BigButton/         # Botones grandes
│   │   ├── StatusCard/        # Tarjetas de estado
│   │   └── VoiceMessage/      # Mensajes de voz
│   ├── screens/               # Pantallas
│   │   ├── HomeScreen.js       # Pantalla principal
│   │   ├── RemindersScreen.js  # Recordatorios
│   │   ├── MessagesScreen.js   # Mensajes del médico
│   │   ├── ProfileScreen.js    # Perfil
│   │   └── CaregiverScreen.js  # Modo cuidador
│   ├── navigation/            # Navegación simple
│   ├── services/              # Servicios
│   │   ├── bluetooth.js       # Conexión con dispositivo
│   │   └── notifications.js   # Notificaciones locales
│   ├── utils/                  # Utilidades
│   │   ├── accessibility.js   # Configuración de accesibilidad
│   │   └── formatters.js      # Formateo de fechas, etc.
│   └── constants/              # Constantes
│       ├── colors.js           # Colores de alto contraste
│       └── strings.js          # Textos grandes
├── App.js
├── index.js
├── package.json
└── README.md
```

---

## Instalación y Configuración

### Requisitos previos
- Node.js 18+
- React Native CLI
- Android Studio / Xcode

### Pasos de instalación

```bash
# Clonar el repositorio
git clone https://github.com/enriqueherbertag-lgtm/Asistente-IA-Geriatrico.git
cd Asistente-IA-Geriatrico/software/app-paciente

# Instalar dependencias
npm install

# Ejecutar en Android
npm run android

# Ejecutar en iOS
npm run ios
```

---

## Accesibilidad

| Característica | Implementación |
|:---|:---|
| **Texto grande** | Fuente mínima 18pt, escalable |
| **Alto contraste** | Colores de alto contraste (negro/blanco) |
| **Botones grandes** | Área táctil mínima 60x60pt |
| **VoiceOver / TalkBack** | Etiquetas descriptivas en todos los elementos |
| **Modo simple** | Solo funciones esenciales, navegación mínima |
| **Confirmaciones** | Diálogos de confirmación para acciones importantes |

---

## Conexión con el Dispositivo

La app se conecta al Asistente IA Geriátrico mediante **Bluetooth Low Energy (BLE)** para:

| Función | Método |
|:---|:---|
| Leer estado | Batería, última conexión |
| Recibir alertas | Notificaciones locales desde el dispositivo |
| Configurar recordatorios | Envío de comandos vía BLE |
| Verificar funcionamiento | Prueba de altavoz, micrófono |

No requiere internet. Funciona incluso sin conexión a la nube.

---

## Notificaciones

| Tipo | Descripción |
|:---|:---|
| **Recordatorios** | Configurados localmente en el dispositivo |
| **Alertas del dispositivo** | Caídas, batería baja, desconexión |
| **Mensajes del médico** | Notificaciones al recibir nuevo mensaje |
| **Llamada de emergencia** | Acción directa desde botón grande |

---

## Pruebas de Usabilidad

La app ha sido diseñada siguiendo principios de **diseño para adultos mayores**:

- Pruebas con usuarios de 70+ años
- Tiempo de reacción considerado (esperas largas)
- Instrucciones por voz disponibles
- Retroalimentación táctil y sonora

```bash
# Pruebas unitarias
npm test

# Pruebas de usabilidad (manual)
npm run usability
```

---

## Construcción para Producción

```bash
# Android
cd android && ./gradlew bundleRelease

# iOS
cd ios && xcodebuild -workspace AppPaciente.xcworkspace -scheme AppPaciente -configuration Release
```

---

## Modo Cuidador (Cambio de Perfil)

La app incluye un **modo cuidador** accesible mediante:
- Código simple (ej: 1234)
- Pregunta de seguridad
- Botón oculto en configuración

En modo cuidador se muestran funciones adicionales sin cambiar la interfaz básica.

---

## Licencia

Este software está licenciado bajo Apache 2.0 con restricción comercial. Para uso comercial, contactar al autor.

---

## Contacto

**Enrique Aguayo H.**  
Mackiber Labs  
eaguayo@migst.cl

---

*Documento v1.0 - Marzo 2026*
