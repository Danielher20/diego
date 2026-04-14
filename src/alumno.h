#ifndef SIGA_ALUMNO_H
#define SIGA_ALUMNO_H

#include "types.h"
#include "utils.h"

void alumnos_json(int usuario_id, int materia_id, StrBuf *json);
int alumno_crear(int usuario_id, const ParamList *params, StrBuf *json);
int alumno_retirar(int usuario_id, const ParamList *params, StrBuf *json);

#endif
