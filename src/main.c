//! cinc is a C compiler in which emits x86-64 assembly in Intel syntax
//!
//! Strategy:
//! - Don't `free` heap memories
//! - Don't use global variables

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
    ParseState pst = pst_from_source(src);
    Node *node = parse_expr(&pst);

    asm_header();
    asm_node(node);
    printf(" pop rax\n");
    printf(" ret\n");

    return 0;
}
