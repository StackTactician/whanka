#ifndef WHANKA_VALUE_H
#define WHANKA_VALUE_H

typedef enum {
    VAL_NUMBER,
    VAL_STRING,
    VAL_ARRAY,
    VAL_MAP,
    VAL_NONE
} ValueType;

typedef struct Value Value;

struct Value {
    ValueType type;
    union {
        double number;
        char *string;
        struct { Value *items; int length; int capacity; } array;
        struct { char **keys; Value *values; int length; int capacity; } map;
    };
};

Value value_number(double n);
Value value_string(const char *s);
Value value_array(int capacity);
Value value_map(void);
Value value_none(void);
void value_free(Value *v);
Value value_copy(Value v);
char *value_to_string(Value v);
void value_print(Value v);

#endif
