#include "error.h"
#include <stdio.h>
#include <stdlib.h>

noreturn void error_undeclared(const char *name, int line) {
    fprintf(stderr, "WHAT THE FUCK?! \"%s\" doesn't exist, you absolute degenerate. (line %d)\n",
            name, line);
    exit(1);
}

noreturn void error_divzero(int line) {
    fprintf(stderr, "YOU LIMP DICK! Can't divide by zero. Even a criminal knows that. (line %d)\n",
            line);
    exit(1);
}

noreturn void error_infinite_loop(int line) {
    fprintf(stderr, "BLUE BALLS! This loop won't stop. I'm cutting you off. (line %d)\n",
            line);
    exit(1);
}

noreturn void error_eof(void) {
    fprintf(stderr, "PREMATURE FINISH! Hit end of file before you were done. How embarrassing.\n");
    exit(1);
}

noreturn void error_func_not_found(const char *name, int line) {
    fprintf(stderr, "COCKBLOCKED! No kink called \"%s\" exists. (line %d)\n",
            name, line);
    exit(1);
}

noreturn void error_oom(void) {
    fprintf(stderr, "DRIED UP! Out of memory. We're done here.\n");
    exit(1);
}

noreturn void error_syntax(const char *msg, int line) {
    fprintf(stderr, "WHAT THE FUCK?! Syntax error: %s (line %d)\n", msg, line);
    exit(1);
}

noreturn void error_type(const char *msg, int line) {
    fprintf(stderr, "WHAT THE FUCK?! Type error: %s (line %d)\n", msg, line);
    exit(1);
}

noreturn void error_args(const char *func, int expected, int got, int line) {
    fprintf(stderr, "COCKBLOCKED! \"%s\" takes %d arg(s) but got %d. (line %d)\n",
            func, expected, got, line);
    exit(1);
}

noreturn void error_recursion(int line) {
    fprintf(stderr, "BLUE BALLS! Recursion too deep. You're going in circles, freak. (line %d)\n",
            line);
    exit(1);
}

noreturn void error_too_many_vars(int line) {
    fprintf(stderr, "COCKBLOCKED! Too many variables in one scope. Clean up your mess. (line %d)\n",
            line);
    exit(1);
}

noreturn void error_too_many_funcs(int line) {
    fprintf(stderr, "COCKBLOCKED! Too many kinks defined. You need to calm down. (line %d)\n",
            line);
    exit(1);
}

noreturn void error_file_too_large(const char *path) {
    fprintf(stderr, "WHAT THE FUCK?! \"%s\" is way too big. Max 10 MB, you greedy bastard.\n",
            path);
    exit(1);
}

noreturn void error_index_out_of_bounds(int index, int length, int line) {
    fprintf(stderr, "WHAT THE FUCK?! Index %d is out of bounds. Array has %d slots. (line %d)\n",
            index, length, line);
    exit(1);
}

noreturn void error_file_read(const char *path, int line) {
    fprintf(stderr, "WHAT THE FUCK?! Can't devour \"%s\". File doesn't exist or can't be read. (line %d)\n",
            path, line);
    exit(1);
}

noreturn void error_file_write(const char *path, int line) {
    fprintf(stderr, "WHAT THE FUCK?! Can't dump to \"%s\". Permission denied or disk full. (line %d)\n",
            path, line);
    exit(1);
}
