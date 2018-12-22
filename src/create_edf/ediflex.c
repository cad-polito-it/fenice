#include <stdio.h>
#include <ctype.h>

#define U(x) x
#define NLSTATE yy_create_edf_previous=YYNEWLINE
#define BEGIN yy_create_edf_bgin = yy_create_edf_svec + 1 +
#define INITIAL 0
#define YYLERR yy_create_edf_svec
#define YYSTATE (yy_create_edf_estate-yy_create_edf_svec-1)
#define YYOPTIM 1
#define YYLMAX BUFSIZ
#define output(c) putc(c,yy_create_edf_out)
#define input() (((yy_create_edf_tchar=yy_create_edf_sptr>yy_create_edf_sbuf?U(*--yy_create_edf_sptr):getc(yy_create_edf_in))==10?(yy_create_edf_lineno++,yy_create_edf_tchar):yy_create_edf_tchar)==EOF?0:yy_create_edf_tchar)
#define unput(c) {yy_create_edf_tchar= (c);if(yy_create_edf_tchar=='\n')yy_create_edf_lineno--;*yy_create_edf_sptr++=yy_create_edf_tchar;}
#define yy_create_edf_more() (yy_create_edf_morfg=1)
#define ECHO fprintf(yy_create_edf_out, "%s",yy_create_edf_text)
#define REJECT { nstr = yy_create_edf_reject(); goto yy_create_edf_fussy;}
int yy_create_edf_leng;
extern char yy_create_edf_text[];
int yy_create_edf_morfg;
extern char *yy_create_edf_sptr, yy_create_edf_sbuf[];
int yy_create_edf_tchar;
static FILE *yy_create_edf_in = NULL, *yy_create_edf_out = NULL;

static void init_streams(void) __attribute__ ((constructor));
static void init_streams(void)
{
    yy_create_edf_in = stdin;
    yy_create_edf_out = stdout;
}

int yy_create_edf_wrap();
int yy_create_edf_look();
int yy_create_edf_back(int *p, int m);

extern int yy_create_edf_lineno;
struct yy_create_edf_svf {
    struct yy_create_edf_work *yy_create_edf_stoff;
    struct yy_create_edf_svf *yy_create_edf_other;
    int *yy_create_edf_stops;
};
struct yy_create_edf_svf *yy_create_edf_estate;
extern struct yy_create_edf_svf yy_create_edf_svec[], *yy_create_edf_bgin;

#undef YYLMAX
#include "defines.h"
extern int err_count;
extern FILE *edifp;

#undef input
#define input() (((yy_create_edf_tchar=yy_create_edf_sptr>yy_create_edf_sbuf?U(*--yy_create_edf_sptr):toupper(getc(edifp)))==10?(yy_create_edf_lineno++,yy_create_edf_tchar):yy_create_edf_tchar)==EOF?0:yy_create_edf_tchar)
#define KEY 2
#define STR 4
#define YYNEWLINE 10
int yy_create_edf_lex()
{
    int nstr;
    extern int yy_create_edf_previous;

    while ((nstr = yy_create_edf_look()) >= 0)
      yy_create_edf_fussy:switch (nstr) {
        case 0:
            if (yy_create_edf_wrap())
                return (0);
            break;
        case 1:
            ;
            break;
        case 2:
            {
                BEGIN 0;
                return (KEYWORD);
            }
            break;
        case 3:
            {
                fprintf(stderr, "Line %d: '%c' illegal after '('!\n",
                        yy_create_edf_lineno, yy_create_edf_text[0]);
                BEGIN 0;
                err_count++;
            }
            break;
        case 4:
            BEGIN KEY;
            break;
        case 5:
            return (IDENT);
            break;
        case 6:
            return (NUMBER);
            break;
        case 7:
            return (STRING);
            break;
        case 8:
            return (ENDLIST);
            break;
        case 9:
            {
                fprintf(stderr, "Line %d: Illegal character '%c'!\n",
                        yy_create_edf_lineno, yy_create_edf_text[0]);
                err_count++;
            }
            break;
        case 10:
            {
                fprintf(stderr, "Line %d: Illegal character octal %o!\n",
                        yy_create_edf_lineno, yy_create_edf_text[0]);
                err_count++;
            }
            break;
        case -1:
            break;
        default:
            fprintf(yy_create_edf_out, "bad switch yy_create_edf_look %d",
                    nstr);
        }
    return (0);
}

