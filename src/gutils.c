/*****************************************************************-*-c-*-*\
*             *                                                           *
*   #####     *  (c) Copyright 2000, Giovanni Squillero                   *
*  ######     *  http://staff.polito.it/giovanni.squillero/               *
*  ###   \    *  giovanni.squillero@polito.it                             *
*   ##G  c\   *                                                           *
*   #     _\  *  This code is licensed under a BSD 2-clause license       *
*   |  _/     *  See <https://github.com/squillero/fenice> for details    *
*             *                                                           *
\*************************************************************************/


char           *_GUTILS_V = "1.0 $Revision: 2.0 $";
char           *_GUTILS_F = __FILE__;
char           *_GUTILS_D = __DATE__ " " __TIME__;

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <memory.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#ifndef __LINUX__
#include <stdarg.h>
#endif
#include <varargs.h>
#include <sys/types.h>
#include <sys/time.h>

#include "Fenice.h"

/*
 * PROTO 
 */

#ifdef DEBUG
static double   _TotAllocated;
#endif

typedef struct _STATUS {
    FILE           *_p_Stream;
    int             _p_UseANSI;
    int             _p_ColsPerPage;
    int             _p_IndentOffset;
    int             _p_CurrCol;
    int             _p_BlankLines;
    char           *tcap_normal, *tcap_boldface, *tcap_underline, *tcap_reverse;
    struct _STATUS *Next;
} STATUS;
static STATUS  *StatusList = NULL;

/*
 * STREAM status
 */
static FILE    *_p_Stream;
static int      _p_UseANSI = 0;
static int      _p_ColsPerPage = 80;
static int      _p_IndentOffset = 4;
static int      _p_CurrCol = 0;
static int      _p_BlankLines = 0;
/*
 * ANSI escapes
 */
char           *tcap_normal, *tcap_boldface, *tcap_underline, *tcap_reverse;

/*
 * LOCAL PROTOS 
 */
static void     NewLine(int indent);
static void     PrintWord(char *w);
static int      GetStatus(FILE * F);
static void     InitANSI(FILE * F);
static int      WordLen(char *w);
static void     ParagraphPrint(char *s);
static char    *GetNextWord(char **s);

/* int             fileno(FILE * F); */

static int      GetStatus(FILE * F)
{
    STATUS         *S;

    for (S = StatusList; S && S->_p_Stream != F; S = S->Next);
    if (!S) {
	Malloc(S, sizeof(STATUS));
	S->_p_Stream = F;
	S->Next = StatusList;
	StatusList = S;

	_p_Stream = F;
	_p_ColsPerPage = 80;
	_p_IndentOffset = 4;
	InitANSI(F);
	_p_CurrCol = 0;
	_p_BlankLines = 0;
	return 0;
    } else {
	_p_Stream = S->_p_Stream;
	_p_UseANSI = S->_p_UseANSI;
	_p_ColsPerPage = S->_p_ColsPerPage;
	_p_IndentOffset = S->_p_IndentOffset;
	_p_CurrCol = S->_p_CurrCol;
	_p_BlankLines = S->_p_BlankLines;
	tcap_normal = S->tcap_normal;
	tcap_boldface = S->tcap_boldface;
	tcap_underline = S->tcap_underline;
	tcap_reverse = S->tcap_reverse;
	return 1;
    }
}

static void     SaveStatus(FILE * F)
{
    char           *_FunctionName = "SaveStatus";
    STATUS         *S;

    for (S = StatusList; S && S->_p_Stream != F; S = S->Next);
    if (!S) {
	CheckFalse("INTERNAL ERROR: _p_Stream DOESN'T EXIST");
    } else {
	S->_p_UseANSI = _p_UseANSI;
	S->_p_ColsPerPage = _p_ColsPerPage;
	S->_p_IndentOffset = _p_IndentOffset;
	S->_p_CurrCol = _p_CurrCol;
	S->_p_BlankLines = _p_BlankLines;
	S->tcap_normal = tcap_normal;
	S->tcap_boldface = tcap_boldface;
	S->tcap_underline = tcap_underline;
	S->tcap_reverse = tcap_reverse;
    }
}

void            SetWidth(FILE * F, int w)
{
    GetStatus(F);
    _p_ColsPerPage = w;
    SaveStatus(F);
}

static void     InitANSI(FILE * F)
{
    char           *cp;
    int             toatty;

    toatty = isatty(fileno(F));
    _p_ColsPerPage = 80;

    _p_UseANSI = 0;
    if ((cp = getenv("TERM")) != NULL) {
	if ((*cp == 'v' && cp[1] == 't')
	    || (*cp == 'c' && cp[1] == 'o' && cp[2] == 'n')
	    || (strcmp(cp, "dtterm") == 0)
	    || (strcmp(cp, "xterm") == 0))
	    _p_UseANSI = 1;
    }
    if (!toatty || !_p_UseANSI)
	tcap_normal = tcap_boldface = tcap_underline = tcap_reverse = "";
    else
	tcap_normal = TCAP_NORMAL,
	    tcap_boldface = TCAP_BOLDFACE,
	    tcap_underline = TCAP_UNDERLINE,
	    tcap_reverse = TCAP_REVERSE;

}

