#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "token.h"
#include "utils.h"

static Token *new_token(TokenKind kind, char *str, int len, Token *cur) {
    Token *tk = calloc(1, sizeof(Token));
    *tk = (Token){
        .kind = kind,
        .slice = (Slice){.str = str, .len = len},
    };
    cur->next = tk;
    return tk;
}

static char *skip_ws(char *p) {
    while (isspace(*p)) {
        p++;
    }
    return p;
}

static bool str_starts_with(char *str, char *part) {
    return memcmp(str, part, strlen(part)) == 0;
}

Token *tokenize(char *src) {
    char *ptr = src;

    Token head;
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
        if (strchr("+-*/()<>=;", *ptr)) {
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

        // identifier
        if ('a' <= *ptr && *ptr <= 'z') {
            tk = new_token(TK_IDENT, ptr, 1, tk);
        }

        panic_at(ptr, src, "Invalid string for the tokenizer");
    }

    new_token(TK_EOF, ptr, 0, tk);
    return head.next;
}
