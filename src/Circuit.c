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
#include "Simulation.h"
#include "DescrVal.h"
#include "CircuitMod.h"

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

/*
 * LOCAL PROTOS
 */

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

/*
 * ARRAYS
 */
void            BuildPIArray(void)
{
    char           *_FunctionName = "BuildPIArray";
    int             t;
    int            *P;

    Free(pi_array);
    for (n_pi = t = 0; t < n_descr; ++t)
	n_pi += descr[t].attr == PI;
    CheckTrue(Malloc(pi_array, n_pi * sizeof(int)));
    for (P = pi_array, t = 0; t < n_descr; ++t)
	if (descr[t].attr == PI)
	    *P++ = t;
}
void            BuildPOArray(void)
{
    char           *_FunctionName = "BuildPOArray";
    int             t;
    int            *P;

    Free(po_array);
    for (n_po = t = 0; t < n_descr; ++t)
	n_po += descr[t].attr == PO;
    CheckTrue(Malloc(po_array, n_po * sizeof(int)));
    for (P = po_array, t = 0; t < n_descr; ++t)
	if (descr[t].attr == PO)
	    *P++ = t;
}
void            BuildFFArray(void)
{
    char           *_FunctionName = "BuildFFArray";
    int             t;
    int            *P;

    Free(ppi_array);
    for (n_ff = t = 0; t < n_descr; ++t)
	n_ff += descr[t].type == FF;
    CheckTrue(Malloc(ppi_array, n_ff * sizeof(int)));
    for (P = ppi_array, t = 0; t < n_descr; ++t)
	if (descr[t].type == FF)
	    *P++ = t;
}

int             CheckCircuit(void)
{
#ifdef DEBUG
    int             t, u;
    int             found;
#endif

    if (!pi_array)
	BuildPIArray();
    if (!po_array)
	BuildPOArray();
    if (!ppi_array)
	BuildFFArray();

#ifdef DEBUG
    for (found = 0, t = 0; !found && t < n_ff; ++t)
	for (u = 0; u < descr[ppi_array[t]].fanout; ++u)
	    if (descr[descr[ppi_array[t]].to[u]].type == FF)
		Print(stderr, "\n%BDEBUG: %NFound a flip flop chain\n"), found = 1;

    for (found = 0, t = 0; !found && t < n_descr; ++t)
	if (descr[t].type == EXOR)
	    Print(stderr, "\n%BDEBUG: %NCircuit contains exor gates\n"), found = 1;
    for (found = 0, t = 0; !found && t < n_descr; ++t)
	if (descr[t].type == EXNOR)
	    Print(stderr, "\n%BDEBUG: %NCircuit contains exnor gates\n"), found = 1;
#endif

    return 1;
}
