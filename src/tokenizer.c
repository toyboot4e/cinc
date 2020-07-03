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

Token *tokenize(char *ptr) {
    char *src = ptr;

    Token head;
    head.next = NULL;
    Token *tk = &head;

    while (true) {
        ptr = skip_ws(ptr);
        if (!*ptr) {
            break; // because C string is null-terminated, we can do this
        }

        // we have to check longer tokens first
        if (str_starts_with(ptr, "==") || str_starts_with(ptr, "!=") ||
            str_starts_with(ptr, "<=") || str_starts_with(ptr, ">=")) {
            tk = new_token(TK_RESERVED, ptr, 2, tk);
            ptr += 2;
            continue;
        }

        // then single character tokens
        if (strchr("+-*/()", *ptr)) {
            tk = new_token(TK_RESERVED, ptr++, 1, tk);
            continue;
        }

        // number
        if (isdigit(*ptr)) {
            tk = new_token(TK_NUM, ptr, 0, tk);
            char *anchor = ptr;
            tk->val = strtol(ptr, &ptr, 10);
            tk->slice.len = ptr - anchor;
            continue;
        }

        panic_at(ptr, src, "Invalid string");
    }

    new_token(TK_EOF, ptr, 0, tk);
    return head.next;
}

