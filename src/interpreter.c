#include "interpreter.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LOOP_ITERATIONS 1000000

/* Function definition storage (global, flat list) */
typedef struct {
    ASTNode *def;
} FuncEntry;

static FuncEntry functions[64];
static int func_count = 0;

static void register_function(ASTNode *def) {
    if (func_count >= 64) error_oom();
    functions[func_count++].def = def;
}

static ASTNode *find_function(const char *name) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(functions[i].def->func_def.name, name) == 0) {
            return functions[i].def;
        }
    }
    return NULL;
}

/* Return value signaling via setjmp/longjmp alternative: simple flag */
typedef struct {
    int returning;
    Value return_value;
} ExecState;

static Value eval(ASTNode *node, Env *env, ExecState *state);
static void exec_stmts(ASTNode **stmts, int count, Env *env, ExecState *state);

static Value eval(ASTNode *node, Env *env, ExecState *state) {
    if (!node) return value_none();
    if (state->returning) return value_none();

    switch (node->type) {
    case NODE_NUMBER_LIT:
        return value_number(node->number_lit.value);

    case NODE_STRING_LIT:
        return value_string(node->string_lit.value);

    case NODE_VAR_REF: {
        Value *v = env_get(env, node->var_ref.name);
        if (!v) error_undeclared(node->var_ref.name, node->line);
        return value_copy(*v);
    }

    case NODE_ARITH: {
        Value left = eval(node->arith.left, env, state);
        Value right = eval(node->arith.right, env, state);

        /* String concatenation with + */
        if (node->arith.op == OP_ADD &&
            (left.type == VAL_STRING || right.type == VAL_STRING)) {
            char *ls = value_to_string(left);
            char *rs = value_to_string(right);
            size_t len = strlen(ls) + strlen(rs) + 1;
            char *cat = malloc(len);
            if (!cat) error_oom();
            strcpy(cat, ls);
            strcat(cat, rs);
            Value result = value_string(cat);
            free(cat);
            free(ls);
            free(rs);
            value_free(&left);
            value_free(&right);
            return result;
        }

        if (left.type != VAL_NUMBER || right.type != VAL_NUMBER) {
            error_type("arithmetic requires numbers, you donkey", node->line);
        }

        double a = left.number, b = right.number;
        value_free(&left);
        value_free(&right);

        switch (node->arith.op) {
        case OP_ADD: return value_number(a + b);
        case OP_SUB: return value_number(a - b);
        case OP_MUL: return value_number(a * b);
        case OP_DIV:
            if (b == 0.0) error_divzero(node->line);
            return value_number(a / b);
        case OP_MOD:
            if (b == 0.0) error_divzero(node->line);
            return value_number(fmod(a, b));
        }
        return value_none();
    }

    case NODE_COMPARE: {
        Value left = eval(node->compare.left, env, state);

        if (node->compare.op == CMP_ZERO) {
            double result = (left.type == VAL_NUMBER && left.number == 0.0) ? 1.0 : 0.0;
            value_free(&left);
            return value_number(result);
        }

        Value right = eval(node->compare.right, env, state);

        if (left.type != VAL_NUMBER || right.type != VAL_NUMBER) {
            error_type("comparison requires numbers", node->line);
        }

        double a = left.number, b = right.number;
        value_free(&left);
        value_free(&right);

        switch (node->compare.op) {
        case CMP_GT:   return value_number(a > b ? 1.0 : 0.0);
        case CMP_LT:   return value_number(a < b ? 1.0 : 0.0);
        case CMP_EQ:   return value_number(a == b ? 1.0 : 0.0);
        case CMP_ZERO: return value_number(0.0); /* handled above */
        }
        return value_none();
    }

    case NODE_FUNC_CALL: {
        ASTNode *def = find_function(node->func_call.name);
        if (!def) error_func_not_found(node->func_call.name, node->line);

        if (node->func_call.arg_count != def->func_def.param_count) {
            error_args(node->func_call.name, def->func_def.param_count,
                       node->func_call.arg_count, node->line);
        }

        Env *call_env = env_new(env);
        for (int i = 0; i < node->func_call.arg_count; i++) {
            Value arg = eval(node->func_call.args[i], env, state);
            env_set(call_env, def->func_def.params[i], arg);
            value_free(&arg);
        }

        ExecState call_state = { .returning = 0, .return_value = value_none() };
        exec_stmts(def->func_def.body, def->func_def.body_count, call_env, &call_state);

        env_free(call_env);

        if (call_state.returning) {
            return call_state.return_value;
        }
        return value_none();
    }

    default:
        break;
    }
    return value_none();
}

