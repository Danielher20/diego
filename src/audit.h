#ifndef SIGA_AUDIT_H
#define SIGA_AUDIT_H

void audit_log(int usuario_id, const char *accion, const char *fmt, ...);

#endif
