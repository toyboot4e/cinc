// TODO: rm unnecessary includes
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

void panic(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void panic_at(char *loc, char *src, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - src;
    fprintf(stderr, "%s\n", src);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool slice_eq(Slice a, Slice b) {
    return a.len == b.len && memcmp(a.str, b.str, a.len) == 0;
}

bool slice_str_eq(Slice a, char *s) {
    return a.len == strlen(s) && memcmp(a.str, s, a.len) == 0;
}

char *slice_to_string(Slice slice) {
    int len = slice.len;

    char *s = malloc(len + 1);
    memcpy(s, slice.str, len);
    s[len] = '\0';

    return s;
}
