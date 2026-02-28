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

Value value_none(void) {
    return (Value){ .type = VAL_NONE, .number = 0 };
}

void value_free(Value *v) {
    if (v->type == VAL_STRING) {
        free(v->string);
        v->string = NULL;
    }
    v->type = VAL_NONE;
}

Value value_copy(Value v) {
    if (v.type == VAL_STRING) {
        return value_string(v.string);
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
