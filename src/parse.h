//! Converts [`Token`]s into [`Node`]s

#ifndef CINC_PARSER_H
#define CINC_PARSER_H

#include "token.h"

typedef struct ParseState ParseState;

/// Parse state, often referred to as `pst`
struct ParseState {
    Token *tk;
    char *src;
};

ParseState pst_init(Token *tk, char *src);
ParseState pst_from_source(char *src);

typedef enum { // forward-declarations for enums are forbidden..
    // primitives
    ND_NUM,

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
    Node *lhs;
    Node *rhs;
    int val;
    /// Next program node
    Node *next;
};

Node *parse_program(ParseState *pst);
Node *parse_stmt(ParseState *pst);
Node *parse_expr(ParseState *pst);

#endif
