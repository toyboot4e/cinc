#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "parse.h"
#include "utils.h"

// TODO: detect invalid pop

void write_program(Scope scope) {
    write_asm_header();
    write_prologue(scope);

    Node *node = scope.node;
    while (node) {
        write_asm_node(node);
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

    // push BSP to the linked list
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    int size = scope_size(scope);
    printf("  sub rsp, %d\n", size);

    printf("\n");
}

void write_epilogue(Scope scope) {
    printf("\n");
    printf("  # epilogue\n");

    // pop BSP of the linked list
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}

static void write_any(Node *node);

void write_asm_node(Node *node) {
    if (node->kind == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }
    write_any(node);
}

/// Write the local varialbe7s address as left value
void write_lval(Node *node) {
    if (node->kind != ND_LVAR) {
        panic("left value expected");
    }

    printf("  # lval\n");
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

/// Sequential number for unique label names
int gSeq = 0;

static void write_any(Node *node) {
    switch (node->kind) {
    case ND_ASSIGN:
        write_lval(node->lhs);
        write_any(node->rhs);

        printf("  # * assign\n");
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;

    case ND_RETURN:
        write_any(node->lhs);
        printf("  pop rax\n");

        // write function prologue
        // (jumping to function epilogue also works)
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;

    case ND_IF:
        int seq = gSeq++;

        if (node->else_) {
            // if then else
            printf("  # if else\n");
            write_any(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");

            // goto else, goto end
            printf("  je .Lelse%d\n", seq);

            // then
            write_any(node->then);
            printf("  jmp .Lend%d\n", seq);

            // else
            printf(".Lelse%d:\n", seq);
            write_any(node->else_);
            printf("  jmp .Lend%d\n", seq);

            // end
            printf(".Lend%d:\n", seq);
        } else {
            // if then no else
            printf("  # if\n");
            write_any(node->cond);

            // goto else
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lend%d\n", seq);

            // then
            write_any(node->then);
            printf("  jmp .Lend%d\n", seq);

            // end
            printf(".Lend%d:\n", seq);
        }

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

    // binary expressions
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
