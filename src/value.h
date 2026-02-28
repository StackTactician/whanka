#ifndef PARDON_VALUE_H
#define PARDON_VALUE_H

typedef enum {
    VAL_NUMBER,
    VAL_STRING,
    VAL_NONE
} ValueType;

typedef struct {
    ValueType type;
    union {
        double number;
        char *string;
    };
} Value;

Value value_number(double n);
Value value_string(const char *s);
Value value_none(void);
void value_free(Value *v);
Value value_copy(Value v);
char *value_to_string(Value v);
void value_print(Value v);

#endif