/* end of yy_create_edf_lex */
int yy_create_edf_wrap()
{
    return (1);
}

int yy_create_edf_vstop[] = {
    0,

    10,
    0,

    1,
    10,
    0,

    1,
    0,

    9,
    10,
    0,

    9,
    10,
    0,

    5,
    9,
    10,
    0,

    4,
    9,
    10,
    0,

    8,
    9,
    10,
    0,

    9,
    10,
    0,

    6,
    10,
    0,

    5,
    10,
    0,

    3,
    9,
    10,
    0,

    3,
    9,
    10,
    0,

    2,
    3,
    5,
    9,
    10,
    0,

    3,
    4,
    9,
    10,
    0,

    3,
    8,
    9,
    10,
    0,

    3,
    9,
    10,
    0,

    3,
    6,
    10,
    0,

    2,
    5,
    10,
    0,

    7,
    0,

    5,
    0,

    6,
    0,

    2,
    5,
    0,
    0
};

#define YYTYPE char
struct yy_create_edf_work {
    YYTYPE verify, advance;
} yy_create_edf_crank[] = {
0, 0, 0, 0, 1, 7, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 8, 1, 9,
        0, 0, 8, 9, 8, 9, 0, 0,
        0, 0, 8, 9, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 10, 1, 11,
        8, 9, 0, 0, 1, 7, 1, 12,
        0, 0, 1, 13, 1, 14, 0, 0,
        1, 15, 2, 13, 2, 14, 3, 18,
        3, 19, 1, 16, 5, 13, 5, 14,
        3, 20, 0, 0, 3, 21, 3, 22,
        0, 0, 3, 23, 6, 13, 6, 14,
        4, 18, 4, 19, 3, 24, 0, 0,
        0, 0, 4, 20, 1, 17, 4, 21,
        4, 22, 0, 0, 4, 23, 0, 0,
        0, 0, 0, 0, 0, 0, 4, 24,
        0, 0, 0, 0, 0, 0, 3, 25,
        15, 29, 15, 29, 15, 29, 15, 29,
        15, 29, 15, 29, 15, 29, 15, 29,
        15, 29, 15, 29, 0, 0, 0, 0,
        4, 25, 0, 0, 0, 0, 0, 0,
        1, 10, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 3, 18, 11, 0, 11, 0,
        11, 0, 11, 0, 11, 0, 11, 0,
        11, 0, 11, 0, 11, 26, 11, 26,
        11, 0, 11, 0, 4, 18, 11, 0,
        11, 0, 11, 0, 11, 0, 11, 0,
        11, 0, 11, 0, 11, 0, 11, 0,
        11, 0, 11, 0, 11, 0, 11, 0,
        11, 0, 11, 0, 11, 0, 11, 0,
        11, 0, 0, 0, 11, 26, 11, 27,
        0, 0, 0, 0, 11, 26, 11, 26,
        0, 0, 0, 0, 0, 0, 0, 0,
        11, 26, 0, 0, 0, 0, 0, 0,
        0, 0, 11, 26, 12, 28, 12, 28,
        12, 28, 12, 28, 12, 28, 12, 28,
        12, 28, 12, 28, 12, 28, 12, 28,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 11, 26, 12, 28,
        12, 28, 12, 28, 12, 28, 12, 28,
        12, 28, 12, 28, 12, 28, 12, 28,
        12, 28, 12, 28, 12, 28, 12, 28,
        12, 28, 12, 28, 12, 28, 12, 28,
        12, 28, 12, 28, 12, 28, 12, 28,
        12, 28, 12, 28, 12, 28, 12, 28,
        12, 28, 0, 0, 0, 0, 0, 0,
        11, 26, 12, 28, 0, 0, 12, 28,
        12, 28, 12, 28, 12, 28, 12, 28,
        12, 28, 12, 28, 12, 28, 12, 28,
        12, 28, 12, 28, 12, 28, 12, 28,
        12, 28, 12, 28, 12, 28, 12, 28,
        12, 28, 12, 28, 12, 28, 12, 28,
        12, 28, 12, 28, 12, 28, 12, 28,
        12, 28, 0, 0, 0, 0, 0, 0,
        11, 0, 19, 0, 19, 0, 19, 0,
        19, 0, 19, 0, 19, 0, 19, 0,
        19, 0, 0, 0, 0, 0, 19, 0,
        19, 0, 0, 0, 19, 0, 19, 0,
        19, 0, 19, 0, 19, 0, 19, 0,
        19, 0, 19, 0, 19, 0, 19, 0,
        19, 0, 19, 0, 19, 0, 19, 0,
        19, 0, 19, 0, 19, 0, 19, 0,
        20, 30, 20, 30, 20, 30, 20, 30,
        20, 30, 20, 30, 20, 30, 20, 30,
        20, 30, 20, 30, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 20, 30, 20, 30, 20, 30,
        20, 30, 20, 30, 20, 30, 20, 30,
        20, 30, 20, 30, 20, 30, 20, 30,
        20, 30, 20, 30, 20, 30, 20, 30,
        20, 30, 20, 30, 20, 30, 20, 30,
        20, 30, 20, 30, 20, 30, 20, 30,
        20, 30, 20, 30, 20, 30, 0, 0,
        0, 0, 0, 0, 0, 0, 20, 30,
        0, 0, 20, 30, 20, 30, 20, 30,
        20, 30, 20, 30, 20, 30, 20, 30,
        20, 30, 20, 30, 20, 30, 20, 30,
        20, 30, 20, 30, 20, 30, 20, 30, 20, 30, 20, 30, 20, 30, 20, 30,
        20, 30, 20, 30, 20, 30, 20, 30, 20, 30, 20, 30, 20, 30, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 0, 26, 0, 26, 0,
        26, 0, 26, 0, 26, 0, 26, 0, 26, 0, 0, 0, 0, 0, 26, 0, 26, 0,
        19, 0, 26, 0, 26, 0, 26, 0, 26, 0, 26, 0, 26, 0, 26, 0, 26, 0,
        26, 0, 26, 0, 26, 0, 26, 0, 26, 0, 26, 0, 26, 0, 26, 0, 26, 0,
        26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 26, 0, 0, 0, 0, 0, 0, 0};
