#ifndef WHANKA_ERROR_H
#define WHANKA_ERROR_H

#include <stdnoreturn.h>

noreturn void error_undeclared(const char *name, int line);
noreturn void error_divzero(int line);
noreturn void error_infinite_loop(int line);
noreturn void error_eof(void);
noreturn void error_func_not_found(const char *name, int line);
noreturn void error_oom(void);
noreturn void error_syntax(const char *msg, int line);
noreturn void error_type(const char *msg, int line);
noreturn void error_args(const char *func, int expected, int got, int line);
noreturn void error_recursion(int line);
noreturn void error_too_many_vars(int line);
noreturn void error_too_many_funcs(int line);
noreturn void error_file_too_large(const char *path);
noreturn void error_index_out_of_bounds(int index, int length, int line);
noreturn void error_file_read(const char *path, int line);
noreturn void error_file_write(const char *path, int line);

#endif
