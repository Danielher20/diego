#ifndef SIGA_AUTH_H
#define SIGA_AUTH_H

#include "types.h"
#include "utils.h"

int auth_create_user(const char *username, const char *password, const char *nombre,
                     const char *rol, int debe_cambiar, Usuario *out);
int auth_get_user(int id, Usuario *out);
int auth_find_user_by_username(const char *username, Usuario *out);
int auth_reset_password(int usuario_id, char *temp_password, size_t temp_size);
int auth_login(const ParamList *params, StrBuf *json);
int auth_logout(const char *token);
int auth_require(const char *token, const char *required_role, Usuario *out, StrBuf *json_error);

#endif
