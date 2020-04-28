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
Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->val = -999; // easier to find errors (hack)
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

Node *mul();
Node *unary();
Node *primary();

/// expr = mul ("+" mul | "-" mul)*
Node *expr() {
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

/// mul = unary ("*" unary | "/" unary)*
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
  }

  if (consume_char('-')) {
    // we treat it as (0 - primary)
    return new_node_binary(ND_SUB, new_node_num(0), primary());
  }

  return primary();
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
