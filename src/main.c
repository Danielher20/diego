#include "admin.h"
#include "alumno.h"
#include "auth.h"
#include "audit.h"
#include "calificacion.h"
#include "db.h"
#include "docente.h"
#include "evaluacion.h"
#include "reporte.h"
#include "types.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#define CLOSESOCKET closesocket
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int socket_t;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define CLOSESOCKET close
#endif

#define PORT 8080
#define REQ_BUF_SIZE 131072

typedef struct {
    char method[8];
    char target[1024];
    char path[1024];
    char query[4096];
    char body[65536];
} HttpRequest;

static void merge_params(ParamList *dst, const ParamList *src) {
    for (int i = 0; i < src->count && dst->count < SIGA_MAX_PARAMS; i++) {
        dst->items[dst->count++] = src->items[i];
    }
}

static const char *status_text(int status) {
    switch (status) {
        case 200: return "OK";
        case 201: return "Created";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 409: return "Conflict";
        case 423: return "Locked";
        case 500: return "Internal Server Error";
        case 503: return "Service Unavailable";
        default: return "OK";
    }
}

static void send_all(socket_t client, const char *data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        int n = send(client, data + sent, (int)(len - sent), 0);
        if (n <= 0) return;
        sent += (size_t)n;
    }
}

static void send_response(socket_t client, int status, const char *content_type, const char *body) {
    char header[512];
    size_t body_len = body ? strlen(body) : 0;
    snprintf(header, sizeof(header),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: %s; charset=utf-8\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "Cache-Control: no-store\r\n"
             "\r\n",
             status, status_text(status), content_type ? content_type : "text/plain", body_len);
    send_all(client, header, strlen(header));
    if (body_len) send_all(client, body, body_len);
}

static const char *mime_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "text/plain";
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".csv") == 0) return "text/csv";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".svg") == 0) return "image/svg+xml";
    return "text/plain";
}

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    long size;
    char *data;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = (char *)malloc((size_t)size + 1);
    if (!data) {
        fclose(f);
        return NULL;
    }
    fread(data, 1, (size_t)size, f);
    data[size] = '\0';
    fclose(f);
    return data;
}

static void send_static(socket_t client, const char *path) {
    char file_path[2048];
    char *data;
    if (strstr(path, "..")) {
        send_response(client, 403, "text/plain", "Forbidden");
        return;
    }
    if (strcmp(path, "/") == 0) {
        snprintf(file_path, sizeof(file_path), "frontend/index.html");
    } else if (strncmp(path, "/css/", 5) == 0 || strncmp(path, "/js/", 4) == 0) {
        snprintf(file_path, sizeof(file_path), "frontend%s", path);
    } else if (strcmp(path, "/siga.html") == 0) {
        snprintf(file_path, sizeof(file_path), "siga.html");
    } else {
        send_response(client, 404, "text/plain", "Not found");
        return;
    }
    data = read_file(file_path);
    if (!data) {
        send_response(client, 404, "text/plain", "Not found");
        return;
    }
    send_response(client, 200, mime_type(file_path), data);
    free(data);
}

static int mark_acta_generada(int usuario_id, int materia_id, StrBuf *json) {
    Materia m;
    if (!docente_owns_materia(usuario_id, materia_id, &m)) {
        sb_append(json, "{\"ok\":false,\"error\":\"No tienes acceso a esta materia\"}");
        return 403;
    }
    m.acta_generada = 1;
    now_string(m.fecha_acta);
    if (!db_actualizar(SIGA_FILE_MATERIAS, m.id, &m, sizeof(m))) {
        sb_append(json, "{\"ok\":false,\"error\":\"No se pudo marcar el acta\"}");
        return 500;
    }
    audit_log(usuario_id, "GENERAR_ACTA", "Acta generada para materia %d", materia_id);
    sb_append(json, "{\"ok\":true}");
    return 200;
}

