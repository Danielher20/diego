#include "admin.h"

#include "audit.h"
#include "auth.h"
#include "calificacion.h"
#include "db.h"
#include "evaluacion.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_DOCENTES 512
#define MAX_MATERIAS 512
#define MAX_ALUMNOS 2048
#define MAX_LOGS 1000
#define MAX_EVALS 512

static void docente_nombre(const Docente *d, char *out, size_t out_size) {
    snprintf(out, out_size, "%s %s", d->nombres, d->apellidos);
}

static int docente_por_id(int id, Docente *out) {
    Docente tmp;
    return db_buscar_por_id(SIGA_FILE_DOCENTES, id, out ? out : &tmp, sizeof(Docente));
}

static void append_docente_json(StrBuf *json, const Docente *d) {
    Usuario u;
    Materia materias[MAX_MATERIAS];
    int mc = db_listar_todos(SIGA_FILE_MATERIAS, materias, MAX_MATERIAS, sizeof(Materia));
    int first_mat = 1;
    sb_appendf(json, "{\"id\":%d,\"usuario_id\":%d,\"cedula\":", d->id, d->usuario_id);
    sb_json_string(json, d->cedula);
    sb_append(json, ",\"nombres\":");
    sb_json_string(json, d->nombres);
    sb_append(json, ",\"apellidos\":");
    sb_json_string(json, d->apellidos);
    sb_append(json, ",\"email\":");
    sb_json_string(json, d->email);
    sb_appendf(json, ",\"activo\":%s,\"username\":", d->activo ? "true" : "false");
    if (auth_get_user(d->usuario_id, &u)) sb_json_string(json, u.username);
    else sb_json_string(json, "");
    sb_append(json, ",\"materias\":[");
    for (int i = 0; i < mc; i++) {
        if (materias[i].docente_id != d->id || !materias[i].activo) continue;
        if (!first_mat) sb_append_char(json, ',');
        first_mat = 0;
        sb_json_string(json, materias[i].nombre);
    }
    sb_append(json, "]}");
}

void admin_docentes_json(StrBuf *json) {
    Docente docentes[MAX_DOCENTES];
    int n = db_listar_todos(SIGA_FILE_DOCENTES, docentes, MAX_DOCENTES, sizeof(Docente));
    int first = 1;
    sb_append(json, "{\"ok\":true,\"items\":[");
    for (int i = 0; i < n; i++) {
        if (!first) sb_append_char(json, ',');
        first = 0;
        append_docente_json(json, &docentes[i]);
    }
    sb_append(json, "]}");
}

int admin_crear_docente(int usuario_id, const ParamList *params, StrBuf *json) {
    const char *cedula = params_get(params, "cedula");
    const char *nombres = params_get(params, "nombres");
    const char *apellidos = params_get(params, "apellidos");
    const char *email = params_get(params, "email");
    Docente docentes[MAX_DOCENTES];
    Docente docente;
    Usuario user;
    char username[50];
    char full_name[140];
    char temp[32];
    int next_docente_id;

    if (!cedula[0] || !nombres[0] || !apellidos[0] || !email[0]) {
        sb_append(json, "{\"ok\":false,\"error\":\"Todos los campos del docente son obligatorios\"}");
        return 400;
    }
    int n = db_listar_todos(SIGA_FILE_DOCENTES, docentes, MAX_DOCENTES, sizeof(Docente));
    for (int i = 0; i < n; i++) {
        if (strcmp(docentes[i].cedula, cedula) == 0) {
            sb_append(json, "{\"ok\":false,\"error\":\"Ya existe un docente con esa cédula\"}");
            return 409;
        }
    }

    next_docente_id = db_siguiente_id(SIGA_FILE_DOCENTES, sizeof(Docente));
    snprintf(username, sizeof(username), "docente%d", next_docente_id);
    snprintf(full_name, sizeof(full_name), "%s %s", nombres, apellidos);
    generate_temp_password(temp, sizeof(temp));
    if (!auth_create_user(username, temp, full_name, SIGA_ROLE_DOCENTE, 1, &user)) {
        sb_append(json, "{\"ok\":false,\"error\":\"No se pudo crear el usuario del docente\"}");
        return 500;
    }

    memset(&docente, 0, sizeof(docente));
    docente.id = next_docente_id;
    docente.usuario_id = user.id;
    docente.activo = 1;
    safe_copy(docente.cedula, sizeof(docente.cedula), cedula);
    safe_copy(docente.nombres, sizeof(docente.nombres), nombres);
    safe_copy(docente.apellidos, sizeof(docente.apellidos), apellidos);
    safe_copy(docente.email, sizeof(docente.email), email);
    if (!db_insertar(SIGA_FILE_DOCENTES, &docente, sizeof(docente))) {
        sb_append(json, "{\"ok\":false,\"error\":\"No se pudo guardar el docente\"}");
        return 500;
    }
    audit_log(usuario_id, "CREAR_DOCENTE", "Docente %s creado con usuario %s", full_name, username);
    sb_append(json, "{\"ok\":true,\"docente\":");
    append_docente_json(json, &docente);
    sb_append(json, ",\"credenciales\":{\"username\":");
    sb_json_string(json, username);
    sb_append(json, ",\"password_temporal\":");
    sb_json_string(json, temp);
    sb_append(json, "}}");
    return 200;
}

