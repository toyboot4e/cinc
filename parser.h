//! † The parser †

#ifndef CINC_PARSER_H
#define CINC_PARSER_H

#include "tokenizer.h"

typedef enum {
  ND_NUM,

  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val; // available only if kind == ND_NUM
};

Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

Node *expr();

#endif
