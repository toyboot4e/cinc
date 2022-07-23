#ifndef CINC_CODEGEN_H
#define CINC_CODEGEN_H

#include "parse.h"

/// Generates assembly header
void asm_header();

/// Generates assembly for a node
void asm_node(Node *node);

#endif