struct yy_create_edf_svf yy_create_edf_svec[] = {
    0, 0, 0,
    yy_create_edf_crank + -1, 0, 0,
    yy_create_edf_crank + -5, yy_create_edf_svec + 1, 0,
    yy_create_edf_crank + -14, yy_create_edf_svec + 1, 0,
    yy_create_edf_crank + -27, yy_create_edf_svec + 1, 0,
    yy_create_edf_crank + -10, yy_create_edf_svec + 1, 0,
    yy_create_edf_crank + -18, yy_create_edf_svec + 1, 0,
    yy_create_edf_crank + 0, 0, yy_create_edf_vstop + 1,
    yy_create_edf_crank + 4, 0, yy_create_edf_vstop + 3,
    yy_create_edf_crank + 0, yy_create_edf_svec + 8,
        yy_create_edf_vstop + 6,
    yy_create_edf_crank + 0, 0, yy_create_edf_vstop + 8,
    yy_create_edf_crank + -109, 0, yy_create_edf_vstop + 11,
    yy_create_edf_crank + 110, 0, yy_create_edf_vstop + 14,
    yy_create_edf_crank + 0, 0, yy_create_edf_vstop + 18,
    yy_create_edf_crank + 0, 0, yy_create_edf_vstop + 22,
    yy_create_edf_crank + 32, 0, yy_create_edf_vstop + 26,
    yy_create_edf_crank + 0, yy_create_edf_svec + 15,
        yy_create_edf_vstop + 29,
    yy_create_edf_crank + 0, yy_create_edf_svec + 12,
        yy_create_edf_vstop + 32,
    yy_create_edf_crank + 0, 0, yy_create_edf_vstop + 35,
    yy_create_edf_crank + -236, yy_create_edf_svec + 11,
        yy_create_edf_vstop + 39,
    yy_create_edf_crank + 220, 0, yy_create_edf_vstop + 43,
    yy_create_edf_crank + 0, 0, yy_create_edf_vstop + 49,
    yy_create_edf_crank + 0, 0, yy_create_edf_vstop + 54,
    yy_create_edf_crank + 0, yy_create_edf_svec + 15,
        yy_create_edf_vstop + 59,
    yy_create_edf_crank + 0, yy_create_edf_svec + 15,
        yy_create_edf_vstop + 63,
    yy_create_edf_crank + 0, yy_create_edf_svec + 20,
        yy_create_edf_vstop + 67,
    yy_create_edf_crank + -350, yy_create_edf_svec + 11, 0,
    yy_create_edf_crank + 0, 0, yy_create_edf_vstop + 71,
    yy_create_edf_crank + 0, yy_create_edf_svec + 12,
        yy_create_edf_vstop + 73,
    yy_create_edf_crank + 0, yy_create_edf_svec + 15,
        yy_create_edf_vstop + 75,
    yy_create_edf_crank + 0, yy_create_edf_svec + 20,
        yy_create_edf_vstop + 77,
    0, 0, 0
};

