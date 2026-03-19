# Dashboard Hospitalario - Asistente IA Geriátrico

## Descripción General

El Dashboard Hospitalario es una aplicación web diseñada para instituciones de salud (hospitales, residencias geriátricas, centros de día) que permite la gestión centralizada de múltiples pacientes con el dispositivo Asistente IA Geriátrico.

---

## Funcionalidades Principales

### 1. Visión General (Home)
| Sección | Descripción |
|:---|:---|
| **Mapa de pacientes** | Ubicación de pacientes con alertas activas |
| **Resumen de alertas** | Alertas por nivel (críticas, altas, moderadas) |
| **Pacientes de alto riesgo** | Lista prioritaria (últimas 24h) |
| **Estado del sistema** | Dispositivos activos, baterías bajas, desconexiones |

### 2. Gestión de Pacientes
- Lista completa de pacientes (búsqueda, filtros)
- Vista detallada de cada paciente
- Historial completo de eventos
- Gráficos de tendencias (glucosa, caídas, actividad)
- Notas clínicas y observaciones

### 3. Alertas y Notificaciones
- Centro de alertas en tiempo real
- Clasificación por nivel de gravedad
- Historial de alertas resueltas
- Opciones de escalamiento (médico, supervisor, emergencias)

### 4. Configuración Masiva
- Umbrales de alerta por grupos (ej: todos los pacientes con diabetes)
- Asignación de médicos a pacientes
- Gestión de voces familiares (carga masiva)
- Programación de recordatorios generales

### 5. Reportes y Estadísticas
| Reporte | Descripción |
|:---|:---|
| **Resumen mensual** | Caídas, hipoglucemias, hiperglucemias |
| **Tendencias poblacionales** | Análisis agregado de todos los pacientes |
| **Efectividad del dispositivo** | Reducción de hospitalizaciones, caídas evitadas |
| **Exportación de datos** | CSV, PDF, FHIR (para integración con EHR) |

### 6. Administración
- Gestión de usuarios (médicos, enfermeros, administrativos)
- Registro de auditoría (quién hizo qué, cuándo)
- Configuración de la institución
- Integración con sistemas externos

---

## Pantallas Principales

```
Dashboard Hospitalario
├── Login (autenticación 2FA)
├── Home
│   ├── Mapa de alertas
│   ├── KPIs principales
│   └── Alertas recientes
├── Pacientes
│   ├── Lista general
│   ├── Detalle del paciente
│   └── Historial
├── Alertas
│   ├── Centro de alertas
│   └── Historial
├── Reportes
│   ├── Generador de reportes
│   └── Reportes guardados
├── Configuración
│   ├── Pacientes masiva
│   ├── Usuarios
│   └── Integraciones
└── Administración
    ├── Auditoría
    └── Mantenimiento
```

---

## Tecnologías Utilizadas

| Componente | Tecnología |
|:---|:---|
| Frontend | React 18 + TypeScript |
| UI Framework | Material-UI (MUI) |
| Estado | Redux Toolkit + RTK Query |
| Gráficos | Recharts / D3.js |
| Mapas | Leaflet / Mapbox |
| Backend API | Node.js + Express (o FastAPI) |
| Base de datos | PostgreSQL + TimescaleDB |
| Tiempo real | WebSockets (Socket.io) |
| Autenticación | JWT + 2FA |
| Documentación | Swagger / OpenAPI |

---

## Estructura del Proyecto

```
dashboard-hospital/
├── frontend/
│   ├── public/
│   ├── src/
│   │   ├── assets/
│   │   ├── components/
│   │   │   ├── common/
│   │   │   ├── charts/
│   │   │   ├── map/
│   │   │   └── alerts/
│   │   ├── pages/
│   │   │   ├── Home/
│   │   │   ├── Patients/
│   │   │   ├── Alerts/
│   │   │   ├── Reports/
│   │   │   └── Admin/
│   │   ├── services/
│   │   │   ├── api/
│   │   │   └── websocket/
│   │   ├── store/
│   │   ├── hooks/
│   │   ├── utils/
│   │   └── types/
│   ├── package.json
│   └── README.md
├── backend/
│   ├── src/
│   │   ├── controllers/
│   │   ├── models/
│   │   ├── routes/
│   │   ├── middleware/
│   │   ├── services/
│   │   └── utils/
│   ├── tests/
│   ├── package.json
│   └── README.md
├── docker-compose.yml
└── README.md (principal)
```

