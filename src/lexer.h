#ifndef WHANKA_LEXER_H
#define WHANKA_LEXER_H

typedef enum {
    /* Literals */
    TOK_IDENT,
    TOK_NUMBER,
    TOK_STRING,

    /* Program structure */
    TOK_PROGRAM_START,   /* enough foreplay */
    TOK_PROGRAM_END,     /* I came. */
    TOK_REGARDS,         /* regards. */

    /* Assignment */
    TOK_MOAN,            /* moan */
    TOK_IS,              /* is */
    TOK_GETS,            /* gets */
    TOK_STASH,           /* stash */
    TOK_WITH,            /* with */

    /* Arithmetic */
    TOK_JACKED_UP_BY,    /* jacked up by */
    TOK_CUT_DOWN_BY,     /* cut down by */
    TOK_BLOWN_UP_BY,     /* blown up by */
    TOK_SPLIT_WITH,      /* split with */
    TOK_LEFTOVER_FROM,   /* leftover from */

    /* Comparison */
    TOK_HOTTER_THAN,     /* hotter than */
    TOK_COLDER_THAN,     /* colder than */
    TOK_AS_HOT_AS,       /* as hot as */
    TOK_IS_FRIGID,       /* is frigid */

    /* Boolean operators */
    TOK_AND,             /* and */
    TOK_OR,              /* or */
    TOK_NOT,             /* not */

    /* Control flow */
    TOK_IF,              /* if */
    TOK_CONSENTS,        /* consents to being */
    TOK_HARD_LIMIT,      /* hard limit: */
    TOK_BOUNDARIES,      /* boundaries respected. */

    /* Loops */
    TOK_SPREE_START,     /* go on a spree: */
    TOK_BUSTED_WHEN,     /* busted when */
    TOK_HITS,            /* hits */

    /* Random */
    TOK_ROLL_THE_DICE,   /* roll the dice */

    /* Arrays */
    TOK_SLOTS,           /* slots */
    TOK_GRAB,            /* grab */
    TOK_AT,              /* at */
    TOK_BODY_COUNT_OF,   /* body count of */

    /* Functions */
    TOK_MY_KINK_IS,      /* my kink is */
    TOK_AND_IT_TAKES,    /* and it takes */
    TOK_FINISH_WITH,     /* finish with */
    TOK_SAFE_WORD,       /* safe word. */
    TOK_GET_FREAKY,      /* get freaky with */
    TOK_USING,           /* using */

    /* I/O */
    TOK_SCREAM,          /* scream */
    TOK_YELL,            /* yell */
    TOK_SEDUCE,          /* seduce the answer out of */

    /* For-each */
    TOK_RUN_A_TRAIN_ON,  /* run a train on */
    TOK_DONE,            /* done. */

    /* Dynamic arrays */
    TOK_SHOVE,           /* shove */
    TOK_INTO,            /* into */
    TOK_YANK,            /* yank */
    TOK_FROM,            /* from */

    /* String ops */
    TOK_CHOP,            /* chop */
    TOK_BY,              /* by */
    TOK_STITCH,          /* stitch */

    /* Math */
    TOK_SQRT,            /* the square root of */
    TOK_ABS,             /* the absolute value of */
    TOK_POW,             /* to the power of */
    TOK_FLOOR,           /* the floor of */
    TOK_CEIL,            /* the ceiling of */

    /* Type coercion */
    TOK_TO_NUM,          /* the number in */
    TOK_TO_STR,          /* the word for */

    /* Hashmaps */
    TOK_ENTRIES,         /* entries */
    TOK_UNDER,           /* under */

    /* File I/O */
    TOK_DEVOUR,          /* devour */
    TOK_DUMP,            /* dump */

    /* Flavor (parsed and discarded) */
    TOK_OH_GOD,          /* oh god */
    TOK_HARDER,          /* harder */
    TOK_NO_QUESTIONS,    /* no questions */
    TOK_CRAP,            /* crap */
    TOK_OH_GOD_YES,      /* oh god yes */

    /* Punctuation */
    TOK_PERIOD,
    TOK_COLON,
    TOK_COMMA,

    TOK_EOF
} TokenType;

typedef struct {
    TokenType type;
    char *text;          /* token text (owned, heap-allocated) */
    int line;
} Token;

typedef struct {
    Token *tokens;
    int count;
    int capacity;
} TokenList;

TokenList lexer_tokenize(const char *source);
void token_list_free(TokenList *list);
const char *token_type_name(TokenType t);

#endif
