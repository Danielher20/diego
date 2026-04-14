# PRD — SiGA: Sistema Integral de Gestión Académica
**Versión:** 1.0  
**Fecha:** Abril 2026  
**Lenguaje principal:** C (backend lógico) + HTML/CSS/JS (frontend web)  
**Estado:** Planificación  
**Autor:** Equipo de desarrollo  

---

## Índice

1. [Resumen Ejecutivo](#1-resumen-ejecutivo)
2. [Contexto y Problemática](#2-contexto-y-problemática)
3. [Objetivos y Métricas de Éxito](#3-objetivos-y-métricas-de-éxito)
4. [Usuarios y Personas](#4-usuarios-y-personas)
5. [Requisitos Funcionales](#5-requisitos-funcionales)
6. [Requisitos No Funcionales](#6-requisitos-no-funcionales)
7. [Arquitectura del Sistema](#7-arquitectura-del-sistema)
8. [Modelo de Datos](#8-modelo-de-datos)
9. [Flujos de Usuario Principales](#9-flujos-de-usuario-principales)
10. [Especificaciones de UI/UX](#10-especificaciones-de-uiux)
11. [Plan de Implementación por Fases](#11-plan-de-implementación-por-fases)
12. [Riesgos y Mitigaciones](#12-riesgos-y-mitigaciones)
13. [Criterios de Aceptación](#13-criterios-de-aceptación)
14. [Glosario](#14-glosario)

---

## 1. Resumen Ejecutivo

**SiGA** es una aplicación web local para instituciones de educación superior que centraliza la gestión de calificaciones académicas. El sistema elimina el uso de planillas físicas o archivos Excel dispersos, reemplazándolos con un entorno digital organizado, auditado y con control de acceso por roles.

El sistema opera con **dos paneles distintos y separados**:

| Panel | Rol | Responsabilidad principal |
|---|---|---|
| **Administrador** | Coordinación / Secretaría | Configurar el sistema, gestionar docentes, supervisar |
| **Docente** | Profesor universitario | Gestionar su sección, cargar y calcular notas, generar actas |

**Stack tecnológico:**
- Backend: Programa en **C** (servidor HTTP embebido o CGI)
- Almacenamiento: **Archivos binarios `.dat`** (sin base de datos externa)
- Frontend: **HTML5 + CSS3 + JavaScript** puro (sin frameworks)
- Exportación: Generación de PDF vía HTML-print y CSV

---

## 2. Contexto y Problemática

### 2.1 Situación Actual (Pain Points)

| # | Problema | Impacto |
|---|---|---|
| P1 | Las notas se llevan en Excel o papel, sin respaldo centralizado | Pérdida de datos, versiones duplicadas |
| P2 | Los cálculos de nota definitiva se hacen manualmente | Errores matemáticos, reclamaciones |
| P3 | No existe distinción formal de roles entre admin y docente | Cualquiera puede modificar cualquier dato |
| P4 | No hay trazabilidad de modificaciones a notas | Imposible auditar cambios o correcciones |
| P5 | Las actas se generan a mano en Word | Proceso lento, formato inconsistente |

### 2.2 Solución Propuesta

SiGA unifica todos estos procesos en un sistema web accesible desde cualquier navegador dentro de la red institucional. El backend en C garantiza rendimiento alto con dependencias mínimas, ideal para entornos con recursos limitados.

---

## 3. Objetivos y Métricas de Éxito

### 3.1 Objetivos del Producto

- **O1:** Centralizar la gestión de calificaciones de toda la institución en un único sistema.
- **O2:** Eliminar errores de cálculo mediante cómputo automático de notas definitivas ponderadas.
- **O3:** Garantizar que cada usuario solo acceda a los datos que le corresponden por rol.
- **O4:** Reducir el tiempo de generación de actas finales de horas a minutos.

### 3.2 Métricas de Éxito (KPIs)

| Métrica | Línea base actual | Meta con SiGA |
|---|---|---|
| Tiempo para generar un acta | ~45 minutos | < 2 minutos |
| Errores de cálculo reportados por lapso | > 10 | 0 (cálculo automático) |
| Tiempo de recuperación ante pérdida de notas | Irrecuperable | < 5 minutos (respaldo en .dat) |
| Adopción de docentes activos | 0% | ≥ 80% al cierre del primer semestre |

---

## 4. Usuarios y Personas

### 4.1 Persona: Administrador — "Carmen, Coordinadora Académica"

```
Nombre:     Carmen Rodríguez
Edad:       42 años
Rol:        Coordinadora del Departamento
Dispositivo: PC de escritorio (Windows)
Contexto:   Ingresa al sistema al inicio de semestre para configurar
            materias y docentes. Luego lo usa para supervisión periódica.

NECESITA:
  ✓ Ver el estado global de todas las secciones rápidamente
  ✓ Crear y desactivar cuentas de docentes sin asistencia técnica
  ✓ Generar reportes de rendimiento institucional para dirección
  ✓ Resetear contraseñas sin comprometer la seguridad

NO NECESITA:
  ✗ Modificar calificaciones individuales
  ✗ Ver información personal sensible de alumnos fuera de su competencia
```

### 4.2 Persona: Docente — "Prof. Luis, Profesor de Cálculo"

```
Nombre:     Luis Martínez
Edad:       35 años
Rol:        Docente de Matemáticas
Dispositivo: Laptop personal o PC del laboratorio
Contexto:   Usa el sistema al final de cada evaluación para cargar notas.
            Al cierre del lapso genera el acta.

NECESITA:
  ✓ Ingresar notas rápido (una tabla editable, no formularios individuales)
  ✓ Que el sistema calcule automáticamente la nota definitiva
  ✓ Poder corregir una nota cargada por error
  ✓ Descargar el acta en PDF con un clic

NO NECESITA:
  ✗ Ver las notas de la materia de otro docente
  ✗ Gestionar usuarios o configuración del sistema
```

---

## 5. Requisitos Funcionales

### 5.1 RF-AUTH: Módulo de Autenticación (Compartido)

| ID | Función | Descripción | Prioridad |
|---|---|---|---|
| RF-AUTH-01 | `autenticar_usuario()` | Verifica credenciales (usuario + contraseña hasheada) contra `usuarios.dat`. Determina el rol y redirige al panel correspondiente. | ALTA |
| RF-AUTH-02 | `cerrar_sesion()` | Invalida el token de sesión activo y redirige al login. | ALTA |
| RF-AUTH-03 | `verificar_sesion()` | Middleware que valida el token en cada request. Si no existe o expiró, redirige a login. | ALTA |
| RF-AUTH-04 | `hashear_contrasena()` | Función interna. Aplica SHA-256 a la contraseña antes de almacenar o comparar. Nunca se guarda en texto plano. | ALTA |

**Reglas de negocio:**
- Máximo 5 intentos fallidos → cuenta bloqueada por 15 minutos.
- Las sesiones expiran tras 8 horas de inactividad.
- El token de sesión se almacena en memoria del servidor (no en archivo).

---

### 5.2 RF-ADMIN: Panel del Administrador

#### 5.2.1 Módulo de Gestión de Docentes

| ID | Función | Descripción | Prioridad |
|---|---|---|---|
| RF-ADMIN-01 | `crear_docente(datos)` | Registra un nuevo docente: cédula, nombres, apellidos, email. Crea automáticamente su cuenta de usuario con contraseña temporal. | ALTA |
| RF-ADMIN-02 | `listar_docentes()` | Devuelve la lista completa de docentes activos con su(s) materia(s) asignada(s). | ALTA |
| RF-ADMIN-03 | `editar_docente(id)` | Modifica datos personales de un docente existente. No permite cambiar cédula. | MEDIA |
| RF-ADMIN-04 | `suspender_docente(id)` | Desactiva la cuenta del docente (campo `activo = 0`). Sus datos e historial se conservan. | ALTA |
| RF-ADMIN-05 | `resetear_contrasena(id)` | Genera una nueva contraseña temporal y la muestra al admin (una sola vez). El docente deberá cambiarla en su primer acceso. | ALTA |

#### 5.2.2 Módulo de Gestión de Unidades Curriculares

| ID | Función | Descripción | Prioridad |
|---|---|---|---|
| RF-ADMIN-06 | `crear_materia(datos)` | Registra una unidad curricular: nombre, código, período académico. | ALTA |
| RF-ADMIN-07 | `asignar_materia(id_docente, id_materia)` | Vincula un docente con una materia. Un docente puede tener múltiples materias. Una materia puede reasignarse entre períodos. | ALTA |
| RF-ADMIN-08 | `listar_materias()` | Lista todas las materias del período activo con su docente asignado. | ALTA |

#### 5.2.3 Módulo de Supervisión y Reportes

| ID | Función | Descripción | Prioridad |
|---|---|---|---|
| RF-ADMIN-09 | `ver_actas_global()` | Consulta en modo **solo lectura** las actas de cualquier materia. No permite modificación. | ALTA |
| RF-ADMIN-10 | `generar_estadisticas()` | Calcula porcentaje de aprobados/reprobados por materia y a nivel global. | MEDIA |
| RF-ADMIN-11 | `exportar_reporte_global(formato)` | Genera reporte institucional en PDF o CSV. | BAJA |
| RF-ADMIN-12 | `ver_log_actividad()` | Consulta el registro de acciones críticas: quién cargó notas, cuándo, qué modificó. | MEDIA |

---

### 5.3 RF-DOCENTE: Panel del Docente

#### 5.3.1 Módulo de Gestión de su Sección (Alumnos)

| ID | Función | Descripción | Prioridad |
|---|---|---|---|
| RF-DOC-01 | `registrar_alumno(datos)` | Agrega un alumno a su sección: cédula, nombres, apellidos. La cédula es el identificador único. | ALTA |
| RF-DOC-02 | `listar_alumnos_seccion()` | Muestra la lista de alumnos de la sección actual del docente, ordenados alfabéticamente. | ALTA |
| RF-DOC-03 | `modificar_datos_alumno(cedula)` | Permite corregir errores tipográficos en nombres/apellidos. No permite cambiar cédula. | MEDIA |
| RF-DOC-04 | `retirar_alumno(cedula)` | Marca al alumno como retirado (no se elimina). Sus notas ya cargadas se conservan. | BAJA |

#### 5.3.2 Módulo de Plan de Evaluación

| ID | Función | Descripción | Prioridad |
|---|---|---|---|
| RF-DOC-05 | `configurar_plan_evaluacion()` | Permite crear las actividades evaluativas: nombre, tipo (parcial/taller/práctica) y ponderación (%). El plan solo puede configurarse una vez por materia/período. | ALTA |
| RF-DOC-06 | `validar_ponderaciones()` | **Función interna.** Verifica que la suma de ponderaciones sea exactamente 100% antes de guardar el plan. Si no suma 100%, bloquea la operación y muestra mensaje de error. | ALTA |
| RF-DOC-07 | `ver_plan_evaluacion()` | Muestra el plan de evaluación configurado para la materia actual. | ALTA |
| RF-DOC-08 | `editar_plan_evaluacion()` | Solo disponible si **aún no se han cargado notas** en la materia. Una vez iniciada la carga, el plan queda bloqueado. | MEDIA |

**Reglas de negocio del Plan de Evaluación:**
- Mínimo 1 evaluación, máximo 10 por materia.
- Ponderación mínima por evaluación: 5%.
- La suma DEBE ser exactamente 100.00%.
- Los tipos disponibles son: `PARCIAL`, `TALLER`, `PRACTICA`, `TRABAJO`, `OTRO`.

#### 5.3.3 Módulo de Calificaciones

| ID | Función | Descripción | Prioridad |
|---|---|---|---|
| RF-DOC-09 | `cargar_calificaciones(id_eval)` | Muestra una tabla editable con todos los alumnos de la sección. El docente ingresa la nota de cada uno para la evaluación seleccionada. Escala: 0 a 20 puntos. | ALTA |
| RF-DOC-10 | `guardar_calificaciones_lote()` | Guarda todas las notas ingresadas en la tabla de una sola vez, tras confirmar. | ALTA |
| RF-DOC-11 | `corregir_calificacion(cedula, id_eval)` | Modifica una nota ya guardada. La corrección queda registrada en el log con fecha, hora y motivo (campo obligatorio). | ALTA |
| RF-DOC-12 | `ver_calificaciones_alumno(cedula)` | Muestra todas las notas de un alumno específico en la materia actual. | MEDIA |

**Reglas de negocio de Calificaciones:**
- Notas válidas: 0.00 a 20.00 (dos decimales).
- No se acepta nota vacía si el alumno está activo (debe ser 0 si no asistió).
- Cada corrección genera un registro en `log_auditoria.dat`.

#### 5.3.4 Módulo de Cálculo y Reportes

| ID | Función | Descripción | Prioridad |
|---|---|---|---|
| RF-DOC-13 | `calcular_definitivas()` | Calcula la nota definitiva de cada alumno: `Σ (nota_i × ponderacion_i)`. Resultado con 2 decimales. | ALTA |
| RF-DOC-14 | `determinar_condicion(nota_def)` | Clasifica al alumno: `APROBADO` (≥10.00), `REPROBADO` (<10.00), `RETIRADO`. | ALTA |
| RF-DOC-15 | `generar_acta_materia()` | Genera el acta final con: lista de alumnos, notas por evaluación, definitiva y condición. Formato imprimible. | ALTA |
| RF-DOC-16 | `exportar_listado_notas(formato)` | Exporta las calificaciones en `PDF` (via print CSS) o `CSV` para respaldo personal. | MEDIA |
| RF-DOC-17 | `previsualizar_acta()` | Muestra cómo quedará el acta antes de generarla definitivamente. | BAJA |

---

## 6. Requisitos No Funcionales

| ID | Categoría | Requisito |
|---|---|---|
| RNF-01 | **Seguridad** | Las contraseñas deben almacenarse hasheadas (SHA-256). Nunca en texto plano. |
| RNF-02 | **Seguridad** | Cada request debe validar el token de sesión y el rol del usuario. |
| RNF-03 | **Seguridad** | Un docente no puede acceder a rutas del administrador ni a datos de otro docente. |
| RNF-04 | **Rendimiento** | El sistema debe responder en <500ms para operaciones sobre listas de hasta 500 alumnos. |
| RNF-05 | **Usabilidad** | La interfaz debe funcionar en Chrome/Firefox/Edge sin instalación adicional. |
| RNF-06 | **Usabilidad** | El docente debe poder cargar notas de una sección completa (30 alumnos) en <5 min. |
| RNF-07 | **Confiabilidad** | Los datos deben persistir entre reinicios del servidor sin pérdida. |
| RNF-08 | **Portabilidad** | El servidor C debe compilar en Linux (GCC) y Windows (MinGW). |
| RNF-09 | **Mantenibilidad** | El código C debe seguir separación de módulos (un `.c/.h` por funcionalidad). |
| RNF-10 | **Auditoría** | Toda modificación de calificaciones debe quedar registrada en log con timestamp. |

---

## 7. Arquitectura del Sistema

### 7.1 Diagrama de Componentes

```
┌─────────────────────────────────────────────────────────┐
│                    NAVEGADOR WEB                         │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────────┐  │
│  │  login.html  │  │  admin.html  │  │ docente.html  │  │
│  │  (CSS + JS)  │  │  (CSS + JS)  │  │  (CSS + JS)  │  │
│  └──────┬───────┘  └──────┬───────┘  └───────┬───────┘  │
└─────────┼────────────────┼──────────────────┼──────────┘
          │   HTTP Requests │ (GET / POST)     │
          ▼                 ▼                  ▼
┌─────────────────────────────────────────────────────────┐
│              SERVIDOR HTTP en C (main.c)                 │
│         Puerto 8080 — Red local institucional            │
│                                                          │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌───────────┐   │
│  │ auth.c   │ │ admin.c  │ │docente.c │ │reporte.c  │   │
│  │          │ │          │ │          │ │           │   │
│  │-login    │ │-CRUD     │ │-alumnos  │ │-gen_acta  │   │
│  │-logout   │ │ docentes │ │-plan_ev. │ │-export CSV│   │
│  │-validar  │ │-asignar  │ │-notas    │ │-estadist. │   │
│  │ sesion   │ │ materias │ │-calcular │ │           │   │
│  └────┬─────┘ └────┬─────┘ └────┬─────┘ └─────┬─────┘   │
│       └─────────────┴───────────┴─────────────┘          │
│                           │                               │
│                      ┌────▼──────┐                        │
│                      │   db.c    │  (lectura/escritura    │
│                      │           │   de archivos .dat)    │
│                      └────┬──────┘                        │
└───────────────────────────┼─────────────────────────────┘
                            │
            ┌───────────────▼────────────────┐
            │       ARCHIVOS DE DATOS         │
            │  /data/                          │
            │  ├── usuarios.dat               │
            │  ├── docentes.dat               │
            │  ├── materias.dat               │
            │  ├── alumnos.dat                │
            │  ├── evaluaciones.dat           │
            │  ├── calificaciones.dat         │
            │  └── log_auditoria.dat          │
            └─────────────────────────────────┘
```

### 7.2 Estructura de Directorios del Proyecto

```
siga/
├── Makefile                    # Compilación del proyecto
├── src/
│   ├── main.c                  # Entry point + servidor HTTP
│   ├── auth.c / auth.h         # Módulo de autenticación
│   ├── admin.c / admin.h       # Funciones del administrador
│   ├── docente.c / docente.h   # Funciones del docente
│   ├── alumno.c / alumno.h     # Gestión de alumnos
│   ├── evaluacion.c / evaluacion.h  # Plan de evaluación
│   ├── calificacion.c / calificacion.h  # Notas
│   ├── reporte.c / reporte.h   # Generación de reportes
│   ├── db.c / db.h             # Operaciones con archivos .dat
│   ├── utils.c / utils.h       # Funciones auxiliares (hash, fecha, etc.)
│   └── types.h                 # Definición de todos los structs
├── data/                       # Archivos binarios de datos
│   └── .gitkeep
├── frontend/
│   ├── login.html
│   ├── admin.html
│   ├── docente.html
│   ├── css/
│   │   └── siga.css
│   └── js/
│       ├── auth.js
│       ├── admin.js
│       └── docente.js
└── docs/
    └── PRD.md                  # Este documento
```

### 7.3 Flujo de una Request HTTP

```
1. Navegador envía:  GET /api/alumnos?token=abc123
2. main.c recibe la conexión TCP
3. Parsea la URL → identifica ruta "/api/alumnos"
4. auth.c → verificar_sesion("abc123") → OK, rol=DOCENTE, id=5
5. docente.c → listar_alumnos_seccion(docente_id=5)
6. db.c → leer_registros("alumnos.dat", filtro: docente_id=5)
7. Se serializa resultado a JSON
8. main.c envía respuesta HTTP 200 con JSON
9. JS en el navegador renderiza la tabla
```

---

## 8. Modelo de Datos

### 8.1 Estructuras C (types.h)

```c
/* ==================== USUARIO ==================== */
typedef struct {
    int    id;
    char   username[50];
    char   password_hash[65];  /* SHA-256 en hex = 64 chars + '\0' */
    char   nombre_completo[120];
    char   rol[10];            /* "ADMIN" | "DOCENTE" */
    int    activo;             /* 1 = activo, 0 = suspendido */
    char   fecha_creacion[20]; /* "YYYY-MM-DD HH:MM:SS" */
} Usuario;

/* ==================== DOCENTE ==================== */
typedef struct {
    int    id;
    int    usuario_id;         /* FK → Usuario.id */
    char   cedula[15];
    char   nombres[60];
    char   apellidos[60];
    char   email[100];
    int    activo;
} Docente;

/* ==================== MATERIA ==================== */
typedef struct {
    int    id;
    char   codigo[20];         /* Ej: "MAT-101" */
    char   nombre[100];        /* Ej: "Cálculo I" */
    int    docente_id;         /* FK → Docente.id */
    char   periodo[20];        /* Ej: "2026-I" */
    int    activo;
} Materia;

/* ==================== ALUMNO ==================== */
typedef struct {
    int    id;
    char   cedula[15];         /* Identificador único */
    char   nombres[60];
    char   apellidos[60];
    int    materia_id;         /* FK → Materia.id */
    int    retirado;           /* 0 = activo, 1 = retirado */
} Alumno;

/* ==================== EVALUACION (ítem del plan) ==================== */
#define TIPO_PARCIAL  0
#define TIPO_TALLER   1
#define TIPO_PRACTICA 2
#define TIPO_TRABAJO  3
#define TIPO_OTRO     4

typedef struct {
    int    id;
    int    materia_id;         /* FK → Materia.id */
    char   nombre[100];        /* Ej: "Parcial 1" */
    int    tipo;               /* TIPO_PARCIAL, etc. */
    float  ponderacion;        /* 0.05 a 1.00 (5% a 100%) */
    int    orden;              /* Posición en el plan: 1, 2, 3... */
} Evaluacion;

/* ==================== CALIFICACION ==================== */
typedef struct {
    int    id;
    int    alumno_id;          /* FK → Alumno.id */
    int    evaluacion_id;      /* FK → Evaluacion.id */
    float  nota;               /* 0.00 a 20.00 */
    char   fecha_carga[20];
    char   fecha_modificacion[20]; /* "" si nunca se modificó */
    int    modificada;         /* 0 = original, 1 = corregida */
} Calificacion;

/* ==================== LOG DE AUDITORÍA ==================== */
typedef struct {
    int    id;
    int    usuario_id;         /* Quién realizó la acción */
    char   accion[50];         /* "CARGA_NOTA", "CORRECCION", "LOGIN", etc. */
    char   detalle[200];       /* Descripción legible de lo que cambió */
    char   timestamp[20];
} LogAuditoria;
```

### 8.2 Relaciones entre Entidades

```
Usuario ──── (1:1) ──── Docente
Docente ──── (1:N) ──── Materia
Materia ──── (1:N) ──── Alumno
Materia ──── (1:N) ──── Evaluacion
Alumno  ──── (1:N) ──── Calificacion
Evaluacion ─(1:N) ──── Calificacion
Usuario ──── (1:N) ──── LogAuditoria
```

### 8.3 Operaciones de Base de Datos (db.c)

```c
/* Funciones genéricas para manejo de archivos binarios */

int    db_insertar(const char* archivo, void* registro, size_t tamanio);
int    db_buscar_por_id(const char* archivo, int id, void* resultado, size_t tam);
int    db_actualizar(const char* archivo, int id, void* nuevo_reg, size_t tam);
int    db_marcar_inactivo(const char* archivo, int id, size_t tam, int offset_activo);
int    db_listar_todos(const char* archivo, void* buffer[], int max, size_t tam);
int    db_filtrar(const char* archivo, void* buffer[], int max, size_t tam,
                  int (*filtro)(void*));

/* Genera el próximo ID autoincremental para un archivo */
int    db_siguiente_id(const char* archivo, size_t tam);
```

---

## 9. Flujos de Usuario Principales

### 9.1 Flujo: Administrador crea un docente y asigna una materia

```
[Admin] → Inicia sesión
       → Panel Admin: Dashboard (estadísticas generales)
       → Navega a "Gestión de Docentes"
       → Clic en "Nuevo Docente"
       → Rellena formulario: cédula, nombres, apellidos, email
       → Sistema crea registro en docentes.dat + usuario en usuarios.dat
                        (con contraseña temporal)
       → Sistema muestra contraseña temporal (UNA VEZ)
       → Admin navega a "Unidades Curriculares"
       → Selecciona una materia existente o crea una nueva
       → Asigna el docente recién creado a esa materia
       → Confirmación de éxito
```

### 9.2 Flujo: Docente configura su plan de evaluación

```
[Docente] → Inicia sesión (primera vez: debe cambiar contraseña temporal)
          → Panel Docente: Dashboard con su(s) materia(s)
          → Selecciona su materia → "Configurar Plan de Evaluación"
          → Agrega evaluaciones:
              Ej: Parcial 1 → 25%
                  Parcial 2 → 25%
                  Taller 1  → 20%
                  Práctica  → 15%
                  Trabajo F → 15%
          → Sistema valida: 25+25+20+15+15 = 100% ✓
          → Plan guardado y bloqueado para modificación
```

### 9.3 Flujo: Docente carga calificaciones (flujo crítico)

```
[Docente] → Panel Docente → Su materia → "Cargar Notas"
          → Selecciona la evaluación: "Parcial 1"
          → Sistema muestra tabla con todos los alumnos activos
          → Docente ingresa nota a cada alumno (0.00 - 20.00)
          → Clic en "Vista previa" → revisa los datos
          → Clic en "Guardar" → confirmación con resumen
          → Sistema guarda en calificaciones.dat
          → Registro en log_auditoria.dat
          → Tabla marcada como "cargada" (no muestra el botón de carga de nuevo)
          
[Si hay error] → Docente → "Corregir Nota"
              → Busca alumno → selecciona evaluación
              → Modifica la nota
              → Ingresa motivo de corrección (obligatorio)
              → Sistema guarda + registra en auditoría
```

### 9.4 Flujo: Generación de Acta Final

```
[Docente] → "Calcular Definitivas"
          → Sistema ejecuta: definitiva_i = Σ(nota_j × ponderacion_j)
          → Determina condición: APROBADO / REPROBADO / RETIRADO
          → Vista previa del acta (tabla completa)
          → Docente revisa → "Generar Acta"
          → Acta disponible para:
              ► Imprimir (CSS de impresión)
              ► Exportar PDF (via ventana de impresión del navegador)
              ► Exportar CSV
          → Acta registrada como "generada" con fecha y hora
```

---

## 10. Especificaciones de UI/UX

### 10.1 Principios de Diseño

1. **Claridad primero:** El docente no debe buscar cómo cargar una nota. Debe ser obvio.
2. **Datos siempre visibles:** Las tablas son el elemento central, no los formularios.
3. **Feedback inmediato:** Cada acción muestra una confirmación o error al instante.
4. **Modo de impresión limpio:** El acta se ve correctamente al imprimir (sin sidebars, sin botones).

### 10.2 Paleta de Colores

```css
--color-primario:     #0D1F3C;   /* Azul navy institucional */
--color-primario-2:   #1A3A6B;   /* Azul medio (hover sidebar) */
--color-acento:       #FBBF24;   /* Ámbar dorado (CTAs, highlights) */
--color-fondo:        #F0F4F8;   /* Gris muy claro (background general) */
--color-superficie:   #FFFFFF;   /* Blanco (cards, tables) */
--color-borde:        #E2E8F0;   /* Gris suave (separadores) */
--color-texto:        #1E293B;   /* Casi negro (texto principal) */
--color-texto-2:      #64748B;   /* Gris (texto secundario) */
--color-exito:        #16A34A;   /* Verde (aprobado, éxito) */
--color-error:        #DC2626;   /* Rojo (reprobado, error) */
--color-advertencia:  #D97706;   /* Naranja (advertencias) */
```

### 10.3 Tipografía

```
Display / Títulos:  "Fraunces"    — Serif con carácter académico
UI / Body:          "Outfit"      — Sans-serif moderna y legible
Datos / Notas:      "Fira Mono"   — Monoespaciada para números y códigos
```

### 10.4 Layout General (Post-Login)

```
┌──────────────────────────────────────────────────────────────┐
│  SIDEBAR (260px fijo)     │  TOPBAR (altura 64px)            │
│  ┌────────────────────┐   │  Breadcrumb           👤 Usuario │
│  │  🎓 SiGA           │   ├──────────────────────────────────┤
│  ├────────────────────┤   │                                   │
│  │  Dashboard         │   │   ÁREA DE CONTENIDO PRINCIPAL    │
│  │  [según rol]       │   │                                   │
│  │                    │   │   ┌──────┐ ┌──────┐ ┌──────┐    │
│  │  Módulo 1          │   │   │ Card │ │ Card │ │ Card │    │
│  │  Módulo 2          │   │   └──────┘ └──────┘ └──────┘    │
│  │  Módulo 3          │   │                                   │
│  │  ...               │   │   ┌─────────────────────────┐    │
│  ├────────────────────┤   │   │      TABLA / FORM        │    │
│  │  Cerrar sesión     │   │   └─────────────────────────┘    │
│  └────────────────────┘   │                                   │
└──────────────────────────────────────────────────────────────┘
```

### 10.5 Componentes Clave

| Componente | Descripción |
|---|---|
| **Tabla de notas** | Editable inline. Cada celda es un `<input type="number">` con validación 0-20. Validación en tiempo real. |
| **Barra de ponderación** | Indicador visual (barra de progreso) que muestra el porcentaje acumulado del plan de evaluación en tiempo real. |
| **Badge de condición** | Pill coloreado: verde=APROBADO, rojo=REPROBADO, gris=RETIRADO. |
| **Modal de confirmación** | Para acciones destructivas o de carga masiva: "¿Confirmas guardar 32 calificaciones?" |
| **Toast de notificaciones** | Mensajes de éxito/error que aparecen 3 segundos y desaparecen. |

---

## 11. Plan de Implementación por Fases

### Fase 0: Preparación (Semana 1)
- [ ] Configurar entorno de desarrollo (GCC + editor)
- [ ] Crear estructura de directorios del proyecto
- [ ] Escribir `types.h` con todos los structs
- [ ] Escribir `db.c` con las funciones genéricas de archivos binarios
- [ ] Escribir `utils.c` (sha256, timestamps, validaciones)
- [ ] Probar escritura y lectura de registros con un programa de prueba

### Fase 1: Autenticación (Semana 2)
- [ ] Implementar `auth.c` (login, hash, verificar sesión)
- [ ] Crear página `login.html` con CSS básico
- [ ] Implementar `main.c` con servidor HTTP mínimo (rutas: GET / y POST /login)
- [ ] Probar login correcto e incorrecto desde el navegador
- [ ] Implementar bloqueo por intentos fallidos

### Fase 2: Panel Administrador (Semanas 3-4)
- [ ] Implementar `admin.c` (CRUD docentes, gestión materias)
- [ ] Crear `admin.html` con sidebar y navegación
- [ ] Implementar las vistas: lista de docentes, formulario de creación, asignación
- [ ] Implementar reset de contraseña
- [ ] Probar flujo completo: crear docente → asignar materia

### Fase 3: Panel Docente — Alumnos y Plan (Semanas 5-6)
- [ ] Implementar `alumno.c` (registro, lista, modificación)
- [ ] Implementar `evaluacion.c` (plan de evaluación, validación de ponderaciones)
- [ ] Crear `docente.html` con sus vistas: lista de alumnos, plan de evaluación
- [ ] Validar ponderaciones en frontend Y backend
- [ ] Probar flujo: registrar alumnos → configurar plan

### Fase 4: Calificaciones y Cálculos (Semanas 7-8)
- [ ] Implementar `calificacion.c` (carga, corrección, auditoría)
- [ ] Vista de carga de notas (tabla editable)
- [ ] Implementar cálculo de definitivas
- [ ] Vista de resultados con badges de condición
- [ ] Implementar log de auditoría
- [ ] Probar flujo completo: notas → definitiva → condición

### Fase 5: Reportes y Exportación (Semana 9)
- [ ] Implementar `reporte.c` (generación de actas)
- [ ] CSS de impresión para el acta
- [ ] Exportación CSV (backend C genera el archivo)
- [ ] Estadísticas globales para el admin
- [ ] Vista de supervisión del admin (actas de solo lectura)

### Fase 6: Pruebas y Refinamiento (Semana 10)
- [ ] Pruebas de seguridad: intentar acceder a rutas de otro rol
- [ ] Pruebas de datos: notas inválidas, ponderaciones incorrectas
- [ ] Prueba de carga: 100+ alumnos, 10 evaluaciones
- [ ] Refinamiento de UI: feedback visual, mensajes de error claros
- [ ] Documentación de uso para administrador y docente

---

## 12. Riesgos y Mitigaciones

| ID | Riesgo | Probabilidad | Impacto | Mitigación |
|---|---|---|---|---|
| R1 | Corrupción de archivos `.dat` | Media | Alto | Función de backup automático al iniciar el servidor. Validar integridad de registros al leer. |
| R2 | Conflictos de concurrencia (dos docentes escriben a la vez) | Baja | Alto | Bloqueo de archivo (`flock`) durante escritura. Para v1.0: usuarios únicos concurrentes. |
| R3 | Olvido de contraseña del admin principal | Baja | Alto | Script de reset de emergencia `reset_admin.c` que se ejecuta desde consola con acceso físico al servidor. |
| R4 | Pérdida de datos por apagado durante escritura | Media | Alto | Escritura atómica: escribir en archivo temporal, luego renombrar (`rename()`). |
| R5 | El frontend no funciona en todos los navegadores | Baja | Medio | Usar solo HTML/CSS/JS estándar. No usar APIs experimentales. Probar en Chrome, Firefox y Edge. |

---

## 13. Criterios de Aceptación

El sistema se considera **completo y aceptable** cuando:

### Administrador
- [x] Puede iniciar sesión y ser redirigido a su panel exclusivo.
- [x] Puede crear un nuevo docente y el sistema genera credenciales automáticas.
- [x] Puede asignar una materia a un docente.
- [x] Puede suspender la cuenta de un docente sin eliminar sus datos.
- [x] Puede ver las actas de cualquier materia en modo solo lectura.
- [x] Puede ver el porcentaje global de aprobados/reprobados.

### Docente
- [x] Puede iniciar sesión y solo ver sus propias materias y alumnos.
- [x] Puede registrar alumnos en su sección.
- [x] Puede configurar un plan de evaluación y el sistema rechaza si no suma 100%.
- [x] Puede cargar notas desde una tabla editable para todos sus alumnos a la vez.
- [x] No puede cargar una nota fuera del rango 0-20.
- [x] Puede corregir una nota con motivo obligatorio, quedando registrado en auditoría.
- [x] El sistema calcula correctamente las notas definitivas ponderadas.
- [x] Puede generar e imprimir el acta final de su materia.
- [x] Puede exportar su listado de notas en CSV.

### Seguridad
- [x] Un docente que intente acceder a una ruta de admin recibe error 403.
- [x] Un docente que intente ver datos de otro docente recibe error 403.
- [x] Las contraseñas nunca aparecen en los archivos `.dat` en texto plano.
- [x] Tras 5 intentos de login fallidos, la cuenta se bloquea 15 minutos.

---

## 14. Glosario

| Término | Definición |
|---|---|
| **Acta** | Documento oficial que certifica las calificaciones finales de todos los alumnos de una materia. |
| **Definitiva** | Nota final del alumno en la materia, calculada como la suma ponderada de todas sus evaluaciones. |
| **Lapso** | Período académico (semestre o trimestre). |
| **Ponderación** | Peso porcentual que tiene una evaluación sobre la nota definitiva (Ej: 25%). |
| **Sección** | Grupo de alumnos inscritos en una materia con un docente específico. |
| **Token de sesión** | Código aleatorio generado al hacer login que identifica al usuario en cada request. |
| **Unidad Curricular** | Sinónimo de "materia" o "asignatura". |
| **Plan de Evaluación** | Conjunto de evaluaciones con sus ponderaciones que suman exactamente 100%. |

---

*Documento preparado para: Sistema Integral de Gestión Académica (SiGA) v1.0*  
*Lenguaje de implementación: C (ANSI C99 / C11) + HTML5/CSS3/JS*
