/*****************************************************************************
The function 'macro()' is repeatedly called by the parser to obtain
its next input token. The default behaviour is just grabbing such a token
from the lexical analyser.
However if the fetched token is an aliased keyword, unaliasing is done.
if the fetched token leads a macro call:
 - it will read the entire macro call in a local parse tree.
 - it will perform a macro expansion
 - subsequently deliver tokens from this expansion, until it is exhausted.
   Thus the parser can perform real grammar checking on the expanded result.
Note that macro calls can nest!
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "defines.h"
#include "types.h"
#include "keywords.h"

/** IMPORTED ***/
extern int yy_create_edf_lex();
extern int yy_create_edf_lineno;
extern int err_count;
extern int debug;
extern char yy_create_edf_text[];
extern char *str_hash(), *treetxt();
extern PARSETREE *get_parsetree(), *put_parsetree();
extern PARSETREE *locate();
extern char *keywords[];

/*** EXPORTED ***/
PARSETREE *token;
int keywordlevelused;

void terminate(int err);
int key_hash(char *name, int new);

/*** LOCAL ***/
static PARSETREE *newkeydef[MAXKEYWORDS];       /* defined keywords */
static int newkeyalias[MAXKEYWORDS];    /* keyword aliases */
static int curr_macro;          /* type number of macro now expanded */
static char *formal_asgn;       /* formal argument being assigned */
static int mlines[2];           /* begin-end line numbers of call */

/*** FORWARD ***/
extern char *lineno(), *tokentxt();
extern PARSETREE *readmacrocall(), *expandmacro(), *assignformal();
extern PARSETREE *buildmacro(), *cpy_parsetree(), *find_actual();

int macro()
{
    static PARSETREE *mstack[MAXDEPTH]; /* pointers in macro storage */
    static int stackp;          /* for sequential output */
    static int lastline;
    int type;
    PARSETREE **p, *exp;

  again:

    /* Nothing on stack from last macro expansion? */
    /* then read new input from file */
    if (stackp == 0 && !mstack[0]) {
        if (!token)
            token = get_parsetree();
        else
            token->next = NULL; /* thus actually always NULL */

        token->type = type = yy_create_edf_lex();
        token->line = yy_create_edf_lineno;

        DEBUG(9)
            "macro: reading token '%s' from yy_create_edf_lex (line %d)\n",
            yy_create_edf_text, yy_create_edf_lineno);

        switch (type) {
        case NUMBER:
            token->data.ival = atoi(yy_create_edf_text);
            Case STRING:case IDENT:token->data.token =
                str_hash(yy_create_edf_text);
            Case KEYWORD:token->data.tree = NULL;

            token->type = type = key_hash(yy_create_edf_text, 0);
        }

        /* Have I got a newly-introduced keyword name? */
        /* If it was a simple keyword alias, then replace */
        if (type > NKEYWORDS && newkeyalias[type]) {
            type = newkeyalias[type];
            DEBUG(3) "macro: keyword alias '%s' to '%s'\n",
                yy_create_edf_text, keywords[type]);
        }
        if (type <= NKEYWORDS)
            return (type);
        /* Return standard EDIF item */

        /* type is macro call requiring expansion.... */
        /* save line numbers, read full macro call */
        mlines[0] = yy_create_edf_lineno;
        mstack[stackp = 0] = readmacrocall(type);
        mlines[1] = lastline = yy_create_edf_lineno;
    }

        /****** read next token from stored macro expansion ***/
        /****** if keyword alias, then unalias ****************/
        /****** if macro call, then replace by its expansion **/
        /** remove token from storage just before it is returned */
        /** only (keyword is removed after ) ******************/

    if (token)
        put_parsetree(token);
    token = mstack[stackp];

    DEBUG(8)
        "macro: reading token '%s' from macro expansion stack, level %d\n",
        treetxt(token), stackp);

    if (!token) {               /* end  of list reached */
        DEBUG(5) "macro expansion returns ')'\n");
        if (stackp <= 0) {
            fprintf(stderr, "Program error: end of macro stack!\n");
            terminate(3);
        }
        stackp--;
        /* all done? then clear inputline info */
        if (stackp == 0 && !mstack[0])
            mlines[0] = mlines[1] = 0;
        if (!token)
            token = get_parsetree();
        else
            token->next = NULL;
        token->line = lastline;
        return (token->type = ENDLIST);
    }
    /*  else .... */
    type = token->type;
    if (ISKEYWORD(type) || type == KEYWORD) {
        if (type > NKEYWORDS && newkeyalias[type]) {
            DEBUG(3) "macro: keyword alias '%s' to '%s'\n", keywords[type],
                keywords[newkeyalias[type]]);
            token->type = type = newkeyalias[type];
        }

        if (type > NKEYWORDS && newkeydef[type]) {      /* macro call: replace by its expansion */
            /* preform macro expansion, result in exp */
            exp = expandmacro(token);
            /* remove macro call from stack */
            mstack[stackp] = put_parsetree(token);
            token = NULL;
            if (exp) {          /* insert nonemtpy (list?) result in stack */
                DEBUG(5) "macro: insert macro-expansion in stack[%d]\n",
                    stackp);
                for (p = &(exp->next); *p; p = &((*p)->next));
                *p = mstack[stackp];
                mstack[stackp] = exp;
            } else
                DEBUG(5) "macro: expansion had empty result\n");
            if (stackp == 0 && !mstack[0])
                mlines[0] = mlines[1] = 0;
            DEBUG(6) "macro: go back, try to return token to parser\n");
            goto again;
        }
        /* reference to standard EDIF keyword */
        /* remove keyword token from stack, add 'next' and 'data.tree' */
        mstack[stackp] = token->next;   /* == mstack[stackp]->next */
        mstack[++stackp] = token->data.tree;
    } else
        mstack[stackp] = token->next;
    token->next = NULL;

    DEBUG(5) "macro expansion returns '%s'\n", tokentxt());
    lastline = token->line;
    return (type);
}

