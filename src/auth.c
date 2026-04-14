#include "auth.h"

#include "audit.h"
#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_USERS 512
#define MAX_SESSIONS 128
#define SESSION_TTL_SECONDS (8 * 60 * 60)
#define LOCK_SECONDS (15 * 60)

typedef struct {
    char token[65];
    int usuario_id;
    char rol[10];
    time_t last_seen;
    int active;
} Session;

typedef struct {
    char username[50];
    int failed;
    time_t locked_until;
} LoginGuard;

static Session sessions[MAX_SESSIONS];
static LoginGuard guards[MAX_USERS];

static LoginGuard *guard_for(const char *username) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (guards[i].username[0] && strcmp(guards[i].username, username) == 0) return &guards[i];
    }
    for (int i = 0; i < MAX_USERS; i++) {
        if (!guards[i].username[0]) {
            safe_copy(guards[i].username, sizeof(guards[i].username), username);
            return &guards[i];
        }
    }
    return NULL;
}

int auth_create_user(const char *username, const char *password, const char *nombre,
                     const char *rol, int debe_cambiar, Usuario *out) {
    Usuario exists;
    Usuario user;
    if (!username || !*username || !password || !*password || !rol || !*rol) return 0;
    if (auth_find_user_by_username(username, &exists)) return 0;

    user.id = db_siguiente_id(SIGA_FILE_USUARIOS, sizeof(Usuario));
    safe_copy(user.username, sizeof(user.username), username);
    sha256_hex(password, user.password_hash);
    safe_copy(user.nombre_completo, sizeof(user.nombre_completo), nombre ? nombre : username);
    safe_copy(user.rol, sizeof(user.rol), rol);
    user.activo = 1;
    user.debe_cambiar_password = debe_cambiar;
    now_string(user.fecha_creacion);
    if (!db_insertar(SIGA_FILE_USUARIOS, &user, sizeof(user))) return 0;
    if (out) *out = user;
    return 1;
}

int auth_get_user(int id, Usuario *out) {
    return db_buscar_por_id(SIGA_FILE_USUARIOS, id, out, sizeof(Usuario));
}

int auth_find_user_by_username(const char *username, Usuario *out) {
    Usuario users[MAX_USERS];
    int n = db_listar_todos(SIGA_FILE_USUARIOS, users, MAX_USERS, sizeof(Usuario));
    for (int i = 0; i < n; i++) {
        if (strcmp(users[i].username, username) == 0) {
            if (out) *out = users[i];
            return 1;
        }
    }
    return 0;
}

int auth_reset_password(int usuario_id, char *temp_password, size_t temp_size) {
    Usuario user;
    if (!auth_get_user(usuario_id, &user)) return 0;
    generate_temp_password(temp_password, temp_size);
    sha256_hex(temp_password, user.password_hash);
    user.debe_cambiar_password = 1;
    return db_actualizar(SIGA_FILE_USUARIOS, usuario_id, &user, sizeof(user));
}

static Session *new_session(const Usuario *user) {
    time_t now = time(NULL);
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!sessions[i].active || now - sessions[i].last_seen > SESSION_TTL_SECONDS) {
            memset(&sessions[i], 0, sizeof(Session));
            random_hex(sessions[i].token, 32);
            sessions[i].usuario_id = user->id;
            safe_copy(sessions[i].rol, sizeof(sessions[i].rol), user->rol);
            sessions[i].last_seen = now;
            sessions[i].active = 1;
            return &sessions[i];
        }
    }
    return NULL;
}

int auth_login(const ParamList *params, StrBuf *json) {
    const char *username = params_get(params, "username");
    const char *password = params_get(params, "password");
    Usuario user;
    char hash[65];
    time_t now = time(NULL);
    LoginGuard *guard = guard_for(username);

    if (!username[0] || !password[0]) {
        sb_append(json, "{\"ok\":false,\"error\":\"Usuario y contraseña son obligatorios\"}");
        return 400;
    }
    if (guard && guard->locked_until > now) {
        int minutes = (int)((guard->locked_until - now + 59) / 60);
        sb_appendf(json, "{\"ok\":false,\"error\":\"Cuenta bloqueada. Intente de nuevo en %d minuto(s)\"}", minutes);
        return 423;
    }
    if (!auth_find_user_by_username(username, &user) || !user.activo) {
        if (guard) guard->failed++;
        sb_append(json, "{\"ok\":false,\"error\":\"Usuario o contraseña incorrectos\"}");
        return 401;
    }

    sha256_hex(password, hash);
    if (strcmp(hash, user.password_hash) != 0) {
        if (guard) {
            guard->failed++;
            if (guard->failed >= 5) {
                guard->locked_until = now + LOCK_SECONDS;
                guard->failed = 0;
            }
        }
        audit_log(user.id, "LOGIN_FALLIDO", "Intento fallido para usuario %s", username);
        sb_append(json, "{\"ok\":false,\"error\":\"Usuario o contraseña incorrectos\"}");
        return 401;
    }

    if (guard) {
        guard->failed = 0;
        guard->locked_until = 0;
    }
    Session *session = new_session(&user);
    if (!session) {
        sb_append(json, "{\"ok\":false,\"error\":\"No hay espacio para nuevas sesiones\"}");
        return 503;
    }
    audit_log(user.id, "LOGIN", "Inicio de sesión correcto");

    sb_append(json, "{\"ok\":true,\"token\":");
    sb_json_string(json, session->token);
    sb_append(json, ",\"usuario\":{\"id\":");
    sb_appendf(json, "%d,\"username\":", user.id);
    sb_json_string(json, user.username);
    sb_append(json, ",\"nombre\":");
    sb_json_string(json, user.nombre_completo);
    sb_append(json, ",\"rol\":");
    sb_json_string(json, user.rol);
    sb_appendf(json, ",\"debe_cambiar_password\":%s}}", user.debe_cambiar_password ? "true" : "false");
    return 200;
}

int auth_logout(const char *token) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].active && strcmp(sessions[i].token, token ? token : "") == 0) {
            sessions[i].active = 0;
            return 1;
        }
    }
    return 0;
}

int auth_require(const char *token, const char *required_role, Usuario *out, StrBuf *json_error) {
    time_t now = time(NULL);
    if (!token || !*token) {
        if (json_error) sb_append(json_error, "{\"ok\":false,\"error\":\"Sesión requerida\"}");
        return 401;
    }
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].active && strcmp(sessions[i].token, token) == 0) {
            if (now - sessions[i].last_seen > SESSION_TTL_SECONDS) {
                sessions[i].active = 0;
                if (json_error) sb_append(json_error, "{\"ok\":false,\"error\":\"Sesión expirada\"}");
                return 401;
            }
            Usuario user;
            if (!auth_get_user(sessions[i].usuario_id, &user) || !user.activo) {
                if (json_error) sb_append(json_error, "{\"ok\":false,\"error\":\"Usuario inactivo\"}");
                return 403;
            }
            if (required_role && *required_role && strcmp(user.rol, required_role) != 0) {
                if (json_error) sb_append(json_error, "{\"ok\":false,\"error\":\"Acceso denegado\"}");
                return 403;
            }
            sessions[i].last_seen = now;
            if (out) *out = user;
            return 200;
        }
    }
    if (json_error) sb_append(json_error, "{\"ok\":false,\"error\":\"Token inválido\"}");
    return 401;
}
