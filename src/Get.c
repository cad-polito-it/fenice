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

#include "Check.h"
#include "Fenice.h"
#include "SharedVars.h"
#include "DescrVal.h"
#include "Faults.h"

char           *GetPIVal(char *str)
{
    char           *_FunctionName = "GetPIVal";
    int             t;

    if (!str) {
	CheckTrue(Malloc(str, n_pi + 1));
    }
    for (t = 0; t < n_pi; ++t) {
	if (ZERO_P(PIValues[t]))
	    str[t] = '0';
	else if (UNO_P(PIValues[t]))
	    str[t] = '1';
	else if (ICS_P(PIValues[t]))
	    str[t] = 'X';
	else
	    str[t] = '?';
    }
    str[t] = 0;

    return str;
}

char           *GetFFVal(char *str)
{
    char           *_FunctionName = "GetFFVal";
    int             t;

    if (!str) {
	CheckTrue(Malloc(str, n_ff + 1));
    }
    for (t = 0; t < n_ff; ++t) {
	if (ZERO_P(GoodFFAfter[t]))
	    str[t] = '0';
	else if (UNO_P(GoodFFAfter[t]))
	    str[t] = '1';
	else if (ICS_P(GoodFFAfter[t]))
	    str[t] = 'X';
	else
	    str[t] = '?';
    }
    str[t] = 0;

    return str;
}

char           *GetPOVal(char *str)
{
    char           *_FunctionName = "GetPOVal";
    int             t;

    if (!str) {
	CheckTrue(Malloc(str, n_po + 1));
    }
    for (t = 0; t < n_po; ++t) {
	if (ZERO_P(DescrVal[po_array[t]].RefValue))
	    str[t] = '0';
	else if (UNO_P(DescrVal[po_array[t]].RefValue))
	    str[t] = '1';
	else if (ICS_P(DescrVal[po_array[t]].RefValue))
	    str[t] = 'X';
	else
	    str[t] = '?';
    }
    str[t] = 0;

    return str;
}
