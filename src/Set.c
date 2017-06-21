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

#include "Fenice.h"
#include "SharedVars.h"
#include "Simulation.h"
#include "DescrVal.h"
#include "Faults.h"

/*
 * INTERNAL PROTOS
 */
static int      CheckInput(char **inp);

/****************************************************************************/
/*                           C I R C U I T S                                */
/****************************************************************************/

void            SetCircuit(void)
{
    char           *_FunctionName = "SetCircuit";
    int             t, u;

    CheckCircuit();

    /* 
     * malloc values array
     */
    CheckTrue(Malloc(DescrVal, n_descr * sizeof(DESCR_VAL)));
    for (t = 0; t < n_descr; ++t)
	DescrVal[t].Value = DescrVal[t].RefValue = F_ICS;

    /*
     * malloc speed-up array
     */
    CheckTrue(Malloc(LatArray, n_descr * sizeof(int)));

    for (t = 0; t < n_ff; ++t)
	LatArray[ppi_array[t]] = t;
    for (t = 0; t < n_pi; ++t)
	LatArray[pi_array[t]] = t;

    /*
     * malloc data structures for events propagation
     */
    CheckTrue(Malloc(Events, (max_level + 2) * sizeof(int *)));

    for (t = 0; t <= max_level; ++t)
	CheckTrue(Malloc(Events[t], (1 + n_descr) * sizeof(int)));
    CheckTrue(Malloc(Events[max_level + 1], (1 + n_descr) * sizeof(int)));
    CheckTrue(Malloc(EventsCount, (max_level + 2) * sizeof(int)));

    /*
     * malloc data structure used in simulation
     */
    CheckTrue(Malloc(GoodFFBefore, n_ff * sizeof(VALUE)));
    CheckTrue(Malloc(GoodFFAfter, n_ff * sizeof(VALUE)));
    for (t = 0; t < n_ff; ++t)
	GoodFFBefore[t] = GoodFFAfter[t] = F_ICS;

    CheckTrue(Malloc(PIValues, (1 + n_pi) * sizeof(VALUE)));
    for (t = 0; t <= n_pi; ++t)
	PIValues[t] = F_ICS;

    CheckTrue(Malloc(InitialFFVal, n_ff * sizeof(VALUE)));
    for (t = 0; t < n_ff; ++t)
	InitialFFVal[t] = F_ICS;
}

void            ReleaseCircuit(void)
{
    int             t;

    Free(DescrVal);
    for (t = 0; t <= max_level + 1; ++t)
	Free(Events[t]);
    Free(Events);
}

/****************************************************************************/
/*                         S I M U L A T I O N                              */
/****************************************************************************/

void            SetSimulationType(unsigned int flags)
{
    char           *_FunctionName = "SetSimulationType";

    if (flags & FLAGS_MASK) {
    }

    switch (flags & VALUE_MASK) {
    }

    switch (flags & DROP_MASK) {
    case DROP_FIRST_PO:
	RunTimeFaultDropping = &FaultDroppingPO;
	LastFaultDropping = NULL;
	break;
    case DROP_LAST_PO:
	RunTimeFaultDropping = &FaultDroppingLastPO;
	LastFaultDropping = NULL;
	break;
    case DROP_FIRST_FF:
	RunTimeFaultDropping = &FaultDroppingFF;
	LastFaultDropping = NULL;
	break;
    case DROP_TRESHOLD_FF:
	RunTimeFaultDropping = &FaultDroppingTresholdFF;
	LastFaultDropping = NULL;
	break;
    case DROP_LAST_FF:
	RunTimeFaultDropping = NULL;
	LastFaultDropping = &FaultDroppingLastFF;
	break;
    case DROP_LAST_FF_UD:
	CheckFalse("use DROP_LAST_FF and AddHotFF() instead");
	break;
    }

    switch (flags & RESET_MASK) {
    }
}

/****************************************************************************/
/*                             F A U L T S                                  */
/****************************************************************************/

