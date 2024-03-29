//! cinc is a C compiler in C, which emits x86-64 assembly in Intel syntax

// Style:
// - Don't `free` heap memories for simplicity
// - Don't use global variables

#include "codegen.h"
#include "parse.h"
#include "token.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid args! `cinc` accepts one argument as an input.\n");
        exit(1);
    }

    char *src = argv[1];
    ParseState pst = pst_from_source(src);

    Scope scope = parse_program(&pst);
    write_program(scope);

    return 0;
}
