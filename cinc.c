#include "tokenizer.h"

#include <stdio.h>
#include <stdlib.h>

void print_header() {
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid args! `cinc` accepts one argument as an input.\n");
    exit(1);
  }

  token = tokenize(argv[1]);

  print_header();
  printf("main:\n");

  // number (operator number)*
  printf("  mov rax, %d\n", expect_number());
  while (!is_at_eof()) {
    if (consume_char('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect_char('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}
