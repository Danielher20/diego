#ifndef SIGA_TYPES_H
#define SIGA_TYPES_H

#define SIGA_ROLE_ADMIN   "ADMIN"
#define SIGA_ROLE_DOCENTE "DOCENTE"

#define TIPO_PARCIAL  0
#define TIPO_TALLER   1
#define TIPO_PRACTICA 2
#define TIPO_TRABAJO  3
#define TIPO_OTRO     4

typedef struct {
    int  id;
    char username[50];
    char password_hash[65];
    char nombre_completo[120];
    char rol[10];
    int  activo;
    int  debe_cambiar_password;
    char fecha_creacion[20];
} Usuario;

typedef struct {
    int  id;
    int  usuario_id;
    char cedula[15];
    char nombres[60];
    char apellidos[60];
    char email[100];
    int  activo;
} Docente;

typedef struct {
    int  id;
    char codigo[20];
    char nombre[100];
    int  docente_id;
    char periodo[20];
    int  activo;
    int  acta_generada;
    char fecha_acta[20];
} Materia;

typedef struct {
    int  id;
    char cedula[15];
    char nombres[60];
    char apellidos[60];
    int  materia_id;
    int  retirado;
} Alumno;

typedef struct {
    int   id;
    int   materia_id;
    char  nombre[100];
    int   tipo;
    float ponderacion;
    int   orden;
} Evaluacion;

typedef struct {
    int   id;
    int   alumno_id;
    int   evaluacion_id;
    float nota;
    char  fecha_carga[20];
    char  fecha_modificacion[20];
    int   modificada;
} Calificacion;

typedef struct {
    int  id;
    int  usuario_id;
    char accion[50];
    char detalle[240];
    char timestamp[20];
} LogAuditoria;

#endif