struct yy_create_edf_work *yy_create_edf_top = yy_create_edf_crank + 477;
struct yy_create_edf_svf *yy_create_edf_bgin = yy_create_edf_svec + 1;

char yy_create_edf_match[] = {
    00, 01, 01, 01, 01, 01, 01, 01,
    01, 011, 012, 01, 01, 011, 01, 01,
    01, 01, 01, 01, 01, 01, 01, 01,
    01, 01, 01, 01, 01, 01, 01, 01,
    011, '!', '"', '!', '!', '%', '&', '!',
    '!', '!', '!', '+', '!', '+', '!', '!',
    '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '!', '!', '!', '!', '!', '!',
    '!', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
    'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
    'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
    'A', 'A', 'A', '!', '!', '!', '!', '_',
    '!', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
    'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
    'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
    'A', 'A', 'A', '!', '!', '!', '!', 01,
    0
};

char yy_create_edf_extra[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0
};

#ifndef lint
static char ncform_sccsid[] = "@(#)ncform 1.6 88/02/08 SMI";    /* from S5R2 1.2 */
#endif

int yy_create_edf_lineno = 1;

#define YYU(x) x
#define NLSTATE yy_create_edf_previous=YYNEWLINE
char yy_create_edf_text[YYLMAX];
struct yy_create_edf_svf *yy_create_edf_lstate[YYLMAX],
    **yy_create_edf_lsp, **yy_create_edf_olsp;
