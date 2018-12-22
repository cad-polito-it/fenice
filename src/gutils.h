/*****************************************************************-*-c-*-*\
*             *                                                           *
*   #####     *  Copyright (c) 2000 Giovanni Squillero                    *
*  ######     *  http://staff.polito.it/giovanni.squillero/               *
*  ###   \    *  giovanni.squillero@polito.it                             *
*   ##G  c\   *                                                           *
*   #     _\  *  This code is licensed under a BSD license.               *
*   |  _/     *  See <https://github.com/squillero/fenice> for details    *
*             *                                                           *
\*************************************************************************/

#ifndef GUTILS_HEADER_ALREADY_INCLUDED

#define GUTILS_HEADER_ALREADY_INCLUDED Yes

#include <stdio.h>
#include <stdlib.h>
#include <Check.h>

#define Free(X)         (X = _Free(X))
#define Malloc(X, Y)    (X = _Malloc(X, Y, #X))
#define Realloc(X, Y)   (X = _Realloc(X, Y, #X))
void           *_Malloc(void *p, size_t size, char *v);
void           *_Realloc(void *p, size_t size, char *v);
void           *_Free(void *ptr);
double          TotalMemAllocated(void);

/*
 * Fancy Print protos & define
 */
void            SetWidth(FILE * F, int w);
void            Print(FILE *, const char *,...);
void            DirtyOutput(FILE * F);
#ifdef HPUX
#define TCAP_NORMAL "\033&d@"	/* Default ANSI escapes */
#define TCAP_BOLDFACE "\033&dH"	/* Half Bright */
#define TCAP_UNDERLINE "\033&dD"
#define TCAP_REVERSE "\033&dB"
#else
#define TCAP_NORMAL "\033[0m"	/* Default ANSI escapes */
#define TCAP_BOLDFACE "\033[1m"
#define TCAP_UNDERLINE "\033[4m"
#define TCAP_REVERSE "\033[7m"
#endif

#endif
