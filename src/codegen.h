#ifndef CINC_CODEGEN_H
#define CINC_CODEGEN_H

#include "parser.h"

void asm_header();

/// Generates assembly for a node
void gen(Node *node);

#endif