void            Print(FILE * Stream, const char *template,...)
{
    char           *_FunctionName = "Print";
    va_list         ap;
    char            temp[1024];
    char            buffer[1024];
    char           *b;
    int             RequiredBL;
    int             AddedBL;
    int             t;

    CheckTrue(Stream);
    CheckTrue(template);
    CheckTrue(strlen(template) < 1024);
    GetStatus(Stream);

    strcpy(temp, template);
    for (b = temp; *b; ++b)
	if (*b == '%' &&
	    (*(b + 1) == 'U' || *(b + 1) == 'B' || *(b + 1) == 'N'
	     || *(b + 1) == 'R' || *(b + 1) == '=' || *(b + 1) == 'T'))
	    *b = '\b';
    for (AddedBL = 0, t = strlen(temp) - 1; t >= 0 && temp[t] == '\n'; --t)
	++AddedBL, temp[t] = 0;

    va_start(ap, template);
    vsprintf(buffer, temp, ap);
    va_end(ap);

    for (RequiredBL = 0;
	 buffer[RequiredBL] && buffer[RequiredBL] == '\n';
	 ++RequiredBL);

    for (; _p_BlankLines < RequiredBL - 1; ++_p_BlankLines)
	NewLine(0);

    ParagraphPrint(&buffer[RequiredBL]);

    for (; AddedBL; --AddedBL)
	NewLine(0);

    fflush(_p_Stream);
    SaveStatus(_p_Stream);
}

static char    *GetNextWord(char **s)
{
    static char     word[128];
    char           *c;

    for (c = word; **s && **s != ' '; ++*s)
	*c++ = **s;
    *c = 0;
    if (**s)
	++ * s;

    return word;
}

static void     ParagraphPrint(char *s)
{
    char           *w;

    while (*s) {
	w = GetNextWord(&s);
	if (_p_ColsPerPage && _p_CurrCol + WordLen(w) > _p_ColsPerPage)
	    NewLine(_p_IndentOffset);
	PrintWord(w);
    }
}

static int      WordLen(char *w)
{
    int             len;

    for (len = 0; *w && *w != '\n'; ++w)
	if (*w == '\b')
	    ++w;
	else
	    ++len;

    return len + 1;
}

static void     PrintWord(char *w)
{
    char           *_FunctionName = "PrintWord";
    char            word[128];
    int             wl;
    int             tab;

    wl = 0;
    while (*w) {
	if (*w == '\b') {
	    word[wl] = 0, fprintf(_p_Stream, word), wl = 0;
	    switch (*++w) {
	      case 'T':
		  tab = 10 * (*++w - '0');
		  tab += *++w - '0';
		  for (; _p_CurrCol % tab; ++_p_CurrCol)
		      fputc(' ', _p_Stream);
		  break;
	      case 'N':
		  fprintf(_p_Stream, "%s", tcap_normal);
		  break;
	      case 'U':
		  fprintf(_p_Stream, "%s", tcap_underline);
		  break;
	      case 'B':
		  fprintf(_p_Stream, "%s", tcap_boldface);
		  break;
	      case 'R':
		  fprintf(_p_Stream, "%s", tcap_reverse);
		  break;
	      case '=':
		  for (; _p_CurrCol < _p_ColsPerPage; ++_p_CurrCol)
		      fputc('-', _p_Stream);
		  break;
	      default:
		  CheckTrue("INTERNAL ERROR");
	    }
	} else if (*w == '\n') {
	    word[wl] = 0, fprintf(_p_Stream, word), wl = 0;
	    NewLine(_p_IndentOffset);
	} else if (*w == '\t') {
	    word[wl] = 0, fprintf(_p_Stream, word), wl = 0;
	    for (; _p_CurrCol % 16; ++_p_CurrCol)
		fputc(' ', _p_Stream);
	} else {
	    word[wl++] = *w;
	    _p_BlankLines = -1;
	    ++_p_CurrCol;
	}
	++w;
    }
    word[wl++] = ' ';
    word[wl] = 0, fprintf(_p_Stream, "%s", word);
    ++_p_CurrCol;
}


static void     NewLine(int indent)
{
    int             t;

    fputc('\n', _p_Stream);
    for (t = 0; t < indent; ++t)
	fputc(' ', _p_Stream);
    _p_CurrCol = indent;
    ++_p_BlankLines;
}

void            DirtyOutput(FILE * F)
{
    GetStatus(F);
    _p_BlankLines = 0;
    _p_CurrCol = 0;
    SaveStatus(F);
}

void           *_Malloc(void *p, size_t size, char *v)
{
#ifdef DEBUG
    char           *_FunctionName = "Malloc";

    if (p) {
	Print(stderr,
	      "\n\n%BWARNING::MEMORY:%N\n"
	  "variable %U%s%N: new chunk of memory (%d byte%s) will shadow "
	      "the previous one at 0x%p\n",
	      v, size, size == 1 ? "" : "s", p);
	BANNER(stderr);
    }
#endif

    p = calloc(size, 1);

#ifdef DEBUG
    if (p) {
	_TotAllocated += size;
    } else {
	Print(stderr,
	      "\n\n%BWARNING::MEMORY:%N\n"
	      "can't malloc %ld bytes for %s\n",
	      size, v);
	BANNER(stderr);
    }
#endif
    return (p);
}

void           *_Realloc(void *p, size_t size, char *v)
{
#ifdef DEBUG
    char           *_FunctionName = "Realloc";

    if (!p) {
	Print(stderr,
	      "\n\n%BWARNING::MEMORY:%N\nvariable %U%s%N is NULL\n", v);
	BANNER(stderr);
    }
#endif

    p = realloc(p, size);

#ifdef DEBUG
    if (p) {
	_TotAllocated += size;
    } else {
	Print(stderr,
	      "\n\n%BWARNING::MEMORY:%N\n"
	      "can't realloc %ld bytes for %s\n",
	      size, v);
	BANNER(stderr);
    }
#endif

    return (p);
}

void           *_Free(void *ptr)
{
    if(ptr)
	free(ptr);

    return (NULL);
}

double          TotalMemAllocated(void)
{
#ifdef DEBUG
    return _TotAllocated;
#else
    return -1.0;
#endif
}
