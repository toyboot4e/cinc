#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "token.h"
#include "utils.h"

ParseState pst_init(Token *tk, char *src) {
    ParseState pst = {
        .tk = tk,
        .src = src,
    };
    return pst;
}

ParseState pst_from_source(char *src) {
    Token *tk = tokenize(src);
    return pst_init(tk, src);
}

static void pst_inc(ParseState *pst) {
    pst->tk = pst->tk->next;
}

// --------------------------------------------------------------------------------
// Token readers

/// True on EoF token
static bool is_at_eof(ParseState *pst) {
    return pst->tk->kind == TK_EOF;
}

/// Consumes a reserved token of a character
static bool consume_kind(ParseState *pst, TokenKind kind) {
    if (pst->tk->kind == kind) {
        pst_inc(pst);
        return true;
    }
    return false;
}

/// Consumes a reserved token of a character
static bool consume_char(ParseState *pst, char c) {
    if (pst->tk->kind != TK_RESERVED || pst->tk->slice.str[0] != c) {
        return false;
    }
    pst_inc(pst);
    return true;
}

/// Expects a reserved token of a character
static void expect_char(ParseState *pst, char op) {
    if (pst->tk->kind != TK_RESERVED || pst->tk->slice.str[0] != op) {
        panic_at(pst->tk->slice.str, pst->src, "Expected a char '%c'", op);
    }
    pst_inc(pst);
}

/// Consumes a reserved token of a word
static bool consume_word(ParseState *pst, char *str) {
    if (pst->tk->kind != TK_RESERVED) {
        return false;
    }

    if (strlen(str) != pst->tk->slice.len || memcmp(pst->tk->slice.str, str, pst->tk->slice.len)) {
        return false;
    }

    pst_inc(pst);
    return true;
}

static bool consume_number(ParseState *pst) {
    if (pst->tk->kind != TK_NUM) {
        return false;
    }

    pst_inc(pst);
    return true;
}

static bool consume_ident(ParseState *pst) {
    if (pst->tk->kind != TK_IDENT) {
        return false;
    }

    pst_inc(pst);
    return true;
}

// --------------------------------------------------------------------------------
// Scope

int scope_size(Scope scope) {
    if (scope.lvar) {
        // offset + variable size
        return scope.lvar->offset + 8;
    } else {
        // always jump over the base pointer
        return 8;
    }
}

LocalVar *find_lvar(LocalVar *lvars, Slice slice) {
    for (LocalVar *v = lvars; v; v = v->next) {
        if (slice_eq(v->slice, slice)) {
            return v;
        }
    }

    return NULL;
}

/// Create new local variable and push it onto the list
static void push_lvar(Scope *scope, Slice slice) {
    int offset = scope_size(*scope);

    LocalVar *root = NULL;
    if (scope->lvar) {
        root = scope->lvar;
    }

    LocalVar *new_root = calloc(1, sizeof(LocalVar));
    *new_root = (LocalVar){.next = root, .slice = slice, .offset = offset};

    scope->lvar = new_root;
}

static LocalVar *find_or_alloc_lvar(Scope *scope, Slice slice) {
    LocalVar *lvar = find_lvar(scope->lvar, slice);
    if (lvar) {
        return lvar;
    }

    push_lvar(scope, slice);
    return scope->lvar;
}

// --------------------------------------------------------------------------------
// Node constructors

/// Just allocates a new node
static Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    *node = (Node){
        .kind = kind,
        .val = -999, // FIXME:
        .lhs = lhs,
        .rhs = rhs,
    };
    return node;
}

/// Number
static Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    *node = (Node){
        .kind = ND_NUM,
        .val = val,
    };
    return node;
}

/// Creates local variable modifying the scope
static Node *new_node_lvar(Slice slice, Scope *scope) {
    LocalVar *lvar = find_or_alloc_lvar(scope, slice);

    Node *node = calloc(1, sizeof(Node));
    *node = (Node){
        .kind = ND_LVAR,
        .offset = lvar->offset,
    };

    return node;
}

// --------------------------------------------------------------------------------
// Parser

Node *parse_expr(ParseState *pst, Scope *scope);
static Node *parse_assign(ParseState *pst, Scope *scope);
static Node *parse_eq(ParseState *pst, Scope *scope);
static Node *parse_rel(ParseState *pst, Scope *scope);
static Node *parse_add(ParseState *pst, Scope *scope);
static Node *parse_mul(ParseState *pst, Scope *scope);
static Node *parse_unary(ParseState *pst, Scope *scope);
static Node *parse_primary(ParseState *pst, Scope *scope);

/// program = stmt*
Scope parse_program(ParseState *pst) {
    Scope scope;

    scope.node = parse_stmt(pst, &scope);
    Node *last_node = scope.node;

    // parse until EoF node
    while (!is_at_eof(pst)) {
        Node *next = parse_stmt(pst, &scope);

        last_node->next = next;
        last_node = next;
    }

    return scope;
}

