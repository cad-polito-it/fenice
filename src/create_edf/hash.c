#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "types.h"

#define HASHSIZE 10             /* arraylength becomes 2 ** HASHSIZE */
#define MSK	((1<<HASHSIZE)-1)

#if (MSK < MAXKEYWORDS)
cpp: ERROR:increase "HASHSIZE" in hash.c !
#endif
static HASHSTRUCT *hashtable[1 << HASHSIZE];
static char *keytoken[1 << HASHSIZE];
static int keytypenum[1 << HASHSIZE];
static int nstrings;

extern HASHSTRUCT *get_hashstruct();
extern int nkeywords;
extern char *keywords[];
int dont_hash;

char *new_string(char *c);
int strcmp_uncase(char *s1, char *s2);

static int hash_value(name)
char *name;
{
    int c, result = 0;

    while (*name) {
        c = *name++;            /* make hash_value case insensitive */
        if (c >= 'A' && c <= 'Z')
            c += 'a' - 'A';
        result = (result << 2) + c;
        result = (result & MSK) + (result >> HASHSIZE);
    }
    return (result & MSK);
}

char *str_hash(name)
char *name;
{
    int h;
    HASHSTRUCT *p;
    extern char *new_string();

    if (dont_hash > 0)
        return (NULL);
    h = hash_value(name);
    for (p = hashtable[h]; p; p = p->next)
        if (!strcmp_uncase(name, p->token))
            break;

    if (!p) {
/***add structure with name***/
        p = get_hashstruct();
        p->next = hashtable[h];
        hashtable[h] = p;
        /* share namespace with keywords: needed for symbolic const */
        while (keytoken[h] && strcmp_uncase(keytoken[h], name))
            h = (h + 1) & MSK;
        if (keytoken[h])
            p->token = keytoken[h];
        else
            p->token = new_string(name), nstrings++;
    }
    return (p->token);
}

/* special hash for keywords: hashing allows return of typenumber*/
/* always returns the proper typenumber of the keyword. */
/* If new, a new keyword type number is introduced. */
/* Initially all known EDIF keywords are feeded in order. (as new!)*/
/* This scheme must implicitly generate typenumbers in correspondence */
/* with the #defined constant values! */
/* keytoken[] maps hashvalues to keyword strings */
/* keytypenum[] maps hashvalues to keyword typenumbers */
/* keywords[] maps typenumbers to keyword strings */
/* The hash values are not to be used outside this file */
/* If new==0 it refuses to save unknown keywords, returning KEYWORD */
/* If new==1 and an existing name is passed, 0 is returned */
int key_hash(name, new)
char *name;
int new;
{
    int n;

    if (dont_hash > 0)
        return (KEYWORD);
    n = hash_value(name);

    while (keytoken[n] && strcmp_uncase(keytoken[n], name))
        n = (n + 1) & MSK;

    if (!keytoken[n]) {         /* new keyword */
        if (!new)
            return (KEYWORD);
        keytoken[n] = new_string(name);
        if (!keytypenum[n])
            keytypenum[n] = ++nkeywords;
        keywords[keytypenum[n]] = keytoken[n];
    } else if (new)
        return (0);
    return (keytypenum[n]);
}

int strcmp_uncase(s1, s2)
char *s1, *s2;                  /* perform case insensitive string compare */
{
    if (*s1 == '"' || *s2 == '"')       /* real STRINGs remain case sensitive */
        return (strcmp(s1, s2));

    while (*s1 && *s2) {
        if (*s1 != *s2) {
            if (!
                ((*s1 - *s2) == 'a' - 'A' && *s1 >= 'a'
                 || (*s2 - *s1) == 'a' - 'A' && *s2 >= 'a'))
                break;
        }
        s1++;
        s2++;
    }
    return (*s1 || *s2);
}

void create_hash_stats()
{                               /* print hash statistics */
    fprintf(stderr, "Hashing: %d names/strings and %d keywords saved.\n",
            nstrings, nkeywords);
}
