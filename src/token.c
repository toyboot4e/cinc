#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "token.h"
#include "utils.h"

static Token *alloc_next_token(TokenKind kind, char *str, int len, Token *cur) {
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

static bool is_ident_head(char c) {
    return isalpha(c) || c == '_';
}

static bool is_ident_body(char c) {
    return isalpha(c) || isdigit(c) || c == '_';
}

/// alpha (alpha | digit | _)*
static int read_ident(char *start) {
    assert(is_ident_head(*start));

    char *end = start + 1;
    while (end) {
        if (is_ident_body(*end)) {
            end += 1;
            continue;
        }

        break;
    }

    return end - start;
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
            tk = alloc_next_token(TK_RESERVED, ptr, 2, tk);
            ptr += 2;
            continue;
        }

        // then single character tokens
        if (strchr("+-*/()<>=;", *ptr)) {
            tk = alloc_next_token(TK_RESERVED, ptr, 1, tk);
            ptr += 1;
            continue;
        }

        // number
        if (isdigit(*ptr)) {
            tk = alloc_next_token(TK_NUM, ptr, 0, tk);
            char *anchor = ptr;
            tk->val = strtol(ptr, &ptr, 10);
            tk->slice.len = ptr - anchor;
            continue;
        }

        // identifier
        if (is_ident_head(*ptr)) {
            int len = read_ident(ptr);
            tk = alloc_next_token(TK_IDENT, ptr, len, tk);
            ptr += len;

            // overwrite the token kind for keywords
            if (slice_str_eq(tk->slice, "return")) {
                tk->kind = TK_RETURN;
            }

            if (slice_str_eq(tk->slice, "if")) {
                tk->kind = TK_IF;
            }

            if (slice_str_eq(tk->slice, "else")) {
                tk->kind = TK_ELSE;
            }

            continue;
        }

        panic_at(ptr, src, "Invalid string for the tokenizer");
    }

    alloc_next_token(TK_EOF, ptr, 0, tk);
    return head.next;
}
