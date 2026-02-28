#include "parser.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- Parser state --- */

typedef struct {
    Token *tokens;
    int count;
    int pos;
} Parser;

static Token *peek(Parser *p) {
    if (p->pos >= p->count) error_eof();
    return &p->tokens[p->pos];
}

static Token *advance(Parser *p) {
    Token *t = peek(p);
    p->pos++;
    return t;
}

static Token *expect(Parser *p, TokenType type) {
    Token *t = peek(p);
    if (t->type != type) {
        char msg[128];
        snprintf(msg, sizeof(msg), "expected %s, got %s (\"%s\")",
                 token_type_name(type), token_type_name(t->type),
                 t->text ? t->text : "");
        error_syntax(msg, t->line);
    }
    return advance(p);
}

static int at(Parser *p, TokenType type) {
    return p->pos < p->count && p->tokens[p->pos].type == type;
}

static void skip_flavor(Parser *p) {
    while (p->pos < p->count) {
        TokenType t = p->tokens[p->pos].type;
        if (t == TOK_OH_GOD || t == TOK_HARDER || t == TOK_NO_QUESTIONS ||
            t == TOK_CRAP || t == TOK_OH_GOD_YES || t == TOK_PERIOD ||
            t == TOK_COMMA) {
            p->pos++;
        } else {
            break;
        }
    }
}

/* --- AST node constructors --- */

static ASTNode *node_new(NodeType type, int line) {
    ASTNode *n = calloc(1, sizeof(ASTNode));
    if (!n) error_oom();
    n->type = type;
    n->line = line;
    return n;
}

/* --- Expression parsing --- */

static ASTNode *parse_expr(Parser *p);

static ASTNode *parse_primary(Parser *p) {
    skip_flavor(p);
    Token *t = peek(p);

    if (t->type == TOK_NUMBER) {
        advance(p);
        ASTNode *n = node_new(NODE_NUMBER_LIT, t->line);
        n->number_lit.value = strtod(t->text, NULL);
        return n;
    }

    if (t->type == TOK_STRING) {
        advance(p);
        ASTNode *n = node_new(NODE_STRING_LIT, t->line);
        n->string_lit.value = strdup(t->text);
        if (!n->string_lit.value) error_oom();
        return n;
    }

    if (t->type == TOK_IDENT) {
        advance(p);
        ASTNode *n = node_new(NODE_VAR_REF, t->line);
        n->var_ref.name = strdup(t->text);
        if (!n->var_ref.name) error_oom();
        return n;
    }

    /* Function call as expression: get freaky with NAME using ARGS */
    if (t->type == TOK_GET_FREAKY) {
        return parse_expr(p); /* delegate to statement parser — handled there */
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "unexpected token %s", token_type_name(t->type));
    error_syntax(msg, t->line);
}

static ASTNode *parse_expr(Parser *p) {
    skip_flavor(p);

    /* Function call expression: get freaky with NAME using ARG, ARG */
    if (at(p, TOK_GET_FREAKY)) {
        Token *start = advance(p);
        Token *name = expect(p, TOK_IDENT);
        ASTNode *n = node_new(NODE_FUNC_CALL, start->line);
        n->func_call.name = strdup(name->text);
        if (!n->func_call.name) error_oom();
        n->func_call.args = NULL;
        n->func_call.arg_count = 0;

        if (at(p, TOK_USING)) {
            advance(p);
            int cap = 4;
            n->func_call.args = malloc(sizeof(ASTNode *) * (size_t)cap);
            if (!n->func_call.args) error_oom();

            n->func_call.args[n->func_call.arg_count++] = parse_primary(p);
            while (at(p, TOK_COMMA)) {
                advance(p);
                skip_flavor(p);
                if (n->func_call.arg_count >= cap) {
                    cap *= 2;
                    n->func_call.args = realloc(n->func_call.args,
                                                sizeof(ASTNode *) * (size_t)cap);
                    if (!n->func_call.args) error_oom();
                }
                n->func_call.args[n->func_call.arg_count++] = parse_primary(p);
            }
        }
        skip_flavor(p);
        return n;
    }

    ASTNode *left = parse_primary(p);
    skip_flavor(p);

    /* Arithmetic: left OP right */
    if (at(p, TOK_JACKED_UP_BY) || at(p, TOK_CUT_DOWN_BY) ||
        at(p, TOK_BLOWN_UP_BY) || at(p, TOK_SPLIT_WITH) ||
        at(p, TOK_LEFTOVER_FROM)) {
        Token *op_tok = advance(p);
        skip_flavor(p);
        ASTNode *right = parse_primary(p);

        ASTNode *n = node_new(NODE_ARITH, op_tok->line);
        n->arith.left = left;
        n->arith.right = right;

        switch (op_tok->type) {
        case TOK_JACKED_UP_BY:  n->arith.op = OP_ADD; break;
        case TOK_CUT_DOWN_BY:   n->arith.op = OP_SUB; break;
        case TOK_BLOWN_UP_BY:   n->arith.op = OP_MUL; break;
        case TOK_SPLIT_WITH:    n->arith.op = OP_DIV; break;
        case TOK_LEFTOVER_FROM: n->arith.op = OP_MOD; break;
        default: break;
        }
        skip_flavor(p);
        return n;
    }

    return left;
}

