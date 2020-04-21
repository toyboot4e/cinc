#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
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

void panic(char *fmt, ...);

/// The shard `Token` among functions
extern Token *token;

Token *new_token(TokenKind kind, Token *cur, char *str);
bool is_at_eof();
bool consume(char op);
void expect(char op);
int expect_number();

Token *tokenize(char *p);