void keywordalias(p)
PARSETREE *p;
{
    int type, newtype;

    if (keywordlevelused < 1)
        keywordlevelused = 1;
    newtype = key_hash(p->data.tree->data.token, 1);
    if (!newtype) {
        fprintf(stderr,
                "%s keywordAlias redefines existing keyword '%s'!\n",
                lineno(), p->data.tree->data.token);
        err_count++;
        return;
    }
    type = key_hash(p->data.tree->next->data.token, 0);
    if (!ISKEYWORD(type)) {
        fprintf(stderr,
                "%s keywordAlias references unknown keyword '%s'!\n",
                lineno(), p->data.tree->next->data.token);
        err_count++;
        return;
    }

    if (newkeyalias[type])      /* resolve multilevel alias */
        type = newkeyalias[type];
    newkeyalias[newtype] = type;
}

void keyworddefine(p)
PARSETREE *p;
{
    int newtype;

    if (keywordlevelused < 2)
        keywordlevelused = 2;
    newtype = key_hash(p->data.tree->data.token, 1);
    if (!newtype) {
        fprintf(stderr,
                "%s keywordDefine redefines existing keyword '%s'!\n",
                lineno(), p->data.tree->data.token);
        err_count++;
        return;
    }
    newkeydef[newtype] = p;
}

PARSETREE *readmacrocall(type)
int type;
{
    PARSETREE *call, **p;

    call = get_parsetree();
    call->type = (newkeyalias[type]) ? newkeyalias[type] : type;
    call->line = yy_create_edf_lineno;
    DEBUG(3) "macro: storing call '(%s'\n", keywords[call->type]);
    p = &(call->data.tree);

    while ((type = yy_create_edf_lex()) != ENDLIST && type != ENDFILE) {
        if (type == KEYWORD)
            *p = readmacrocall(key_hash(yy_create_edf_text, 0));
        else {
            *p = get_parsetree();
            (*p)->type = type;
            (*p)->line = yy_create_edf_lineno;
            if (type == IDENT || type == STRING)
                (*p)->data.token = str_hash(yy_create_edf_text);
            else if (type == NUMBER)
                (*p)->data.ival = atoi(yy_create_edf_text);
        }
        p = &((*p)->next);
    }
    return (call);
}

