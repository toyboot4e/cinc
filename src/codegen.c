#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "parse.h"
#include "utils.h"

/// - `discard`: pops the last value if true
static void write_any(Node *node, bool discard);

static const bool DISCARD = true;
static const bool KEEP = false;

void write_program(Scope scope) {
    write_asm_header();

    write_prologue(scope);

    for (Node *node = scope.node; node; node = node->next) {
        write_any(node, DISCARD);
    }

    printf("\n");
    printf("  # epilogue\n");
    write_epilogue();
}

void write_asm_header() {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
}

void write_prologue(Scope scope) {
    // push BSP to the linked list
    printf("  # prologue\n");
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    int size = scope_size(scope);
    printf("    sub rsp, %d\n", size);
    printf("\n");
}

void write_epilogue() {
    // pop BSP of the linked list
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
}

static void discard_if(bool b) {
    if (b) {
        printf("  # discard\n");
        printf("    pop rax\n");
    }
}

static void write_addr(Node *node) {
    if (node->kind != ND_LVAR) {
        panic("left value expected");
    }

    printf("  # push address\n");
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

/// Sequential number for unique label names
int gSeq = 0;

static void write_any(Node *node, bool discard) {
    switch (node->kind) {
    case ND_ASSIGN:
        write_addr(node->lhs);
        write_any(node->rhs, KEEP);

        printf("  # assign\n");
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");

        discard_if(discard);
        return;

    case ND_RETURN:
        write_any(node->lhs, KEEP);
        printf("  pop rax\n");

        // jumping to function epilogue also works
        printf("  # return (embedded epilogue)\n");
        write_epilogue();
        return;

    case ND_IF: {
        int seq = gSeq++;

        if (node->else_) {
            // if then else
            printf("  # if else\n");
            write_any(node->cond, KEEP);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");

            // goto else, goto end
            printf("  je .Lelse%d\n", seq);

            // then
            write_any(node->then, DISCARD);
            printf("  jmp .Lend_if%d\n", seq);

            // else
            printf(".Lelse%d:\n", seq);
            write_any(node->else_, DISCARD);
            printf("  jmp .Lend_if%d\n", seq);

            // end
            printf(".Lend_if%d:\n", seq);
        } else {
            // if then no else
            printf("  # if\n");
            write_any(node->cond, KEEP);

            // goto else
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lend_if%d\n", seq);

            // then
            write_any(node->then, DISCARD);
            printf("  jmp .Lend_if%d\n", seq);

            // end
            printf(".Lend_if%d:\n", seq);
        }

        return;
    }

    case ND_WHILE: {
        int seq = gSeq++;

        printf(".Lloop_while%d:\n", seq);
        write_any(node->cond, KEEP);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend_while%d\n", seq);

        write_any(node->then, DISCARD);
        printf("  jmp .Lloop_while%d\n", seq);

        printf(".Lend_while%d:\n", seq);
        return;
    }

    case ND_FOR: {
        int seq = gSeq++;

        write_any(node->for_init, DISCARD);
        printf(".Lloop_for%d:\n", seq);

        write_any(node->cond, KEEP);
        printf("  cmp rax, 0\n");
        printf("  je .Lend_for%d\n", seq);

        write_any(node->for_inc, DISCARD);
        write_any(node->then, DISCARD);
        printf("  jmp .Lloop_for%d\n", seq);

        printf(".Lend_for%d:\n", seq);
        return;
    }

    case ND_BLOCK: {
        for (Node *n = node->body; n; n = n->next) {
            write_any(n, DISCARD);
        }

        return;
    };

    case ND_CALL:
        printf("  call %.*s\n", node->fname.len, node->fname.str);
        printf("  push rax\n");

        discard_if(discard);
        return;

    case ND_LVAR:
        printf("  # local variable (push address + dereference rax)\n");
        write_addr(node);

        printf("  # dereference rax\n");
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");

        discard_if(discard);
        return;

    case ND_NUM:
        printf("  push %d\n", node->val);

        discard_if(discard);
        return;

    default:
        break;
    }

    // binary expressions
    write_any(node->lhs, false);
    write_any(node->rhs, false);

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
        printf("  # /\n");
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;

        // comparison operators
    case ND_EQ:
        // ==
        printf("  # ==\n");
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;

    case ND_NE:
        // !=
        printf("  # !=\n");
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;

    case ND_LT:
        // <
        printf("  # <\n");
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;

    case ND_LE:
        // <=
        printf("  # <=\n");
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;

    case ND_GT:
        // >
        printf("  # >\n");
        printf("    cmp rdi, rax\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;

    case ND_GE:
        // >=
        printf("  # >=\n");
        printf("    cmp rdi, rax\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;

    default:
        fprintf(stderr, "Tried to parse a binary node, found non-operator (NodeKind: %d)\n",
                node->kind);
        exit(1);
        break;
    }

    printf("  push rax\n");
    discard_if(discard);
}
