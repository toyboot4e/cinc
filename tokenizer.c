#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "tokenizer.h"

char *user_input;

void panic_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void panic(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// The shared `Token` among functions
Token *token;

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *t = calloc(1, sizeof(Token));
  t->kind = kind;
  t->slice.str = str;
  t->slice.len = len;
  cur->next = t;
  return t;
}

bool is_at_eof() { return token->kind == TK_EOF; }

/// Used mainly for tokenizing operators
bool consume_char(char c) {
  if (token->kind != TK_RESERVED || token->slice.str[0] != c) {
    return false;
  }
  token = token->next;
  return true;
}

/// Used mainly for tokenizing operators
bool consume_str(char *str) {
  if (token->kind != TK_RESERVED) {
    return false;
  }

  if (strlen(str) != token->slice.len ||
      memcmp(token->slice.str, str, token->slice.len)) {
    return false;
  }

  token = token->next;
  return true;
}

// Panics if it finds something other than the expected char
void expect_char(char op) {
  if (token->kind != TK_RESERVED || token->slice.str[0] != op) {
    panic_at(token->slice.str, "Expected a char '%c'", op);
  }
  token = token->next;
}

// Panics if it find something other than a number
int expect_number() {
  if (token->kind != TK_NUM) {
    panic_at(token->slice.str, "Expected a number");
  }

  int val = token->val;
  token = token->next;
  return val;
}

char *skip_ws(char *p) {
  while (isspace(*p)) {
    p++;
  }
  return p;
}

bool starts_with(char *str, char *part) {
  return memcmp(str, part, strlen(part)) == 0;
}

Token *tokenize(char *p) {
  user_input = p;

  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (true) {
    p = skip_ws(p);
    if (!*p) {
      break;
    }

    // we have to check longer tokens first
    if (starts_with(p, "==") || starts_with(p, "!=") || starts_with(p, "<=") ||
        starts_with(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/()", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *anchor = p;
      cur->val = strtol(p, &p, 10);
      cur->slice.len = p - anchor;
      continue;
    }

    panic_at(p, "Given invalid `char*`");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}
