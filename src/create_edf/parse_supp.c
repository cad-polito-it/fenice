/**********************************************************/
/*   support functions for the edif parser                */
/**********************************************************/

#include <stdio.h>
#include <stdbool.h>
#include "defines.h"
#include "types.h"
#include "keywords.h"

/********* IMPORTED *************/
extern int debug;               /* be verbose instead of silent */
extern int dont_hash;           /* (in)activate hashing */
extern PARSETREE *token;        /* pointer to current token */
extern char *lineno();          /* tells current line */
extern char *tokentxt();        /* text of token for err messages */
extern PARSETREE *get_parsetree();      /* memory allocation */
extern PARSETREE *put_parsetree();      /* memory allocation */
extern char *newstring();       /* memory allocation */

/************* EXPORTED **********/
int err_count;                  /* number of parse errors */
int ncells;                     /* total number of cells in input file */
PARSETREE *parsetree;           /* root of parse tree */
char *keywords[MAXKEYWORDS];    /* from keyword typenumber to string */
int nkeywords;                  /* current number of keywords in use */

/************* FOR THIS FILE ONLY **********/
static PARSETREE *edifstack[MAXDEPTH];  /* current nesting of edif rules */
static int level;               /* current top of edif stack */

/**************************************************************************/
#if (MAXKEYWORDS<(NKEYWORDS+30))
cpp:ERROR "MAXKEYWORDS" is set too small ! ! !
#endif
/******* start actual parsing *****/
int macro();
void terminate(int err);
void parse_edif();
int match_endlist(int b);
void keywordalias(PARSETREE * p);
void keyworddefine(PARSETREE * p);
int key_hash(char *name, int new);
int Pedif();

void parse_edif()
{
    macro();                    /* get first token */
    if (!Pedif() || err_count > 0 || token->type != ENDFILE) {  /*WRONG! */
        if (token->type != ENDFILE)
            fprintf(stderr, "%s Unexpected token '%s'!\n", lineno(),
                    tokentxt());
        fprintf(stderr, "Program aborted due to errors in EDIF input!\n");
        terminate(1);
    }
    if (debug)
        fprintf(stderr, "%s EDIF input successfully parsed.\n", lineno());
}

/*********** really add something to the parsetree ****/
/*** Note that at call-time the value of 'type' 'KEYWORD' */
/*** is modified in 'parse.c' to the value of the actual keyword */
void add_token()
{
    static PARSETREE **growpoint = &parsetree;

    if (ISKEYWORD(token->type) && level >= MAXDEPTH) {
        fprintf(stderr, "%s\n%s (=%d)\n%s\n", "Fatal error in EDIF input:",
                "Maximum nesting level of (()) reached", MAXDEPTH,
                "Program aborted...");
        terminate(1);
    }
    if (token->type == ENDLIST) {
        if (level <= 0)         /*exit with error message */
            match_endlist(FALSE);
        growpoint = &(edifstack[--level]->next);
        macro();
        return;
    }

    *growpoint = token;         /*add latest struct */
    if (ISKEYWORD(token->type)) {
        edifstack[level++] = token;
        if (token->type == Kcell)
            ncells++;
        growpoint = &(token->data.tree);
    } else
        growpoint = &(token->next);

    token = NULL;               /* indicate that token is absorbed */
    macro();
}

/****** match token and skip it ************/
int match_anyitem()
{
    switch (token->type) {
    case IDENT:
    case VALUEREF:
    case NUMBER:
    case STRING:
        macro();                /* skip this token, do hash next one! */
        return (TRUE);
    case ENDLIST:
    case ENDFILE:              /* nothing to match */
        return (FALSE);
    default:                   /*must be some keyword, skip list without hashing */
        dont_hash++;
        macro();
        while (match_anyitem());
        dont_hash--;
        if (token->type == ENDLIST) {
            macro();            /*new (hashed) token after matching ) */
            return (TRUE);
        } else
            return (FALSE);
    }
}

int match_tail(b)
int b;
{
    while (b && token->type != ENDLIST)
        b = match_anyitem();
    return (match_endlist(b));
}

/******** check for syntax, and tailor datastruct ****/
int match_endlist(b)
int b;
{
    if (!b || token->type != ENDLIST || level == 0) {   /* syntax error */
        fprintf(stderr, "%s token '%s' unexpected in '%s' grammar rule!\n",
                lineno(), tokentxt(token->type),
                (level >
                 0) ? keywords[edifstack[level - 1]->type] : "???");

        if (++err_count > 10 || level == 0) {
            fprintf(stderr,
                    "Program aborted due to errors in EDIF input!\n");
            terminate(1);
        }
        while (match_anyitem());        /* skip to ')' or EndOfFile */
    } else {                    /* all OK */
        if (edifstack[level - 1]->type == KkeywordAlias)
            keywordalias(edifstack[level - 1]);
        if (edifstack[level - 1]->type == KkeywordDefine)
            keyworddefine(edifstack[level - 1]);
    }

    /* Always add endlist ')' to datastructure */
    add_token();

    return (TRUE);              /* recovered from any error */
}

/*** generate adresses for all edif words ***/
void parse_init()
{
#include "keywords.c"
}