/**** generate new parsetree with the expansion of the macro call ***/
PARSETREE *expandmacro(call)
PARSETREE *call;
{
    PARSETREE *formal, *mdef, *result, *args;

    DEBUG(3) "macro: perform macro expansion of '%s' call (line %d)\n",
        keywords[call->type], call->line);

    curr_macro = call->type;

        /*** get formal par decl ***/
    formal = locate(newkeydef[curr_macro], KkeywordParameters, Kformal, 0);

        /*** assign formal arguments ***/
    args = assignformal(formal, call->data.tree);

        /*** get body of macro definition ***/
    mdef = locate(newkeydef[curr_macro], Kgenerate, 0)->data.tree;

        /*** building expanded result **/
    result = buildmacro(args, mdef);

        /*** free argument values, and return result ***/
    while (args)
        args = put_parsetree(args);
    return (result);
}

PARSETREE *assignformal(formal, call)
PARSETREE *formal, *call;
{
    PARSETREE *f, *result, **rp, *coll, *opt, **vp;
    int ncolls, nopts, cnt, formal_type;;

    cnt = ncolls = nopts = formal_type = 0;

    /* scan once for the characteristics of the declared formal args */
    for (f = formal; f; f = f->next) {
        cnt++;                  /* count the number of declared arguments */
        if (coll = locate(f, Kcollector, 0)) {
            ncolls++;           /* collector found */
            if (!formal_type)
                formal_type = coll->type;
        }
        if (opt = locate(f, Koptional, 0)) {
            nopts++;            /* optional arg found */
            if (!formal_type)
                formal_type = opt->type;
        }
        /* once a 'collector' or 'optional' is found, */
        /* following ones must be of the same type */
        if (!coll && formal_type == Kcollector || !opt
            && formal_type == Koptional) {
            fprintf(stderr, "%s Evaluation of keywordDefine of (%s:\n",
                    lineno(), keywords[curr_macro]);
            fprintf(stderr,
                    "    (formal %s must also have (%s attached!\n",
                    f->data.tree->data.token, keywords[formal_type]);
            err_count++;
        }
    }
    DEBUG(6) "macro: definition has %d args (%d collector, %d optional)\n",
        cnt, ncolls, nopts);
    if (ncolls > 0 && nopts > 0)
        return (NULL);          /*Illegal, message already made */
    if (ncolls > 0)
        keywordlevelused = 3;

    result = NULL;

    /* repeat to fill collector type arguments */
    for (cnt = 0; call && (cnt == 0 || ncolls > 0); cnt++)
        /* scan over the formals, one assignment for each */
        for (f = formal, rp = &result; f; f = f->next, rp = &((*rp)->next)) {
            formal_asgn = f->data.tree->data.token;
            coll = (ncolls) ? locate(f, Kcollector, 0) : 0;
            opt = (nopts) ? locate(f, Koptional, 0) : 0;
            if (!call && !opt)
                break;          /* error: no actual args for non-opt */
            if (cnt > 0 && !coll)
                continue;       /*skip non-coll after 1st round */
            if (cnt == 0) {     /*create list to contain result */
                *rp = get_parsetree();
                (*rp)->type = Kformal;  /*misuse type field as forEach flag */
                (*rp)->data.tree = get_parsetree();
                (*rp)->data.tree->type = IDENT; /* attach par name */
                (*rp)->data.tree->data.token = formal_asgn;
            }
            DEBUG(7) "macro: attaching value %d to formal argument '%s'\n",
                cnt + 1, formal_asgn);
            for (vp = &((*rp)->data.tree->next); *vp; vp = &((*vp)->next));
            /* locate end of values list */
            if (call)           /* value available */
                *vp = cpy_parsetree(call);
            else {              /* use (optional field */
                DEBUG(7) "macro: no actual, evaluate '(optional'\n");
                *vp = buildmacro(result, opt->data.tree);
                if (!(*vp) || (*vp)->next) {
                    fprintf(stderr,
                            "%s Evaluation of keywordDefine of (%s:\n",
                            lineno(), keywords[curr_macro]);
                    fprintf(stderr,
                            "    (formal %s (optional returns %s!\n",
                            formal_asgn,
                            (*vp) ? "several values" : "empty value");
                    err_count++;
                }
            }
            if (call)
                call = call->next;
        }
    formal_asgn = NULL;
    if (call || f) {
        fprintf(stderr, "%s macro call (%s has too %s arguments!\n",
                lineno(), keywords[curr_macro], call ? "many" : "few");
        err_count++;
    }
    if (debug >= 4)
        for (rp = &result; *rp; rp = &((*rp)->next)) {
            fprintf(stderr, "call (%s, arg '%s' =", keywords[curr_macro],
                    (*rp)->data.tree->data.token);
            for (vp = &((*rp)->data.tree->next); *vp; vp = &((*vp)->next))
                fprintf(stderr, " %s", treetxt(*vp));
            fprintf(stderr, "\n");
        }
    return (result);
}

