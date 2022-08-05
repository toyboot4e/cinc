//! Converts [`Token`]s into [`Node`]s

#ifndef CINC_PARSER_H
#define CINC_PARSER_H

#include "token.h"

/// Parse state, often referred to as `pst`
typedef struct {
    Token *tk;
    char *src;
} ParseState;

ParseState pst_init(Token *tk, char *src);
ParseState pst_from_source(char *src);

typedef enum { // forward-declarations for enums are forbidden..
    // statements
    ND_ASSIGN,
    ND_RETURN,
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,

    // primitives
    ND_NUM,

    /// Local variable
    ND_LVAR,

    // arithmetic operators
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,

    // comparison operators
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_GT,
    ND_GE,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;

    /// Next program node
    Node *next;

    /// (Binary)
    Node *lhs;
    /// (Binary)
    Node *rhs;

    /// (Number) Value
    int val;

    /// (Local variable) Byte offset of the local variable starting from the stack base pointer
    int offset;

    /// (`if`, `while`, `for`)
    Node *cond;
    /// (`if`, `while`, `for`)
    Node *then;
    /// (`if`)
    Node *else_;

    /// (`for`)
    Node *for_init;
    /// (`for`)
    Node *for_inc;

    /// (Block)
    Node *body;
};

typedef struct LocalVar LocalVar;

struct LocalVar {
    LocalVar *next;
    Slice slice;
    /// Byte offset of the local variable starting from the stack base pointer
    int offset;
};

LocalVar *find_lvar(LocalVar *lvar, Slice slice);

typedef struct {
    /// Linked list of local variables
    LocalVar *lvar;
    /// Linked list of nodes
    Node *node;
} Scope;

/// Returns 8 byte + sum of local variable sizes
int scope_size(Scope scope);

Scope parse_program(ParseState *pst);
Node *parse_stmt(ParseState *pst, Scope *scope);
Node *parse_expr(ParseState *pst, Scope *scope);

#endif
