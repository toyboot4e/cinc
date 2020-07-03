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

Token *g_token;

// --------------------------------------------------------------------------------
// Token readers
//
// All tokens are once toknized into a linked list and then read one by one by
// parser

bool consume_char(char c) {
    if (g_token->kind != TK_RESERVED || g_token->slice.str[0] != c) {
        return false;
    }
    g_token = g_token->next;
    return true;
}

bool consume_str(char *str) {
    if (g_token->kind != TK_RESERVED) {
        return false;
    }

    if (strlen(str) != g_token->slice.len ||
        memcmp(g_token->slice.str, str, g_token->slice.len)) {
        return false;
    }

    g_token = g_token->next;
    return true;
}

// Panics if it finds something other than the expected char
void expect_char(char op) {
    if (g_token->kind != TK_RESERVED || g_token->slice.str[0] != op) {
        panic_at(g_token->slice.str, g_input, "Expected a char '%c'", op);
    }
    g_token = g_token->next;
}

// Panics if it find something other than a number
int expect_number() {
    if (g_token->kind != TK_NUM) {
        panic_at(g_token->slice.str, g_input, "Expected a number");
    }
    int val = g_token->val;
    g_token = g_token->next;
    return val;
}

// --------------------------------------------------------------------------------
// Parsers

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

Node *eq();
Node *rel();
Node *add();
Node *mul();
Node *unary();
Node *primary();

/// expr = equality
Node *expr() {
    return eq();
    //
}

/// equality = relational ("==" relational | "!=" relational)*
Node *eq() {
    Node *node = rel();
    for (;;) {
        if (consume_str("==")) {
            node = new_node_binary(ND_EQ, node, rel());
        } else if (consume_str("!=")) {
            node = new_node_binary(ND_NE, node, rel());
        } else {
            return node;
        }
    }
}

/// relational = add (("<" | "<=" | ">" | ">=") add)*
Node *rel() {
    Node *node = add();
    for (;;) {
        if (consume_char('<')) {
            node = new_node_binary(ND_LT, node, add());
        } else if (consume_char('>')) {
            node = new_node_binary(ND_GT, node, add());
        } else if (consume_str("<=")) {
            node = new_node_binary(ND_LE, node, add());
        } else if (consume_str(">=")) {
            node = new_node_binary(ND_GE, node, add());
        } else {
            return node;
        }
    }
}

/// add = mul (("+" | "-") mul)*
Node *add() {
    Node *node = mul();
    for (;;) {
        if (consume_char('+')) {
            node = new_node_binary(ND_ADD, node, mul());
        } else if (consume_char('-')) {
            node = new_node_binary(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

/// mul = unary (("*" | "/") unary)*
Node *mul() {
    Node *node = unary();
    for (;;) {
        if (consume_char('*')) {
            node = new_node_binary(ND_MUL, node, unary());
        } else if (consume_char('/')) {
            node = new_node_binary(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

/// unary = ("+" | "-")? primary
Node *unary() {
    if (consume_char('+')) {
        return primary(); // this is enable by design
    } else if (consume_char('-')) {
        // we treat it as (0 - primary)
        return new_node_binary(ND_SUB, new_node_num(0), primary());
    } else {
        return primary();
    }
}

/// primary = num | "(" expr ")"
Node *primary() {
    if (consume_char('(')) {
        Node *node = expr();
        expect_char(')');
        return node;
    } else {
        return new_node_num(expect_number());
    }
}