/* create parsetree from a macro-body definition and an assigned parameter set*/
PARSETREE *buildmacro(args, body)
PARSETREE *args, *body;
{
    PARSETREE *result, **pr, *pf, *pa, **pp;
    char *parname;
    int i, n;

    DEBUG(5) "Entering buildmacro: curr_macro=%s, formal_asgn=%s\n",
        keywords[curr_macro], formal_asgn ? formal_asgn : "-");

    for (result = NULL, pr = &result; body; body = body->next)
        switch (body->type) {
        case Kbuild:
            DEBUG(6) "    processing (build %s\n",
                body->data.tree->data.token);
            *pr = get_parsetree();
            (*pr)->type = key_hash(body->data.tree->data.token, 0);
            if ((*pr)->type == KEYWORD) {       /* yet unknown keyword: edif input error */
                fprintf(stderr,
                        "%s and %d: (build of unknown keyword '%s', form skipped!\n",
                        lineno(), body->line, body->data.tree->data.token);
                err_count++;
                put_parsetree(*pr);
                *pr = NULL;
            } else {            /* O.K. */
                (*pr)->data.tree = buildmacro(args, body->data.tree->next);
                pr = &((*pr)->next);
            }
            break;
        case Kactual:
            DEBUG(6) "    processing (actual %s\n",
                body->data.tree->data.token);
            parname = body->data.tree->data.token;
            pf = find_actual(args, parname);
            if (!pf)
                break;          /* error message already generated */
            if (pf->type == KforEach) { /* take one element of forEach */
                *pr = cpy_parsetree(pf->data.tree->next);
                pr = &((*pr)->next);
            } else {            /* take actual list */
                for (pf = pf->data.tree->next; pf; pf = pf->next) {
                    *pr = cpy_parsetree(pf);
                    pr = &((*pr)->next);
                }
            }
            break;
        case Kliteral:
            DEBUG(6) "    processing (literal ...\n");
            for (pf = body->data.tree; pf; pf = pf->next) {
                *pr = cpy_parsetree(pf);
                pr = &((*pr)->next);
            }
            break;
        case KforEach:
            keywordlevelused = 3;
                /*** initialise loop ****/
            DEBUG(6) "    starting (forEach\n");
            pa = NULL;
            pf = body->data.tree;
            if (pf->type == KformalList)
                pf = pf->data.tree;
            while (pf && pf->type == IDENT) {
                pa = find_actual(args, pf->data.token);
                if (!pa)
                    break;
                /* set forEach loop flag */
                pa->type = KforEach;
                /* create cycle of actual values, count items */
                n = 0;
                for (pp = &(pa->data.tree->next); *pp; pp = &((*pp)->next))
                    n++;
                *pp = pa->data.tree->next;
                pf = pf->next;
            }
            if (!pa)
                break;
                /***** iterate body ******/
            for (i = 0; i < n; i++) {
                *pr = buildmacro(args, body->data.tree->next);
                while (*pr)
                    pr = &((*pr)->next);

                /* next actual values  by rotating list */
                pf = body->data.tree;
                if (pf->type == KformalList)
                    pf = pf->data.tree;
                while (pf && pf->type == IDENT) {
                    pa = find_actual(args, pf->data.token);
                    pa = pa->data.tree;
                    pa->next = pa->next->next;
                    pf = pf->next;
                }
                DEBUG(6) "    foreach: cycle %d finished\n", i);
            }
                /***** terminate loop ******/
            pf = body->data.tree;
            if (pf->type == KformalList)
                pf = pf->data.tree;
            while (pf && pf->type == IDENT) {
                pa = find_actual(args, pf->data.token);
                pa->type = Kformal;
                /* break cycle again in normal list */
                for (pp = &(pa->data.tree->next), i = 0; i < n;
                     pp = &((*pp)->next), i++);
                *pp = NULL;
                pf = pf->next;
            }
            break;
        case Kcomment:
            break;
        }
    DEBUG(5) "buildmacro exit: makes )\n");
    return (result);
}

