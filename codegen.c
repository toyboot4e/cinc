#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

void gen_binary_node(Node *node);
void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf(" push %d\n", node->val);
    return;
  }
  gen_binary_node(node);
}

void gen_binary_node(Node *node) {
  gen(node->lhs);
  gen(node->rhs);
  printf(" pop rdi\n");
  printf(" pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf(" add rax, rdi\n");
    break;
  case ND_SUB:
    printf(" sub rax, rdi\n");
    break;
  case ND_MUL:
    printf(" imul rax, rdi\n");
    break;
  case ND_DIV:
    printf(" cqo\n");
    printf(" idiv rdi\n");
    break;
  default:
    fprintf(stderr, "Tried to parse a binary node, where non-operator found\n");
    exit(1);
    break;
  }
  printf(" push rax\n");
}