char yy_create_edf_sbuf[YYLMAX];
char *yy_create_edf_sptr = yy_create_edf_sbuf;
int *yy_create_edf_fnd;
extern struct yy_create_edf_svf *yy_create_edf_estate;
int yy_create_edf_previous = YYNEWLINE;
int yy_create_edf_look()
{
    register struct yy_create_edf_svf *yy_create_edf_state, **lsp;
    register struct yy_create_edf_work *yy_create_edf_t;
    struct yy_create_edf_svf *yy_create_edf_z;
    int yy_create_edf_ch, yy_create_edf_first;
    struct yy_create_edf_work *yy_create_edf_r;

#ifdef LEXDEBUG
    int debug;
#endif
    char *yy_create_edf_lastch;

    /* start off machines */
#ifdef LEXDEBUG
    debug = 0;
#endif
    yy_create_edf_first = 1;
    if (!yy_create_edf_morfg)
        yy_create_edf_lastch = yy_create_edf_text;
    else {
        yy_create_edf_morfg = 0;
        yy_create_edf_lastch = yy_create_edf_text + yy_create_edf_leng;
    }
    for (;;) {
        lsp = yy_create_edf_lstate;
        yy_create_edf_estate = yy_create_edf_state = yy_create_edf_bgin;
        if (yy_create_edf_previous == YYNEWLINE)
            yy_create_edf_state++;
        for (;;) {
#ifdef LEXDEBUG
            if (debug)
                fprintf(yy_create_edf_out, "state %d\n",
                        yy_create_edf_state - yy_create_edf_svec - 1);
#endif
            yy_create_edf_t = yy_create_edf_state->yy_create_edf_stoff;
            if (yy_create_edf_t == yy_create_edf_crank && !yy_create_edf_first) {       /* may not be any transitions */
                yy_create_edf_z = yy_create_edf_state->yy_create_edf_other;
                if (yy_create_edf_z == 0)
                    break;
                if (yy_create_edf_z->yy_create_edf_stoff ==
                    yy_create_edf_crank)
                    break;
            }
            *yy_create_edf_lastch++ = yy_create_edf_ch = input();
            yy_create_edf_first = 0;
          tryagain:
#ifdef LEXDEBUG
            if (debug) {
                fprintf(yy_create_edf_out, "char ");
                allprint(yy_create_edf_ch);
                putchar('\n');
            }
#endif
            yy_create_edf_r = yy_create_edf_t;
            if ((int) yy_create_edf_t > (int) yy_create_edf_crank) {
                yy_create_edf_t = yy_create_edf_r + yy_create_edf_ch;
                if (yy_create_edf_t <= yy_create_edf_top
                    && yy_create_edf_t->verify + yy_create_edf_svec ==
                    yy_create_edf_state) {
                    if (yy_create_edf_t->advance + yy_create_edf_svec == YYLERR) {      /* error transitions */
                        unput(*--yy_create_edf_lastch);
                        break;
                    }
                    *lsp++ = yy_create_edf_state =
                        yy_create_edf_t->advance + yy_create_edf_svec;
                    goto contin;
                }
            }
#ifdef YYOPTIM
            else if ((int) yy_create_edf_t < (int) yy_create_edf_crank) {       /* r < yy_create_edf_crank */
                yy_create_edf_t = yy_create_edf_r =
                    yy_create_edf_crank + (yy_create_edf_crank -
                                           yy_create_edf_t);
#ifdef LEXDEBUG
                if (debug)
                    fprintf(yy_create_edf_out, "compressed state\n");
#endif
                yy_create_edf_t = yy_create_edf_t + yy_create_edf_ch;
                if (yy_create_edf_t <= yy_create_edf_top
                    && yy_create_edf_t->verify + yy_create_edf_svec ==
                    yy_create_edf_state) {
                    if (yy_create_edf_t->advance + yy_create_edf_svec == YYLERR) {      /* error transitions */
                        unput(*--yy_create_edf_lastch);
                        break;
                    }
                    *lsp++ = yy_create_edf_state =
                        yy_create_edf_t->advance + yy_create_edf_svec;
                    goto contin;
                }
                yy_create_edf_t =
                    yy_create_edf_r +
                    YYU(yy_create_edf_match[yy_create_edf_ch]);
#ifdef LEXDEBUG
                if (debug) {
                    fprintf(yy_create_edf_out, "try fall back character ");
                    allprint(YYU(yy_create_edf_match[yy_create_edf_ch]));
                    putchar('\n');
                }
#endif
                if (yy_create_edf_t <= yy_create_edf_top
                    && yy_create_edf_t->verify + yy_create_edf_svec ==
                    yy_create_edf_state) {
                    if (yy_create_edf_t->advance + yy_create_edf_svec == YYLERR) {      /* error transition */
                        unput(*--yy_create_edf_lastch);
                        break;
                    }
                    *lsp++ = yy_create_edf_state =
                        yy_create_edf_t->advance + yy_create_edf_svec;
                    goto contin;
                }
            }
            if ((yy_create_edf_state =
                 yy_create_edf_state->yy_create_edf_other)
                && (yy_create_edf_t =
                    yy_create_edf_state->yy_create_edf_stoff) !=
                yy_create_edf_crank) {
#ifdef LEXDEBUG
                if (debug)
                    fprintf(yy_create_edf_out, "fall back to state %d\n",
                            yy_create_edf_state - yy_create_edf_svec - 1);
#endif
                goto tryagain;
            }
#endif
            else {
                unput(*--yy_create_edf_lastch);
                break;
            }
          contin:
#ifdef LEXDEBUG
            if (debug) {
                fprintf(yy_create_edf_out, "state %d char ",
                        yy_create_edf_state - yy_create_edf_svec - 1);
                allprint(yy_create_edf_ch);
                putchar('\n');
            }
#endif
            ;
        }
#ifdef LEXDEBUG
        if (debug) {
            fprintf(yy_create_edf_out, "stopped at %d with ",
                    *(lsp - 1) - yy_create_edf_svec - 1);
            allprint(yy_create_edf_ch);
            putchar('\n');
        }
#endif
        while (lsp-- > yy_create_edf_lstate) {
            *yy_create_edf_lastch-- = 0;
            if (*lsp != 0
                && (yy_create_edf_fnd = (*lsp)->yy_create_edf_stops)
                && *yy_create_edf_fnd > 0) {
                yy_create_edf_olsp = lsp;
                if (yy_create_edf_extra[*yy_create_edf_fnd]) {  /* must backup */
                    while (yy_create_edf_back
                           ((*lsp)->yy_create_edf_stops,
                            -*yy_create_edf_fnd) != 1
                           && lsp > yy_create_edf_lstate) {
                        lsp--;
                        unput(*yy_create_edf_lastch--);
                    }
                }
                yy_create_edf_previous = YYU(*yy_create_edf_lastch);
                yy_create_edf_lsp = lsp;
                yy_create_edf_leng =
                    yy_create_edf_lastch - yy_create_edf_text + 1;
                yy_create_edf_text[yy_create_edf_leng] = 0;
#ifdef LEXDEBUG
                if (debug) {
                    fprintf(yy_create_edf_out, "\nmatch ");
                    sprint(yy_create_edf_text);
                    fprintf(yy_create_edf_out, " action %d\n",
                            *yy_create_edf_fnd);
                }
#endif
                return (*yy_create_edf_fnd++);
            }
            unput(*yy_create_edf_lastch);
        }
        if (yy_create_edf_text[0] == 0 /* && feof(yy_create_edf_in) */ ) {
            yy_create_edf_sptr = yy_create_edf_sbuf;
            return (0);
        }
        yy_create_edf_previous = yy_create_edf_text[0] = input();
        if (yy_create_edf_previous > 0)
            output(yy_create_edf_previous);
        yy_create_edf_lastch = yy_create_edf_text;
#ifdef LEXDEBUG
        if (debug)
            putchar('\n');
#endif
    }
}

int yy_create_edf_back(int *p, int m)
{
    if (p == 0)
        return (0);
    while (*p) {
        if (*p++ == m)
            return (1);
    }
    return (0);
}

        /* the following are only used in the lex library */
int yy_create_edf_input()
{
    return (input());
}

int yy_create_edf_output(c)
int c;
{
    output(c);
}

int yy_create_edf_unput(c)
int c;
{
    unput(c);
}
