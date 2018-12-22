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
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include <Fenice.h>
#include <CircuitMod.h>

/*
 * INTERNAL PROTOS
 */

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/
/*
 * create() variables
 */
int             n_descr;
DESCRIPTOR     *descr;
int             n_pi;
int             n_po;
int             n_ff;
int             max_level;
int            *pi_array;
int            *po_array;
int            *ppi_array;
int            *ppo_array;
/*
 * create_fau(), create_nfau() variables
 */
int             n_fault;
FAULT          *faultlist;

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

void            main(int argc, char *argv[])
{
    char           *_FunctionName = "SimpleSimulator::main";
    char            circuit[256];
    char            fault[256];
    char            testpattern[256];
    VALUE          *ff_val = NULL;
    char          **input;
    int            *coverage;
    int             totfault_col, detected_col;
    int             totfault_com, detected_com;
    int             t;

    Print(stderr, "%BSimple Simulator %N- "
	  "the simple parallel, event-driven fault simulator.\n"
	  "Using %UFenice%N version %s, built at %s.\n\n",
	  GetFeniceVersion(), GetFeniceDate());

    if (argc == 2) {
	strcpy(circuit, argv[1]), strcat(circuit, ".edf");
	strcpy(fault, argv[1]), strcat(fault, ".fau");
	strcpy(testpattern, argv[1]), strcat(testpattern, ".inp");
    } else if (argc == 4) {
	strcpy(circuit, argv[1]);
	strcpy(fault, argv[2]);
	strcpy(testpattern, argv[3]);
    }

    create_silent = 1;
    CheckFalse(create(circuit));

    if (!create_nfau(fault))
	create_fau(fault);

    CircuitMod_RemoveClock();
    CircuitMod_Commit(faultlist, n_fault);

    SetCircuit();
    SetFaults(faultlist, n_fault);

    PrintCircuitStats();
    PrintFaultStats();

    input = SetInput(testpattern, NULL);

    SetSimulationType(DROP_FIRST_PO);
    CheckTrue(Malloc(ff_val, sizeof(VALUE)*n_ff));
    for(t=0; t<n_ff; ++t)
	ff_val[t] = F_ZERO;
    SetInitialFFValues(ff_val);

    totfault_col = n_fault;
    detected_col = Simulation(input);

    coverage = GetCoverage(NULL);
    totfault_com = detected_com = 0;
    for (t = 0; t < n_fault; ++t) {
	totfault_com += faultlist[t].size;
	if (coverage[t])
	    detected_com += faultlist[t].size;
    }

    Print(stderr, "\n\n%B%Fault simulation results%N\n");
    Print(stderr, "\n    Flip flop activity: %0.2f%% (allocated %d buffers)\n",
	  100.0 * GetAllocatedBuffers() / (n_ff * n_fault), GetAllocatedBuffers());
    Print(stderr, "\n    Complete fault list: detected %d faults on %d (%0.2f%%)\n",
	detected_com, totfault_com, 100.0 * detected_com / totfault_com);
    Print(stderr, "\n    Collapsed fault list: detected %d faults on %d (%0.2f%%)\n\n",
	detected_col, totfault_col, 100.0 * detected_col / totfault_col);

    Free(coverage);
    Free(ff_val);
}
