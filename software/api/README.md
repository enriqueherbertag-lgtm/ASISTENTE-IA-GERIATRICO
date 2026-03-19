# API REST - Asistente IA Geriátrico

## Descripción General

La API REST del Asistente IA Geriátrico permite la comunicación entre los dispositivos, las aplicaciones móviles y el dashboard hospitalario. Está diseñada para ser **escalable, segura y eficiente**, siguiendo los estándares RESTful y buenas prácticas de desarrollo.

---

## Base URL

```
Producción: https://api.asistenteia.cl/v1
Pruebas:    https://staging.api.asistenteia.cl/v1
Local:      http://localhost:3000/v1
```

---

## Autenticación

La API utiliza **JWT (JSON Web Tokens)** para autenticación.

### Obtener Token

```http
POST /auth/login
Content-Type: application/json

{
  "email": "medico@hospital.cl",
  "password": "contraseña"
}
```

**Respuesta:**
```json
{
  "token": "eyJhbGciOiJIUzI1NiIs...",
  "refreshToken": "eyJhbGciOiJIUzI1NiIs...",
  "user": {
    "id": 123,
    "name": "Dr. Juan Pérez",
    "role": "medico"
  }
}
```

### Uso del Token
Incluir el token en el header `Authorization`:

```http
Authorization: Bearer eyJhbGciOiJIUzI1NiIs...
```

---

## Endpoints

### Pacientes

#### Listar pacientes
```http
GET /patients
Authorization: Bearer <token>
```

**Parámetros opcionales:**
| Parámetro | Tipo | Descripción |
|:---|:---|:---|
| `page` | number | Página (default: 1) |
| `limit` | number | Resultados por página (default: 20) |
| `status` | string | Filtrar por estado (active, inactive, alert) |
| `search` | string | Búsqueda por nombre |
| `doctor_id` | number | Filtrar por médico asignado |

**Respuesta:**
```json
{
  "data": [
    {
      "id": 1,
      "name": "Juan Pérez",
      "age": 78,
      "diagnosis": ["diabetes", "alzheimer"],
      "device_id": "AIG-001-2345",
      "last_reading": "2026-03-19T10:30:00Z",
      "battery": 85,
      "status": "active"
    }
  ],
  "pagination": {
    "total": 150,
    "page": 1,
    "pages": 8
  }
}
```

#### Obtener paciente por ID
```http
GET /patients/{id}
Authorization: Bearer <token>
```

#### Crear paciente
```http
POST /patients
Authorization: Bearer <token>
Content-Type: application/json

{
  "name": "María González",
  "birth_date": "1945-03-12",
  "gender": "F",
  "diagnosis": ["diabetes", "hipertensión"],
  "doctor_id": 123,
  "emergency_contacts": [
    {
      "name": "Carlos González",
      "relation": "hijo",
      "phone": "+56912345678"
    }
  ]
}
```

#### Actualizar paciente
```http
PUT /patients/{id}
Authorization: Bearer <token>
```

#### Eliminar paciente
```http
DELETE /patients/{id}
Authorization: Bearer <token>
```

### Eventos

#### Obtener eventos de un paciente
```http
GET /patients/{id}/events
Authorization: Bearer <token>
```

**Parámetros opcionales:**
| Parámetro | Tipo | Descripción |
|:---|:---|:---|
| `type` | string | fall, glucose, voice, battery |
| `from` | date | Fecha inicio (YYYY-MM-DD) |
| `to` | date | Fecha fin |
| `limit` | number | Máximo de resultados |

**Respuesta:**
```json
{
  "data": [
    {
      "id": 4567,
      "type": "fall",
      "timestamp": "2026-03-19T08:15:00Z",
      "severity": 8,
      "description": "Caída confirmada",
      "data": {
        "magnitude": 2.8
      }
    },
    {
      "id": 4568,
      "type": "glucose",
      "timestamp": "2026-03-19T07:30:00Z",
      "severity": 2,
      "description": "Glucosa normal",
      "data": {
        "glucose": 110,
        "trend": 0
      }
    }
  ]
}
```

#### Agregar evento manual
```http
POST /patients/{id}/events
Authorization: Bearer <token>
Content-Type: application/json

{
  "type": "note",
  "description": "Paciente refiere mareos",
  "severity": 3
}
```

### Alertas

#### Alertas activas
```http
GET /alerts/active
Authorization: Bearer <token>
```

**Respuesta:**
```json
{
  "critical": 2,
  "high": 5,
  "moderate": 12,
  "alerts": [
    {
      "id": 789,
      "patient_id": 1,
      "patient_name": "Juan Pérez",
      "type": "fall",
      "level": "critical",
      "timestamp": "2026-03-19T10:45:00Z",
      "message": "Caída detectada - sin respuesta"
    }
  ]
}
```

#### Resolver alerta
```http
POST /alerts/{id}/resolve
Authorization: Bearer <token>
Content-Type: application/json

{
  "resolution": "Paciente atendido, se encuentra bien",
  "resolved_by": 123
}
```

### Dispositivos

#### Registrar dispositivo
```http
POST /devices
Authorization: Bearer <token>
Content-Type: application/json

{
  "device_id": "AIG-001-2345",
  "patient_id": 1,
  "firmware_version": "1.0.0",
  "battery": 85
}
```

