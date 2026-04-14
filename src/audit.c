#include "audit.h"

#include "db.h"
#include "types.h"
#include "utils.h"

#include <stdarg.h>
#include <stdio.h>

void audit_log(int usuario_id, const char *accion, const char *fmt, ...) {
    LogAuditoria log;
    va_list ap;
    log.id = db_siguiente_id(SIGA_FILE_LOGS, sizeof(LogAuditoria));
    log.usuario_id = usuario_id;
    safe_copy(log.accion, sizeof(log.accion), accion ? accion : "ACCION");
    now_string(log.timestamp);
    va_start(ap, fmt);
    vsnprintf(log.detalle, sizeof(log.detalle), fmt ? fmt : "", ap);
    va_end(ap);
    db_insertar(SIGA_FILE_LOGS, &log, sizeof(log));
}
