//! Converts string into [`Token`]s

#ifndef CINC_TOKEN_H
#define CINC_TOKEN_H

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

typedef enum {
    /// One of `+-*/()<>=;` or comparison.
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_FOR,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

/// Linked list of source slices
struct Token {
    TokenKind kind;
    Slice slice;
    /// Never freed
    Token *next;
    // TODO: do we need this?
    int val; // if kind == TK_NUM
};

/// Tokenizes an input string and builds a linked list of it
Token *tokenize(char *src);

#endif
