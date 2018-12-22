#include <stdio.h>
#include <string.h>
#include "defines.h"
#include "types.h"

extern char *sbrk();
extern int debug;
extern char *treetxt();

#define NPARSETREE 128          /* allocate PARSETREE structs in bundled units */
static PARSETREE *parselist;    /* list of returned structs */

#define NHASHSTRUCT 128         /* allocate HASHSTRUCTs in bundled units */
#define STRINGSPACE 2000        /* allocate string space in blocks */
#if (STRINGSPACE<=YYLMAX)
cpp:ERROR "YYLMAX" is set too small ! ! !
#endif
static int memalloc;

PARSETREE *get_parsetree()
{
    static PARSETREE *parsearray;
    static int array_free;
    PARSETREE *p;

    if (parselist) {            /* structs available on returned list */
        p = parselist;
        parselist = p->next;
        p->type = p->data.ival = 0;
        p->next = NULL;
        return (p);
    }
    if (array_free == 0) {
        parsearray =
            (PARSETREE *) calloc((unsigned) NPARSETREE, sizeof(PARSETREE));
        if (!parsearray) {
            fprintf(stderr, "Memory allocation failed!\n");
            terminate(2);
        }
        array_free = NPARSETREE;
        memalloc += NPARSETREE * sizeof(PARSETREE);
    }
    array_free--;
    return (parsearray++);
}

/***** free a parsetree struct, return value of original next field ***/
PARSETREE *put_parsetree(p)
PARSETREE *p;
{
    PARSETREE *l;
    int type;

    if (!p)
        return (NULL);

    type = p->type;
    if (type == IN_FREE_LIST) {
        fprintf(stderr,
                "BUG (put_parsetree): p=%d, ALREADY IN FREE LIST!\n", p);
        return (NULL);
    }

    DEBUG(9) "put_parsetree: p=%d, type='%s'\n", p, treetxt(p));
    p->type = IN_FREE_LIST;

        /**** first free descenders ***/
    if (ISKEYWORD(type))
        for (l = p->data.tree; l; l = put_parsetree(l));

        /***** now free this item *****/
    l = p->next;
    p->next = parselist;
    parselist = p;
    return (l);
}

static VARIABLE *vlist;

VARIABLE *get_variable() {
    VARIABLE *v;
    char *c;
    int i;

    if (!vlist)
        v = (VARIABLE *) calloc(1, sizeof(VARIABLE));
    else {
        v = vlist;
        vlist = vlist->next;
        for (c = (char *) v, i = 0; i < sizeof(VARIABLE); i++)
            *c++ = '\0';
    }
    return (v);
}

VARIABLE *put_variable(p)
VARIABLE *p;
{
    VARIABLE *v;

    v = p->next;
    p->next = vlist;
    vlist = p;

    return (v);
}

HASHSTRUCT *get_hashstruct() {
    static HASHSTRUCT *hasharray;
    static int hash_free;

    if (hash_free == 0) {
        hasharray =
            (HASHSTRUCT *) calloc((unsigned) NHASHSTRUCT,
                                  sizeof(HASHSTRUCT));
        if (!hasharray) {
            fprintf(stderr, "Memory allocation failed!\n");
            terminate(2);
        }
        hash_free = NHASHSTRUCT;
        memalloc += NHASHSTRUCT * sizeof(HASHSTRUCT);
    }
    hash_free--;
    return (hasharray++);
}

char *new_string(c)             /* generate a new copy of *c */
char *c;
{
    static char *stringspace;
    static int string_free;
    int l;

    l = strlen(c) + 1;

    if (string_free < l) {
        stringspace = malloc((unsigned) STRINGSPACE);
        if (!stringspace) {
            fprintf(stderr, "Memory allocation failed!\n");
            terminate(2);
        }
        string_free = STRINGSPACE;
        memalloc += STRINGSPACE;
    }
    string_free -= l;
    strcpy(stringspace + string_free, c);
    return (stringspace + string_free);
}

char *mycalloc(n, size)
int n, size;
{
    char *c;

    c = calloc(n, size);
    if (!c) {
        fprintf(stderr, "Memory allocation failed!\n");
        terminate(2);
    }
    memalloc += n * size;
    return (c);
}

void mem_stats() {
/*	fprintf( stderr, "The systems idea on my memory space is %d bytes.\n",
		(int)sbrk(1));
*/
    fprintf(stderr, "I runtime allocated %d bytes memory.\n", memalloc);
}
