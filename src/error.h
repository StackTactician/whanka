#ifndef PARDON_ERROR_H
#define PARDON_ERROR_H

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

#endif
