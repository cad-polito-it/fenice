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
    char           *_FunctionName = "StateExample::main2";
    char            circuit[256];
    char            fault[256];
    char            testpattern[256];
    char          **vec[2] = { NULL, NULL };
    VALUE          *ff_val = NULL;
    char          **input;
    int            *coverage1;
    int            *coverage2;
    int             totfault_col, detected_col;
    int             totfault_com, detected_com;
    int             t;
    int             len;
    int             split;
    STATE          *State = NULL;

    Print(stderr, "%BState Example %N- "
	  "a demo propram for GetState and SetState.\n"
	  "Using %UFenice%N version %s, built at %s.\n\n",
	  GetFeniceVersion(), GetFeniceDate());

    CheckTrue(argc==2);
    strcpy(circuit, argv[1]), strcat(circuit, ".edf");
    strcpy(fault, argv[1]), strcat(fault, ".fau");
    strcpy(testpattern, argv[1]), strcat(testpattern, ".inp");

    /*
     * load circuit & fault list
     */
    create_silent = 1;
    CheckFalse(create(circuit));
    if (!create_nfau(fault))
	create_fau(fault);

    /*
     * remove clock
     */
    CircuitMod_RemoveClock();
    CircuitMod_Commit(faultlist, n_fault);

    /*
     * set 
     */
    SetCircuit();
    SetFaults(faultlist, n_fault);
    PrintCircuitStats();
    PrintFaultStats();

    /*
     * sim type
     */
    SetSimulationType(DROP_FIRST_PO);
    CheckTrue(Malloc(ff_val, sizeof(VALUE)*n_ff));
    for(t=0; t<n_ff; ++t)
	ff_val[t] = F_ZERO;
    SetInitialFFValues(ff_val);

    /*
     * split input
     */
    input = SetInput(testpattern, NULL);
    for(len=0; input[len]; ++len); --len;
    srand(time(NULL));
    split = rand()%(len-2);
    CheckTrue(Malloc(vec[0], (split+1)*sizeof(char *)));
    CheckTrue(Malloc(vec[1], (len-split+1)*sizeof(char *)));
    for(t=0; t<split; ++t)
	vec[0][t] = input[t];
    vec[0][t] = "";
    for(t=split; t<len; ++t)
	vec[1][t-split] = input[t];
    vec[1][t-split] = "";
    Print(stderr, "Test pattern splitted %d - %d\n", split, len-split);

    totfault_col = n_fault;

    /*
     * step 2 (alone)
     */
    Print(stderr, "\n\n%BSimulating last %d test vectors%N\n", len-split);
    detected_col = Simulation(vec[1]);
    State = GetState(State);
    coverage2 = GetCoverage(NULL);
    totfault_com = detected_com = 0;
    for (t = 0; t < n_fault; ++t) {
	totfault_com += faultlist[t].size;
	if (coverage2[t])
	    detected_com += faultlist[t].size;
    }
    Print(stderr, "\n    Complete fault list: detected %d faults on %d (%0.2f%%)\n",
	detected_com, totfault_com, 100.0 * detected_com / totfault_com);
    Print(stderr, "\n    Collapsed fault list: detected %d faults on %d (%0.2f%%)\n\n",
	detected_col, totfault_col, 100.0 * detected_col / totfault_col);

    /*
     * step 1
     */
    Print(stderr, "\n\n%BSimulating first %d test vectors%N\n", split);
    detected_col = Simulation(vec[0]);
    State = GetState(State);
    coverage2 = GetCoverage(NULL);
    totfault_com = detected_com = 0;
    for (t = 0; t < n_fault; ++t) {
	totfault_com += faultlist[t].size;
	if (coverage2[t])
	    detected_com += faultlist[t].size;
    }
    Print(stderr, "\n    Complete fault list: detected %d faults on %d (%0.2f%%)\n",
	detected_com, totfault_com, 100.0 * detected_com / totfault_com);
    Print(stderr, "\n    Collapsed fault list: detected %d faults on %d (%0.2f%%)\n\n",
	detected_col, totfault_col, 100.0 * detected_col / totfault_col);

    /*
     * All
     */
    Print(stderr, "\n\n%BSimulating all %d test vectors%N\n", len);
    detected_col = Simulation(input);
    coverage1 = GetCoverage(NULL);
    totfault_com = detected_com = 0;
    for (t = 0; t < n_fault; ++t) {
	totfault_com += faultlist[t].size;
	if (coverage1[t])
	    detected_com += faultlist[t].size;
    }
    Print(stderr, "\n    Complete fault list: detected %d faults on %d (%0.2f%%)\n",
	detected_com, totfault_com, 100.0 * detected_com / totfault_com);
    Print(stderr, "\n    Collapsed fault list: detected %d faults on %d (%0.2f%%)\n\n",
	detected_col, totfault_col, 100.0 * detected_col / totfault_col);

    /*
     * step 2
     */
    Print(stderr, "\n\n%BSimulating last %d test vectors after the first %d%N\n", len-split, split);
    SetState(State);
    detected_col = Simulation(vec[1]);
    coverage2 = GetCoverage(coverage2);
    totfault_com = detected_com = 0;
    for (t = 0; t < n_fault; ++t) {
	totfault_com += faultlist[t].size;
	if (coverage2[t])
	    detected_com += faultlist[t].size;
    }
    Print(stderr, "\n    Complete fault list: detected %d faults on %d (%0.2f%%)\n",
	detected_com, totfault_com, 100.0 * detected_com / totfault_com);
    Print(stderr, "\n    Collapsed fault list: detected %d faults on %d (%0.2f%%)\n\n",
	detected_col, totfault_col, 100.0 * detected_col / totfault_col);

    Print(stderr, "\n\nChecking results...");
    for (t = 0; t < n_fault; ++t)
	CheckTrue(coverage1[t]==coverage2[t] || coverage1[t]==coverage2[t]+split);
    Print(stderr, "done.\n\n");
    

    Free(coverage1);
    Free(coverage2);
    Free(ff_val);
}