static void exec_stmt(ASTNode *node, Env *env, ExecState *state) {
    if (!node || state->returning) return;

    switch (node->type) {
    case NODE_ASSIGN: {
        Value val = eval(node->assign.value, env, state);
        env_set(env, node->assign.name, val);
        value_free(&val);
        break;
    }

    case NODE_COMPOUND_ASSIGN: {
        Value val = eval(node->compound_assign.value, env, state);
        env_set(env, node->compound_assign.name, val);
        value_free(&val);
        break;
    }

    case NODE_PRINT: {
        Value val = eval(node->print.value, env, state);
        value_print(val);
        if (node->print.newline) printf("\n");
        fflush(stdout);
        value_free(&val);
        break;
    }

    case NODE_IF: {
        Value cond = eval(node->if_stmt.condition, env, state);
        int truthy = 0;
        if (cond.type == VAL_NUMBER) truthy = (cond.number != 0.0);
        else if (cond.type == VAL_STRING) truthy = (cond.string[0] != '\0');
        value_free(&cond);

        if (truthy) {
            exec_stmts(node->if_stmt.then_body, node->if_stmt.then_count, env, state);
        } else {
            exec_stmts(node->if_stmt.else_body, node->if_stmt.else_count, env, state);
        }
        break;
    }

    case NODE_LOOP: {
        int iterations = 0;
        while (1) {
            if (iterations++ > MAX_LOOP_ITERATIONS) {
                error_infinite_loop(node->line);
            }

            exec_stmts(node->loop.body, node->loop.body_count, env, state);
            if (state->returning) break;

            /* Check break condition: var hits limit */
            if (node->loop.var && node->loop.limit) {
                Value *var_val = env_get(env, node->loop.var);
                if (!var_val) error_undeclared(node->loop.var, node->line);

                Value limit_val = eval(node->loop.limit, env, state);

                if (var_val->type == VAL_NUMBER && limit_val.type == VAL_NUMBER) {
                    if (var_val->number >= limit_val.number) {
                        value_free(&limit_val);
                        break;
                    }
                }
                value_free(&limit_val);
            }
        }
        break;
    }

    case NODE_FUNC_DEF:
        register_function(node);
        break;

    case NODE_RETURN: {
        Value val = eval(node->ret.value, env, state);
        state->returning = 1;
        state->return_value = val;
        break;
    }

    case NODE_INPUT: {
        char buf[1024];
        if (fgets(buf, sizeof(buf), stdin)) {
            /* Strip trailing newline */
            size_t len = strlen(buf);
            if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';

            /* Try to parse as number */
            char *end;
            double num = strtod(buf, &end);
            if (*end == '\0' && end != buf) {
                Value v = value_number(num);
                env_set(env, node->input.var, v);
            } else {
                Value v = value_string(buf);
                env_set(env, node->input.var, v);
                value_free(&v);
            }
        }
        break;
    }

    case NODE_FUNC_CALL: {
        Value result = eval(node, env, state);
        value_free(&result);
        break;
    }

    default:
        break;
    }
}

static void exec_stmts(ASTNode **stmts, int count, Env *env, ExecState *state) {
    for (int i = 0; i < count && !state->returning; i++) {
        exec_stmt(stmts[i], env, state);
    }
}

void interpreter_run(ASTNode *program) {
    if (!program || program->type != NODE_PROGRAM) return;

    func_count = 0;
    Env *global = env_new(NULL);
    ExecState state = { .returning = 0, .return_value = value_none() };

    exec_stmts(program->program.stmts, program->program.count, global, &state);

    if (state.returning) {
        value_free(&state.return_value);
    }
    env_free(global);
}
