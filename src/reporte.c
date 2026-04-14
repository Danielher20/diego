#include "reporte.h"

#include "calificacion.h"

void reporte_acta_json(int usuario_id, int materia_id, int admin_mode, StrBuf *json) {
    acta_json(usuario_id, materia_id, admin_mode, json);
}

void reporte_acta_csv(int usuario_id, int materia_id, int admin_mode, StrBuf *csv) {
    acta_csv(usuario_id, materia_id, admin_mode, csv);
}
