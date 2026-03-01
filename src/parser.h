#ifndef WHANKA_PARSER_H
#define WHANKA_PARSER_H

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
    NODE_ARITH,
    NODE_LOGIC,
    NODE_RANDOM,
    NODE_ARRAY_DECL,
    NODE_ARRAY_GET,
    NODE_ARRAY_SET,
    NODE_ARRAY_LENGTH,
    NODE_FOREACH,
    NODE_FILE_READ,
    NODE_FILE_WRITE,
    NODE_STRING_SPLIT,
    NODE_STRING_JOIN,
    NODE_ARRAY_PUSH,
    NODE_ARRAY_POP,
    NODE_MATH,
    NODE_CAST,
    NODE_MAP_DECL,
    NODE_MAP_SET,
    NODE_MAP_GET
} NodeType;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_POW
} ArithOp;

typedef enum { MATH_SQRT, MATH_ABS, MATH_FLOOR, MATH_CEIL } MathOp;
typedef enum { CAST_TO_NUM, CAST_TO_STR } CastOp;

typedef enum {
    CMP_GT,         /* hotter than */
    CMP_LT,         /* colder than */
    CMP_EQ,         /* as hot as */
    CMP_ZERO,       /* is frigid */
    CMP_GTE         /* hits (>=) */
} CmpOp;

typedef enum {
    LOGIC_AND,
    LOGIC_OR,
    LOGIC_NOT
} LogicOp;

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

        /* NODE_LOOP: general condition (busted when BOOL_EXPR) */
        struct {
            ASTNode *condition;
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

        /* NODE_LOGIC */
        struct { ASTNode *left; LogicOp op; ASTNode *right; } logic;

        /* NODE_RANDOM: roll the dice EXPR */
        struct { ASTNode *max; } random_expr;

        /* NODE_ARRAY_DECL: stash X with N slots */
        struct { char *name; ASTNode *size; } array_decl;

        /* NODE_ARRAY_GET: grab X at EXPR */
        struct { char *name; ASTNode *index; } array_get;

        /* NODE_ARRAY_SET: X at EXPR gets EXPR */
        struct { char *name; ASTNode *index; ASTNode *value; } array_set;

        /* NODE_ARRAY_LENGTH: body count of X */
        struct { char *name; } array_length;

        /* NODE_FOREACH: run a train on ITERABLE ... done. */
        struct { ASTNode *iterable; ASTNode **body; int body_count; } foreach;

        /* NODE_FILE_READ: devour PATH into VAR */
        struct { ASTNode *path; char *var; } file_read;

        /* NODE_FILE_WRITE: dump CONTENT into PATH */
        struct { ASTNode *content; ASTNode *path; } file_write;

        /* NODE_STRING_SPLIT: chop STR by DELIM */
        struct { ASTNode *str; ASTNode *delimiter; } string_split;

        /* NODE_STRING_JOIN: stitch ARRAY with SEP */
        struct { ASTNode *array; ASTNode *separator; } string_join;

        /* NODE_ARRAY_PUSH: shove VALUE into ARRAY */
        struct { ASTNode *value; char *array_name; } array_push;

        /* NODE_ARRAY_POP: yank from ARRAY */
        struct { char *array_name; } array_pop;

        /* NODE_MATH: unary math (sqrt/abs/floor/ceil) */
        struct { MathOp op; ASTNode *operand; } math_expr;

        /* NODE_CAST: type coercion */
        struct { CastOp op; ASTNode *operand; } cast_expr;

        /* NODE_MAP_DECL: stash X with entries */
        struct { char *name; } map_decl;

        /* NODE_MAP_SET: X under KEY gets VALUE */
        struct { char *name; ASTNode *key; ASTNode *value; } map_set;

        /* NODE_MAP_GET: grab X under KEY */
        struct { char *name; ASTNode *key; } map_get;
    };
};

ASTNode *parser_parse(TokenList *tokens);
void ast_free(ASTNode *node);

#endif