static int handle_api(const HttpRequest *req, const ParamList *params, StrBuf *json, const char **content_type) {
    Usuario user;
    int status;
    const char *token = params_get(params, "token");
    *content_type = "application/json";

    if (strcmp(req->path, "/api/health") == 0) {
        sb_append(json, "{\"ok\":true,\"service\":\"SiGA\",\"version\":\"1.0\"}");
        return 200;
    }
    if (strcmp(req->path, "/api/login") == 0 && strcmp(req->method, "POST") == 0) {
        return auth_login(params, json);
    }
    if (strcmp(req->path, "/api/logout") == 0 && strcmp(req->method, "POST") == 0) {
        auth_logout(token);
        sb_append(json, "{\"ok\":true}");
        return 200;
    }

    if (strcmp(req->path, "/api/session") == 0) {
        status = auth_require(token, NULL, &user, json);
        if (status != 200) return status;
        sb_appendf(json, "{\"ok\":true,\"usuario\":{\"id\":%d,\"username\":", user.id);
        sb_json_string(json, user.username);
        sb_append(json, ",\"nombre\":");
        sb_json_string(json, user.nombre_completo);
        sb_append(json, ",\"rol\":");
        sb_json_string(json, user.rol);
        sb_append(json, "}}");
        return 200;
    }

    if (strncmp(req->path, "/api/admin/", 11) == 0) {
        status = auth_require(token, SIGA_ROLE_ADMIN, &user, json);
        if (status != 200) return status;
        if (strcmp(req->path, "/api/admin/docentes") == 0 && strcmp(req->method, "GET") == 0) {
            admin_docentes_json(json); return 200;
        }
        if (strcmp(req->path, "/api/admin/docentes") == 0 && strcmp(req->method, "POST") == 0) {
            return admin_crear_docente(user.id, params, json);
        }
        if (strcmp(req->path, "/api/admin/docentes/suspender") == 0 && strcmp(req->method, "POST") == 0) {
            return admin_suspender_docente(user.id, params, json);
        }
        if (strcmp(req->path, "/api/admin/docentes/reset") == 0 && strcmp(req->method, "POST") == 0) {
            return admin_reset_docente(user.id, params, json);
        }
        if (strcmp(req->path, "/api/admin/materias") == 0 && strcmp(req->method, "GET") == 0) {
            admin_materias_json(json); return 200;
        }
        if (strcmp(req->path, "/api/admin/materias") == 0 && strcmp(req->method, "POST") == 0) {
            return admin_crear_materia(user.id, params, json);
        }
        if (strcmp(req->path, "/api/admin/materias/asignar") == 0 && strcmp(req->method, "POST") == 0) {
            return admin_asignar_materia(user.id, params, json);
        }
        if (strcmp(req->path, "/api/admin/stats") == 0 && strcmp(req->method, "GET") == 0) {
            admin_stats_json(json); return 200;
        }
        if (strcmp(req->path, "/api/admin/logs") == 0 && strcmp(req->method, "GET") == 0) {
            admin_logs_json(json); return 200;
        }
        if (strcmp(req->path, "/api/admin/acta") == 0 && strcmp(req->method, "GET") == 0) {
            reporte_acta_json(user.id, params_get_int(params, "materia_id", 0), 1, json); return 200;
        }
        if (strcmp(req->path, "/api/admin/acta.csv") == 0 && strcmp(req->method, "GET") == 0) {
            *content_type = "text/csv";
            reporte_acta_csv(user.id, params_get_int(params, "materia_id", 0), 1, json); return 200;
        }
    }

    if (strncmp(req->path, "/api/docente/", 13) == 0) {
        status = auth_require(token, SIGA_ROLE_DOCENTE, &user, json);
        if (status != 200) return status;
        if (strcmp(req->path, "/api/docente/materias") == 0 && strcmp(req->method, "GET") == 0) {
            docente_materias_json(user.id, json); return 200;
        }
        if (strcmp(req->path, "/api/docente/alumnos") == 0 && strcmp(req->method, "GET") == 0) {
            alumnos_json(user.id, params_get_int(params, "materia_id", 0), json); return 200;
        }
        if (strcmp(req->path, "/api/docente/alumnos") == 0 && strcmp(req->method, "POST") == 0) {
            return alumno_crear(user.id, params, json);
        }
        if (strcmp(req->path, "/api/docente/alumnos/retirar") == 0 && strcmp(req->method, "POST") == 0) {
            return alumno_retirar(user.id, params, json);
        }
        if (strcmp(req->path, "/api/docente/evaluaciones") == 0 && strcmp(req->method, "GET") == 0) {
            evaluaciones_json(user.id, params_get_int(params, "materia_id", 0), json); return 200;
        }
        if (strcmp(req->path, "/api/docente/evaluaciones") == 0 && strcmp(req->method, "POST") == 0) {
            return evaluacion_guardar_plan(user.id, params, json);
        }
        if (strcmp(req->path, "/api/docente/calificaciones") == 0 && strcmp(req->method, "GET") == 0) {
            calificaciones_json(user.id, params_get_int(params, "materia_id", 0),
                                params_get_int(params, "evaluacion_id", 0), json); return 200;
        }
        if (strcmp(req->path, "/api/docente/calificaciones") == 0 && strcmp(req->method, "POST") == 0) {
            return calificaciones_guardar_lote(user.id, params, json);
        }
        if (strcmp(req->path, "/api/docente/calificaciones/corregir") == 0 && strcmp(req->method, "POST") == 0) {
            return calificacion_corregir(user.id, params, json);
        }
        if (strcmp(req->path, "/api/docente/acta") == 0 && strcmp(req->method, "GET") == 0) {
            reporte_acta_json(user.id, params_get_int(params, "materia_id", 0), 0, json); return 200;
        }
        if (strcmp(req->path, "/api/docente/acta.csv") == 0 && strcmp(req->method, "GET") == 0) {
            *content_type = "text/csv";
            reporte_acta_csv(user.id, params_get_int(params, "materia_id", 0), 0, json); return 200;
        }
        if (strcmp(req->path, "/api/docente/acta/generar") == 0 && strcmp(req->method, "POST") == 0) {
            return mark_acta_generada(user.id, params_get_int(params, "materia_id", 0), json);
        }
    }

    sb_append(json, "{\"ok\":false,\"error\":\"Ruta no encontrada\"}");
    return 404;
}

