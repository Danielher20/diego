#include "calificacion.h"

#include "audit.h"
#include "db.h"
#include "docente.h"
#include "evaluacion.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ALUMNOS 2048
#define MAX_EVALS 512
#define MAX_NOTAS 10000
#define MAX_MATERIAS 512

static int docente_can_read_materia(int usuario_id, int materia_id, int admin_mode, Materia *out) {
    if (admin_mode) {
        Materia m;
        if (!db_buscar_por_id(SIGA_FILE_MATERIAS, materia_id, &m, sizeof(Materia)) || !m.activo) return 0;
        if (out) *out = m;
        return 1;
    }
    return docente_owns_materia(usuario_id, materia_id, out);
}

static int calificacion_por_alumno_eval(int alumno_id, int eval_id, Calificacion *out) {
    Calificacion notas[MAX_NOTAS];
    int n = db_listar_todos(SIGA_FILE_NOTAS, notas, MAX_NOTAS, sizeof(Calificacion));
    for (int i = 0; i < n; i++) {
        if (notas[i].alumno_id == alumno_id && notas[i].evaluacion_id == eval_id) {
            if (out) *out = notas[i];
            return 1;
        }
    }
    return 0;
}

static int alumno_de_materia(int alumno_id, int materia_id, Alumno *out) {
    Alumno a;
    if (!db_buscar_por_id(SIGA_FILE_ALUMNOS, alumno_id, &a, sizeof(Alumno))) return 0;
    if (a.materia_id != materia_id) return 0;
    if (out) *out = a;
    return 1;
}

void calificaciones_json(int usuario_id, int materia_id, int evaluacion_id, StrBuf *json) {
    Materia materia;
    Evaluacion eval;
    Alumno alumnos[MAX_ALUMNOS];
    int first = 1;
    if (!docente_owns_materia(usuario_id, materia_id, &materia)) {
        sb_append(json, "{\"ok\":false,\"error\":\"No tienes acceso a esta materia\"}");
        return;
    }
    if (!db_buscar_por_id(SIGA_FILE_EVALS, evaluacion_id, &eval, sizeof(Evaluacion)) || eval.materia_id != materia_id) {
        sb_append(json, "{\"ok\":false,\"error\":\"Evaluación no encontrada\"}");
        return;
    }
    int n = db_listar_todos(SIGA_FILE_ALUMNOS, alumnos, MAX_ALUMNOS, sizeof(Alumno));
    sb_append(json, "{\"ok\":true,\"items\":[");
    for (int i = 0; i < n; i++) {
        if (alumnos[i].materia_id != materia_id || alumnos[i].retirado) continue;
        Calificacion nota;
        int has = calificacion_por_alumno_eval(alumnos[i].id, evaluacion_id, &nota);
        if (!first) sb_append_char(json, ',');
        first = 0;
        sb_appendf(json, "{\"alumno_id\":%d,\"cedula\":", alumnos[i].id);
        sb_json_string(json, alumnos[i].cedula);
        sb_append(json, ",\"nombres\":");
        sb_json_string(json, alumnos[i].nombres);
        sb_append(json, ",\"apellidos\":");
        sb_json_string(json, alumnos[i].apellidos);
        if (has) sb_appendf(json, ",\"nota\":%.2f,\"modificada\":%s}", nota.nota, nota.modificada ? "true" : "false");
        else sb_append(json, ",\"nota\":null,\"modificada\":false}");
    }
    sb_append(json, "]}");
}

