#ifndef CINC_TOKENIZER
#define CINC_TOKENIZER "tokenizer.c"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"

void panic(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// The shard `Token` among functions
Token *token;

Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *t = calloc(1, sizeof(Token));
  t->kind = kind;
  t->str = str;
  cur->next = t;
  return t;
}

bool is_at_eof() { return token->kind == TK_EOF; }

bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

// panics if it find something other than the expected char
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    panic("Expected a char '%c'", op);
  token = token->next;
}

// panics if it find something other than a numer
int expect_number() {
  if (token->kind != TK_NUM)
    panic("Expected a number");
  int val = token->val;
  token = token->next;
  return val;
}

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (true) {
    while (isspace(*p)) {
      p++;
    }
    if (!*p)
      break;

    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    panic("Given invalid `char*`");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

#endif
