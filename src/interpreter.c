#include "interpreter.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_LOOP_ITERATIONS 1000000
#define MAX_CALL_DEPTH 512

/* Function definition storage (global, flat list) */
typedef struct {
    ASTNode *def;
} FuncEntry;

static FuncEntry functions[64];
static int func_count = 0;
static int random_seeded = 0;

static void register_function(ASTNode *def) {
    if (func_count >= 64) error_too_many_funcs(def->line);
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
    int call_depth;
} ExecState;

static Value eval(ASTNode *node, Env *env, ExecState *state);
static void exec_stmts(ASTNode **stmts, int count, Env *env, ExecState *state);

static int is_truthy(Value v) {
    if (v.type == VAL_NUMBER) return v.number != 0.0;
    if (v.type == VAL_STRING) return v.string[0] != '\0';
    if (v.type == VAL_ARRAY) return v.array.length > 0;
    if (v.type == VAL_MAP) return v.map.length > 0;
    return 0;
}

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
        case OP_POW:
            return value_number(pow(a, b));
        }
        return value_none();
    }

    case NODE_COMPARE: {
        Value left = eval(node->compare.left, env, state);

        if (node->compare.op == CMP_ZERO) {
            double result;
            if (left.type == VAL_NUMBER)
                result = (left.number == 0.0) ? 1.0 : 0.0;
            else if (left.type == VAL_STRING)
                result = (left.string[0] == '\0') ? 1.0 : 0.0;
            else
                result = 1.0;
            value_free(&left);
            return value_number(result);
        }

        Value right = eval(node->compare.right, env, state);

        /* String comparison */
        if (left.type == VAL_STRING && right.type == VAL_STRING) {
            int cmp = strcmp(left.string, right.string);
            value_free(&left);
            value_free(&right);
            switch (node->compare.op) {
            case CMP_GT:   return value_number(cmp > 0 ? 1.0 : 0.0);
            case CMP_LT:   return value_number(cmp < 0 ? 1.0 : 0.0);
            case CMP_EQ:   return value_number(cmp == 0 ? 1.0 : 0.0);
            case CMP_GTE:  return value_number(cmp >= 0 ? 1.0 : 0.0);
            case CMP_ZERO: return value_number(0.0);
            }
            return value_none();
        }

        /* Mixed types -> error */
        if (left.type != right.type) {
            value_free(&left);
            value_free(&right);
            error_type("can't compare strings with numbers, you animal", node->line);
        }

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
        case CMP_GTE:  return value_number(a >= b ? 1.0 : 0.0);
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

        if (state->call_depth >= MAX_CALL_DEPTH) {
            error_recursion(node->line);
        }

        Env *call_env = env_new(env);
        for (int i = 0; i < node->func_call.arg_count; i++) {
            Value arg = eval(node->func_call.args[i], env, state);
            env_set(call_env, def->func_def.params[i], arg);
            value_free(&arg);
        }

        ExecState call_state = {
            .returning = 0,
            .return_value = value_none(),
            .call_depth = state->call_depth + 1
        };
        exec_stmts(def->func_def.body, def->func_def.body_count, call_env, &call_state);

        env_free(call_env);

        if (call_state.returning) {
            return call_state.return_value;
        }
        return value_none();
    }

    case NODE_LOGIC: {
        if (node->logic.op == LOGIC_NOT) {
            Value operand = eval(node->logic.left, env, state);
            int result = !is_truthy(operand);
            value_free(&operand);
            return value_number(result ? 1.0 : 0.0);
        }
        if (node->logic.op == LOGIC_AND) {
            Value left = eval(node->logic.left, env, state);
            if (!is_truthy(left)) {
                value_free(&left);
                return value_number(0.0);
            }
            value_free(&left);
            Value right = eval(node->logic.right, env, state);
            int result = is_truthy(right);
            value_free(&right);
            return value_number(result ? 1.0 : 0.0);
        }
        if (node->logic.op == LOGIC_OR) {
            Value left = eval(node->logic.left, env, state);
            if (is_truthy(left)) {
                value_free(&left);
                return value_number(1.0);
            }
            value_free(&left);
            Value right = eval(node->logic.right, env, state);
            int result = is_truthy(right);
            value_free(&right);
            return value_number(result ? 1.0 : 0.0);
        }
        return value_none();
    }

    case NODE_RANDOM: {
        if (!random_seeded) {
            srand((unsigned int)time(NULL));
            random_seeded = 1;
        }
        Value max_val = eval(node->random_expr.max, env, state);
        if (max_val.type != VAL_NUMBER || max_val.number < 1.0) {
            value_free(&max_val);
            error_type("roll the dice needs a positive number, you freak", node->line);
        }
        int max = (int)max_val.number;
        value_free(&max_val);
        return value_number((double)(rand() % max + 1));
    }

    case NODE_ARRAY_GET: {
        Value *arr = env_get(env, node->array_get.name);
        if (!arr) error_undeclared(node->array_get.name, node->line);
        Value idx_val = eval(node->array_get.index, env, state);
        if (idx_val.type != VAL_NUMBER) {
            value_free(&idx_val);
            error_type("index must be a number", node->line);
        }
        int idx = (int)idx_val.number;
        value_free(&idx_val);
        if (arr->type == VAL_STRING) {
            int len = (int)strlen(arr->string);
            if (idx < 0 || idx >= len) {
                error_index_out_of_bounds(idx, len, node->line);
            }
            char ch[2] = { arr->string[idx], '\0' };
            return value_string(ch);
        }
        if (arr->type != VAL_ARRAY) {
            error_type("that's not an array or string, you pervert", node->line);
        }
        if (idx < 0 || idx >= arr->array.length) {
            error_index_out_of_bounds(idx, arr->array.length, node->line);
        }
        return value_copy(arr->array.items[idx]);
    }

    case NODE_ARRAY_LENGTH: {
        Value *arr = env_get(env, node->array_length.name);
        if (!arr) error_undeclared(node->array_length.name, node->line);
        if (arr->type == VAL_ARRAY) {
            return value_number((double)arr->array.length);
        }
        if (arr->type == VAL_STRING) {
            return value_number((double)strlen(arr->string));
        }
        if (arr->type == VAL_MAP) {
            return value_number((double)arr->map.length);
        }
        error_type("that's not an array, string, or map, you pervert", node->line);
    }

    case NODE_ARRAY_POP: {
        Value *arr = env_get(env, node->array_pop.array_name);
        if (!arr) error_undeclared(node->array_pop.array_name, node->line);
        if (arr->type != VAL_ARRAY) {
            error_type("can't yank from something that's not an array", node->line);
        }
        if (arr->array.length <= 0) {
            error_type("can't yank from an empty array, nothing to grab", node->line);
        }
        arr->array.length--;
        Value result = arr->array.items[arr->array.length];
        arr->array.items[arr->array.length] = value_none();
        return result;
    }

    case NODE_STRING_SPLIT: {
        Value str_val = eval(node->string_split.str, env, state);
        Value delim_val = eval(node->string_split.delimiter, env, state);
        if (str_val.type != VAL_STRING || delim_val.type != VAL_STRING) {
            value_free(&str_val);
            value_free(&delim_val);
            error_type("chop requires strings", node->line);
        }
        const char *s = str_val.string;
        const char *d = delim_val.string;
        size_t dlen = strlen(d);

        /* Count parts and build result array */
        int cap = 8;
        Value result;
        result.type = VAL_ARRAY;
        result.array.length = 0;
        result.array.capacity = cap;
        result.array.items = malloc(sizeof(Value) * (size_t)cap);
        if (!result.array.items) error_oom();

        if (dlen == 0) {
            /* Split into individual characters */
            int slen = (int)strlen(s);
            for (int i = 0; i < slen; i++) {
                if (result.array.length >= result.array.capacity) {
                    result.array.capacity *= 2;
                    result.array.items = realloc(result.array.items,
                        sizeof(Value) * (size_t)result.array.capacity);
                    if (!result.array.items) error_oom();
                }
                char ch[2] = { s[i], '\0' };
                result.array.items[result.array.length++] = value_string(ch);
            }
        } else {
            const char *p = s;
            while (1) {
                const char *found = strstr(p, d);
                if (result.array.length >= result.array.capacity) {
                    result.array.capacity *= 2;
                    result.array.items = realloc(result.array.items,
                        sizeof(Value) * (size_t)result.array.capacity);
                    if (!result.array.items) error_oom();
                }
                if (found) {
                    size_t partlen = (size_t)(found - p);
                    char *part = malloc(partlen + 1);
                    if (!part) error_oom();
                    memcpy(part, p, partlen);
                    part[partlen] = '\0';
                    result.array.items[result.array.length++] = value_string(part);
                    free(part);
                    p = found + dlen;
                } else {
                    result.array.items[result.array.length++] = value_string(p);
                    break;
                }
            }
        }
        value_free(&str_val);
        value_free(&delim_val);
        return result;
    }

    case NODE_STRING_JOIN: {
        Value arr_val = eval(node->string_join.array, env, state);
        Value sep_val = eval(node->string_join.separator, env, state);
        if (arr_val.type != VAL_ARRAY) {
            value_free(&arr_val);
            value_free(&sep_val);
            error_type("stitch requires an array", node->line);
        }
        if (sep_val.type != VAL_STRING) {
            value_free(&arr_val);
            value_free(&sep_val);
            error_type("stitch separator must be a string", node->line);
        }
        /* Calculate total length */
        char **parts = malloc(sizeof(char *) * (size_t)arr_val.array.length);
        if (arr_val.array.length > 0 && !parts) error_oom();
        size_t total = 0;
        size_t seplen = strlen(sep_val.string);
        for (int i = 0; i < arr_val.array.length; i++) {
            parts[i] = value_to_string(arr_val.array.items[i]);
            total += strlen(parts[i]);
            if (i > 0) total += seplen;
        }
        char *result = malloc(total + 1);
        if (!result) error_oom();
        char *wp = result;
        for (int i = 0; i < arr_val.array.length; i++) {
            if (i > 0) {
                memcpy(wp, sep_val.string, seplen);
                wp += seplen;
            }
            size_t plen = strlen(parts[i]);
            memcpy(wp, parts[i], plen);
            wp += plen;
            free(parts[i]);
        }
        *wp = '\0';
        free(parts);
        Value ret = value_string(result);
        free(result);
        value_free(&arr_val);
        value_free(&sep_val);
        return ret;
    }

    case NODE_MATH: {
        Value operand = eval(node->math_expr.operand, env, state);
        if (operand.type != VAL_NUMBER) {
            value_free(&operand);
            error_type("math functions require numbers", node->line);
        }
        double x = operand.number;
        value_free(&operand);
        switch (node->math_expr.op) {
        case MATH_SQRT:  return value_number(sqrt(x));
        case MATH_ABS:   return value_number(fabs(x));
        case MATH_FLOOR: return value_number(floor(x));
        case MATH_CEIL:  return value_number(ceil(x));
        }
        return value_none();
    }

    case NODE_CAST: {
        Value operand = eval(node->cast_expr.operand, env, state);
        if (node->cast_expr.op == CAST_TO_NUM) {
            if (operand.type == VAL_NUMBER) return operand;
            if (operand.type == VAL_STRING) {
                char *end;
                double num = strtod(operand.string, &end);
                value_free(&operand);
                if (*end != '\0') {
                    error_type("can't convert that string to a number", node->line);
                }
                return value_number(num);
            }
            value_free(&operand);
            error_type("can't convert that to a number", node->line);
        }
        if (node->cast_expr.op == CAST_TO_STR) {
            char *s = value_to_string(operand);
            value_free(&operand);
            Value ret = value_string(s);
            free(s);
            return ret;
        }
        value_free(&operand);
        return value_none();
    }

    case NODE_MAP_GET: {
        Value *map = env_get(env, node->map_get.name);
        if (!map) error_undeclared(node->map_get.name, node->line);
        if (map->type != VAL_MAP) {
            error_type("that's not a map, you pervert", node->line);
        }
        Value key_val = eval(node->map_get.key, env, state);
        char *key_str = value_to_string(key_val);
        value_free(&key_val);
        for (int i = 0; i < map->map.length; i++) {
            if (strcmp(map->map.keys[i], key_str) == 0) {
                free(key_str);
                return value_copy(map->map.values[i]);
            }
        }
        free(key_str);
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
        int truthy = is_truthy(cond);
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

            /* Check break condition */
            if (node->loop.condition) {
                Value cond = eval(node->loop.condition, env, state);
                int truthy = is_truthy(cond);
                value_free(&cond);
                if (truthy) break;
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
        char *line = NULL;
        size_t cap = 0;
        ssize_t nread = getline(&line, &cap, stdin);
        if (nread > 0) {
            /* Strip trailing newline */
            if (nread > 0 && line[nread - 1] == '\n') line[nread - 1] = '\0';

            /* Try to parse as number */
            char *end;
            double num = strtod(line, &end);
            if (*end == '\0' && end != line) {
                Value v = value_number(num);
                env_set(env, node->input.var, v);
            } else {
                Value v = value_string(line);
                env_set(env, node->input.var, v);
                value_free(&v);
            }
        }
        free(line);
        break;
    }

    case NODE_FUNC_CALL: {
        Value result = eval(node, env, state);
        value_free(&result);
        break;
    }

    case NODE_ARRAY_DECL: {
        Value size_val = eval(node->array_decl.size, env, state);
        if (size_val.type != VAL_NUMBER || size_val.number < 0.0) {
            value_free(&size_val);
            error_type("array size must be a positive number", node->line);
        }
        int size = (int)size_val.number;
        value_free(&size_val);
        Value arr = value_array(size);
        env_set(env, node->array_decl.name, arr);
        value_free(&arr);
        break;
    }

    case NODE_ARRAY_SET: {
        Value *arr = env_get(env, node->array_set.name);
        if (!arr) error_undeclared(node->array_set.name, node->line);
        if (arr->type != VAL_ARRAY) {
            error_type("that's not an array, you pervert", node->line);
        }
        Value idx_val = eval(node->array_set.index, env, state);
        if (idx_val.type != VAL_NUMBER) {
            value_free(&idx_val);
            error_type("array index must be a number", node->line);
        }
        int idx = (int)idx_val.number;
        value_free(&idx_val);
        if (idx < 0 || idx >= arr->array.length) {
            error_index_out_of_bounds(idx, arr->array.length, node->line);
        }
        Value val = eval(node->array_set.value, env, state);
        value_free(&arr->array.items[idx]);
        arr->array.items[idx] = value_copy(val);
        value_free(&val);
        break;
    }

    case NODE_ARRAY_PUSH: {
        Value *arr = env_get(env, node->array_push.array_name);
        if (!arr) error_undeclared(node->array_push.array_name, node->line);
        if (arr->type != VAL_ARRAY) {
            error_type("can't shove into something that's not an array", node->line);
        }
        if (arr->array.length >= arr->array.capacity) {
            int new_cap = arr->array.capacity == 0 ? 4 : arr->array.capacity * 2;
            arr->array.items = realloc(arr->array.items,
                sizeof(Value) * (size_t)new_cap);
            if (!arr->array.items) error_oom();
            arr->array.capacity = new_cap;
        }
        Value val = eval(node->array_push.value, env, state);
        arr->array.items[arr->array.length] = value_copy(val);
        arr->array.length++;
        value_free(&val);
        break;
    }

    case NODE_FOREACH: {
        Value iterable = eval(node->foreach.iterable, env, state);
        if (iterable.type == VAL_ARRAY) {
            for (int i = 0; i < iterable.array.length; i++) {
                if (state->returning) break;
                Value each_val = value_copy(iterable.array.items[i]);
                env_set(env, "each", each_val);
                value_free(&each_val);
                Value tally_val = value_number((double)i);
                env_set(env, "tally", tally_val);
                exec_stmts(node->foreach.body, node->foreach.body_count, env, state);
            }
        } else if (iterable.type == VAL_STRING) {
            int len = (int)strlen(iterable.string);
            for (int i = 0; i < len; i++) {
                if (state->returning) break;
                char ch[2] = { iterable.string[i], '\0' };
                Value each_val = value_string(ch);
                env_set(env, "each", each_val);
                value_free(&each_val);
                Value tally_val = value_number((double)i);
                env_set(env, "tally", tally_val);
                exec_stmts(node->foreach.body, node->foreach.body_count, env, state);
            }
        } else {
            value_free(&iterable);
            error_type("can only run a train on arrays or strings", node->line);
        }
        value_free(&iterable);
        break;
    }

    case NODE_MAP_DECL: {
        Value m = value_map();
        env_set(env, node->map_decl.name, m);
        value_free(&m);
        break;
    }

    case NODE_MAP_SET: {
        Value *map = env_get(env, node->map_set.name);
        if (!map) error_undeclared(node->map_set.name, node->line);
        if (map->type != VAL_MAP) {
            error_type("that's not a map, you pervert", node->line);
        }
        Value key_val = eval(node->map_set.key, env, state);
        char *key_str = value_to_string(key_val);
        value_free(&key_val);
        Value val = eval(node->map_set.value, env, state);

        /* Search for existing key */
        for (int i = 0; i < map->map.length; i++) {
            if (strcmp(map->map.keys[i], key_str) == 0) {
                value_free(&map->map.values[i]);
                map->map.values[i] = value_copy(val);
                free(key_str);
                value_free(&val);
                goto map_set_done;
            }
        }
        /* Append new entry */
        if (map->map.length >= map->map.capacity) {
            int new_cap = map->map.capacity == 0 ? 4 : map->map.capacity * 2;
            map->map.keys = realloc(map->map.keys, sizeof(char *) * (size_t)new_cap);
            map->map.values = realloc(map->map.values, sizeof(Value) * (size_t)new_cap);
            if (!map->map.keys || !map->map.values) error_oom();
            map->map.capacity = new_cap;
        }
        map->map.keys[map->map.length] = key_str;
        map->map.values[map->map.length] = value_copy(val);
        map->map.length++;
        value_free(&val);
        map_set_done:
        break;
    }

    case NODE_FILE_READ: {
        Value path_val = eval(node->file_read.path, env, state);
        if (path_val.type != VAL_STRING) {
            value_free(&path_val);
            error_type("file path must be a string", node->line);
        }
        FILE *f = fopen(path_val.string, "r");
        if (!f) {
            error_file_read(path_val.string, node->line);
        }
        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (len < 0) { fclose(f); error_file_read(path_val.string, node->line); }
        char *buf = malloc((size_t)len + 1);
        if (!buf) { fclose(f); error_oom(); }
        size_t nread = fread(buf, 1, (size_t)len, f);
        buf[nread] = '\0';
        fclose(f);
        Value content = value_string(buf);
        free(buf);
        env_set(env, node->file_read.var, content);
        value_free(&content);
        value_free(&path_val);
        break;
    }

    case NODE_FILE_WRITE: {
        Value content_val = eval(node->file_write.content, env, state);
        Value path_val = eval(node->file_write.path, env, state);
        if (path_val.type != VAL_STRING) {
            value_free(&content_val);
            value_free(&path_val);
            error_type("file path must be a string", node->line);
        }
        char *content_str = value_to_string(content_val);
        FILE *f = fopen(path_val.string, "w");
        if (!f) {
            free(content_str);
            error_file_write(path_val.string, node->line);
        }
        fwrite(content_str, 1, strlen(content_str), f);
        fclose(f);
        free(content_str);
        value_free(&content_val);
        value_free(&path_val);
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
    random_seeded = 0;
    memset(functions, 0, sizeof(functions));
    Env *global = env_new(NULL);
    ExecState state = { .returning = 0, .return_value = value_none(), .call_depth = 0 };

    exec_stmts(program->program.stmts, program->program.count, global, &state);

    if (state.returning) {
        value_free(&state.return_value);
    }
    env_free(global);
}
