#ifndef CINC_TOKENIZER_H
#define CINC_TOKENIZER_H

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  /// One of `+-/*()`. Tokenized via `consume_char`
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val; // if kind == TK_NUM
  char *str;
};

/// The shard input
extern char *user_input;

/// The shard `Token`
extern Token *token;

Token *tokenize(char *p);
bool consume_char(char op);
void expect_char(char op);
int expect_number();

#endif
