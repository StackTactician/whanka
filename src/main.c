#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "error.h"

static char *read_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "WHAT THE FUCK?! Can't open \"%s\". File doesn't exist, genius.\n", path);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc((size_t)len + 1);
    if (!buf) { fclose(f); error_oom(); }

    size_t read = fread(buf, 1, (size_t)len, f);
    buf[read] = '\0';
    fclose(f);
    return buf;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: pardon <file.aids>\n");
        fprintf(stderr, "  You forgot the file, you absolute walnut.\n");
        return 1;
    }

    char *source = read_file(argv[1]);

    TokenList tokens = lexer_tokenize(source);
    ASTNode *ast = parser_parse(&tokens);
    interpreter_run(ast);

    ast_free(ast);
    token_list_free(&tokens);
    free(source);

    return 0;
}
