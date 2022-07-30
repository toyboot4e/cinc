#ifndef CINC_CODEGEN_H
#define CINC_CODEGEN_H

#include "parse.h"

/// Generates assembly header
void write_asm_header();

/// Generates assembly for a node
void write_asm_node(Node *node);

#endif

