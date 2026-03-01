#include "lexer.h"
#include "error.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- Keyword table (sorted longest first for greedy matching) --- */

typedef struct {
    const char *phrase;
    TokenType type;
} Keyword;

static const Keyword keywords[] = {
    /* Multi-word keywords — longest first */
    {"seduce the answer out of", TOK_SEDUCE},
    {"the absolute value of",   TOK_ABS},
    {"consents to being",       TOK_CONSENTS},
    {"the square root of",      TOK_SQRT},
    {"boundaries respected.",    TOK_BOUNDARIES},
    {"to the power of",         TOK_POW},
    {"go on a spree:",          TOK_SPREE_START},
    {"run a train on",          TOK_RUN_A_TRAIN_ON},
    {"enough foreplay",         TOK_PROGRAM_START},
    {"the ceiling of",          TOK_CEIL},
    {"get freaky with",         TOK_GET_FREAKY},
    {"the number in",           TOK_TO_NUM},
    {"roll the dice",           TOK_ROLL_THE_DICE},
    {"body count of",           TOK_BODY_COUNT_OF},
    {"the floor of",            TOK_FLOOR},
    {"the word for",            TOK_TO_STR},
    {"and it takes",            TOK_AND_IT_TAKES},
    {"jacked up by",            TOK_JACKED_UP_BY},
    {"blown up by",             TOK_BLOWN_UP_BY},
    {"leftover from",           TOK_LEFTOVER_FROM},
    {"cut down by",             TOK_CUT_DOWN_BY},
    {"finish with",             TOK_FINISH_WITH},
    {"hotter than",             TOK_HOTTER_THAN},
    {"colder than",             TOK_COLDER_THAN},
    {"busted when",             TOK_BUSTED_WHEN},
    {"no questions",            TOK_NO_QUESTIONS},
    {"my kink is",              TOK_MY_KINK_IS},
    {"hard limit:",             TOK_HARD_LIMIT},
    {"split with",              TOK_SPLIT_WITH},
    {"oh god yes",              TOK_OH_GOD_YES},
    {"as hot as",               TOK_AS_HOT_AS},
    {"safe word.",              TOK_SAFE_WORD},
    {"is frigid",               TOK_IS_FRIGID},
    {"I came.",                 TOK_PROGRAM_END},
    {"entries",                 TOK_ENTRIES},
    {"oh god",                  TOK_OH_GOD},
    {"regards.",                TOK_REGARDS},
    {"devour",                  TOK_DEVOUR},
    {"stitch",                  TOK_STITCH},
    {"harder",                  TOK_HARDER},
    {"scream",                  TOK_SCREAM},
    {"stash",                   TOK_STASH},
    {"under",                   TOK_UNDER},
    {"shove",                   TOK_SHOVE},
    {"slots",                   TOK_SLOTS},
    {"using",                   TOK_USING},
    {"done.",                   TOK_DONE},
    {"yank",                    TOK_YANK},
    {"into",                    TOK_INTO},
    {"moan",                    TOK_MOAN},
    {"dump",                    TOK_DUMP},
    {"chop",                    TOK_CHOP},
    {"yell",                    TOK_YELL},
    {"from",                    TOK_FROM},
    {"with",                    TOK_WITH},
    {"grab",                    TOK_GRAB},
    {"gets",                    TOK_GETS},
    {"hits",                    TOK_HITS},
    {"crap",                    TOK_CRAP},
    {"not",                     TOK_NOT},
    {"and",                     TOK_AND},
    {"is",                      TOK_IS},
    {"if",                      TOK_IF},
    {"or",                      TOK_OR},
    {"by",                      TOK_BY},
    {"at",                      TOK_AT},
};

static const int NUM_KEYWORDS = sizeof(keywords) / sizeof(keywords[0]);

/* --- Token list management --- */

static void token_list_init(TokenList *list) {
    list->count = 0;
    list->capacity = 64;
    list->tokens = malloc(sizeof(Token) * (size_t)list->capacity);
    if (!list->tokens) error_oom();
}

static void token_list_push(TokenList *list, TokenType type, const char *text, int line) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->tokens = realloc(list->tokens, sizeof(Token) * (size_t)list->capacity);
        if (!list->tokens) error_oom();
    }
    list->tokens[list->count].type = type;
    list->tokens[list->count].text = text ? strdup(text) : NULL;
    list->tokens[list->count].line = line;
    list->count++;
}

void token_list_free(TokenList *list) {
    for (int i = 0; i < list->count; i++) {
        free(list->tokens[i].text);
    }
    free(list->tokens);
    list->tokens = NULL;
    list->count = 0;
}

