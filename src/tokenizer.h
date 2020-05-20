#ifndef CINC_TOKENIZER_H
#define CINC_TOKENIZER_H

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    /// One of `+-/*()` or comparison. Tokenized via `consume_char` or
    /// `consume_str`
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Slice Slice;

struct Slice {
    char *str;
    int len;
};

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val; // if kind == TK_NUM
    Slice slice;
};

/// The shared input
extern char *user_input;

/// The shared `Token`
extern Token *token;

Token *tokenize(char *p);
bool consume_char(char op);
bool consume_str(char *str);
void expect_char(char op);
int expect_number();

#endif
