#include "utils.h"
#include "types.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

void safe_copy(char *dst, size_t dst_size, const char *src) {
    if (!dst || dst_size == 0) return;
    if (!src) src = "";
    snprintf(dst, dst_size, "%s", src);
}

void now_string(char out[20]) {
    time_t t = time(NULL);
    struct tm tmv;
#ifdef _WIN32
    localtime_s(&tmv, &t);
#else
    struct tm *tmp;
    tmp = localtime(&t);
    if (tmp) tmv = *tmp;
    else memset(&tmv, 0, sizeof(tmv));
#endif
    strftime(out, 20, "%Y-%m-%d %H:%M:%S", &tmv);
}

int ensure_dir(const char *path) {
    if (!path || !*path) return 0;
    if (MKDIR(path) == 0) return 1;
    return 1;
}

int file_exists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

void random_hex(char *out, size_t bytes) {
    static int seeded = 0;
    static const char hex[] = "0123456789abcdef";
    size_t i;
    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = 1;
    }
    for (i = 0; i < bytes; i++) {
        unsigned char v = (unsigned char)(rand() & 0xff);
        out[i * 2] = hex[v >> 4];
        out[i * 2 + 1] = hex[v & 15];
    }
    out[bytes * 2] = '\0';
}

void generate_temp_password(char *out, size_t out_size) {
    char hex[9];
    random_hex(hex, 4);
    snprintf(out, out_size, "Tmp#%s", hex);
}