/* --- Lexer core --- */

static void skip_spaces(const char **p) {
    while (**p == ' ' || **p == '\t') (*p)++;
}

/* Case-sensitive prefix match for keywords (some start with uppercase like "I came.") */
static int match_keyword(const char *src, const Keyword *kw) {
    size_t len = strlen(kw->phrase);
    if (strncmp(src, kw->phrase, len) != 0) return 0;

    /* Make sure it's a word boundary (end of line, space, or punctuation) */
    char after = src[len];
    if (after == '\0' || after == ' ' || after == '\t' || after == '\n' ||
        after == '.' || after == ':' || after == ',') {
        return 1;
    }
    /* keywords ending in punctuation don't need word boundary */
    char last = kw->phrase[len - 1];
    if (last == '.' || last == ':') return 1;

    return isalnum((unsigned char)after) ? 0 : 1;
}

static int is_ident_start(char c) {
    return isalpha((unsigned char)c) || c == '_';
}

static int is_ident_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

static void lex_line(const char *line, int lineno, TokenList *list) {
    const char *p = line;

    /* Strip comment (lines starting with // after optional whitespace) */
    {
        const char *tmp = p;
        while (*tmp == ' ' || *tmp == '\t') tmp++;
        if (tmp[0] == '/' && tmp[1] == '/') return;
    }

    while (*p) {
        skip_spaces(&p);
        if (*p == '\0' || *p == '\n' || *p == '\r') break;

        /* Try keyword match (longest first) */
        int matched = 0;
        for (int i = 0; i < NUM_KEYWORDS; i++) {
            if (match_keyword(p, &keywords[i])) {
                size_t len = strlen(keywords[i].phrase);
                char *text = malloc(len + 1);
                if (!text) error_oom();
                memcpy(text, p, len);
                text[len] = '\0';
                token_list_push(list, keywords[i].type, text, lineno);
                free(text);
                p += len;
                matched = 1;
                break;
            }
        }
        if (matched) continue;

        /* String literal */
        if (*p == '"') {
            p++;
            const char *start = p;
            while (*p && *p != '"' && *p != '\n') p++;
            size_t len = (size_t)(p - start);
            char *text = malloc(len + 1);
            if (!text) error_oom();
            memcpy(text, start, len);
            text[len] = '\0';
            token_list_push(list, TOK_STRING, text, lineno);
            free(text);
            if (*p == '"') p++;
            continue;
        }

        /* Number literal */
        if (isdigit((unsigned char)*p) ||
            (*p == '-' && isdigit((unsigned char)*(p + 1)))) {
            const char *start = p;
            if (*p == '-') p++;
            while (isdigit((unsigned char)*p)) p++;
            if (*p == '.') {
                p++;
                while (isdigit((unsigned char)*p)) p++;
            }
            size_t len = (size_t)(p - start);
            char *text = malloc(len + 1);
            if (!text) error_oom();
            memcpy(text, start, len);
            text[len] = '\0';
            token_list_push(list, TOK_NUMBER, text, lineno);
            free(text);
            continue;
        }

        /* Punctuation */
        if (*p == '.') {
            token_list_push(list, TOK_PERIOD, ".", lineno);
            p++;
            continue;
        }
        if (*p == ':') {
            token_list_push(list, TOK_COLON, ":", lineno);
            p++;
            continue;
        }
        if (*p == ',') {
            token_list_push(list, TOK_COMMA, ",", lineno);
            p++;
            continue;
        }

        /* Identifier */
        if (is_ident_start(*p)) {
            const char *start = p;
            while (is_ident_char(*p)) p++;
            size_t len = (size_t)(p - start);
            char *text = malloc(len + 1);
            if (!text) error_oom();
            memcpy(text, start, len);
            text[len] = '\0';
            token_list_push(list, TOK_IDENT, text, lineno);
            free(text);
            continue;
        }

        /* Skip unknown characters (advance full UTF-8 codepoint) */
        {
            unsigned char c = (unsigned char)*p++;
            if (c >= 0xF0)      { if (*p) p++; if (*p) p++; if (*p) p++; }
            else if (c >= 0xE0) { if (*p) p++; if (*p) p++; }
            else if (c >= 0xC0) { if (*p) p++; }
        }
    }
}

TokenList lexer_tokenize(const char *source) {
    TokenList list;
    token_list_init(&list);

    const char *p = source;
    int lineno = 1;

    while (*p) {
        const char *line_start = p;
        while (*p && *p != '\n') p++;

        size_t len = (size_t)(p - line_start);
        char *line = malloc(len + 1);
        if (!line) error_oom();
        memcpy(line, line_start, len);
        line[len] = '\0';

        lex_line(line, lineno, &list);
        free(line);

        if (*p == '\n') { p++; lineno++; }
    }

    token_list_push(&list, TOK_EOF, NULL, lineno);
    return list;
}