/// stmt = expr ";"
///      | "return" expr ";"
///      | "if" stmt ("else" stmt)? ";"
///      | "while" "(" expr ")" stmt
///      | "for" "(" stmt stmt  ")" stmt
///      | "{" stmt* "}"
Node *parse_stmt(ParseState *pst, Scope *scope) {
    // return statement
    if (consume_kind(pst, TK_RETURN)) {
        Node *ret = new_node(ND_RETURN, NULL, NULL);
        ret->lhs = parse_expr(pst, scope);
        expect_char(pst, ';');

        return ret;
    }

    // if statement
    if (consume_kind(pst, TK_IF)) {
        Node *if_ = new_node(ND_IF, NULL, NULL);
        expect_char(pst, '(');
        if_->cond = parse_expr(pst, scope);
        expect_char(pst, ')');
        if_->then = parse_stmt(pst, scope);

        if (consume_kind(pst, TK_ELSE)) {
            if_->else_ = parse_stmt(pst, scope);
        }

        return if_;
    }

    // while statement
    if (consume_kind(pst, TK_WHILE)) {
        Node *while_ = new_node(ND_WHILE, NULL, NULL);
        expect_char(pst, '(');
        while_->cond = parse_expr(pst, scope);
        expect_char(pst, ')');
        while_->then = parse_stmt(pst, scope);

        return while_;
    }

    // for statement
    if (consume_kind(pst, TK_FOR)) {
        Node *for_ = new_node(ND_FOR, NULL, NULL);
        expect_char(pst, '(');

        for_->for_init = parse_expr(pst, scope);
        expect_char(pst, ';');
        for_->cond = parse_expr(pst, scope);
        expect_char(pst, ';');
        for_->for_inc = parse_expr(pst, scope);

        expect_char(pst, ')');
        for_->then = parse_stmt(pst, scope);

        return for_;
    }

    // compound statement (code block)
    if (consume_char(pst, '{')) {
        Node *block = new_node(ND_BLOCK, NULL, NULL);

        Node list;
        Node *tail = &list;

        while (!consume_char(pst, '}')) {
            tail->next = parse_stmt(pst, scope);
            tail = tail->next;
        }

        block->body = list.next;
        return block;
    }

    // expression statement
    Node *expr = parse_expr(pst, scope);
    expect_char(pst, ';');

    return expr;
}

/// expr = assign
Node *parse_expr(ParseState *pst, Scope *scope) {
    return parse_assign(pst, scope);
}

/// assign = equality ("=" assign)*
Node *parse_assign(ParseState *pst, Scope *scope) {
    Node *node = parse_eq(pst, scope);
    if (consume_word(pst, "=")) {
        node = new_node(ND_ASSIGN, node, parse_assign(pst, scope));
    }

    return node;
}

/// equality = relational ("==" relational | "!=" relational)*
static Node *parse_eq(ParseState *pst, Scope *scope) {
    Node *node = parse_rel(pst, scope);
    for (;;) {
        if (consume_word(pst, "==")) {
            node = new_node(ND_EQ, node, parse_rel(pst, scope));
        } else if (consume_word(pst, "!=")) {
            node = new_node(ND_NE, node, parse_rel(pst, scope));
        } else {
            return node;
        }
    }
}

/// relational = add (("<" | "<=" | ">" | ">=") add)*
static Node *parse_rel(ParseState *pst, Scope *scope) {
    Node *node = parse_add(pst, scope);
    for (;;) {
        // match onto longer words first!
        if (consume_word(pst, "<=")) {
            node = new_node(ND_LE, node, parse_add(pst, scope));
        } else if (consume_word(pst, ">=")) {
            node = new_node(ND_GE, node, parse_add(pst, scope));
        } else if (consume_char(pst, '<')) {
            node = new_node(ND_LT, node, parse_add(pst, scope));
        } else if (consume_char(pst, '>')) {
            node = new_node(ND_GT, node, parse_add(pst, scope));
        } else {
            return node;
        }
    }
}

/// add = mul (("+" | "-") mul)*
static Node *parse_add(ParseState *pst, Scope *scope) {
    Node *node = parse_mul(pst, scope);
    for (;;) {
        if (consume_char(pst, '+')) {
            node = new_node(ND_ADD, node, parse_mul(pst, scope));
        } else if (consume_char(pst, '-')) {
            node = new_node(ND_SUB, node, parse_mul(pst, scope));
        } else {
            return node;
        }
    }
}

/// mul = unary (("*" | "/") unary)*
static Node *parse_mul(ParseState *pst, Scope *scope) {
    Node *node = parse_unary(pst, scope);
    for (;;) {
        if (consume_char(pst, '*')) {
            node = new_node(ND_MUL, node, parse_unary(pst, scope));
        } else if (consume_char(pst, '/')) {
            node = new_node(ND_DIV, node, parse_unary(pst, scope));
        } else {
            return node;
        }
    }
}

/// unary = ("+" | "-") unary | primary
//
// Plus operator can be used like `3 + +5` by design.
static Node *parse_unary(ParseState *pst, Scope *scope) {
    if (consume_char(pst, '+')) {
        return parse_unary(pst, scope);
    } else if (consume_char(pst, '-')) {
        // we treat it as (0 - primary)
        return new_node(ND_SUB, new_node_num(0), parse_unary(pst, scope));
    } else {
        return parse_primary(pst, scope);
    }
}

/// primary = (num | ident | call) | "(" expr ")"
/// call = ident "(" args ")"
/// args =
static Node *parse_primary(ParseState *pst, Scope *scope) {
    if (consume_char(pst, '(')) {
        Node *node = parse_expr(pst, scope);
        expect_char(pst, ')');
        return node;
    }

    Token *tk = pst->tk;

    if (consume_number(pst)) {
        return new_node_num(tk->val);
    }

    if (consume_ident(pst)) {
        if (consume_char(pst, '(')) {
            Node *call = new_node(ND_CALL, NULL, NULL);
            call->fname = tk->slice;

            expect_char(pst, ')');
            return call;
        } else {
            return new_node_lvar(tk->slice, scope);
        }
    }

    // Panic:
    char *s = slice_to_string(pst->tk->slice);
    panic_at(pst->tk->slice.str, pst->src, "Expected number or ident: %s", s);

    // unreachable, just for the analyzer
    return NULL;
}
