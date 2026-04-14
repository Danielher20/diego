#ifndef SIGA_EVALUACION_H
#define SIGA_EVALUACION_H

#include "types.h"
#include "utils.h"

void evaluaciones_json(int usuario_id, int materia_id, StrBuf *json);
int evaluacion_guardar_plan(int usuario_id, const ParamList *params, StrBuf *json);
int evaluaciones_de_materia(int materia_id, Evaluacion *out, int max);
int materia_tiene_notas(int materia_id);

#endif
