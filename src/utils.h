//! Utilities

#ifndef CINC_UTILS_H
#define CINC_UTILS_H

void panic(char *fmt, ...);
void panic_at(char *loc, char *src, char *fmt, ...);

/// A slice of a string
typedef struct {
    char *str;
    int len;
} Slice;

bool slice_eq(Slice a, Slice b);
char *slice_to_string(Slice s);

#endif
