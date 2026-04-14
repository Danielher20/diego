#include "alumno.h"

#include "audit.h"
#include "db.h"
#include "docente.h"

#include <stdio.h>
#include <string.h>

#define MAX_ALUMNOS 2048

static int alumno_cedula_existe_en_materia(const char *cedula, int materia_id) {
    Alumno alumnos[MAX_ALUMNOS];
    int n = db_listar_todos(SIGA_FILE_ALUMNOS, alumnos, MAX_ALUMNOS, sizeof(Alumno));
    for (int i = 0; i < n; i++) {
        if (alumnos[i].materia_id == materia_id && strcmp(alumnos[i].cedula, cedula) == 0) return 1;
    }
    return 0;
}

void alumnos_json(int usuario_id, int materia_id, StrBuf *json) {
    Materia materia;
    Alumno alumnos[MAX_ALUMNOS];
    int first = 1;
    if (!docente_owns_materia(usuario_id, materia_id, &materia)) {
        sb_append(json, "{\"ok\":false,\"error\":\"No tienes acceso a esta materia\"}");
        return;
    }
    int n = db_listar_todos(SIGA_FILE_ALUMNOS, alumnos, MAX_ALUMNOS, sizeof(Alumno));
    sb_append(json, "{\"ok\":true,\"materia\":");
    sb_json_string(json, materia.nombre);
    sb_append(json, ",\"items\":[");
    for (int i = 0; i < n; i++) {
        if (alumnos[i].materia_id != materia_id) continue;
        if (!first) sb_append_char(json, ',');
        first = 0;
        sb_appendf(json, "{\"id\":%d,\"cedula\":", alumnos[i].id);
        sb_json_string(json, alumnos[i].cedula);
        sb_append(json, ",\"nombres\":");
        sb_json_string(json, alumnos[i].nombres);
        sb_append(json, ",\"apellidos\":");
        sb_json_string(json, alumnos[i].apellidos);
        sb_appendf(json, ",\"retirado\":%s}", alumnos[i].retirado ? "true" : "false");
    }
    sb_append(json, "]}");
}

int alumno_crear(int usuario_id, const ParamList *params, StrBuf *json) {
    int materia_id = params_get_int(params, "materia_id", 0);
    const char *cedula = params_get(params, "cedula");
    const char *nombres = params_get(params, "nombres");
    const char *apellidos = params_get(params, "apellidos");
    Alumno alumno;

    if (!docente_owns_materia(usuario_id, materia_id, NULL)) {
        sb_append(json, "{\"ok\":false,\"error\":\"No tienes acceso a esta materia\"}");
        return 403;
    }
    if (!cedula[0] || !nombres[0] || !apellidos[0]) {
        sb_append(json, "{\"ok\":false,\"error\":\"Cédula, nombres y apellidos son obligatorios\"}");
        return 400;
    }
    if (alumno_cedula_existe_en_materia(cedula, materia_id)) {
        sb_append(json, "{\"ok\":false,\"error\":\"Ya existe un alumno con esa cédula en la materia\"}");
        return 409;
    }

    memset(&alumno, 0, sizeof(alumno));
    alumno.id = db_siguiente_id(SIGA_FILE_ALUMNOS, sizeof(Alumno));
    alumno.materia_id = materia_id;
    alumno.retirado = 0;
    safe_copy(alumno.cedula, sizeof(alumno.cedula), cedula);
    safe_copy(alumno.nombres, sizeof(alumno.nombres), nombres);
    safe_copy(alumno.apellidos, sizeof(alumno.apellidos), apellidos);
    if (!db_insertar(SIGA_FILE_ALUMNOS, &alumno, sizeof(alumno))) {
        sb_append(json, "{\"ok\":false,\"error\":\"No se pudo guardar el alumno\"}");
        return 500;
    }
    audit_log(usuario_id, "CREAR_ALUMNO", "Alumno %s %s registrado en materia %d", nombres, apellidos, materia_id);
    sb_appendf(json, "{\"ok\":true,\"id\":%d}", alumno.id);
    return 200;
}

int alumno_retirar(int usuario_id, const ParamList *params, StrBuf *json) {
    int id = params_get_int(params, "id", 0);
    Alumno alumno;
    if (!db_buscar_por_id(SIGA_FILE_ALUMNOS, id, &alumno, sizeof(Alumno))) {
        sb_append(json, "{\"ok\":false,\"error\":\"Alumno no encontrado\"}");
        return 404;
    }
    if (!docente_owns_materia(usuario_id, alumno.materia_id, NULL)) {
        sb_append(json, "{\"ok\":false,\"error\":\"No tienes acceso a este alumno\"}");
        return 403;
    }
    alumno.retirado = 1;
    if (!db_actualizar(SIGA_FILE_ALUMNOS, id, &alumno, sizeof(alumno))) {
        sb_append(json, "{\"ok\":false,\"error\":\"No se pudo retirar el alumno\"}");
        return 500;
    }
    audit_log(usuario_id, "RETIRAR_ALUMNO", "Alumno %s retirado", alumno.cedula);
    sb_append(json, "{\"ok\":true}");
    return 200;
}
