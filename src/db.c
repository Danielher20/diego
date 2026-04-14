#include "db.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *all_files[] = {
    SIGA_FILE_USUARIOS,
    SIGA_FILE_DOCENTES,
    SIGA_FILE_MATERIAS,
    SIGA_FILE_ALUMNOS,
    SIGA_FILE_EVALS,
    SIGA_FILE_NOTAS,
    SIGA_FILE_LOGS
};

int db_prepare_storage(void) {
    ensure_dir(SIGA_DATA_DIR);
    for (size_t i = 0; i < sizeof(all_files) / sizeof(all_files[0]); i++) {
        FILE *f = fopen(all_files[i], "ab");
        if (!f) return 0;
        fclose(f);
    }
    return 1;
}

int db_backup_all(void) {
    char backup_dir[128];
    char ts[32];
    time_t t = time(NULL);
    struct tm tmv;
#ifdef _WIN32
    localtime_s(&tmv, &t);
#else
    struct tm *tmp_tm;
    tmp_tm = localtime(&t);
    if (tmp_tm) tmv = *tmp_tm;
    else memset(&tmv, 0, sizeof(tmv));
#endif
    strftime(ts, sizeof(ts), "%Y%m%d_%H%M%S", &tmv);
    snprintf(backup_dir, sizeof(backup_dir), "%s/backup_%s", SIGA_DATA_DIR, ts);
    ensure_dir(backup_dir);

    for (size_t i = 0; i < sizeof(all_files) / sizeof(all_files[0]); i++) {
        FILE *in = fopen(all_files[i], "rb");
        if (!in) continue;
        const char *base = strrchr(all_files[i], '/');
        base = base ? base + 1 : all_files[i];
        char out_path[256];
        snprintf(out_path, sizeof(out_path), "%s/%s", backup_dir, base);
        FILE *out = fopen(out_path, "wb");
        if (!out) {
            fclose(in);
            continue;
        }
        char buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), in)) > 0) fwrite(buf, 1, n, out);
        fclose(out);
        fclose(in);
    }
    return 1;
}

int db_insertar(const char *archivo, const void *registro, size_t tamanio) {
    FILE *f = fopen(archivo, "ab");
    if (!f) return 0;
    int ok = fwrite(registro, tamanio, 1, f) == 1;
    fclose(f);
    return ok;
}

int db_buscar_por_id(const char *archivo, int id, void *resultado, size_t tamanio) {
    FILE *f = fopen(archivo, "rb");
    if (!f) return 0;
    unsigned char *reg = (unsigned char *)malloc(tamanio);
    if (!reg) {
        fclose(f);
        return 0;
    }
    while (fread(reg, tamanio, 1, f) == 1) {
        int rid;
        memcpy(&rid, reg, sizeof(int));
        if (rid == id) {
            memcpy(resultado, reg, tamanio);
            free(reg);
            fclose(f);
            return 1;
        }
    }
    free(reg);
    fclose(f);
    return 0;
}

int db_actualizar(const char *archivo, int id, const void *nuevo_reg, size_t tamanio) {
    FILE *in = fopen(archivo, "rb");
    if (!in) return 0;
    char tmp[260];
    snprintf(tmp, sizeof(tmp), "%s.tmp", archivo);
    FILE *out = fopen(tmp, "wb");
    if (!out) {
        fclose(in);
        return 0;
    }

    unsigned char *reg = (unsigned char *)malloc(tamanio);
    int found = 0;
    if (!reg) {
        fclose(in);
        fclose(out);
        remove(tmp);
        return 0;
    }
    while (fread(reg, tamanio, 1, in) == 1) {
        int rid;
        memcpy(&rid, reg, sizeof(int));
        if (rid == id) {
            fwrite(nuevo_reg, tamanio, 1, out);
            found = 1;
        } else {
            fwrite(reg, tamanio, 1, out);
        }
    }
    free(reg);
    fclose(in);
    fclose(out);
    if (!found) {
        remove(tmp);
        return 0;
    }
    remove(archivo);
    return rename(tmp, archivo) == 0;
}

int db_marcar_inactivo(const char *archivo, int id, size_t tamanio, size_t offset_activo) {
    unsigned char *reg = (unsigned char *)malloc(tamanio);
    if (!reg) return 0;
    int ok = db_buscar_por_id(archivo, id, reg, tamanio);
    if (ok) {
        int cero = 0;
        memcpy(reg + offset_activo, &cero, sizeof(int));
        ok = db_actualizar(archivo, id, reg, tamanio);
    }
    free(reg);
    return ok;
}

int db_listar_todos(const char *archivo, void *buffer, int max, size_t tamanio) {
    FILE *f = fopen(archivo, "rb");
    int count = 0;
    if (!f) return 0;
    while (count < max && fread((unsigned char *)buffer + ((size_t)count * tamanio), tamanio, 1, f) == 1) {
        count++;
    }
    fclose(f);
    return count;
}

int db_filtrar(const char *archivo, void *buffer, int max, size_t tamanio, DbPredicate filtro, void *ctx) {
    FILE *f = fopen(archivo, "rb");
    int count = 0;
    if (!f) return 0;
    unsigned char *reg = (unsigned char *)malloc(tamanio);
    if (!reg) {
        fclose(f);
        return 0;
    }
    while (count < max && fread(reg, tamanio, 1, f) == 1) {
        if (!filtro || filtro(reg, ctx)) {
            memcpy((unsigned char *)buffer + ((size_t)count * tamanio), reg, tamanio);
            count++;
        }
    }
    free(reg);
    fclose(f);
    return count;
}

int db_eliminar_filtrados(const char *archivo, size_t tamanio, DbPredicate filtro, void *ctx) {
    FILE *in = fopen(archivo, "rb");
    if (!in) return 0;
    char tmp[260];
    snprintf(tmp, sizeof(tmp), "%s.tmp", archivo);
    FILE *out = fopen(tmp, "wb");
    if (!out) {
        fclose(in);
        return 0;
    }
    unsigned char *reg = (unsigned char *)malloc(tamanio);
    int removed = 0;
    if (!reg) {
        fclose(in);
        fclose(out);
        remove(tmp);
        return 0;
    }
    while (fread(reg, tamanio, 1, in) == 1) {
        if (filtro && filtro(reg, ctx)) {
            removed++;
        } else {
            fwrite(reg, tamanio, 1, out);
        }
    }
    free(reg);
    fclose(in);
    fclose(out);
    remove(archivo);
    return rename(tmp, archivo) == 0 ? removed : -1;
}

int db_siguiente_id(const char *archivo, size_t tamanio) {
    FILE *f = fopen(archivo, "rb");
    int max_id = 0;
    if (!f) return 1;
    unsigned char *reg = (unsigned char *)malloc(tamanio);
    if (!reg) {
        fclose(f);
        return 1;
    }
    while (fread(reg, tamanio, 1, f) == 1) {
        int rid;
        memcpy(&rid, reg, sizeof(int));
        if (rid > max_id) max_id = rid;
    }
    free(reg);
    fclose(f);
    return max_id + 1;
}

int db_total_registros(const char *archivo, size_t tamanio) {
    FILE *f = fopen(archivo, "rb");
    long size;
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fclose(f);
    if (size <= 0) return 0;
    return (int)(size / (long)tamanio);
}
