#ifndef CINC_CODEGEN_H
#define CINC_CODEGEN_H

#include "parser.h"

/// Generates assembly header
void asm_header();

/// Generates assembly for a node
void gen(Node *node);

#endif