const char *token_type_name(TokenType t) {
    switch (t) {
    case TOK_IDENT:          return "IDENT";
    case TOK_NUMBER:         return "NUMBER";
    case TOK_STRING:         return "STRING";
    case TOK_PROGRAM_START:  return "PROGRAM_START";
    case TOK_PROGRAM_END:    return "PROGRAM_END";
    case TOK_REGARDS:        return "REGARDS";
    case TOK_MOAN:           return "MOAN";
    case TOK_IS:             return "IS";
    case TOK_GETS:           return "GETS";
    case TOK_STASH:          return "STASH";
    case TOK_WITH:           return "WITH";
    case TOK_JACKED_UP_BY:   return "JACKED_UP_BY";
    case TOK_CUT_DOWN_BY:    return "CUT_DOWN_BY";
    case TOK_BLOWN_UP_BY:    return "BLOWN_UP_BY";
    case TOK_SPLIT_WITH:     return "SPLIT_WITH";
    case TOK_LEFTOVER_FROM:  return "LEFTOVER_FROM";
    case TOK_HOTTER_THAN:    return "HOTTER_THAN";
    case TOK_COLDER_THAN:    return "COLDER_THAN";
    case TOK_AS_HOT_AS:      return "AS_HOT_AS";
    case TOK_IS_FRIGID:      return "IS_FRIGID";
    case TOK_AND:            return "AND";
    case TOK_OR:             return "OR";
    case TOK_NOT:            return "NOT";
    case TOK_IF:             return "IF";
    case TOK_CONSENTS:       return "CONSENTS";
    case TOK_HARD_LIMIT:     return "HARD_LIMIT";
    case TOK_BOUNDARIES:     return "BOUNDARIES";
    case TOK_SPREE_START:    return "SPREE_START";
    case TOK_BUSTED_WHEN:    return "BUSTED_WHEN";
    case TOK_HITS:           return "HITS";
    case TOK_ROLL_THE_DICE:  return "ROLL_THE_DICE";
    case TOK_SLOTS:          return "SLOTS";
    case TOK_GRAB:           return "GRAB";
    case TOK_AT:             return "AT";
    case TOK_BODY_COUNT_OF:  return "BODY_COUNT_OF";
    case TOK_MY_KINK_IS:     return "MY_KINK_IS";
    case TOK_AND_IT_TAKES:   return "AND_IT_TAKES";
    case TOK_FINISH_WITH:    return "FINISH_WITH";
    case TOK_SAFE_WORD:      return "SAFE_WORD";
    case TOK_GET_FREAKY:     return "GET_FREAKY";
    case TOK_USING:          return "USING";
    case TOK_SCREAM:         return "SCREAM";
    case TOK_YELL:           return "YELL";
    case TOK_SEDUCE:         return "SEDUCE";
    case TOK_RUN_A_TRAIN_ON: return "RUN_A_TRAIN_ON";
    case TOK_DONE:           return "DONE";
    case TOK_SHOVE:          return "SHOVE";
    case TOK_INTO:           return "INTO";
    case TOK_YANK:           return "YANK";
    case TOK_FROM:           return "FROM";
    case TOK_CHOP:           return "CHOP";
    case TOK_BY:             return "BY";
    case TOK_STITCH:         return "STITCH";
    case TOK_SQRT:           return "SQRT";
    case TOK_ABS:            return "ABS";
    case TOK_POW:            return "POW";
    case TOK_FLOOR:          return "FLOOR";
    case TOK_CEIL:           return "CEIL";
    case TOK_TO_NUM:         return "TO_NUM";
    case TOK_TO_STR:         return "TO_STR";
    case TOK_ENTRIES:        return "ENTRIES";
    case TOK_UNDER:          return "UNDER";
    case TOK_DEVOUR:         return "DEVOUR";
    case TOK_DUMP:           return "DUMP";
    case TOK_OH_GOD:         return "OH_GOD";
    case TOK_HARDER:         return "HARDER";
    case TOK_NO_QUESTIONS:   return "NO_QUESTIONS";
    case TOK_CRAP:           return "CRAP";
    case TOK_OH_GOD_YES:     return "OH_GOD_YES";
    case TOK_PERIOD:         return "PERIOD";
    case TOK_COLON:          return "COLON";
    case TOK_COMMA:          return "COMMA";
    case TOK_EOF:            return "EOF";
    }
    return "UNKNOWN";
}
