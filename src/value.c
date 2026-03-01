#include "value.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

Value value_number(double n) {
    return (Value){ .type = VAL_NUMBER, .number = n };
}

Value value_string(const char *s) {
    Value v;
    v.type = VAL_STRING;
    v.string = strdup(s);
    if (!v.string) error_oom();
    return v;
}

Value value_array(int capacity) {
    Value v;
    v.type = VAL_ARRAY;
    v.array.length = capacity;
    v.array.capacity = capacity;
    v.array.items = malloc(sizeof(Value) * (size_t)capacity);
    if (!v.array.items) error_oom();
    for (int i = 0; i < capacity; i++) {
        v.array.items[i] = value_none();
    }
    return v;
}

Value value_map(void) {
    Value v;
    v.type = VAL_MAP;
    v.map.keys = NULL;
    v.map.values = NULL;
    v.map.length = 0;
    v.map.capacity = 0;
    return v;
}

Value value_none(void) {
    return (Value){ .type = VAL_NONE, .number = 0 };
}

void value_free(Value *v) {
    if (v->type == VAL_STRING) {
        free(v->string);
        v->string = NULL;
    } else if (v->type == VAL_ARRAY) {
        for (int i = 0; i < v->array.length; i++) {
            value_free(&v->array.items[i]);
        }
        free(v->array.items);
        v->array.items = NULL;
        v->array.length = 0;
        v->array.capacity = 0;
    } else if (v->type == VAL_MAP) {
        for (int i = 0; i < v->map.length; i++) {
            free(v->map.keys[i]);
            value_free(&v->map.values[i]);
        }
        free(v->map.keys);
        free(v->map.values);
        v->map.keys = NULL;
        v->map.values = NULL;
        v->map.length = 0;
        v->map.capacity = 0;
    }
    v->type = VAL_NONE;
}

Value value_copy(Value v) {
    if (v.type == VAL_STRING) {
        return value_string(v.string);
    }
    if (v.type == VAL_ARRAY) {
        Value copy;
        copy.type = VAL_ARRAY;
        copy.array.length = v.array.length;
        copy.array.capacity = v.array.capacity;
        copy.array.items = malloc(sizeof(Value) * (size_t)v.array.capacity);
        if (!copy.array.items) error_oom();
        for (int i = 0; i < v.array.length; i++) {
            copy.array.items[i] = value_copy(v.array.items[i]);
        }
        return copy;
    }
    if (v.type == VAL_MAP) {
        Value copy;
        copy.type = VAL_MAP;
        copy.map.length = v.map.length;
        copy.map.capacity = v.map.capacity;
        if (v.map.capacity > 0) {
            copy.map.keys = malloc(sizeof(char *) * (size_t)v.map.capacity);
            copy.map.values = malloc(sizeof(Value) * (size_t)v.map.capacity);
            if (!copy.map.keys || !copy.map.values) error_oom();
            for (int i = 0; i < v.map.length; i++) {
                copy.map.keys[i] = strdup(v.map.keys[i]);
                if (!copy.map.keys[i]) error_oom();
                copy.map.values[i] = value_copy(v.map.values[i]);
            }
        } else {
            copy.map.keys = NULL;
            copy.map.values = NULL;
        }
        return copy;
    }
    return v;
}

char *value_to_string(Value v) {
    char buf[64];
    switch (v.type) {
    case VAL_NUMBER: {
        double intpart;
        if (modf(v.number, &intpart) == 0.0 &&
            v.number >= -1e15 && v.number <= 1e15) {
            snprintf(buf, sizeof(buf), "%.0f", v.number);
        } else {
            snprintf(buf, sizeof(buf), "%g", v.number);
        }
        return strdup(buf);
    }
    case VAL_STRING:
        return strdup(v.string);
    case VAL_ARRAY: {
        /* Format as [elem0, elem1, ...] */
        size_t total = 2; /* "[]" */
        char **parts = malloc(sizeof(char *) * (size_t)v.array.length);
        if (!parts) error_oom();
        for (int i = 0; i < v.array.length; i++) {
            parts[i] = value_to_string(v.array.items[i]);
            total += strlen(parts[i]);
            if (i > 0) total += 2; /* ", " */
        }
        char *result = malloc(total + 1);
        if (!result) error_oom();
        char *p = result;
        *p++ = '[';
        for (int i = 0; i < v.array.length; i++) {
            if (i > 0) { *p++ = ','; *p++ = ' '; }
            size_t len = strlen(parts[i]);
            memcpy(p, parts[i], len);
            p += len;
            free(parts[i]);
        }
        *p++ = ']';
        *p = '\0';
        free(parts);
        return result;
    }
    case VAL_MAP: {
        size_t total = 2; /* "{}" */
        char **kparts = malloc(sizeof(char *) * (size_t)v.map.length);
        char **vparts = malloc(sizeof(char *) * (size_t)v.map.length);
        if (v.map.length > 0 && (!kparts || !vparts)) error_oom();
        for (int i = 0; i < v.map.length; i++) {
            kparts[i] = strdup(v.map.keys[i]);
            vparts[i] = value_to_string(v.map.values[i]);
            if (!kparts[i]) error_oom();
            total += strlen(kparts[i]) + 2 + strlen(vparts[i]); /* "key: val" */
            if (i > 0) total += 2; /* ", " */
        }
        char *result = malloc(total + 1);
        if (!result) error_oom();
        char *p = result;
        *p++ = '{';
        for (int i = 0; i < v.map.length; i++) {
            if (i > 0) { *p++ = ','; *p++ = ' '; }
            size_t klen = strlen(kparts[i]);
            memcpy(p, kparts[i], klen); p += klen;
            *p++ = ':'; *p++ = ' ';
            size_t vlen = strlen(vparts[i]);
            memcpy(p, vparts[i], vlen); p += vlen;
            free(kparts[i]);
            free(vparts[i]);
        }
        *p++ = '}';
        *p = '\0';
        free(kparts);
        free(vparts);
        return result;
    }
    case VAL_NONE:
        return strdup("none");
    }
    return strdup("???");
}

void value_print(Value v) {
    char *s = value_to_string(v);
    printf("%s", s);
    free(s);
}
