#ifndef PARDON_INTERPRETER_H
#define PARDON_INTERPRETER_H

#include "parser.h"
#include "env.h"

void interpreter_run(ASTNode *program);

#endif