/* Comparison: expr CMP_OP expr (or expr IS_FRIGID) */
static ASTNode *parse_comparison(Parser *p) {
    ASTNode *left = parse_expr(p);
    skip_flavor(p);

    if (at(p, TOK_HOTTER_THAN) || at(p, TOK_COLDER_THAN) ||
        at(p, TOK_AS_HOT_AS) || at(p, TOK_IS_FRIGID)) {
        Token *op_tok = advance(p);
        skip_flavor(p);

        ASTNode *n = node_new(NODE_COMPARE, op_tok->line);
        n->compare.left = left;

        switch (op_tok->type) {
        case TOK_HOTTER_THAN: n->compare.op = CMP_GT; break;
        case TOK_COLDER_THAN: n->compare.op = CMP_LT; break;
        case TOK_AS_HOT_AS:   n->compare.op = CMP_EQ; break;
        case TOK_IS_FRIGID:   n->compare.op = CMP_ZERO; break;
        default: break;
        }

        if (op_tok->type != TOK_IS_FRIGID) {
            n->compare.right = parse_expr(p);
        } else {
            n->compare.right = NULL;
        }
        skip_flavor(p);
        return n;
    }

    return left;
}

/* --- Statement parsing --- */

static ASTNode *parse_statement(Parser *p);

/* Collect statements until a terminator token appears */
static void parse_body(Parser *p, ASTNode ***body, int *count, TokenType term1, TokenType term2) {
    int cap = 8;
    *body = malloc(sizeof(ASTNode *) * (size_t)cap);
    if (!*body) error_oom();
    *count = 0;

    while (!at(p, term1) && !at(p, term2) && !at(p, TOK_EOF)) {
        skip_flavor(p);
        if (at(p, term1) || at(p, term2) || at(p, TOK_EOF)) break;

        ASTNode *stmt = parse_statement(p);
        if (!stmt) continue;

        if (*count >= cap) {
            cap *= 2;
            *body = realloc(*body, sizeof(ASTNode *) * (size_t)cap);
            if (!*body) error_oom();
        }
        (*body)[(*count)++] = stmt;
    }
}

