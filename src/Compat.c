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
#include <sys/types.h>
#include <sys/time.h>

#include "Fenice.h"

/*
 * INTERNAL PROTOS
 */

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

int             create_nfau(const char *name)
{
    char           *_FunctionName = "create_nfau";
    FILE           *F;
    char            line[128];
    int             n1, n2, n3, n4;
    int             t, zero;
    extern int      create_silent;
    extern int      n_fault;
    extern FAULT   *faultlist;

    CheckTrue(F = fopen(name, "r"));
    CheckTrue(fgets(line, 256, F));
    if (strncmp("2.0A", line, 4)) {
	fclose(F);
	return 0;
    }
    n_fault = 1;
    while (fgets(line, 256, F))
	++n_fault;

    /* malloc */
    CheckTrue(Malloc(faultlist, sizeof(FAULT) * n_fault));

    rewind(F);
    CheckTrue(fgets(line, 256, F));
    CheckTrue(!strncmp("2.0A", line, 4));

    for (n_fault = 0; fgets(line, 256, F); ++n_fault) {
	sscanf(line, "%d %d %d %d", &n1, &n2, &n3, &n4);

	faultlist[n_fault].val = n1 > 127 ? 1 : 0;
	faultlist[n_fault].descr = n2;
	faultlist[n_fault].from = n3;
	faultlist[n_fault].size = n4;
	faultlist[n_fault].pin = -1;
	faultlist[n_fault].activation = 0;
	faultlist[n_fault].type = FAULT_PERMANENT_STUCK_AT;
	if (faultlist[n_fault].from != -1)
	    for (t = 0; t < descr[n2].fanin; ++t)
		if (descr[n2].from[t] == n3)
		    faultlist[n_fault].pin = t;
	CheckFalse(faultlist[n_fault].from != -1 && faultlist[n_fault].pin == -1);
    }
    fclose(F);

    for (zero = 0, t = 0; !zero && t < n_fault; ++t)
	zero = !faultlist[t].size;
    if (zero)
	for (t = 0; t < n_fault; ++t)
	    faultlist[t].size = 1;

    return n_fault;
}

void            InitFSim(void)
{
    VALUE  zero = {f_ZERO, f_UNO};
    SetCircuit();
    SetSimulationType(DROP_FIRST_PO);
    SetInitialFFValue(zero);
}

void            FSim(char **Seq, unsigned int *ActiveFault, unsigned int FaultLeft, int *Detected, int *DNum)
{
    int             t;
    int             det;
    int            *cov;
    extern FAULT   *faultlist;

    ReleaseFaults();
    for (t = 0; t < FaultLeft; ++t)
 	SpecialAddFault(faultlist[ActiveFault[t]], ActiveFault[t]);
/*  	AddFault(faultlist[ActiveFault[t]]); */
    Simulation(Seq);
    cov = GetCoverage(NULL);
    for (det = 0, t = 0; t < FaultLeft; ++t) {
	Detected[ActiveFault[t]] = cov[t];
	det += !!cov[t];
    }
    Free(cov);
    *DNum = det;
}