static void split_target(HttpRequest *req) {
    char *q;
    safe_copy(req->path, sizeof(req->path), req->target);
    req->query[0] = '\0';
    q = strchr(req->path, '?');
    if (q) {
        *q = '\0';
        safe_copy(req->query, sizeof(req->query), q + 1);
    }
}

static int parse_request(char *raw, HttpRequest *req) {
    char *line_end = strstr(raw, "\r\n");
    char *headers_end = strstr(raw, "\r\n\r\n");
    char *body = headers_end ? headers_end + 4 : NULL;
    if (!line_end) return 0;
    *line_end = '\0';
    if (sscanf(raw, "%7s %1023s", req->method, req->target) != 2) return 0;
    split_target(req);
    safe_copy(req->body, sizeof(req->body), body ? body : "");
    return 1;
}

static int header_content_length(const char *raw) {
    const char *p = raw;
    while (p && *p) {
        const char *line_end = strstr(p, "\r\n");
        size_t len = line_end ? (size_t)(line_end - p) : strlen(p);
        if (len >= 15 && strncmp(p, "Content-Length:", 15) == 0) {
            return atoi(p + 15);
        }
        if (len == 0) break;
        p = line_end ? line_end + 2 : NULL;
    }
    return 0;
}

static void handle_client(socket_t client) {
    char *raw = (char *)calloc(REQ_BUF_SIZE, 1);
    int received;
    HttpRequest req;
    if (!raw) {
        CLOSESOCKET(client);
        return;
    }
    received = recv(client, raw, REQ_BUF_SIZE - 1, 0);
    if (received <= 0) {
        free(raw);
        CLOSESOCKET(client);
        return;
    }
    raw[received] = '\0';

    char *headers_end = strstr(raw, "\r\n\r\n");
    if (headers_end) {
        int content_length = header_content_length(raw);
        int header_bytes = (int)((headers_end + 4) - raw);
        while (content_length > 0 &&
               received < header_bytes + content_length &&
               received < REQ_BUF_SIZE - 1) {
            int n = recv(client, raw + received, REQ_BUF_SIZE - 1 - received, 0);
            if (n <= 0) break;
            received += n;
            raw[received] = '\0';
        }
    }

    memset(&req, 0, sizeof(req));
    if (!parse_request(raw, &req)) {
        send_response(client, 400, "application/json", "{\"ok\":false,\"error\":\"Request inválida\"}");
        free(raw);
        CLOSESOCKET(client);
        return;
    }

    if (strncmp(req.path, "/api/", 5) == 0) {
        ParamList query_params, body_params, params;
        StrBuf json;
        const char *content_type = "application/json";
        int status;
        params.count = 0;
        params_parse(&query_params, req.query);
        params_parse(&body_params, req.body);
        merge_params(&params, &query_params);
        merge_params(&params, &body_params);
        sb_init(&json);
        status = handle_api(&req, &params, &json, &content_type);
        send_response(client, status, content_type, json.data ? json.data : "");
        sb_free(&json);
    } else {
        send_static(client, req.path);
    }
    free(raw);
    CLOSESOCKET(client);
}

