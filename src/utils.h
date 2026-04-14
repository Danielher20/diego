#ifndef SIGA_UTILS_H
#define SIGA_UTILS_H

#include <stddef.h>

#define SIGA_MAX_PARAMS 96

typedef struct {
    char key[80];
    char value[512];
} KeyValue;

typedef struct {
    KeyValue items[SIGA_MAX_PARAMS];
    int count;
} ParamList;

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} StrBuf;

void safe_copy(char *dst, size_t dst_size, const char *src);
void now_string(char out[20]);
int ensure_dir(const char *path);
int file_exists(const char *path);
void random_hex(char *out, size_t bytes);
void generate_temp_password(char *out, size_t out_size);

void sha256_hex(const char *input, char output[65]);

void params_parse(ParamList *params, const char *encoded);
const char *params_get(const ParamList *params, const char *key);
int params_get_int(const ParamList *params, const char *key, int fallback);
float params_get_float(const ParamList *params, const char *key, float fallback);
void url_decode(char *dst, size_t dst_size, const char *src);

void sb_init(StrBuf *sb);
void sb_free(StrBuf *sb);
void sb_append(StrBuf *sb, const char *text);
void sb_append_char(StrBuf *sb, char c);
void sb_appendf(StrBuf *sb, const char *fmt, ...);
void sb_json_string(StrBuf *sb, const char *text);

const char *tipo_to_string(int tipo);
int tipo_from_string(const char *tipo);
int float_equal_2(float a, float b);

#endif