static ASTNode *parse_statement(Parser *p) {
    skip_flavor(p);
    if (at(p, TOK_EOF)) return NULL;

    Token *t = peek(p);

    /* moan X is EXPR */
    if (t->type == TOK_MOAN) {
        advance(p);
        Token *name = expect(p, TOK_IDENT);
        expect(p, TOK_IS);
        skip_flavor(p);
        ASTNode *val = parse_expr(p);
        skip_flavor(p);

        ASTNode *n = node_new(NODE_ASSIGN, t->line);
        n->assign.name = strdup(name->text);
        if (!n->assign.name) error_oom();
        n->assign.value = val;
        return n;
    }

    /* stash X with EXPR */
    if (t->type == TOK_STASH) {
        advance(p);
        Token *name = expect(p, TOK_IDENT);
        expect(p, TOK_WITH);
        skip_flavor(p);
        ASTNode *val = parse_expr(p);
        skip_flavor(p);

        ASTNode *n = node_new(NODE_ASSIGN, t->line);
        n->assign.name = strdup(name->text);
        if (!n->assign.name) error_oom();
        n->assign.value = val;
        return n;
    }

    /* X gets EXPR, harder. (compound assignment / reassignment) */
    if (t->type == TOK_IDENT) {
        /* Look ahead for 'gets' */
        if (p->pos + 1 < p->count && p->tokens[p->pos + 1].type == TOK_GETS) {
            Token *name = advance(p);
            advance(p); /* skip 'gets' */
            skip_flavor(p);
            ASTNode *val = parse_expr(p);
            skip_flavor(p);

            ASTNode *n = node_new(NODE_COMPOUND_ASSIGN, t->line);
            n->compound_assign.name = strdup(name->text);
            if (!n->compound_assign.name) error_oom();
            n->compound_assign.value = val;
            return n;
        }
    }

    /* yell "string" / yell EXPR */
    if (t->type == TOK_YELL) {
        advance(p);
        skip_flavor(p);
        ASTNode *val = parse_expr(p);
        skip_flavor(p);

        ASTNode *n = node_new(NODE_PRINT, t->line);
        n->print.value = val;
        n->print.newline = 1;
        return n;
    }

    /* scream EXPR */
    if (t->type == TOK_SCREAM) {
        advance(p);
        skip_flavor(p);
        ASTNode *val = parse_expr(p);
        skip_flavor(p);

        ASTNode *n = node_new(NODE_PRINT, t->line);
        n->print.value = val;
        n->print.newline = 1;
        return n;
    }

    /* if X consents to being COMPARISON ... hard limit: ... boundaries respected. */
    if (t->type == TOK_IF) {
        advance(p);
        skip_flavor(p);
        ASTNode *cond = parse_comparison(p);
        skip_flavor(p);

        /* optional "consents to being" already consumed as part of comparison? No.
           The syntax is: if EXPR consents to being CMPOP EXPR
           But we parsed the comparison above. The "consents to being" is flavor after the condition expr. */
        if (at(p, TOK_CONSENTS)) advance(p);
        skip_flavor(p);

        /* Re-parse: the condition is actually parsed differently.
           "if X consents to being hotter than Y" means:
           condition = X hotter than Y
           But we might have parsed X as the full comparison already. Let's check. */

        ASTNode *n = node_new(NODE_IF, t->line);
        n->if_stmt.condition = cond;

        /* Parse then-body until hard limit or boundaries */
        parse_body(p, &n->if_stmt.then_body, &n->if_stmt.then_count,
                   TOK_HARD_LIMIT, TOK_BOUNDARIES);

        n->if_stmt.else_body = NULL;
        n->if_stmt.else_count = 0;

        /* Optional else: hard limit: */
        if (at(p, TOK_HARD_LIMIT)) {
            advance(p);
            skip_flavor(p);
            parse_body(p, &n->if_stmt.else_body, &n->if_stmt.else_count,
                       TOK_BOUNDARIES, TOK_BOUNDARIES);
        }

        /* boundaries respected. */
        if (at(p, TOK_BOUNDARIES)) {
            advance(p);
        }
        skip_flavor(p);
        return n;
    }

    /* go on a spree: ... busted when X hits N */
    if (t->type == TOK_SPREE_START) {
        advance(p);
        skip_flavor(p);

        ASTNode *n = node_new(NODE_LOOP, t->line);
        n->loop.var = NULL;
        n->loop.limit = NULL;

        parse_body(p, &n->loop.body, &n->loop.body_count,
                   TOK_BUSTED_WHEN, TOK_BUSTED_WHEN);

        /* busted when X hits N */
        if (at(p, TOK_BUSTED_WHEN)) {
            advance(p);
            skip_flavor(p);
            Token *var = expect(p, TOK_IDENT);
            n->loop.var = strdup(var->text);
            if (!n->loop.var) error_oom();
            expect(p, TOK_HITS);
            skip_flavor(p);
            n->loop.limit = parse_expr(p);
        }
        skip_flavor(p);
        return n;
    }

    /* my kink is NAME and it takes PARAM, PARAM ... safe word. */
    if (t->type == TOK_MY_KINK_IS) {
        advance(p);
        Token *name = expect(p, TOK_IDENT);

        ASTNode *n = node_new(NODE_FUNC_DEF, t->line);
        n->func_def.name = strdup(name->text);
        if (!n->func_def.name) error_oom();
        n->func_def.params = NULL;
        n->func_def.param_count = 0;

        /* Optional params: and it takes X, Y */
        if (at(p, TOK_AND_IT_TAKES)) {
            advance(p);
            int cap = 4;
            n->func_def.params = malloc(sizeof(char *) * (size_t)cap);
            if (!n->func_def.params) error_oom();

            Token *param = expect(p, TOK_IDENT);
            n->func_def.params[n->func_def.param_count++] = strdup(param->text);

            while (at(p, TOK_COMMA)) {
                advance(p);
                skip_flavor(p);
                if (n->func_def.param_count >= cap) {
                    cap *= 2;
                    n->func_def.params = realloc(n->func_def.params,
                                                 sizeof(char *) * (size_t)cap);
                    if (!n->func_def.params) error_oom();
                }
                param = expect(p, TOK_IDENT);
                n->func_def.params[n->func_def.param_count++] = strdup(param->text);
            }
        }
        skip_flavor(p);

        /* Body until safe word. */
        parse_body(p, &n->func_def.body, &n->func_def.body_count,
                   TOK_SAFE_WORD, TOK_SAFE_WORD);

        if (at(p, TOK_SAFE_WORD)) advance(p);
        skip_flavor(p);
        return n;
    }

    /* finish with EXPR */
    if (t->type == TOK_FINISH_WITH) {
        advance(p);
        skip_flavor(p);
        ASTNode *val = parse_expr(p);
        skip_flavor(p);

        ASTNode *n = node_new(NODE_RETURN, t->line);
        n->ret.value = val;
        return n;
    }

    /* get freaky with NAME using ARGS (statement form) */
    if (t->type == TOK_GET_FREAKY) {
        ASTNode *call = parse_expr(p);
        skip_flavor(p);
        return call;
    }

    /* seduce the answer out of X */
    if (t->type == TOK_SEDUCE) {
        advance(p);
        skip_flavor(p);
        Token *var = expect(p, TOK_IDENT);

        ASTNode *n = node_new(NODE_INPUT, t->line);
        n->input.var = strdup(var->text);
        if (!n->input.var) error_oom();
        skip_flavor(p);
        return n;
    }

    /* Skip stray tokens we don't recognize as statement starts */
    advance(p);
    return NULL;
}

