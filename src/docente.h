#ifndef SIGA_DOCENTE_H
#define SIGA_DOCENTE_H

#include "types.h"
#include "utils.h"

int docente_por_usuario(int usuario_id, Docente *out);
int docente_owns_materia(int usuario_id, int materia_id, Materia *out);
void docente_materias_json(int usuario_id, StrBuf *json);

#endif
