//! cinc is a C compiler in which emits x86-64 assembly in Intel syntax
//!
//! Strategy: cinc doesn't `free` heap memories. cinc uses global variables for tokenizer

#include "asm.h"
#include "parse.h"
#include "token.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr,
                "Invalid args! `cinc` accepts one argument as an input.\n");
        exit(1);
    }

    char *src = argv[1];
    Token *tk = tokenize(src);
    ParseState pst = pst_init(tk, src);
    Node *node = expr(&pst);

    asm_header();
    gen(node);
    printf(" pop rax\n");
    printf(" ret\n");

    return 0;
}
