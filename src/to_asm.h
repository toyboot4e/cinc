//! Outputs x86-64 assembly in Intel syntax

#ifndef CINC_CODEGEN_H
#define CINC_CODEGEN_H

#include "parse.h"

/// Outputs x86-64 assembly
void write_program(Scope scope);

/// Outputs assembly header
void write_asm_header();

/// Outputs function prologue
void write_prologue();

/// Outputs function epilogue
void write_epilogue();

/// Outputs assembly for a node
void write_asm_node(Node *node);

#endif
