#ifndef PARDON_PARSER_H
#define PARDON_PARSER_H

#include "lexer.h"

typedef enum {
    NODE_PROGRAM,
    NODE_ASSIGN,
    NODE_COMPOUND_ASSIGN,
    NODE_IF,
    NODE_LOOP,
    NODE_FUNC_DEF,
    NODE_FUNC_CALL,
    NODE_RETURN,
    NODE_PRINT,
    NODE_INPUT,
    NODE_COMPARE,
    NODE_VAR_REF,
    NODE_NUMBER_LIT,
    NODE_STRING_LIT,
    NODE_ARITH
} NodeType;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD
} ArithOp;

typedef enum {
    CMP_GT,         /* hotter than */
    CMP_LT,         /* colder than */
    CMP_EQ,         /* as hot as */
    CMP_ZERO        /* is frigid */
} CmpOp;

typedef struct ASTNode ASTNode;

struct ASTNode {
    NodeType type;
    int line;

    union {
        /* NODE_PROGRAM */
        struct { ASTNode **stmts; int count; } program;

        /* NODE_ASSIGN: moan X is EXPR / stash X with EXPR */
        struct { char *name; ASTNode *value; } assign;

        /* NODE_COMPOUND_ASSIGN: X gets EXPR */
        struct { char *name; ASTNode *value; } compound_assign;

        /* NODE_ARITH: left OP right */
        struct { ASTNode *left; ArithOp op; ASTNode *right; } arith;

        /* NODE_COMPARE: left CMP right (right may be NULL for is_frigid) */
        struct { ASTNode *left; CmpOp op; ASTNode *right; } compare;

        /* NODE_IF */
        struct {
            ASTNode *condition;
            ASTNode **then_body; int then_count;
            ASTNode **else_body; int else_count;
        } if_stmt;

        /* NODE_LOOP */
        struct {
            char *var;
            ASTNode *limit;
            ASTNode **body; int body_count;
        } loop;

        /* NODE_FUNC_DEF */
        struct {
            char *name;
            char **params; int param_count;
            ASTNode **body; int body_count;
        } func_def;

        /* NODE_FUNC_CALL */
        struct {
            char *name;
            ASTNode **args; int arg_count;
        } func_call;

        /* NODE_RETURN */
        struct { ASTNode *value; } ret;

        /* NODE_PRINT */
        struct { ASTNode *value; int newline; } print;

        /* NODE_INPUT */
        struct { char *var; } input;

        /* NODE_VAR_REF */
        struct { char *name; } var_ref;

        /* NODE_NUMBER_LIT */
        struct { double value; } number_lit;

        /* NODE_STRING_LIT */
        struct { char *value; } string_lit;
    };
};

ASTNode *parser_parse(TokenList *tokens);
void ast_free(ASTNode *node);

#endif
