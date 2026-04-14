#ifndef SIGA_REPORTE_H
#define SIGA_REPORTE_H

#include "utils.h"

void reporte_acta_json(int usuario_id, int materia_id, int admin_mode, StrBuf *json);
void reporte_acta_csv(int usuario_id, int materia_id, int admin_mode, StrBuf *csv);

#endif