int calificaciones_guardar_lote(int usuario_id, const ParamList *params, StrBuf *json) {
    int evaluacion_id = params_get_int(params, "evaluacion_id", 0);
    const char *items = params_get(params, "items");
    const char *motivo = params_get(params, "motivo");
    Evaluacion eval;
    char copy[8192];
    int saved = 0;

    if (!db_buscar_por_id(SIGA_FILE_EVALS, evaluacion_id, &eval, sizeof(Evaluacion))) {
        sb_append(json, "{\"ok\":false,\"error\":\"Evaluación no encontrada\"}");
        return 404;
    }
    if (!docente_owns_materia(usuario_id, eval.materia_id, NULL)) {
        sb_append(json, "{\"ok\":false,\"error\":\"No tienes acceso a esta evaluación\"}");
        return 403;
    }
    if (!items[0]) {
        sb_append(json, "{\"ok\":false,\"error\":\"No hay calificaciones para guardar\"}");
        return 400;
    }

    safe_copy(copy, sizeof(copy), items);
    char *item = strtok(copy, ";");
    while (item) {
        char *sep = strchr(item, ':');
        int alumno_id;
        float nota_val;
        Alumno alumno;
        Calificacion nota;
        int exists;
        if (!sep) {
            sb_append(json, "{\"ok\":false,\"error\":\"Formato de calificaciones inválido\"}");
            return 400;
        }
        *sep = '\0';
        alumno_id = atoi(item);
        nota_val = (float)atof(sep + 1);
        if (nota_val < 0.0f || nota_val > 20.0f) {
            sb_append(json, "{\"ok\":false,\"error\":\"Todas las notas deben estar entre 0 y 20\"}");
            return 400;
        }
        if (!alumno_de_materia(alumno_id, eval.materia_id, &alumno) || alumno.retirado) {
            sb_append(json, "{\"ok\":false,\"error\":\"Alumno inválido para esta materia\"}");
            return 400;
        }
        exists = calificacion_por_alumno_eval(alumno_id, evaluacion_id, &nota);
        if (exists) {
            if (!float_equal_2(nota.nota, nota_val) && !motivo[0]) {
                safe_copy(nota.fecha_modificacion, sizeof(nota.fecha_modificacion), "");
                sb_append(json, "{\"ok\":false,\"error\":\"Para modificar notas existentes debes indicar un motivo\"}");
                return 400;
            }
            if (!float_equal_2(nota.nota, nota_val)) {
                nota.nota = nota_val;
                now_string(nota.fecha_modificacion);
                nota.modificada = 1;
                db_actualizar(SIGA_FILE_NOTAS, nota.id, &nota, sizeof(nota));
                audit_log(usuario_id, "CORRECCION", "Nota corregida por lote para alumno %s eval %d. Motivo: %s", alumno.cedula, evaluacion_id, motivo);
                saved++;
            }
        } else {
            memset(&nota, 0, sizeof(nota));
            nota.id = db_siguiente_id(SIGA_FILE_NOTAS, sizeof(Calificacion));
            nota.alumno_id = alumno_id;
            nota.evaluacion_id = evaluacion_id;
            nota.nota = nota_val;
            now_string(nota.fecha_carga);
            db_insertar(SIGA_FILE_NOTAS, &nota, sizeof(nota));
            audit_log(usuario_id, "CARGA_NOTA", "Nota %.2f cargada para alumno %s eval %d", nota_val, alumno.cedula, evaluacion_id);
            saved++;
        }
        item = strtok(NULL, ";");
    }
    sb_appendf(json, "{\"ok\":true,\"guardadas\":%d}", saved);
    return 200;
}

int calificacion_corregir(int usuario_id, const ParamList *params, StrBuf *json) {
    int alumno_id = params_get_int(params, "alumno_id", 0);
    int evaluacion_id = params_get_int(params, "evaluacion_id", 0);
    float nota_val = params_get_float(params, "nota", -1.0f);
    const char *motivo = params_get(params, "motivo");
    Evaluacion eval;
    Alumno alumno;
    Calificacion nota;

    if (nota_val < 0.0f || nota_val > 20.0f || !motivo[0]) {
        sb_append(json, "{\"ok\":false,\"error\":\"La nota debe estar entre 0 y 20 y el motivo es obligatorio\"}");
        return 400;
    }
    if (!db_buscar_por_id(SIGA_FILE_EVALS, evaluacion_id, &eval, sizeof(Evaluacion)) ||
        !docente_owns_materia(usuario_id, eval.materia_id, NULL) ||
        !alumno_de_materia(alumno_id, eval.materia_id, &alumno)) {
        sb_append(json, "{\"ok\":false,\"error\":\"No tienes acceso a esta calificación\"}");
        return 403;
    }
    if (!calificacion_por_alumno_eval(alumno_id, evaluacion_id, &nota)) {
        sb_append(json, "{\"ok\":false,\"error\":\"La nota aún no existe; usa la carga por lote\"}");
        return 404;
    }
    nota.nota = nota_val;
    now_string(nota.fecha_modificacion);
    nota.modificada = 1;
    if (!db_actualizar(SIGA_FILE_NOTAS, nota.id, &nota, sizeof(nota))) {
        sb_append(json, "{\"ok\":false,\"error\":\"No se pudo corregir la nota\"}");
        return 500;
    }
    audit_log(usuario_id, "CORRECCION", "Corrección alumno %s eval %d: %.2f. Motivo: %s", alumno.cedula, evaluacion_id, nota_val, motivo);
    sb_append(json, "{\"ok\":true}");
    return 200;
}

float calificacion_definitiva(int alumno_id, const Evaluacion *evals, int eval_count, int *completo) {
    float total = 0.0f;
    if (completo) *completo = 0;
    for (int i = 0; i < eval_count; i++) {
        Calificacion nota;
        if (!calificacion_por_alumno_eval(alumno_id, evals[i].id, &nota)) return 0.0f;
        total += nota.nota * evals[i].ponderacion;
    }
    if (completo) *completo = 1;
    return total;
}

static const char *condicion_text(float def, int completo, int retirado) {
    if (retirado) return "RETIRADO";
    if (!completo) return "INCOMPLETO";
    return def >= 10.0f ? "APROBADO" : "REPROBADO";
}

