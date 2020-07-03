//! † The tokenizer † which slices input string

#ifndef CINC_TOKENIZER_H
#define CINC_TOKENIZER_H

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    /// Non-strict token. One of `+-/*()` or comparison.
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Str Slice;
/// A slice of a string
struct Str {
    char *str;
    int len;
};

typedef struct Token Token;
/// A slice of source string
struct Token {
    TokenKind kind;
    Slice slice;
    /// Makes a linked list of parsed tokens. This is useful in C because we
    /// don't have a generic vector type.
    Token *next;
    // TODO: do we need this?
    int val; // if kind == TK_NUM
};

/// Tokenizes an input string and builds a linked list of it
Token *tokenize(char *p);

#endif
