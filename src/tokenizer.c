//! † The tokenizer † which slices input string
//!
//! Note that we won't free the allocated tokens because `cinc` is a short-lived
//! program.

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "tokenizer.h"
#include "utils.h"

Token *new_token(TokenKind kind, char *str, int len, Token *cur) {
    Token *t = calloc(1, sizeof(Token));
    t->kind = kind;
    t->slice.str = str;
    t->slice.len = len;
    cur->next = t;
    return t;
}

char *skip_ws(char *p) {
    while (isspace(*p)) {
        p++;
    }
    return p;
}

bool str_starts_with(char *str, char *part) {
    return memcmp(str, part, strlen(part)) == 0;
}

Token *tokenize(char *src) {
    char *ptr = src;

    Token head;
    head.next = NULL;
    Token *tk = &head;

    while (true) {
        src = skip_ws(src);
        if (!*src) {
            break; // because C string is null-terminated, we can do this
        }

        // we have to check longer tokens first
        if (str_starts_with(src, "==") || str_starts_with(src, "!=") ||
            str_starts_with(src, "<=") || str_starts_with(src, ">=")) {
            tk = new_token(TK_RESERVED, src, 2, tk);
            src += 2;
            continue;
        }

        // then single character tokens
        if (strchr("+-*/()", *src)) {
            tk = new_token(TK_RESERVED, src++, 1, tk);
            continue;
        }

        // number
        if (isdigit(*src)) {
            tk = new_token(TK_NUM, src, 0, tk);
            char *anchor = src;
            tk->val = strtol(src, &src, 10);
            tk->slice.len = src - anchor;
            continue;
        }

        panic_at(src, ptr, "Invalid string");
    }

    new_token(TK_EOF, src, 0, tk);
    return head.next;
}