static uint32_t rotr32(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

void sha256_hex(const char *input, char output[65]) {
    static const uint32_t k[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };
    uint32_t h[8] = {
        0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
        0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
    };
    const unsigned char *msg = (const unsigned char *)(input ? input : "");
    size_t len = strlen((const char *)msg);
    uint64_t bit_len = (uint64_t)len * 8u;
    size_t new_len = len + 1;
    while ((new_len % 64) != 56) new_len++;

    unsigned char *buf = (unsigned char *)calloc(new_len + 8, 1);
    if (!buf) {
        output[0] = '\0';
        return;
    }
    memcpy(buf, msg, len);
    buf[len] = 0x80;
    for (int i = 0; i < 8; i++) {
        buf[new_len + i] = (unsigned char)(bit_len >> (56 - 8 * i));
    }

    for (size_t offset = 0; offset < new_len + 8; offset += 64) {
        uint32_t w[64];
        for (int i = 0; i < 16; i++) {
            w[i] = ((uint32_t)buf[offset + i * 4] << 24) |
                   ((uint32_t)buf[offset + i * 4 + 1] << 16) |
                   ((uint32_t)buf[offset + i * 4 + 2] << 8) |
                   ((uint32_t)buf[offset + i * 4 + 3]);
        }
        for (int i = 16; i < 64; i++) {
            uint32_t s0 = rotr32(w[i - 15], 7) ^ rotr32(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = rotr32(w[i - 2], 17) ^ rotr32(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];
        for (int i = 0; i < 64; i++) {
            uint32_t S1 = rotr32(e, 6) ^ rotr32(e, 11) ^ rotr32(e, 25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t temp1 = hh + S1 + ch + k[i] + w[i];
            uint32_t S0 = rotr32(a, 2) ^ rotr32(a, 13) ^ rotr32(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;
            hh = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }
    free(buf);
    snprintf(output, 65, "%08x%08x%08x%08x%08x%08x%08x%08x",
             (unsigned)h[0], (unsigned)h[1], (unsigned)h[2], (unsigned)h[3],
             (unsigned)h[4], (unsigned)h[5], (unsigned)h[6], (unsigned)h[7]);
}

static int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    if (c >= 'A' && c <= 'F') return 10 + c - 'A';
    return -1;
}

void url_decode(char *dst, size_t dst_size, const char *src) {
    size_t di = 0;
    if (!dst || dst_size == 0) return;
    if (!src) src = "";
    for (size_t si = 0; src[si] && di + 1 < dst_size; si++) {
        if (src[si] == '+') {
            dst[di++] = ' ';
        } else if (src[si] == '%' && isxdigit((unsigned char)src[si + 1]) && isxdigit((unsigned char)src[si + 2])) {
            int hi = hex_val(src[si + 1]);
            int lo = hex_val(src[si + 2]);
            dst[di++] = (char)((hi << 4) | lo);
            si += 2;
        } else {
            dst[di++] = src[si];
        }
    }
    dst[di] = '\0';
}

void params_parse(ParamList *params, const char *encoded) {
    char pair[1024];
    int pi = 0;
    const char *p = encoded ? encoded : "";
    params->count = 0;
    while (1) {
        if (*p == '&' || *p == '\0') {
            pair[pi] = '\0';
            if (pi > 0 && params->count < SIGA_MAX_PARAMS) {
                char *eq = strchr(pair, '=');
                if (eq) {
                    *eq = '\0';
                    url_decode(params->items[params->count].key, sizeof(params->items[params->count].key), pair);
                    url_decode(params->items[params->count].value, sizeof(params->items[params->count].value), eq + 1);
                    params->count++;
                }
            }
            pi = 0;
            if (*p == '\0') break;
        } else if (pi + 1 < (int)sizeof(pair)) {
            pair[pi++] = *p;
        }
        p++;
    }
}

const char *params_get(const ParamList *params, const char *key) {
    if (!params || !key) return "";
    for (int i = 0; i < params->count; i++) {
        if (strcmp(params->items[i].key, key) == 0) return params->items[i].value;
    }
    return "";
}

int params_get_int(const ParamList *params, const char *key, int fallback) {
    const char *v = params_get(params, key);
    return *v ? atoi(v) : fallback;
}

float params_get_float(const ParamList *params, const char *key, float fallback) {
    const char *v = params_get(params, key);
    return *v ? (float)atof(v) : fallback;
}

void sb_init(StrBuf *sb) {
    sb->cap = 4096;
    sb->len = 0;
    sb->data = (char *)malloc(sb->cap);
    if (sb->data) sb->data[0] = '\0';
}

void sb_free(StrBuf *sb) {
    if (!sb) return;
    free(sb->data);
    sb->data = NULL;
    sb->len = 0;
    sb->cap = 0;
}

static void sb_reserve(StrBuf *sb, size_t extra) {
    size_t need = sb->len + extra + 1;
    if (need <= sb->cap) return;
    while (sb->cap < need) sb->cap *= 2;
    sb->data = (char *)realloc(sb->data, sb->cap);
}

void sb_append(StrBuf *sb, const char *text) {
    size_t n = text ? strlen(text) : 0;
    if (!sb->data) sb_init(sb);
    sb_reserve(sb, n);
    memcpy(sb->data + sb->len, text ? text : "", n + 1);
    sb->len += n;
}

void sb_append_char(StrBuf *sb, char c) {
    if (!sb->data) sb_init(sb);
    sb_reserve(sb, 1);
    sb->data[sb->len++] = c;
    sb->data[sb->len] = '\0';
}

void sb_appendf(StrBuf *sb, const char *fmt, ...) {
    char stack[2048];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(stack, sizeof(stack), fmt, ap);
    va_end(ap);
    if (n < 0) return;
    if ((size_t)n < sizeof(stack)) {
        sb_append(sb, stack);
        return;
    }
    char *heap = (char *)malloc((size_t)n + 1);
    if (!heap) return;
    va_start(ap, fmt);
    vsnprintf(heap, (size_t)n + 1, fmt, ap);
    va_end(ap);
    sb_append(sb, heap);
    free(heap);
}

void sb_json_string(StrBuf *sb, const char *text) {
    const unsigned char *p = (const unsigned char *)(text ? text : "");
    sb_append_char(sb, '"');
    while (*p) {
        switch (*p) {
            case '"': sb_append(sb, "\\\""); break;
            case '\\': sb_append(sb, "\\\\"); break;
            case '\b': sb_append(sb, "\\b"); break;
            case '\f': sb_append(sb, "\\f"); break;
            case '\n': sb_append(sb, "\\n"); break;
            case '\r': sb_append(sb, "\\r"); break;
            case '\t': sb_append(sb, "\\t"); break;
            default:
                if (*p < 32) sb_appendf(sb, "\\u%04x", *p);
                else sb_append_char(sb, (char)*p);
        }
        p++;
    }
    sb_append_char(sb, '"');
}

const char *tipo_to_string(int tipo) {
    switch (tipo) {
        case TIPO_PARCIAL: return "PARCIAL";
        case TIPO_TALLER: return "TALLER";
        case TIPO_PRACTICA: return "PRACTICA";
        case TIPO_TRABAJO: return "TRABAJO";
        default: return "OTRO";
    }
}

int tipo_from_string(const char *tipo) {
    if (!tipo) return TIPO_OTRO;
    if (strcmp(tipo, "PARCIAL") == 0) return TIPO_PARCIAL;
    if (strcmp(tipo, "TALLER") == 0) return TIPO_TALLER;
    if (strcmp(tipo, "PRACTICA") == 0) return TIPO_PRACTICA;
    if (strcmp(tipo, "TRABAJO") == 0) return TIPO_TRABAJO;
    return TIPO_OTRO;
}

int float_equal_2(float a, float b) {
    int ia = (int)(a * 100.0f + (a >= 0 ? 0.5f : -0.5f));
    int ib = (int)(b * 100.0f + (b >= 0 ? 0.5f : -0.5f));
    return ia == ib;
}
