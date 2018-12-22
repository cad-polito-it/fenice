/***** storing the parsed and macroexpanded edif tree ***/
typedef struct parsetree {
    struct parsetree *next;
    int line;                   /* line number in input file */
    int type;                   /* lex tokens <=0, proper edif (keywords >0 */
    union {
        char *token;            /* for edif strings idents */
        int ival;               /* for edif integers */
        struct parsetree *tree; /* for edif keywords */
    } data;
} PARSETREE, *PARSETREE_P;

typedef struct hashstruct {
    char *token;
    struct hashstruct *next;
} HASHSTRUCT;

typedef struct scale {
    struct scale *next;
    char *unit;
    double factor;
} SCALE;

typedef struct techno {
    struct techno *next;
    char *libname;
    PARSETREE *tree;
    struct variable *scope;
    SCALE *scales;
} TECHNO;

typedef struct parmset {
    struct parmset *next;
    int line;                   /* approximate location of -some- parameterAssign */
    int n_parms;                /* number of variables in the parm list */
    TECHNO *technology;         /* technology of library of caller */
    struct variable *parm;
} PARMSET;

typedef struct view {
    struct view *next;
    char *name;
    PARSETREE *tree;
    int nparmsets;
    PARMSET *parmset;
} VIEW;

typedef struct celltable {
    char *name;
    char *libname;
    PARSETREE *cell;
    TECHNO *technology;
    VIEW *views;
} CELLTABLE;

typedef struct point {
    int x;
    int y;
} POINT;

typedef struct mnm {
    double min;
    double val;
    double max;
    char asg;                   /* keep track of (undefined or (unconstrained */
} MNM;

typedef struct value {
    int type;
    union {
        int i;
        int b;
        double f;
        char *s;
        POINT p;
        MNM mnm;
    } val;
} VALUE;

typedef struct variable {
    struct variable *next;
    char *name;
    int origin;                 /* Kconstant, Kparameter, Kvariable */
    int type;                   /* Kinteger, Kboolean, Knumber, Kpoint, Kstring, KmiNoMax */
    int ndims;                  /* number of dimensions of arry variable */
    int *member;                /* if !=0 specify scalar value of single array member */
    union {
        struct {
            int assigned;
            union {
                int i;
                int b;
                double f;
                char *s;
                POINT p;
                MNM *mnm;
            } val;
        } sca;
        struct {
            int *dim;
            char *assigned;
            union {
                int *i;
                char *b;
                double *f;
                char **s;
                POINT *p;
                MNM *mnm;
            } val;
        } vec;
    } dat;
} VARIABLE;
#define SCA dat.sca.val
#define VEC dat.vec.val
#define SCA_asg dat.sca.assigned
#define VEC_asg dat.vec.assigned
#define VEC_dim dat.vec.dim
