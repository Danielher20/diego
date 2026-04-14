#ifndef SIGA_DB_H
#define SIGA_DB_H

#include <stddef.h>

#define SIGA_DATA_DIR        "data"
#define SIGA_FILE_USUARIOS   "data/usuarios.dat"
#define SIGA_FILE_DOCENTES   "data/docentes.dat"
#define SIGA_FILE_MATERIAS   "data/materias.dat"
#define SIGA_FILE_ALUMNOS    "data/alumnos.dat"
#define SIGA_FILE_EVALS      "data/evaluaciones.dat"
#define SIGA_FILE_NOTAS      "data/calificaciones.dat"
#define SIGA_FILE_LOGS       "data/log_auditoria.dat"

typedef int (*DbPredicate)(const void *registro, void *ctx);

int db_prepare_storage(void);
int db_backup_all(void);

int db_insertar(const char *archivo, const void *registro, size_t tamanio);
int db_buscar_por_id(const char *archivo, int id, void *resultado, size_t tamanio);
int db_actualizar(const char *archivo, int id, const void *nuevo_reg, size_t tamanio);
int db_marcar_inactivo(const char *archivo, int id, size_t tamanio, size_t offset_activo);
int db_listar_todos(const char *archivo, void *buffer, int max, size_t tamanio);
int db_filtrar(const char *archivo, void *buffer, int max, size_t tamanio, DbPredicate filtro, void *ctx);
int db_eliminar_filtrados(const char *archivo, size_t tamanio, DbPredicate filtro, void *ctx);
int db_siguiente_id(const char *archivo, size_t tamanio);
int db_total_registros(const char *archivo, size_t tamanio);

#endif