void            SetFaults(const FAULT * flist, int nfault)
{
    char           *_FunctionName = "SetFaults";
    int             t;

    CheckTrue(descr);

    ReleaseFaults();	/* (!)20000915 */

    CheckTrue(Malloc(InternalFaultList, nfault * sizeof(F_FAULT)));
    CheckTrue(Malloc(ActiveFaults, (1 + nfault) * sizeof(F_FAULT *)));
    InternalFNum = nfault;
    for (t = 0; t < nfault; ++t) {
	CheckTrue(flist[t].size);
	CheckTrue(flist[t].descr < n_descr);
	CheckTrue(flist[t].pin < descr[flist[t].descr].fanin);
	CheckTrue(flist[t].pin == -1 || flist[t].from == descr[flist[t].descr].from[flist[t].pin]);

	CheckTrue(flist[t].type == FAULT_PERMANENT_STUCK_AT || flist[t].type == FAULT_TRANSIENT_STUCK_AT || flist[t].type == FAULT_TRANSIENT_BIT_FLIP);
	CheckTrue(flist[t].pin == -1 || descr[flist[t].descr].from[flist[t].pin] == flist[t].from);
	CheckTrue(!(flist[t].type & FAULT_MASK_TRANSIENT) ^ !(flist[t].type & FAULT_MASK_PERMANENT));
	/* a=>b (!a||b) */
	CheckTrue(!(flist[t].type & FAULT_MASK_PERMANENT) || flist[t].activation == 0);
	CheckTrue(!(flist[t].type & FAULT_MASK_TRANSIENT) || flist[t].activation > 0);
	CheckTrue(!(flist[t].type & FAULT_MASK_BIT_FLIP) || flist[t].type & FAULT_MASK_TRANSIENT);

	InternalFaultList[t].num = t + 1;
	InternalFaultList[t].gotcha = 0;
	InternalFaultList[t].descr = flist[t].descr;
	InternalFaultList[t].pin = flist[t].pin;
	InternalFaultList[t].from = flist[t].from;
	InternalFaultList[t].value = flist[t].val;
	InternalFaultList[t].size = flist[t].size;
	InternalFaultList[t].activation = flist[t].activation;
	InternalFaultList[t].type = flist[t].type;
    }
}

void            AddFault(FAULT f)
{
    char           *_FunctionName = "AddFault";

    CheckTrue(descr);
    CheckTrue(f.pin == -1 || descr[f.descr].from[f.pin] == f.from);
    if (!InternalFNum) {
	CheckTrue(Malloc(InternalFaultList, sizeof(F_FAULT)));
	CheckTrue(Malloc(ActiveFaults, 2 * sizeof(F_FAULT *)));
    } else {
	CheckTrue(Realloc(InternalFaultList, (InternalFNum + 1) * sizeof(F_FAULT)));
	CheckTrue(Realloc(ActiveFaults, (InternalFNum + 2) * sizeof(F_FAULT *)));
    }
    InternalFaultList[InternalFNum].num = InternalFNum + 1;
    InternalFaultList[InternalFNum].gotcha = 0;
    CheckTrue(f.size);
    InternalFaultList[InternalFNum].descr = f.descr;
    InternalFaultList[InternalFNum].pin = f.pin;
    InternalFaultList[InternalFNum].from = f.from;
    InternalFaultList[InternalFNum].value = f.val;
    InternalFaultList[InternalFNum].size = f.size;
    InternalFaultList[InternalFNum].modified = NULL;

    ++InternalFNum;
}

void            SpecialAddFault(FAULT f, int n)
{
    char           *_FunctionName = "SpecialAddFault";

    CheckTrue(descr);
    CheckTrue(f.pin == -1 || descr[f.descr].from[f.pin] == f.from);
    if (!InternalFNum) {
	CheckTrue(Malloc(InternalFaultList, sizeof(F_FAULT)));
	CheckTrue(Malloc(ActiveFaults, 2 * sizeof(F_FAULT *)));
    } else {
	CheckTrue(Realloc(InternalFaultList, (InternalFNum + 1) * sizeof(F_FAULT)));
	CheckTrue(Realloc(ActiveFaults, (InternalFNum + 2) * sizeof(F_FAULT *)));
    }
    InternalFaultList[InternalFNum].num = n + 1;
    InternalFaultList[InternalFNum].gotcha = 0;
    CheckTrue(f.size);
    InternalFaultList[InternalFNum].descr = f.descr;
    InternalFaultList[InternalFNum].pin = f.pin;
    InternalFaultList[InternalFNum].from = f.from;
    InternalFaultList[InternalFNum].value = f.val;
    InternalFaultList[InternalFNum].size = f.size;
    InternalFaultList[InternalFNum].modified = NULL;

    ++InternalFNum;
}

int             AddHotFF(int num)
{
    char           *_FunctionName = "AddHotFF";
    int             last;

    CheckTrue(descr[num].type == FF);
    if (!HotFFList) {
	CheckTrue(Malloc(HotFFList, 2 * sizeof(int)));

	last = 0;
    } else {
	for (last = 0; HotFFList[last] != -1; ++last)
	    CheckFalse(num == HotFFList[last]);
	CheckTrue(Realloc(HotFFList, (last + 2) * sizeof(F_FAULT)));
    }
    HotFFList[last++] = num;
    HotFFList[last] = -1;

    return last;
}

