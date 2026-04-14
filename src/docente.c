#include "docente.h"

#include "db.h"

#include <string.h>

#define MAX_DOCENTES 512
#define MAX_MATERIAS 512

int docente_por_usuario(int usuario_id, Docente *out) {
    Docente docentes[MAX_DOCENTES];
    int n = db_listar_todos(SIGA_FILE_DOCENTES, docentes, MAX_DOCENTES, sizeof(Docente));
    for (int i = 0; i < n; i++) {
        if (docentes[i].usuario_id == usuario_id && docentes[i].activo) {
            if (out) *out = docentes[i];
            return 1;
        }
    }
    return 0;
}

int docente_owns_materia(int usuario_id, int materia_id, Materia *out) {
    Docente docente;
    Materia materia;
    if (!docente_por_usuario(usuario_id, &docente)) return 0;
    if (!db_buscar_por_id(SIGA_FILE_MATERIAS, materia_id, &materia, sizeof(Materia))) return 0;
    if (!materia.activo || materia.docente_id != docente.id) return 0;
    if (out) *out = materia;
    return 1;
}

void docente_materias_json(int usuario_id, StrBuf *json) {
    Docente docente;
    Materia materias[MAX_MATERIAS];
    int first = 1;
    if (!docente_por_usuario(usuario_id, &docente)) {
        sb_append(json, "{\"ok\":false,\"error\":\"No existe perfil docente asociado\"}");
        return;
    }
    int n = db_listar_todos(SIGA_FILE_MATERIAS, materias, MAX_MATERIAS, sizeof(Materia));
    sb_append(json, "{\"ok\":true,\"items\":[");
    for (int i = 0; i < n; i++) {
        if (!materias[i].activo || materias[i].docente_id != docente.id) continue;
        if (!first) sb_append_char(json, ',');
        first = 0;
        sb_appendf(json, "{\"id\":%d,\"codigo\":", materias[i].id);
        sb_json_string(json, materias[i].codigo);
        sb_append(json, ",\"nombre\":");
        sb_json_string(json, materias[i].nombre);
        sb_append(json, ",\"periodo\":");
        sb_json_string(json, materias[i].periodo);
        sb_appendf(json, ",\"acta_generada\":%s}", materias[i].acta_generada ? "true" : "false");
    }
    sb_append(json, "]}");
}