PARSETREE *find_actual(args, name)
PARSETREE *args;
char *name;
{
    PARSETREE *pf;

    for (pf = args; pf && pf->data.tree->data.token != name; pf = pf->next);    /* find actual argument */
    if (!pf) {
        fprintf(stderr, "%s Evaluating keywordDefinition '(%s':\n",
                lineno(), keywords[curr_macro]);
        if (formal_asgn)
            fprintf(stderr, "   (formal %s illegally references %s!\n",
                    formal_asgn, name);
        else
            fprintf(stderr, "   formal parameter %s unknown!\n", name);
        err_count++;
    }
    return (pf);
}

PARSETREE *cpy_parsetree(org)
PARSETREE *org;
{
    PARSETREE *cpy, **p;

    if (!org)
        return (NULL);

    cpy = get_parsetree();
    cpy->type = org->type;
    switch (cpy->type) {
    case IDENT:
    case STRING:
        cpy->data.token = org->data.token;
        break;
    case NUMBER:
        cpy->data.ival = org->data.ival;
        break;
    }

    if (!ISKEYWORD(org->type))
        return (cpy);

    for (org = org->data.tree, p = &(cpy->data.tree); org;
         org = org->next, p = &((*p)->next))
        *p = cpy_parsetree(org);

    return (cpy);
}

/**** aid for generating err messages ***/
char *lineno() {
    static char buf[20];

    if (mlines[1] > mlines[0])
        sprintf(buf, "Line %d-%d:", mlines[0], mlines[1]);
    else
        sprintf(buf, "Line %d:", yy_create_edf_lineno);
    return (buf);
}

char *tokentxt() {
    char c1, c2, *pc;

    switch (token->type) {
    case STRING:
        if (mlines[0])
            strcpy(yy_create_edf_text, token->data.token);
        if (strlen(yy_create_edf_text) > 25)
            strcpy(yy_create_edf_text + 22, "...");
        return (yy_create_edf_text);
    case IDENT:
        if (mlines[0])
            return (token->data.token);
        else
            return (yy_create_edf_text);
    case NUMBER:
        if (mlines[0])
            sprintf(yy_create_edf_text, "%d", token->data.ival);
        return (yy_create_edf_text);
    case ENDLIST:
        return (")");
    case ENDFILE:
        return ("<end-of-file>");
    default:                   /* must be keyword */
        if (mlines[0])
            sprintf(yy_create_edf_text, "(%s", keywords[token->type]);
        else {                  /* insert '(' */
            for (c1 = '(', pc = yy_create_edf_text; c1;
                 c2 = *pc, *pc++ = c1, c1 = c2);
            *pc = '\0';
        }
        return (yy_create_edf_text);
    }
}
