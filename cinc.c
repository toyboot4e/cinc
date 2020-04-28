//! cinc is a C compiler in C which only works on Linux.
//!
//! Stratedy: cinc doesn't `free` heap memories

#include "codegen.h"
#include "parser.h"
#include "tokenizer.h"

#include <stdio.h>
#include <stdlib.h>

void print_header() {
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid args! `cinc` accepts one argument as an input.\n");
    exit(1);
  }

  token = tokenize(argv[1]);
  Node *node = expr(token);

  print_header();
  gen(node);
  printf(" pop rax\n");
  printf(" ret\n");

  return 0;
}
