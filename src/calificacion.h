#ifndef SIGA_CALIFICACION_H
#define SIGA_CALIFICACION_H

#include "types.h"
#include "utils.h"

void calificaciones_json(int usuario_id, int materia_id, int evaluacion_id, StrBuf *json);
int calificaciones_guardar_lote(int usuario_id, const ParamList *params, StrBuf *json);
int calificacion_corregir(int usuario_id, const ParamList *params, StrBuf *json);
float calificacion_definitiva(int alumno_id, const Evaluacion *evals, int eval_count, int *completo);
void acta_json(int usuario_id, int materia_id, int admin_mode, StrBuf *json);
void acta_csv(int usuario_id, int materia_id, int admin_mode, StrBuf *csv);

#endif
