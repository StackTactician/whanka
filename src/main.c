#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "error.h"

#define MAX_SOURCE_SIZE (10 * 1024 * 1024) /* 10 MB */

static char *read_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "WHAT THE FUCK?! Can't open \"%s\". File doesn't exist, genius.\n", path);
        exit(1);
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        fprintf(stderr, "WHAT THE FUCK?! Can't read \"%s\". Not a seekable file.\n", path);
        exit(1);
    }

    long len = ftell(f);
    if (len < 0) {
        fclose(f);
        fprintf(stderr, "WHAT THE FUCK?! Can't determine size of \"%s\".\n", path);
        exit(1);
    }
    if ((size_t)len > MAX_SOURCE_SIZE) {
        fclose(f);
        error_file_too_large(path);
    }

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
        fprintf(stderr, "Usage: Whanka <file.aids>\n");
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
