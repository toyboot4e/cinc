#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>

void print_header() {
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
}

int main(int argc, char **argv) {
  token = tokenize(argv[1]);

  print_header();
  printf("main:\n");

  // number (operator number)*
  printf("  mov rax, %d\n", expect_number());
  while (!is_at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}