/* --- Top-level parse --- */

ASTNode *parser_parse(TokenList *tokens) {
    Parser p = { .tokens = tokens->tokens, .count = tokens->count, .pos = 0 };

    skip_flavor(&p);

    /* Expect program start */
    expect(&p, TOK_PROGRAM_START);
    skip_flavor(&p);

    ASTNode *program = node_new(NODE_PROGRAM, 1);
    int cap = 16;
    program->program.stmts = malloc(sizeof(ASTNode *) * (size_t)cap);
    if (!program->program.stmts) error_oom();
    program->program.count = 0;

    while (!at(&p, TOK_PROGRAM_END) && !at(&p, TOK_EOF)) {
        skip_flavor(&p);
        if (at(&p, TOK_PROGRAM_END) || at(&p, TOK_EOF)) break;

        /* Skip regards. if present */
        if (at(&p, TOK_REGARDS)) {
            advance(&p);
            skip_flavor(&p);
            continue;
        }

        ASTNode *stmt = parse_statement(&p);
        if (!stmt) continue;

        if (program->program.count >= cap) {
            cap *= 2;
            program->program.stmts = realloc(program->program.stmts,
                                             sizeof(ASTNode *) * (size_t)cap);
            if (!program->program.stmts) error_oom();
        }
        program->program.stmts[program->program.count++] = stmt;
    }

    /* Expect program end */
    if (at(&p, TOK_PROGRAM_END)) {
        advance(&p);
    }

    /* Optional regards. at end */
    skip_flavor(&p);
    if (at(&p, TOK_REGARDS)) advance(&p);

    return program;
}

/* --- AST cleanup --- */

void ast_free(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
    case NODE_PROGRAM:
        for (int i = 0; i < node->program.count; i++)
            ast_free(node->program.stmts[i]);
        free(node->program.stmts);
        break;
    case NODE_ASSIGN:
        free(node->assign.name);
        ast_free(node->assign.value);
        break;
    case NODE_COMPOUND_ASSIGN:
        free(node->compound_assign.name);
        ast_free(node->compound_assign.value);
        break;
    case NODE_ARITH:
        ast_free(node->arith.left);
        ast_free(node->arith.right);
        break;
    case NODE_COMPARE:
        ast_free(node->compare.left);
        ast_free(node->compare.right);
        break;
    case NODE_IF:
        ast_free(node->if_stmt.condition);
        for (int i = 0; i < node->if_stmt.then_count; i++)
            ast_free(node->if_stmt.then_body[i]);
        free(node->if_stmt.then_body);
        for (int i = 0; i < node->if_stmt.else_count; i++)
            ast_free(node->if_stmt.else_body[i]);
        free(node->if_stmt.else_body);
        break;
    case NODE_LOOP:
        free(node->loop.var);
        ast_free(node->loop.limit);
        for (int i = 0; i < node->loop.body_count; i++)
            ast_free(node->loop.body[i]);
        free(node->loop.body);
        break;
    case NODE_FUNC_DEF:
        free(node->func_def.name);
        for (int i = 0; i < node->func_def.param_count; i++)
            free(node->func_def.params[i]);
        free(node->func_def.params);
        for (int i = 0; i < node->func_def.body_count; i++)
            ast_free(node->func_def.body[i]);
        free(node->func_def.body);
        break;
    case NODE_FUNC_CALL:
        free(node->func_call.name);
        for (int i = 0; i < node->func_call.arg_count; i++)
            ast_free(node->func_call.args[i]);
        free(node->func_call.args);
        break;
    case NODE_RETURN:
        ast_free(node->ret.value);
        break;
    case NODE_PRINT:
        ast_free(node->print.value);
        break;
    case NODE_INPUT:
        free(node->input.var);
        break;
    case NODE_VAR_REF:
        free(node->var_ref.name);
        break;
    case NODE_NUMBER_LIT:
        break;
    case NODE_STRING_LIT:
        free(node->string_lit.value);
        break;
    }
    free(node);
}
