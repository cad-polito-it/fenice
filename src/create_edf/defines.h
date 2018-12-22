
/*** general programming aids ***/
#define TRUE  -1
#define FALSE 0
#define Case break;case
#define Default break;default
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG(k) if (debug >= (k)) fprintf( stderr,

/*** return values of yylex(), indicating type of item read, stored in token->type **/
#define ENDFILE	0               /* yylex() always returns 0 at end-of-file! */
#define NUMBER 	-1
#define IDENT	-2
#define VALUEREF -3
#define STRING	-4
#define ENDLIST	-5
#define KEYWORD	-6              /* individual keywords are numbered from 1 up */
#define IN_FREE_LIST -7         /* structure not used now, available for later */
#define ISKEYWORD(type) ((type)>0)      /*must work on all recognised keywords! */

/*** reduction of edif complexity ****/
#define LOGICMODEL 1
#define GRAPHIC 2
#define COMMENT 4
#define USERDATA 8
#define SUBNET 16
#define LEVEL1 32
#define LEVEL2 64

/*** kinds of variables ***/
#define IS_VAR 1                /* declared as variable */
#define IS_CONST 2              /* declared as constant */
#define IS_PAR 3                /* declared as formal parameter */
#define VAL_ASG 1               /* check whether a value is assigned to a variable */
#define MIN_ASG 2               /* check whether a (mnm) value has a lower bound */
#define MAX_ASG 4               /* check whether a (mnm) value has a upper bound */
#define VAL_UNC 8               /* check whether a nominal value is 'unconstrained' */
#define MIN_UNC 16              /* check whether a lower bound is 'unconstrained' */
#define MAX_UNC 32              /* check whether a upper bound is 'unconstrained' */

/*** constants for array sizes whose boundaries ***/
/*** could be touched by growing user input     ***/
#define MAXDEPTH 256            /* maximum nesting level of ((((  */
#define MAXVIEWS 64             /* maximum number of different view names */
#define MAXKEYWORDS 512         /* maximum # different keywords and aliases */
#define YYLMAX 1024             /* maximum length of lex token */
#define MAXSCOPEDEPTH 32        /* maximum scope nesting depth */
#define MAXVARARRAYSIZE (1<<10) /* maximum number of elements in an (array */

/*** other program constants ****/
#define PRECISION 9             /* number of digits in generating (e construct */
#define INDENT 4                /* indent characters for (key in output printing */
