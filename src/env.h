#ifndef PARDON_ENV_H
#define PARDON_ENV_H

#include "value.h"

#define ENV_SLOTS 64

typedef struct EnvEntry {
    char *key;
    Value value;
} EnvEntry;

typedef struct Env {
    EnvEntry slots[ENV_SLOTS];
    struct Env *parent;
} Env;

Env *env_new(Env *parent);
void env_free(Env *env);
void env_set(Env *env, const char *key, Value val);
Value *env_get(Env *env, const char *key);

#endif
