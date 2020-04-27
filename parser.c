//! † The parser †

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "tokenizer.h"

/// Sets up a binary node
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

/// Sets up a number node
Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

/// expr = mul ("+" mul | "-" mul)*
Node *expr() {
  Node *node = mul();
  for (;;) {
    if (consume_char('+')) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume_char('-')) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

/// mul = primary ("*" primary | "/" primary)*
Node *mul() {
  Node *node = primary();
  for (;;) {
    if (consume_char('*')) {
      node = new_node(ND_MUL, node, primary());
    } else if (consume_char('/')) {
      node = new_node(ND_DIV, node, primary());
    } else {
      return node;
    }
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
