#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "defines.h"
#include "keywords.h"
#include "types.h"

extern char *keywords[];

char *treetxt(PARSETREE * p);
void terminate(int err);        //create.c

/********** search for keywords through parsetree *********/
/** called as locate( parsetree, Klibrary, 0)   ****/
/** or        locate( cell, -1, Kcell, 0) moves to next brother **/
PARSETREE *locate(char *fmt, ...)
{
    va_list pvar;
    PARSETREE *p;
    int keywd;

    va_start(pvar, fmt);
    p = va_arg(pvar, PARSETREE_P);
    if (!p) {
        fprintf(stderr, "Program Error: locate( 0,..\n");
        terminate(3);
    }

    keywd = va_arg(pvar, int);

    while (keywd && p) {
        if (keywd == -1) {
            p = p->next;
            keywd = va_arg(pvar, int);
        } else
            p = p->data.tree;

        while (p && keywd != p->type)
            p = p->next;
        keywd = va_arg(pvar, int);
    }
    return (p);
}

/****** find name after (keyword :either ident or (name or (rename or (array*/
char *nameof(p)
PARSETREE *p;
{
    char *treetxt();
    PARSETREE *h = p;

    while (p && p->type > 0)    /* is some keyword */
        p = p->data.tree;       /* name should be in first argument */

    if (p && (p->type == VALUEREF || p->type == IDENT))
        return (p->data.token);

    fprintf(stderr, "BUG: nameof() unsuccessfull!\n");
    if (h)
        fprintf(stderr, "     Line %d, %s\n", h->line, treetxt(h));
    else
        fprintf(stderr, "     had NULL argument!\n");
    terminate(3);
}

/****** find name after (keyword :either ident or (name or (rename or (array*/
char *realnameof(p)
PARSETREE *p;
{
    char *treetxt();
    PARSETREE *h = p;
    int k, l;

    while (p && p->type > 0) {  /* is some keyword */
        k = p->type;
        p = p->data.tree;       /* name should be in first argument */
    }

/*	if (p && k==Krename) p = p->next;  JvE ???? opnieuw bekijken 941116
*/

    if (p && (p->type == VALUEREF || p->type == IDENT || p->type == STRING)) {  /*
                                                                                   if (p->type == STRING)
                                                                                   {    ** remove surrounding "" **
                                                                                   l = strlen( p->data.token);
                                                                                   if (p->data.token[l-1] == '"')
                                                                                   p->data.token[l-1] = '\0';
                                                                                   return( p->data.token + 1);
                                                                                   } else
                                                                                 */
        return (p->data.token);
    }

    fprintf(stderr, "BUG: nameof() unsuccessfull!\n");
    if (h)
        fprintf(stderr, "     Line %d, %s\n", h->line, treetxt(h));
    else
        fprintf(stderr, "     had NULL argument!\n");
    terminate(3);
}

/*** get array size of object (by now used for (net **/
PARSETREE *get_array_size(obj)
PARSETREE *obj;
{
    PARSETREE *p;

    p = obj->data.tree;

    if (!(p && p->type == Karray))
        return (NULL);

    p = p->data.tree->next;
    if (!(p && p->type == NUMBER)) {
        fprintf(stderr, "BUG: (array has no integerValues ???\n");
        fprintf(stderr, "     Line %d: %s\n", p->line, treetxt(p));
        terminate(3);
    }

    return (p);
}

/*** generate string about parsetree datastruct for err messages **/
char *treetxt(p)
PARSETREE *p;
{
    static char line[64];

    if (!p)
        return ("");

    switch (p->type) {
    case NUMBER:
        sprintf(line, "%d", p->data.ival);
        Case IDENT:case STRING:case VALUEREF:strncpy(line, p->data.token,
                                                     63);

        line[63] = '\0';
        Case KEYWORD:strcpy(line, "unset keyword");
        Case ENDFILE:strcpy(line, "End-of-file");
        Case ENDLIST:strcpy(line, "End-of-form");

      Default:sprintf(line, "(%s", keywords[p->type]);
    }
    return (line);
}

/*** generate string about value datastruct for err messages **/
char *valuetxt(p)
VALUE *p;
{
    static char line[64];
    char *c;

    switch (p->type) {
    case Kboolean:
        sprintf(line, "%s", p->val.b ? "(true)" : "(false)");
        Case Kinteger:sprintf(line, "%d", p->val.i);
        Case Knumber:sprintf(line, "%g", p->val.f);
        Case Kpoint:sprintf(line, "[%d,%d]", p->val.p.x, p->val.p.y);
        Case Kstring:sprintf(line, "%s", p->val.s);
        Case KmiNoMax:if (p->val.mnm.asg & MIN_ASG)
             sprintf(line, "[%g,", p->val.mnm.min);

        else
            sprintf(line, "[%s,", (p->val.mnm.asg & MIN_UNC)
                    ? "unconstrained" : "undefined");
        c = line + strlen(line);
        if (p->val.mnm.asg & VAL_ASG)
            sprintf(c, "%g,", p->val.mnm.val);
        else
            sprintf(c, "%s,", (p->val.mnm.asg & VAL_UNC)
                    ? "unconstrained" : "undefined");
        c = line + strlen(line);
        if (p->val.mnm.asg & MAX_ASG)
            sprintf(c, "%g]", p->val.mnm.max);
        else
            sprintf(c, "%s]", (p->val.mnm.asg & MAX_UNC)
                    ? "unconstrained" : "undefined");
      Default:sprintf(line, "type %d unknown!", p->type);
    }
    return (line);
}