void acta_json(int usuario_id, int materia_id, int admin_mode, StrBuf *json) {
    Materia materia;
    Evaluacion evals[MAX_EVALS];
    Alumno alumnos[MAX_ALUMNOS];
    int ec;
    int ac;
    int aprobados = 0, reprobados = 0, retirados = 0, completos = 0;

    if (!docente_can_read_materia(usuario_id, materia_id, admin_mode, &materia)) {
        sb_append(json, "{\"ok\":false,\"error\":\"No tienes acceso a esta acta\"}");
        return;
    }
    ec = evaluaciones_de_materia(materia_id, evals, MAX_EVALS);
    ac = db_listar_todos(SIGA_FILE_ALUMNOS, alumnos, MAX_ALUMNOS, sizeof(Alumno));

    sb_append(json, "{\"ok\":true,\"materia\":{");
    sb_appendf(json, "\"id\":%d,\"codigo\":", materia.id);
    sb_json_string(json, materia.codigo);
    sb_append(json, ",\"nombre\":");
    sb_json_string(json, materia.nombre);
    sb_append(json, ",\"periodo\":");
    sb_json_string(json, materia.periodo);
    sb_append(json, "},\"evaluaciones\":[");
    for (int i = 0; i < ec; i++) {
        if (i) sb_append_char(json, ',');
        sb_appendf(json, "{\"id\":%d,\"nombre\":", evals[i].id);
        sb_json_string(json, evals[i].nombre);
        sb_appendf(json, ",\"ponderacion\":%.2f}", evals[i].ponderacion * 100.0f);
    }
    sb_append(json, "],\"alumnos\":[");
    int first = 1;
    for (int i = 0; i < ac; i++) {
        if (alumnos[i].materia_id != materia_id) continue;
        int completo = 0;
        float def = alumnos[i].retirado ? 0.0f : calificacion_definitiva(alumnos[i].id, evals, ec, &completo);
        const char *cond = condicion_text(def, completo, alumnos[i].retirado);
        if (alumnos[i].retirado) retirados++;
        else if (completo && def >= 10.0f) aprobados++;
        else if (completo) reprobados++;
        if (completo) completos++;
        if (!first) sb_append_char(json, ',');
        first = 0;
        sb_appendf(json, "{\"id\":%d,\"cedula\":", alumnos[i].id);
        sb_json_string(json, alumnos[i].cedula);
        sb_append(json, ",\"nombres\":");
        sb_json_string(json, alumnos[i].nombres);
        sb_append(json, ",\"apellidos\":");
        sb_json_string(json, alumnos[i].apellidos);
        sb_append(json, ",\"notas\":[");
        for (int j = 0; j < ec; j++) {
            Calificacion nota;
            if (j) sb_append_char(json, ',');
            if (calificacion_por_alumno_eval(alumnos[i].id, evals[j].id, &nota)) sb_appendf(json, "%.2f", nota.nota);
            else sb_append(json, "null");
        }
        sb_append(json, "],\"definitiva\":");
        if (completo && !alumnos[i].retirado) sb_appendf(json, "%.2f", def);
        else sb_append(json, "null");
        sb_append(json, ",\"condicion\":");
        sb_json_string(json, cond);
        sb_append(json, "}");
    }
    sb_appendf(json, "],\"resumen\":{\"aprobados\":%d,\"reprobados\":%d,\"retirados\":%d,\"completos\":%d}}",
               aprobados, reprobados, retirados, completos);
}

void acta_csv(int usuario_id, int materia_id, int admin_mode, StrBuf *csv) {
    Materia materia;
    Evaluacion evals[MAX_EVALS];
    Alumno alumnos[MAX_ALUMNOS];
    int ec;
    int ac;
    if (!docente_can_read_materia(usuario_id, materia_id, admin_mode, &materia)) {
        sb_append(csv, "error\nNo tienes acceso a esta acta\n");
        return;
    }
    ec = evaluaciones_de_materia(materia_id, evals, MAX_EVALS);
    ac = db_listar_todos(SIGA_FILE_ALUMNOS, alumnos, MAX_ALUMNOS, sizeof(Alumno));
    sb_append(csv, "Cedula,Apellidos,Nombres");
    for (int i = 0; i < ec; i++) {
        sb_append_char(csv, ',');
        sb_append(csv, evals[i].nombre);
    }
    sb_append(csv, ",Definitiva,Condicion\n");
    for (int i = 0; i < ac; i++) {
        if (alumnos[i].materia_id != materia_id) continue;
        int completo = 0;
        float def = alumnos[i].retirado ? 0.0f : calificacion_definitiva(alumnos[i].id, evals, ec, &completo);
        sb_appendf(csv, "%s,%s,%s", alumnos[i].cedula, alumnos[i].apellidos, alumnos[i].nombres);
        for (int j = 0; j < ec; j++) {
            Calificacion nota;
            sb_append_char(csv, ',');
            if (calificacion_por_alumno_eval(alumnos[i].id, evals[j].id, &nota)) sb_appendf(csv, "%.2f", nota.nota);
        }
        sb_append_char(csv, ',');
        if (completo && !alumnos[i].retirado) sb_appendf(csv, "%.2f", def);
        sb_append_char(csv, ',');
        sb_append(csv, condicion_text(def, completo, alumnos[i].retirado));
        sb_append_char(csv, '\n');
    }
}