int admin_suspender_docente(int usuario_id, const ParamList *params, StrBuf *json) {
    int id = params_get_int(params, "id", 0);
    Docente d;
    Usuario u;
    if (!docente_por_id(id, &d)) {
        sb_append(json, "{\"ok\":false,\"error\":\"Docente no encontrado\"}");
        return 404;
    }
    d.activo = 0;
    db_actualizar(SIGA_FILE_DOCENTES, id, &d, sizeof(d));
    if (auth_get_user(d.usuario_id, &u)) {
        u.activo = 0;
        db_actualizar(SIGA_FILE_USUARIOS, u.id, &u, sizeof(u));
    }
    audit_log(usuario_id, "SUSPENDER_DOCENTE", "Docente %d suspendido", id);
    sb_append(json, "{\"ok\":true}");
    return 200;
}

int admin_reset_docente(int usuario_id, const ParamList *params, StrBuf *json) {
    int id = params_get_int(params, "id", 0);
    Docente d;
    char temp[32];
    if (!docente_por_id(id, &d)) {
        sb_append(json, "{\"ok\":false,\"error\":\"Docente no encontrado\"}");
        return 404;
    }
    if (!auth_reset_password(d.usuario_id, temp, sizeof(temp))) {
        sb_append(json, "{\"ok\":false,\"error\":\"No se pudo resetear la contraseña\"}");
        return 500;
    }
    audit_log(usuario_id, "RESET_PASSWORD", "Contraseña temporal regenerada para docente %d", id);
    sb_append(json, "{\"ok\":true,\"password_temporal\":");
    sb_json_string(json, temp);
    sb_append(json, "}");
    return 200;
}

void admin_materias_json(StrBuf *json) {
    Materia materias[MAX_MATERIAS];
    Docente docentes[MAX_DOCENTES];
    int mc = db_listar_todos(SIGA_FILE_MATERIAS, materias, MAX_MATERIAS, sizeof(Materia));
    int dc = db_listar_todos(SIGA_FILE_DOCENTES, docentes, MAX_DOCENTES, sizeof(Docente));
    int first = 1;
    sb_append(json, "{\"ok\":true,\"items\":[");
    for (int i = 0; i < mc; i++) {
        char docente_name[140] = "Sin asignar";
        if (!materias[i].activo) continue;
        for (int j = 0; j < dc; j++) {
            if (docentes[j].id == materias[i].docente_id) docente_nombre(&docentes[j], docente_name, sizeof(docente_name));
        }
        if (!first) sb_append_char(json, ',');
        first = 0;
        sb_appendf(json, "{\"id\":%d,\"codigo\":", materias[i].id);
        sb_json_string(json, materias[i].codigo);
        sb_append(json, ",\"nombre\":");
        sb_json_string(json, materias[i].nombre);
        sb_append(json, ",\"periodo\":");
        sb_json_string(json, materias[i].periodo);
        sb_appendf(json, ",\"docente_id\":%d,\"docente\":", materias[i].docente_id);
        sb_json_string(json, docente_name);
        sb_appendf(json, ",\"acta_generada\":%s}", materias[i].acta_generada ? "true" : "false");
    }
    sb_append(json, "]}");
}

int admin_crear_materia(int usuario_id, const ParamList *params, StrBuf *json) {
    const char *codigo = params_get(params, "codigo");
    const char *nombre = params_get(params, "nombre");
    const char *periodo = params_get(params, "periodo");
    int docente_id = params_get_int(params, "docente_id", 0);
    Materia materias[MAX_MATERIAS];
    Materia m;
    int n;
    if (!codigo[0] || !nombre[0] || !periodo[0]) {
        sb_append(json, "{\"ok\":false,\"error\":\"Código, nombre y período son obligatorios\"}");
        return 400;
    }
    if (docente_id && !docente_por_id(docente_id, NULL)) {
        sb_append(json, "{\"ok\":false,\"error\":\"Docente asignado no existe\"}");
        return 400;
    }
    n = db_listar_todos(SIGA_FILE_MATERIAS, materias, MAX_MATERIAS, sizeof(Materia));
    for (int i = 0; i < n; i++) {
        if (strcmp(materias[i].codigo, codigo) == 0 && strcmp(materias[i].periodo, periodo) == 0) {
            sb_append(json, "{\"ok\":false,\"error\":\"Ya existe esa materia en el período indicado\"}");
            return 409;
        }
    }
    memset(&m, 0, sizeof(m));
    m.id = db_siguiente_id(SIGA_FILE_MATERIAS, sizeof(Materia));
    m.docente_id = docente_id;
    m.activo = 1;
    safe_copy(m.codigo, sizeof(m.codigo), codigo);
    safe_copy(m.nombre, sizeof(m.nombre), nombre);
    safe_copy(m.periodo, sizeof(m.periodo), periodo);
    if (!db_insertar(SIGA_FILE_MATERIAS, &m, sizeof(m))) {
        sb_append(json, "{\"ok\":false,\"error\":\"No se pudo guardar la materia\"}");
        return 500;
    }
    audit_log(usuario_id, "CREAR_MATERIA", "Materia %s %s creada", codigo, nombre);
    sb_appendf(json, "{\"ok\":true,\"id\":%d}", m.id);
    return 200;
}

