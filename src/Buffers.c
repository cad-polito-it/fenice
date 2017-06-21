/*****************************************************************-*-c-*-*\
*             *                                                           *
*   #####     *  (c) Copyright 2010, Giovanni Squillero                   *
*  ######     *  http://staff.polito.it/giovanni.squillero/               *
*  ###   \    *  giovanni.squillero@polito.it                             *
*   ##G  c\   *                                                           *
*   #     _\  *  This code is licensed under a BSD 2-clause license       *
*   |  _/     *  See <https://github.com/squillero/fenice> for details    *
*             *                                                           *
\*************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <memory.h>
#include <stdarg.h>
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
#include "Simulation.h"
#include "Faults.h"

/*
 * INTERNAL PROTOS
 */

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

FF_BUFFER      *_fenice_internal_FreeBufferList;
int             _fenice_internal_AllocatedBuffers;

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

void            AllocateBuffers(int num)
{
    char           *_FunctionName = "AllocateBuffers";
    int             t;
    FF_BUFFER      *B = NULL;

    for (t = 0; t < num; ++t) {
	CheckTrue(Malloc(B, sizeof(FF_BUFFER)));
	B->next = _fenice_internal_FreeBufferList;
	_fenice_internal_FreeBufferList = B;
	B = NULL;
    }

    _fenice_internal_AllocatedBuffers += num;
}

void            ReleaseBuffers(void)
{
    FF_BUFFER      *B1, *B2;

    for (B2 = NULL, B1 = _fenice_internal_FreeBufferList; B1; B2 = B1, B1 = B1->next)
	Free(B2);
    Free(B2);

    _fenice_internal_AllocatedBuffers = 0;
    _fenice_internal_FreeBufferList = NULL;
}

void            DropBufferChain(FF_BUFFER * ptr)
{
    register FF_BUFFER *last;

    for (last = ptr; last->next; last = last->next);
    last->next = _fenice_internal_FreeBufferList;
    _fenice_internal_FreeBufferList = ptr;
}

FF_BUFFER      *funcTakeBuffer(void)
{
    char           *_FunctionName = "BUFFER";
    register FF_BUFFER *buf;

    if (!_fenice_internal_FreeBufferList) {
	CheckTrue(Malloc(_fenice_internal_FreeBufferList, sizeof(FF_BUFFER)));
	++_fenice_internal_AllocatedBuffers;
    }
    buf = _fenice_internal_FreeBufferList;
    _fenice_internal_FreeBufferList = buf->next;
    buf->next = NULL;

    return buf;
}

int             GetAllocatedBuffers(void)
{
    return _fenice_internal_AllocatedBuffers;
}
