//! † The parser †

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
    ParseState pst;
    pst.tk = tk;
    pst.src = src;
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

bool consume_char(ParseState *pst, char c) {
    if (pst->tk->kind != TK_RESERVED || pst->tk->slice.str[0] != c) {
        return false;
    }
    pst_inc(pst);
    return true;
}

bool consume_str(ParseState *pst, char *str) {
    if (pst->tk->kind != TK_RESERVED) {
        return false;
    }

    if (strlen(str) != pst->tk->slice.len ||
        memcmp(pst->tk->slice.str, str, pst->tk->slice.len)) {
        return false;
    }

    pst_inc(pst);
    return true;
}

// Panics if it finds something other than the expected char
void expect_char(ParseState *pst, char op) {
    if (pst->tk->kind != TK_RESERVED || pst->tk->slice.str[0] != op) {
        panic_at(pst->tk->slice.str, pst->src, "Expected a char '%c'", op);
    }
    pst_inc(pst);
}

// Panics if it find something other than a number
int expect_number(ParseState *pst) {
    if (pst->tk->kind != TK_NUM) {
        panic_at(pst->tk->slice.str, pst->src, "Expected a number");
    }
    int val = pst->tk->val;
    pst_inc(pst);
    return val;
}

// --------------------------------------------------------------------------------
// Node

Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->val = -999; // FIXME:
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// --------------------------------------------------------------------------------
// Parsers

static Node *parse_eq(ParseState *pst);
static Node *parse_rel(ParseState *pst);
static Node *parse_add(ParseState *pst);
static Node *parse_mul(ParseState *pst);
static Node *parse_unary(ParseState *pst);
static Node *parse_primary(ParseState *pst);

/// expr = equality
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

/// primary = num | "(" expr ")"
static Node *parse_primary(ParseState *pst) {
    if (consume_char(pst, '(')) {
        Node *node = parse_expr(pst);
        expect_char(pst, ')');
        return node;
    } else {
        return new_node_num(expect_number(pst));
    }
}
