#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "parse.h"
#include "utils.h"

void write_program(Scope scope) {
    write_asm_header();
    write_prologue(scope);

    Node *node = scope.node;
    while (node) {
        write_asm_node(node);
        // pop last value on the stack
        printf("  pop rax\n");
        node = node->next;
    }

    write_epilogue(scope);
}

void write_asm_header() {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
}

void write_prologue(Scope scope) {
    printf("  # prologue\n");
    // save the last RBP
    printf("  push rbp\n");
    // save the current RSP (which refers to the last RBP) to RBP
    printf("  mov rbp, rsp\n");
    // add space for base pointer and local variables
    int size = scope_size(scope);
    printf("  sub rsp, %d\n", size);

    printf("\n");
}

void write_epilogue(Scope scope) {
    printf("\n");

    printf("  # epilogue\n");
    // go back to the last point
    printf("  mov rsp, rbp\n");
    // now, [RSP] is the last RBP
    printf("  pop rbp\n");
    printf("  ret\n");
}

static void write_bin_node(Node *node);

void write_asm_node(Node *node) {
    if (node->kind == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }
    write_bin_node(node);
}

/// Write left value (the address of the target value)
void write_lval(Node *node) {
    if (node->kind != ND_LVAR) {
        panic("left value expected");
    }

    printf("  # lval\n");
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

static void write_bin_node(Node *node) {
    switch (node->kind) {
    case ND_ASSIGN:
        write_lval(node->lhs);
        write_bin_node(node->rhs);

        printf("  # * assign\n");
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;

    case ND_NUM:
        printf("  push %d\n", node->val);
        return;

    case ND_LVAR:
        write_lval(node);

        printf("  # lvar (dereference the last lval)\n");
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;

    default:
        break;
    }

    write_asm_node(node->lhs);
    write_asm_node(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        // arithmetic operators
    case ND_ADD:
        printf("  add rax, rdi\n");
        break;

    case ND_SUB:
        printf("  sub rax, rdi\n");
        break;

    case ND_MUL:
        printf("  imul rax, rdi\n");
        break;

    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;

        // comparison operators
    case ND_EQ:
        // ==
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;

    case ND_NE:
        // !=
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;

    case ND_LT:
        // <
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;

    case ND_LE:
        // <=
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;

    case ND_GT:
        // >
        printf("  cmp rdi, rax\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;

    case ND_GE:
        // >=
        printf("  cmp rdi, rax\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;

    default:
        fprintf(stderr, "Tried to parse a binary node, found non-operator (NodeKind: %d)\n",
                node->kind);
        exit(1);
        break;
    }

    printf("  push rax\n");
}
