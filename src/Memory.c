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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/times.h>
#include "macros.h"

#include <signal.h>
#include "Messages.h"
#include "Memory.h"

#ifdef GC
#include "gc.h"
#define malloc(X)	GC_MALLOC(X)
#define realloc(X)	GC_REALLOC(X)
#define free(X)	GC_FREE(X)
#endif

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

void *CheckRealloc(void *ptr, size_t size)
{
    register void *r;

    if (!(r = realloc(ptr, size)))
        msgMessage(MSG_ERROR, "Can't realloc %d byte%s", size,
                   size == 1 ? "" : "s");
    return r;
}

void *CheckMalloc(size_t size)
{
    register void *r;

    if (!(r = malloc(size)))
        msgMessage(MSG_ERROR, "Can't malloc %d byte%s", size,
                   size == 1 ? "" : "s");
    return r;
}

void *CheckCalloc(size_t nelem, size_t elsize)
{
    register void *r;

    if (!(r = malloc(nelem * elsize)))
        msgMessage(MSG_ERROR, "Can't calloc %d element%s of %d byte%s",
                   nelem, nelem == 1 ? "" : "s", elsize,
                   elsize == 1 ? "" : "s");
    memset(r, 0, nelem * elsize);
    return r;
}

char *CheckStrdup(const char *s)
{
    register char *r;

    if (!(r = strdup(s)))
        msgMessage(MSG_ERROR, "Can't strdup \"%s\"", s);
    return r;
}

void _CheckFree(void *ptr)
{
    if (!ptr)
        msgMessage(MSG_ERROR, "Can't free a NULL block");

    free(ptr);
}

void SafeMemCpy(void *dst, void *src, size_t size)
{
    register int t;
    char *s = src, *d = dst;

    if (!dst)
        msgMessage(MSG_ERROR, "Can't memcpy to NULL");
    if (!src)
        msgMessage(MSG_ERROR, "Can't memcpy from NULL");
    for (t = 0; t < size; ++t)
        d[t] = s[t];
}