void            ReleaseFaults(void)
{
    int             t;

    if (!InternalFNum) {
	Free(ActiveFaults);
	Free(InternalFaultList);
	return;
    }

    for (t = 0; ActiveFaults[t]; ++t) {
	if (ActiveFaults[t]->modified) {
	    DropBufferChain(ActiveFaults[t]->modified);
	    ActiveFaults[t]->modified = NULL;
	}
    }
    ReleaseBuffers();
    Free(ActiveFaults);
    Free(InternalFaultList);
    InternalFNum = 0;
}

/****************************************************************************/
/*                              I N P U T                                   */
/****************************************************************************/

char          **SetInput(const char *file, char **ptr)
{
    char           *_FunctionName = "SetInput";
    FILE           *F;
    int             lines;
    char            dummy[MAX_LINE];
    int             t, u;
    int             resets;

    CheckTrue(F = fopen(file, "r"));
    for (lines = 0; fgets(dummy, MAX_LINE, F); ++lines);
    CheckTrue(lines);
    if (!ptr) {
	CheckTrue(Malloc(ptr, (2 + lines) * sizeof(char *)));

	for (t = 0; t < lines; ++t)
	    CheckTrue(Malloc(ptr[t], (3 + n_pi) * sizeof(char *)));

	CheckTrue(Malloc(ptr[lines], 1));
    }
    rewind(F);
    for (resets = 0, t = u = 0; t < lines; ++t) {
	fgets(dummy, MAX_LINE, F);
	CheckTrue(*dummy);
	if (*dummy == '#')
	    ++resets, dummy[1] = 0;	/* kill comments */
	else
	    resets = 0;
	if (resets < 2) {
	    sscanf(dummy, "%s", ptr[u]);
	    CheckTrue(*ptr[u]);
	    ++u;
	}
    }
    for (--u; u && *ptr[u] == '#'; --u)
	*ptr[u] = 0;
    CheckInput(ptr);

    return ptr;
}

int             CheckInput(char **inp)
{
    char           *_FunctionName = "CheckInput";
    int             t;
    int             hardreset, softreset;

    hardreset = softreset = 0;
    for (t = 0; *inp[t]; ++t) {
	if (*inp[t] == '#') {
	    if (inp[t][1]) {
		++softreset;
		CheckTrue(strlen(&inp[t][1]) == n_pi);
	    } else {
		++hardreset;
	    }
	} else {
	    CheckTrue(strlen(inp[t]) == n_pi);
	}
    }

    return 0;
}

/****************************************************************************/
/*                           F F V A L U E S                                */
/****************************************************************************/

void            SetSignatureFF(int num, int *list)
{
    char           *_FunctionName = "SetSignatureFF";

    int             t;

    SignatureFFNum = num;
    Free(SignatureFFList);
    CheckTrue(Malloc(SignatureFFList, num * sizeof(int)));

    for (t = 0; t < num; ++t)
	SignatureFFList[t] = list[t];
}

void            SetInitialFFValue(VALUE val)
{
    int             t;

    for (t = 0; t < n_ff; ++t)
	InitialFFVal[t] = val;
}

void            SetInitialFFValues(VALUE * val)
{
    int             t;

    for (t = 0; t < n_ff; ++t)
	InitialFFVal[t] = val[t];
}

void            SetDropFFTreshold(int t)
{
    extern int      FD_TresholdFF;

    FD_TresholdFF = t;
}

/****************************************************************************/
/*                              H O O K S                                   */
/****************************************************************************/

VALUE(*SetHook(int hook, VALUE(*func) (int, VALUE *))) (int, VALUE *) {
    char           *_FunctionName = "SetHook";

    VALUE(*oldHook) (int, VALUE *) = NULL;

    switch (hook) {
    case AFTER_SIMULATION_HOOK:
	oldHook = AfterSimulationHook;
	AfterSimulationHook = func;
	break;
    case USER_CALLBACK_GATE:
	oldHook = CallGateEvalFunction;
	CallGateEvalFunction = func;
	break;
    case AFTER_FAULT_DROPPING_HOOK:
	oldHook = AfterFaultDropping;
	AfterFaultDropping = func;
	break;
    default:
	CheckFalse("UNSUPPORTED HOOK TYPE");
    }

    return oldHook;
}