static void seed_if_empty(void) {
    Usuario admin_user, docente_user;
    Docente docente;
    Materia materia;
    Alumno alumnos[] = {
        {0, "V-25.100.001", "Ana", "Rodriguez", 0, 0},
        {0, "V-25.100.002", "Carlos", "Mendez", 0, 0},
        {0, "V-25.100.003", "Diana", "Torres", 0, 0},
        {0, "V-25.100.004", "Eduardo", "Gomez", 0, 0},
        {0, "V-25.100.005", "Fernanda", "Silva", 0, 0},
        {0, "V-25.100.006", "Gabriel", "Ramos", 0, 1}
    };
    const char *eval_names[] = {"Parcial 1", "Taller 1", "Parcial 2", "Practica Lab.", "Trabajo Final"};
    int eval_types[] = {TIPO_PARCIAL, TIPO_TALLER, TIPO_PARCIAL, TIPO_PRACTICA, TIPO_TRABAJO};
    float eval_pond[] = {0.25f, 0.20f, 0.25f, 0.15f, 0.15f};
    float demo_notes[][5] = {
        {14, 15, 13, 16, 14},
        {10, 11, 9, 12, 10},
        {18, 17, 19, 18, 17},
        {8, 9, 7, 8, 6},
        {12, 13, 11, 14, 12},
        {0, 0, 0, 0, 0}
    };

    if (db_total_registros(SIGA_FILE_USUARIOS, sizeof(Usuario)) > 0) return;
    auth_create_user("admin", "1234", "Carmen Rodriguez", SIGA_ROLE_ADMIN, 0, &admin_user);
    auth_create_user("docente", "1234", "Luis Martinez", SIGA_ROLE_DOCENTE, 0, &docente_user);

    memset(&docente, 0, sizeof(docente));
    docente.id = db_siguiente_id(SIGA_FILE_DOCENTES, sizeof(Docente));
    docente.usuario_id = docente_user.id;
    safe_copy(docente.cedula, sizeof(docente.cedula), "V-12.345.678");
    safe_copy(docente.nombres, sizeof(docente.nombres), "Luis");
    safe_copy(docente.apellidos, sizeof(docente.apellidos), "Martinez");
    safe_copy(docente.email, sizeof(docente.email), "lmartinez@institucion.edu");
    docente.activo = 1;
    db_insertar(SIGA_FILE_DOCENTES, &docente, sizeof(docente));

    memset(&materia, 0, sizeof(materia));
    materia.id = db_siguiente_id(SIGA_FILE_MATERIAS, sizeof(Materia));
    safe_copy(materia.codigo, sizeof(materia.codigo), "MAT-101");
    safe_copy(materia.nombre, sizeof(materia.nombre), "Calculo I");
    safe_copy(materia.periodo, sizeof(materia.periodo), "2026-I");
    materia.docente_id = docente.id;
    materia.activo = 1;
    db_insertar(SIGA_FILE_MATERIAS, &materia, sizeof(materia));

    for (int i = 0; i < 6; i++) {
        alumnos[i].id = db_siguiente_id(SIGA_FILE_ALUMNOS, sizeof(Alumno));
        alumnos[i].materia_id = materia.id;
        db_insertar(SIGA_FILE_ALUMNOS, &alumnos[i], sizeof(Alumno));
    }
    Evaluacion evals[5];
    for (int i = 0; i < 5; i++) {
        memset(&evals[i], 0, sizeof(Evaluacion));
        evals[i].id = db_siguiente_id(SIGA_FILE_EVALS, sizeof(Evaluacion));
        evals[i].materia_id = materia.id;
        safe_copy(evals[i].nombre, sizeof(evals[i].nombre), eval_names[i]);
        evals[i].tipo = eval_types[i];
        evals[i].ponderacion = eval_pond[i];
        evals[i].orden = i + 1;
        db_insertar(SIGA_FILE_EVALS, &evals[i], sizeof(Evaluacion));
    }
    for (int a = 0; a < 5; a++) {
        for (int e = 0; e < 5; e++) {
            Calificacion c;
            memset(&c, 0, sizeof(c));
            c.id = db_siguiente_id(SIGA_FILE_NOTAS, sizeof(Calificacion));
            c.alumno_id = alumnos[a].id;
            c.evaluacion_id = evals[e].id;
            c.nota = demo_notes[a][e];
            now_string(c.fecha_carga);
            db_insertar(SIGA_FILE_NOTAS, &c, sizeof(c));
        }
    }
    audit_log(admin_user.id, "SEED", "Datos iniciales creados: admin/1234 y docente/1234");
}

int main(void) {
    socket_t server_fd;
    struct sockaddr_in addr;
    int opt = 1;

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "No se pudo inicializar Winsock\n");
        return 1;
    }
#endif

    if (!db_prepare_storage()) {
        fprintf(stderr, "No se pudo preparar data/\n");
        return 1;
    }
    db_backup_all();
    seed_if_empty();

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        fprintf(stderr, "No se pudo crear socket\n");
        return 1;
    }

#ifdef _WIN32
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#else
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "No se pudo abrir el puerto %d\n", PORT);
        CLOSESOCKET(server_fd);
        return 1;
    }
    if (listen(server_fd, 16) == SOCKET_ERROR) {
        fprintf(stderr, "No se pudo escuchar en el puerto %d\n", PORT);
        CLOSESOCKET(server_fd);
        return 1;
    }

    printf("SiGA escuchando en http://localhost:%d\n", PORT);
    printf("Usuarios demo: admin/1234 y docente/1234\n");

    while (1) {
        socket_t client = accept(server_fd, NULL, NULL);
        if (client == INVALID_SOCKET) continue;
        handle_client(client);
    }

    CLOSESOCKET(server_fd);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
