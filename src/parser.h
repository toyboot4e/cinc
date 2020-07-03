//! † The parser †

#ifndef CINC_PARSER_H
#define CINC_PARSER_H

#include "tokenizer.h"

typedef struct ParseState ParseState;
/// An instance of `ParseState` is often referred to as `pst`
struct ParseState {
    Token *tk;
    char *src;
};
ParseState pst_init(Token *tk, char *src);

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
    int val; // available only if kind == ND_NUM
};

Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

Node *expr();

#endif
