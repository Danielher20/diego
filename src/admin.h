#ifndef SIGA_ADMIN_H
#define SIGA_ADMIN_H

#include "types.h"
#include "utils.h"

void admin_docentes_json(StrBuf *json);
int admin_crear_docente(int usuario_id, const ParamList *params, StrBuf *json);
int admin_suspender_docente(int usuario_id, const ParamList *params, StrBuf *json);
int admin_reset_docente(int usuario_id, const ParamList *params, StrBuf *json);
void admin_materias_json(StrBuf *json);
int admin_crear_materia(int usuario_id, const ParamList *params, StrBuf *json);
int admin_asignar_materia(int usuario_id, const ParamList *params, StrBuf *json);
void admin_stats_json(StrBuf *json);
void admin_logs_json(StrBuf *json);

#endif
