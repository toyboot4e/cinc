#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "token.h"
#include "utils.h"

ParseState pst_init(Token *tk, char *src) {
    ParseState pst = {
        .tk = tk,
        .src = src,
    };
    return pst;
}

ParseState pst_from_source(char *src) {
    Token *tk = tokenize(src);
    return pst_init(tk, src);
}

static void pst_inc(ParseState *pst) {
    pst->tk = pst->tk->next;
}

// --------------------------------------------------------------------------------
// Token readers

static bool is_at_eof(ParseState *pst) {
    return pst->tk->kind == TK_EOF;
}

static bool consume_char(ParseState *pst, char c) {
    if (pst->tk->kind != TK_RESERVED || pst->tk->slice.str[0] != c) {
        return false;
    }
    pst_inc(pst);
    return true;
}

static void expect_char(ParseState *pst, char op) {
    if (pst->tk->kind != TK_RESERVED || pst->tk->slice.str[0] != op) {
        panic_at(pst->tk->slice.str, pst->src, "Expected a char '%c'", op);
    }
    pst_inc(pst);
}

static bool consume_str(ParseState *pst, char *str) {
    if (pst->tk->kind != TK_RESERVED) {
        return false;
    }

    if (strlen(str) != pst->tk->slice.len || memcmp(pst->tk->slice.str, str, pst->tk->slice.len)) {
        return false;
    }

    pst_inc(pst);
    return true;
}

static bool consume_number(ParseState *pst) {
    if (pst->tk->kind != TK_NUM) {
        return false;
    }

    pst_inc(pst);
    return true;
}

// --------------------------------------------------------------------------------
// Node

static Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    *node = (Node){
        .kind = kind,
        .val = -999, // FIXME:
        .lhs = lhs,
        .rhs = rhs,
    };
    return node;
}

/// Number
static Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    *node = (Node){
        .kind = ND_NUM,
        .val = val,
    };
    return node;
}

// --------------------------------------------------------------------------------
// Parsers

Node *parse_expr(ParseState *pst);
static Node *parse_eq(ParseState *pst);
static Node *parse_rel(ParseState *pst);
static Node *parse_add(ParseState *pst);
static Node *parse_mul(ParseState *pst);
static Node *parse_unary(ParseState *pst);
static Node *parse_primary(ParseState *pst);

/// program = stmt*
Node *parse_program(ParseState *pst) {
    Node *root = parse_stmt(pst);
    Node *node = root;

    // parse until EoF node
    while (!is_at_eof(pst)) {
        node->next = parse_stmt(pst);
        node = node->next;
    }

    return root;
}

/// stmt = expr ";"
Node *parse_stmt(ParseState *pst) {
    Node *node = parse_expr(pst);
    expect_char(pst, ';');
    return node;
}

Node *parse_expr(ParseState *pst) {
    return parse_eq(pst);
    //
}

/// equality = relational ("==" relational | "!=" relational)*
static Node *parse_eq(ParseState *pst) {
    Node *node = parse_rel(pst);
    for (;;) {
        if (consume_str(pst, "==")) {
            node = new_node_binary(ND_EQ, node, parse_rel(pst));
        } else if (consume_str(pst, "!=")) {
            node = new_node_binary(ND_NE, node, parse_rel(pst));
        } else {
            return node;
        }
    }
}

/// relational = add (("<" | "<=" | ">" | ">=") add)*
static Node *parse_rel(ParseState *pst) {
    Node *node = parse_add(pst);
    for (;;) {
        // match onto longer words first!
        if (consume_str(pst, "<=")) {
            node = new_node_binary(ND_LE, node, parse_add(pst));
        } else if (consume_str(pst, ">=")) {
            node = new_node_binary(ND_GE, node, parse_add(pst));
        } else if (consume_char(pst, '<')) {
            node = new_node_binary(ND_LT, node, parse_add(pst));
        } else if (consume_char(pst, '>')) {
            node = new_node_binary(ND_GT, node, parse_add(pst));
        } else {
            return node;
        }
    }
}

/// add = mul (("+" | "-") mul)*
static Node *parse_add(ParseState *pst) {
    Node *node = parse_mul(pst);
    for (;;) {
        if (consume_char(pst, '+')) {
            node = new_node_binary(ND_ADD, node, parse_mul(pst));
        } else if (consume_char(pst, '-')) {
            node = new_node_binary(ND_SUB, node, parse_mul(pst));
        } else {
            return node;
        }
    }
}

/// mul = unary (("*" | "/") unary)*
static Node *parse_mul(ParseState *pst) {
    Node *node = parse_unary(pst);
    for (;;) {
        if (consume_char(pst, '*')) {
            node = new_node_binary(ND_MUL, node, parse_unary(pst));
        } else if (consume_char(pst, '/')) {
            node = new_node_binary(ND_DIV, node, parse_unary(pst));
        } else {
            return node;
        }
    }
}

/// unary = ("+" | "-") unary | primary
//
// Plus operator can be used like `3 + +5` by design.
static Node *parse_unary(ParseState *pst) {
    if (consume_char(pst, '+')) {
        return parse_unary(pst);
    } else if (consume_char(pst, '-')) {
        // we treat it as (0 - primary)
        return new_node_binary(ND_SUB, new_node_num(0), parse_unary(pst));
    } else {
        return parse_primary(pst);
    }
}

/// primary = (num | ident) | "(" expr ")"
static Node *parse_primary(ParseState *pst) {
    if (consume_char(pst, '(')) {
        Node *node = parse_expr(pst);
        expect_char(pst, ')');
        return node;
    } else {
        Token *tk = pst->tk;

        if (consume_number(pst)) {
            return new_node_num(tk->val);
        }

        // retrieve null-terminated string from the slice
        int len = pst->tk->slice.len;
        char *s = malloc(len + 1);
        memcpy(s, pst->tk->slice.str, len);
        s[len] = '\n';

        panic_at(pst->tk->slice.str, pst->src, "Expected number or ident: %s", s);

        // unreachable
        return NULL;
    }
}