#### Obtener estado del dispositivo
```http
GET /devices/{device_id}/status
Authorization: Bearer <token>
```

#### Enviar configuración al dispositivo
```http
POST /devices/{device_id}/config
Authorization: Bearer <token>
Content-Type: application/json

{
  "glucose_thresholds": {
    "low": 70,
    "critical": 54,
    "high": 250
  },
  "fall_sensitivity": "medium",
  "reminders": [
    {
      "time": "08:00",
      "message": "Tomar medicación"
    }
  ]
}
```

### Médicos

#### Listar médicos
```http
GET /doctors
Authorization: Bearer <token>
```

#### Obtener médico por ID
```http
GET /doctors/{id}
Authorization: Bearer <token>
```

#### Asignar pacientes
```http
POST /doctors/{id}/patients
Authorization: Bearer <token>
Content-Type: application/json

{
  "patient_ids": [1, 2, 3]
}
```

### Reportes

#### Generar reporte
```http
POST /reports
Authorization: Bearer <token>
Content-Type: application/json

{
  "type": "monthly_summary",
  "patient_id": 1,
  "month": "2026-03",
  "format": "pdf"
}
```

**Respuesta:**
```json
{
  "report_id": "RPT-202603-001",
  "status": "processing",
  "download_url": "/reports/RPT-202603-001.pdf"
}
```

#### Descargar reporte
```http
GET /reports/{report_id}
Authorization: Bearer <token>
```

---

## WebSockets

La API también soporta conexiones WebSocket para actualizaciones en tiempo real.

### Conexión
```javascript
const socket = io('https://api.asistenteia.cl', {
  auth: {
    token: 'Bearer eyJhbGciOiJIUzI1NiIs...'
  }
});
```

### Eventos
| Evento | Dirección | Descripción |
|:---|:---|:---|
| `alert:new` | server → client | Nueva alerta |
| `patient:update` | server → client | Cambio en estado del paciente |
| `device:status` | server → client | Actualización de dispositivo |
| `subscribe:patient` | client → server | Suscribirse a un paciente |
| `unsubscribe:patient` | client → server | Cancelar suscripción |

### Ejemplo
```javascript
// Cliente
socket.emit('subscribe:patient', { patient_id: 1 });

socket.on('alert:new', (data) => {
  console.log('Nueva alerta:', data);
});
```

---

## Modelos de Datos

### Patient
```json
{
  "id": "integer",
  "name": "string",
  "birth_date": "date",
  "gender": "string (M/F)",
  "diagnosis": "array[string]",
  "doctor_id": "integer",
  "emergency_contacts": "array[object]",
  "device_id": "string",
  "created_at": "datetime",
  "updated_at": "datetime"
}
```

### Event
```json
{
  "id": "integer",
  "patient_id": "integer",
  "type": "string (fall, glucose, voice, battery, note)",
  "timestamp": "datetime",
  "severity": "integer (0-10)",
  "description": "string",
  "data": "object"
}
```

### Alert
```json
{
  "id": "integer",
  "patient_id": "integer",
  "type": "string",
  "level": "string (critical, high, moderate, low)",
  "message": "string",
  "timestamp": "datetime",
  "resolved": "boolean",
  "resolved_at": "datetime",
  "resolved_by": "integer"
}
```

### Device
```json
{
  "device_id": "string",
  "patient_id": "integer",
  "firmware_version": "string",
  "battery": "integer (0-100)",
  "last_seen": "datetime",
  "status": "string (active, inactive, low_battery)",
  "config": "object"
}
```

---

## Códigos de Error

| Código | Descripción |
|:---|:---|
| `400` | Bad Request - Parámetros inválidos |
| `401` | Unauthorized - Token no válido |
| `403` | Forbidden - Sin permisos |
| `404` | Not Found - Recurso no existe |
| `409` | Conflict - Conflicto de datos |
| `422` | Unprocessable Entity - Validación fallida |
| `429` | Too Many Requests - Límite de peticiones |
| `500` | Internal Server Error |

---

## Límites

| Recurso | Límite |
|:---|:---|
| Peticiones por minuto (autenticado) | 120 |
| Peticiones por minuto (no autenticado) | 30 |
| Tamaño máximo de payload | 10 MB |
| Timeout de respuesta | 30 segundos |

---

## SDKs

### JavaScript/TypeScript
```bash
npm install @asistenteia/api-client
```

```javascript
import { AsistenteIAClient } from '@asistenteia/api-client';

const client = new AsistenteIAClient({
  token: 'eyJhbGciOiJIUzI1NiIs...'
});

const patients = await client.patients.list();
```

### Python
```bash
pip install asistenteia-api
```

```python
from asistenteia import Client

client = Client(token='eyJhbGciOiJIUzI1NiIs...')
patients = client.patients.list()
```

---

## Documentación Interactiva

La documentación completa de la API está disponible en:

- Swagger UI: `https://api.asistenteia.cl/docs`
- OpenAPI JSON: `https://api.asistenteia.cl/swagger.json`

---

## Licencia

Esta API está licenciada bajo Apache 2.0 con restricción comercial. Para uso comercial, contactar al autor.

---

## Contacto

**Enrique Aguayo H.**  
Mackiber Labs  
eaguayo@migst.cl

---

*Documento v1.0 - Marzo 2026*
