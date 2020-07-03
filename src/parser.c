//! † The parser †

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "tokenizer.h"
#include "utils.h"

ParseState pst_init(Token *tk, char *src) {
    ParseState pst;
    pst.tk = tk;
    pst.src = src;
    return pst;
}

void pst_inc(ParseState *pst) { pst->tk = pst->tk->next; }

// --------------------------------------------------------------------------------
// Token readers
//
// All tokens are once toknized into a linked list and then read one by one by
// parser

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

Node *eq();
Node *rel();
Node *add();
Node *mul();
Node *unary();
Node *primary();

/// expr = equality
Node *expr(ParseState *pst) {
    return eq(pst);
    //
}

/// equality = relational ("==" relational | "!=" relational)*
Node *eq(ParseState *pst) {
    Node *node = rel(pst);
    for (;;) {
        if (consume_str(pst, "==")) {
            node = new_node_binary(ND_EQ, node, rel(pst));
        } else if (consume_str(pst, "!=")) {
            node = new_node_binary(ND_NE, node, rel(pst));
        } else {
            return node;
        }
    }
}

/// relational = add (("<" | "<=" | ">" | ">=") add)*
Node *rel(ParseState *pst) {
    Node *node = add(pst);
    for (;;) {
        if (consume_char(pst, '<')) {
            node = new_node_binary(ND_LT, node, add());
        } else if (consume_char(pst, '>')) {
            node = new_node_binary(ND_GT, node, add());
        } else if (consume_str(pst, "<=")) {
            node = new_node_binary(ND_LE, node, add());
        } else if (consume_str(pst, ">=")) {
            node = new_node_binary(ND_GE, node, add());
        } else {
            return node;
        }
    }
}

/// add = mul (("+" | "-") mul)*
Node *add(ParseState *pst) {
    Node *node = mul();
    for (;;) {
        if (consume_char(pst, '+')) {
            node = new_node_binary(ND_ADD, node, mul());
        } else if (consume_char(pst, '-')) {
            node = new_node_binary(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

/// mul = unary (("*" | "/") unary)*
Node *mul(ParseState *pst) {
    Node *node = unary(pst);
    for (;;) {
        if (consume_char(pst, '*')) {
            node = new_node_binary(ND_MUL, node, unary());
        } else if (consume_char(pst, '/')) {
            node = new_node_binary(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

/// unary = ("+" | "-")? primary
Node *unary(ParseState *pst) {
    if (consume_char(pst, '+')) {
        return primary(pst); // this is enable by design
    } else if (consume_char(pst, '-')) {
        // we treat it as (0 - primary)
        return new_node_binary(ND_SUB, new_node_num(0), primary());
    } else {
        return primary(pst);
    }
}

/// primary = num | "(" expr ")"
Node *primary(ParseState *pst) {
    if (consume_char(pst, '(')) {
        Node *node = expr(pst);
        expect_char(pst, ')');
        return node;
    } else {
        return new_node_num(expect_number(pst));
    }
}
