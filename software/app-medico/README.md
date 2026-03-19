# App Médico - Asistente IA Geriátrico

## Descripción General

La App Médico es una aplicación móvil (iOS/Android) diseñada para que los profesionales de la salud puedan monitorear a sus pacientes con el dispositivo Asistente IA Geriátrico, recibir alertas y ajustar configuraciones de forma remota.

---

## Funcionalidades Principales

### 1. Lista de Pacientes
- Vista general de todos los pacientes asignados
- Estado actual de cada paciente (batería, última conexión, alertas)
- Filtros por nivel de riesgo, última visita, etc.

### 2. Monitoreo en Tiempo Real
- Datos de glucosa actualizados
- Alertas de caídas
- Nivel de batería del dispositivo
- Estado de conectividad

### 3. Historial de Eventos
- Registro completo de caídas, lecturas de glucosa, comandos de voz
- Gráficos de tendencias (glucosa, actividad, caídas)
- Exportación de informes

### 4. Configuración de Pacientes
- Umbrales de alerta personalizados (glucosa, caídas)
- Grabación y asignación de voces familiares
- Programación de recordatorios
- Gestión de recetas familiares

### 5. Comunicación con el Paciente
- Envío de mensajes de voz al dispositivo
- Notificaciones push de alertas
- Llamada directa a emergencias desde la app

---

## Pantallas Principales

```
App Médico
├── Login / Autenticación
├── Dashboard Principal
│   ├── Resumen de pacientes
│   ├── Alertas activas
│   └── Accesos rápidos
├── Lista de Pacientes
│   ├── Búsqueda y filtros
│   └── Vista de cada paciente
├── Detalle del Paciente
│   ├── Información general
│   ├── Datos en tiempo real
│   ├── Historial de eventos
│   └── Configuración
└── Configuración de la App
    ├── Perfil del médico
    ├── Notificaciones
    └── Preferencias
```

---

## Tecnologías Utilizadas

| Componente | Tecnología |
|:---|:---|
| Framework | React Native (iOS/Android) |
| Estado | Redux Toolkit |
| Navegación | React Navigation |
| Gráficos | Victory Native / Recharts |
| Notificaciones | Firebase Cloud Messaging |
| Base de datos local | SQLite |
| Autenticación | JWT + Biometría |
| API | REST (conexión con servidor central) |

---

## Estructura del Proyecto

```
app-medico/
├── src/
│   ├── assets/               # Imágenes, fuentes
│   ├── components/           # Componentes reutilizables
│   │   ├── common/            # Botones, inputs, cards
│   │   ├── charts/            # Gráficos de glucosa, actividad
│   │   └── modals/            # Ventanas modales
│   ├── screens/               # Pantallas de la app
│   │   ├── LoginScreen.js
│   │   ├── DashboardScreen.js
│   │   ├── PatientListScreen.js
│   │   ├── PatientDetailScreen.js
│   │   └── SettingsScreen.js
│   ├── navigation/            # Configuración de navegación
│   ├── services/              # Servicios (API, Bluetooth, etc.)
│   │   ├── api.js
│   │   ├── notifications.js
│   │   └── bluetooth.js
│   ├── store/                  # Redux store
│   │   ├── actions/
│   │   ├── reducers/
│   │   └── selectors/
│   ├── utils/                  # Utilidades
│   │   ├── formatters.js
│   │   └── validators.js
│   └── constants/              # Constantes
│       ├── colors.js
│       └── config.js
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
- Firebase project (para notificaciones)

### Pasos de instalación

```bash
# Clonar el repositorio
git clone https://github.com/enriqueherbertag-lgtm/Asistente-IA-Geriatrico.git
cd Asistente-IA-Geriatrico/software/app-medico

# Instalar dependencias
npm install

# Configurar variables de entorno
cp .env.example .env
# Editar .env con las configuraciones necesarias

# Ejecutar en Android
npm run android

# Ejecutar en iOS
npm run ios
```

---

## API Endpoints

La app se comunica con un servidor central a través de una API REST:

| Endpoint | Método | Descripción |
|:---|:---|:---|
| `/api/auth/login` | POST | Autenticación de médico |
| `/api/patients` | GET | Lista de pacientes |
| `/api/patients/:id` | GET | Detalle de paciente |
| `/api/patients/:id/events` | GET | Historial de eventos |
| `/api/patients/:id/config` | PUT | Actualizar configuración |
| `/api/patients/:id/voice` | POST | Enviar mensaje de voz |
| `/api/alerts` | GET | Alertas activas |

---

## Seguridad

| Aspecto | Implementación |
|:---|:---|
| Autenticación | JWT + refresh token |
| Biometría | Huella digital / Face ID |
| Cifrado | TLS 1.3 en comunicaciones |
| Almacenamiento local | SQLite cifrado (SQLCipher) |
| Tokens | Almacenados en Keychain / Keystore |

---

## Notificaciones

La app recibe notificaciones push en tiempo real para:

- Alertas de caídas
- Lecturas de glucosa fuera de rango
- Batería baja del dispositivo
- Recordatorios programados
- Conexión/desconexión del dispositivo

---

## Pruebas

```bash
# Pruebas unitarias
npm test

# Pruebas de integración
npm run test:integration

# Pruebas end-to-end
npm run test:e2e
```

---

## Construcción para Producción

```bash
# Android
cd android && ./gradlew bundleRelease

# iOS
cd ios && xcodebuild -workspace AppMedico.xcworkspace -scheme AppMedico -configuration Release
```

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
