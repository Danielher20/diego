#include "evaluacion.h"

#include "audit.h"
#include "db.h"
#include "docente.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EVALS 512
#define MAX_NOTAS 10000

int evaluaciones_de_materia(int materia_id, Evaluacion *out, int max) {
    Evaluacion evals[MAX_EVALS];
    int n = db_listar_todos(SIGA_FILE_EVALS, evals, MAX_EVALS, sizeof(Evaluacion));
    int count = 0;
    for (int i = 0; i < n && count < max; i++) {
        if (evals[i].materia_id == materia_id) out[count++] = evals[i];
    }
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (out[j].orden < out[i].orden) {
                Evaluacion tmp = out[i];
                out[i] = out[j];
                out[j] = tmp;
            }
        }
    }
    return count;
}

int materia_tiene_notas(int materia_id) {
    Evaluacion evals[MAX_EVALS];
    Calificacion notas[MAX_NOTAS];
    int ec = evaluaciones_de_materia(materia_id, evals, MAX_EVALS);
    int nc = db_listar_todos(SIGA_FILE_NOTAS, notas, MAX_NOTAS, sizeof(Calificacion));
    for (int i = 0; i < nc; i++) {
        for (int j = 0; j < ec; j++) {
            if (notas[i].evaluacion_id == evals[j].id) return 1;
        }
    }
    return 0;
}

void evaluaciones_json(int usuario_id, int materia_id, StrBuf *json) {
    Evaluacion evals[MAX_EVALS];
    int n;
    if (!docente_owns_materia(usuario_id, materia_id, NULL)) {
        sb_append(json, "{\"ok\":false,\"error\":\"No tienes acceso a esta materia\"}");
        return;
    }
    n = evaluaciones_de_materia(materia_id, evals, MAX_EVALS);
    sb_appendf(json, "{\"ok\":true,\"bloqueado\":%s,\"items\":[", materia_tiene_notas(materia_id) ? "true" : "false");
    for (int i = 0; i < n; i++) {
        if (i) sb_append_char(json, ',');
        sb_appendf(json, "{\"id\":%d,\"nombre\":", evals[i].id);
        sb_json_string(json, evals[i].nombre);
        sb_append(json, ",\"tipo\":");
        sb_json_string(json, tipo_to_string(evals[i].tipo));
        sb_appendf(json, ",\"ponderacion\":%.2f,\"orden\":%d}", evals[i].ponderacion * 100.0f, evals[i].orden);
    }
    sb_append(json, "]}");
}

static int remove_eval_materia(const void *registro, void *ctx) {
    const Evaluacion *ev = (const Evaluacion *)registro;
    int materia_id = *(int *)ctx;
    return ev->materia_id == materia_id;
}

int evaluacion_guardar_plan(int usuario_id, const ParamList *params, StrBuf *json) {
    int materia_id = params_get_int(params, "materia_id", 0);
    const char *items = params_get(params, "items");
    char copy[4096];
    float total = 0.0f;
    int count = 0;
    Evaluacion parsed[10];

    if (!docente_owns_materia(usuario_id, materia_id, NULL)) {
        sb_append(json, "{\"ok\":false,\"error\":\"No tienes acceso a esta materia\"}");
        return 403;
    }
    if (materia_tiene_notas(materia_id)) {
        sb_append(json, "{\"ok\":false,\"error\":\"El plan está bloqueado porque ya existen calificaciones\"}");
        return 409;
    }
    if (!items[0]) {
        sb_append(json, "{\"ok\":false,\"error\":\"Debe enviar al menos una evaluación\"}");
        return 400;
    }

    safe_copy(copy, sizeof(copy), items);
    char *item = strtok(copy, ";");
    while (item && count < 10) {
        char *p1 = strchr(item, '|');
        char *p2 = p1 ? strchr(p1 + 1, '|') : NULL;
        float pond;
        if (!p1 || !p2) {
            sb_append(json, "{\"ok\":false,\"error\":\"Formato de plan inválido\"}");
            return 400;
        }
        *p1 = '\0';
        *p2 = '\0';
        pond = (float)atof(p2 + 1);
        if (!item[0] || pond < 5.0f || pond > 100.0f) {
            sb_append(json, "{\"ok\":false,\"error\":\"Cada evaluación requiere nombre y ponderación entre 5% y 100%\"}");
            return 400;
        }
        memset(&parsed[count], 0, sizeof(Evaluacion));
        parsed[count].id = db_siguiente_id(SIGA_FILE_EVALS, sizeof(Evaluacion)) + count;
        parsed[count].materia_id = materia_id;
        safe_copy(parsed[count].nombre, sizeof(parsed[count].nombre), item);
        parsed[count].tipo = tipo_from_string(p1 + 1);
        parsed[count].ponderacion = pond / 100.0f;
        parsed[count].orden = count + 1;
        total += pond;
        count++;
        item = strtok(NULL, ";");
    }
    if (count == 0 || count > 10) {
        sb_append(json, "{\"ok\":false,\"error\":\"El plan debe tener entre 1 y 10 evaluaciones\"}");
        return 400;
    }
    if (!float_equal_2(total, 100.0f)) {
        sb_appendf(json, "{\"ok\":false,\"error\":\"Las ponderaciones suman %.2f%%. Deben sumar exactamente 100%%\"}", total);
        return 400;
    }

    db_eliminar_filtrados(SIGA_FILE_EVALS, sizeof(Evaluacion), remove_eval_materia, &materia_id);
    for (int i = 0; i < count; i++) {
        parsed[i].id = db_siguiente_id(SIGA_FILE_EVALS, sizeof(Evaluacion));
        db_insertar(SIGA_FILE_EVALS, &parsed[i], sizeof(Evaluacion));
    }
    audit_log(usuario_id, "PLAN_EVALUACION", "Plan de evaluación guardado para materia %d con %d items", materia_id, count);
    sb_append(json, "{\"ok\":true}");
    return 200;
}
