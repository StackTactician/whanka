#include "env.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int hash(const char *key) {
    unsigned int h = 5381;
    while (*key) {
        h = ((h << 5) + h) ^ (unsigned char)*key++;
    }
    return h;
}

Env *env_new(Env *parent) {
    Env *env = calloc(1, sizeof(Env));
    if (!env) error_oom();
    env->parent = parent;
    return env;
}

void env_free(Env *env) {
    if (!env) return;
    for (int i = 0; i < ENV_SLOTS; i++) {
        if (env->slots[i].key) {
            free(env->slots[i].key);
            value_free(&env->slots[i].value);
        }
    }
    free(env);
}

void env_set(Env *env, const char *key, Value val) {
    /* Walk up scope chain to find existing binding */
    for (Env *e = env; e; e = e->parent) {
        unsigned int idx = hash(key) % ENV_SLOTS;
        for (int i = 0; i < ENV_SLOTS; i++) {
            unsigned int slot = (idx + i) % ENV_SLOTS;
            if (!e->slots[slot].key) break;
            if (strcmp(e->slots[slot].key, key) == 0) {
                value_free(&e->slots[slot].value);
                e->slots[slot].value = value_copy(val);
                return;
            }
        }
    }

    /* New binding in current scope */
    unsigned int idx = hash(key) % ENV_SLOTS;
    for (int i = 0; i < ENV_SLOTS; i++) {
        unsigned int slot = (idx + i) % ENV_SLOTS;
        if (!env->slots[slot].key) {
            env->slots[slot].key = strdup(key);
            if (!env->slots[slot].key) error_oom();
            env->slots[slot].value = value_copy(val);
            return;
        }
    }
    error_too_many_vars(0); /* table full */
}

Value *env_get(Env *env, const char *key) {
    for (Env *e = env; e; e = e->parent) {
        unsigned int idx = hash(key) % ENV_SLOTS;
        for (int i = 0; i < ENV_SLOTS; i++) {
            unsigned int slot = (idx + i) % ENV_SLOTS;
            if (!e->slots[slot].key) break;
            if (strcmp(e->slots[slot].key, key) == 0) {
                return &e->slots[slot].value;
            }
        }
    }
    return NULL;
}