int admin_asignar_materia(int usuario_id, const ParamList *params, StrBuf *json) {
    int materia_id = params_get_int(params, "id", 0);
    int docente_id = params_get_int(params, "docente_id", 0);
    Materia m;
    Docente d;
    if (!db_buscar_por_id(SIGA_FILE_MATERIAS, materia_id, &m, sizeof(Materia)) || !m.activo) {
        sb_append(json, "{\"ok\":false,\"error\":\"Materia no encontrada\"}");
        return 404;
    }
    if (docente_id && (!docente_por_id(docente_id, &d) || !d.activo)) {
        sb_append(json, "{\"ok\":false,\"error\":\"Docente no encontrado o inactivo\"}");
        return 400;
    }
    m.docente_id = docente_id;
    if (!db_actualizar(SIGA_FILE_MATERIAS, materia_id, &m, sizeof(m))) {
        sb_append(json, "{\"ok\":false,\"error\":\"No se pudo asignar la materia\"}");
        return 500;
    }
    audit_log(usuario_id, "ASIGNAR_MATERIA", "Materia %d asignada a docente %d", materia_id, docente_id);
    sb_append(json, "{\"ok\":true}");
    return 200;
}

void admin_stats_json(StrBuf *json) {
    Docente docentes[MAX_DOCENTES];
    Materia materias[MAX_MATERIAS];
    Alumno alumnos[MAX_ALUMNOS];
    int dc = db_listar_todos(SIGA_FILE_DOCENTES, docentes, MAX_DOCENTES, sizeof(Docente));
    int mc = db_listar_todos(SIGA_FILE_MATERIAS, materias, MAX_MATERIAS, sizeof(Materia));
    int ac = db_listar_todos(SIGA_FILE_ALUMNOS, alumnos, MAX_ALUMNOS, sizeof(Alumno));
    int docentes_activos = 0, materias_activas = 0, actas = 0, aprobados = 0, reprobados = 0;
    for (int i = 0; i < dc; i++) if (docentes[i].activo) docentes_activos++;
    for (int i = 0; i < mc; i++) {
        if (!materias[i].activo) continue;
        materias_activas++;
        if (materias[i].acta_generada) actas++;
        Evaluacion evals[MAX_EVALS];
        int ec = evaluaciones_de_materia(materias[i].id, evals, MAX_EVALS);
        for (int j = 0; j < ac; j++) {
            if (alumnos[j].materia_id != materias[i].id || alumnos[j].retirado || ec == 0) continue;
            int completo = 0;
            float def = calificacion_definitiva(alumnos[j].id, evals, ec, &completo);
            if (!completo) continue;
            if (def >= 10.0f) aprobados++;
            else reprobados++;
        }
    }
    sb_appendf(json,
        "{\"ok\":true,\"docentes_activos\":%d,\"materias_activas\":%d,\"total_alumnos\":%d,"
        "\"actas_generadas\":%d,\"aprobados\":%d,\"reprobados\":%d}",
        docentes_activos, materias_activas, ac, actas, aprobados, reprobados);
}

void admin_logs_json(StrBuf *json) {
    LogAuditoria logs[MAX_LOGS];
    int n = db_listar_todos(SIGA_FILE_LOGS, logs, MAX_LOGS, sizeof(LogAuditoria));
    sb_append(json, "{\"ok\":true,\"items\":[");
    for (int i = n - 1, shown = 0; i >= 0 && shown < 100; i--, shown++) {
        if (shown) sb_append_char(json, ',');
        sb_appendf(json, "{\"id\":%d,\"usuario_id\":%d,\"accion\":", logs[i].id, logs[i].usuario_id);
        sb_json_string(json, logs[i].accion);
        sb_append(json, ",\"detalle\":");
        sb_json_string(json, logs[i].detalle);
        sb_append(json, ",\"timestamp\":");
        sb_json_string(json, logs[i].timestamp);
        sb_append(json, "}");
    }
    sb_append(json, "]}");
}