---

## API Endpoints (Backend)

| Endpoint | Método | Descripción |
|:---|:---|:---|
| `/api/auth/login` | POST | Autenticación |
| `/api/auth/2fa` | POST | Verificación 2FA |
| `/api/patients` | GET | Lista paginada de pacientes |
| `/api/patients/:id` | GET | Detalle del paciente |
| `/api/patients/:id/events` | GET | Eventos (filtros por fecha) |
| `/api/patients/:id/alerts` | GET | Alertas del paciente |
| `/api/patients/:id/config` | PUT | Configuración individual |
| `/api/alerts/active` | GET | Alertas activas (tiempo real) |
| `/api/alerts/history` | GET | Historial de alertas |
| `/api/reports` | POST | Generar reporte |
| `/api/users` | GET | Lista de usuarios |
| `/api/audit` | GET | Registro de auditoría |
| `/api/integrations/fhir` | POST | Exportar a FHIR |

---

## Instalación y Despliegue

### Requisitos previos
- Docker y Docker Compose
- Node.js 18+ (para desarrollo local)
- PostgreSQL 15+
- Redis (para sesiones y caché)

### Instalación con Docker

```bash
# Clonar el repositorio
git clone https://github.com/enriqueherbertag-lgtm/Asistente-IA-Geriatrico.git
cd Asistente-IA-Geriatrico/software/dashboard-hospital

# Configurar variables de entorno
cp .env.example .env
# Editar .env con configuraciones

# Iniciar con Docker Compose
docker-compose up -d

# Acceder al dashboard
# http://localhost:3000
```

### Desarrollo local

```bash
# Backend
cd backend
npm install
npm run dev

# Frontend (otra terminal)
cd frontend
npm install
npm start
```

---

## Seguridad

| Aspecto | Implementación |
|:---|:---|
| Autenticación | JWT + refresh token |
| 2FA | TOTP (Google Authenticator) |
| HTTPS | TLS 1.3 en producción |
| Roles | Admin, Médico, Enfermero, Observador |
| Auditoría | Registro de todas las acciones |
| GDPR | Consentimiento, derecho al olvido |
| Datos de salud | Cifrado en reposo (AES-256) |

---

## Tiempo Real

El dashboard utiliza **WebSockets** para actualizaciones en tiempo real:

| Evento | Frecuencia | Descripción |
|:---|:---|:---|
| Nueva alerta | Inmediato | Caída, glucosa crítica |
| Cambio de estado | Inmediato | Batería baja, desconexión |
| Lectura de glucosa | 5 min | Actualización de gráficos |
| Heartbeat | 30 seg | Verificación de conexión |

---

## Integración con FHIR

El dashboard puede exportar datos al estándar **HL7 FHIR** para integrarse con sistemas de historia clínica electrónica (EHR):

| Recurso FHIR | Descripción |
|:---|:---|
| `Patient` | Datos demográficos del paciente |
| `Observation` | Lecturas de glucosa, caídas |
| `RiskAssessment` | Nivel de riesgo del paciente |
| `Device` | Información del dispositivo |
| `Communication` | Alertas enviadas al médico |

---

## Reportes Predefinidos

| Reporte | Uso | Formato |
|:---|:---|:---|
| **Resumen mensual por paciente** | Seguimiento individual | PDF |
| **Estadísticas institucionales** | Dirección médica | PDF / Excel |
| **Tendencias de glucosa** | Endocrinología | Gráficos |
| **Análisis de caídas** | Calidad | CSV |
| **Auditoría de accesos** | Seguridad | CSV |

---

## Pruebas

```bash
# Backend
cd backend
npm test
npm run test:coverage

# Frontend
cd frontend
npm test
npm run test:e2e
```

---

## Despliegue en Producción

```bash
# Construir imágenes Docker
docker-compose -f docker-compose.prod.yml build

# Desplegar
docker-compose -f docker-compose.prod.yml up -d

# Migraciones de base de datos
docker-compose exec backend npm run migrate
```

---

## Monitoreo

El dashboard incluye métricas de uso y rendimiento:

| Métrica | Herramienta |
|:---|:---|
| Logs de aplicación | Winston / ELK Stack |
| Métricas de rendimiento | Prometheus + Grafana |
| Alertas del sistema | Sentry |
| Disponibilidad | Uptime Robot |

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
