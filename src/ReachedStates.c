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
#include <DescrVal.h>

#include <cudd.h>

/*
 * INTERNAL PROTOS
 */
VALUE           hook(int, VALUE *);

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

/*
 * BDD
 */
DdManager      *ddman;		// global ddmanager pointer variable
DdNode         *States;

short           numVars = 0;	// num of vars; if unknown set to 0
short           numVarsZ = 0;	// num of vars for ZBDDs; if unknown set to 0
int             numSlots = CUDD_UNIQUE_SLOTS;	// default for CUDD package
int             cacheSize = CUDD_CACHE_SLOTS;	// default for CUDD package
int             maxCacheSize = 8192;	// ditto

int LoopDetected = 0;

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

void            main(int argc, char *argv[])
{
    char           *_FunctionName = "SimpleSimulator::main";
    char            circuit[256];
    char            testpattern[256];
    VALUE          *ff_val = NULL;
    char          **input;
    int             t;

    Print(stderr, "%BSimple Simulator %N- "
	  "the simple parallel, event-driven fault simulator.\n"
	  "Using %UFenice%N version %s, built at %s.\n\n", GetFeniceVersion(), GetFeniceDate());

    if (argc == 2) {
	strcpy(circuit, argv[1]), strcat(circuit, ".edf");
	strcpy(testpattern, argv[1]), strcat(testpattern, ".inp");
    } else if (argc == 4) {
	strcpy(circuit, argv[1]);
	strcpy(testpattern, argv[2]);
    }

    create_silent = 1;
    CheckFalse(create(circuit));

    CircuitMod_RemoveClock();
    SetCircuit();
    SetFaults(faultlist, n_fault);

    PrintCircuitStats();

    input = SetInput(testpattern, NULL);

    SetSimulationType(DROP_FIRST_PO);
    CheckTrue(Malloc(ff_val, sizeof(VALUE) * n_ff));
    for (t = 0; t < n_ff; ++t)
	ff_val[t] = F_ZERO;
    SetInitialFFValues(ff_val);

    /*
     * bdd
     */
/*     ddman = Cudd_Init(numVars, numVarsZ, numSlots, cacheSize, maxCacheSize); */
    ddman = Cudd_Init(numVars, numVarsZ, numSlots, cacheSize, 0);
    States = Cudd_Not(Cudd_ReadOne(ddman));
    Cudd_Ref(States);

    Cudd_AutodynEnable(ddman,CUDD_REORDER_SIFT);

    SetHook(AFTER_SIMULATION_HOOK, &hook);
    Simulation(input);

    if(LoopDetected) {
	printf("\nLoop detected!\n\n");
    } else {
	printf("\nNo loops\n\n");
    }
    printf("ACTIVITY STATS:\n");
    for(t=0; t<n_descr; ++t) {
	printf("Gate %3d %-20s:    0->1:%3d    1->0:%3d\n",
	       t, descr[t].name?descr[t].name:"(null)",
	       DescrVal[t].Stats[1], DescrVal[t].Stats[0]);
    }

    Free(ff_val);
    Cudd_Quit(ddman);
}

VALUE           hook(int x, VALUE * y)
{
    static char    *ff = NULL;
    DdNode         *var, *tmp, *tmp2;
    int             t;

    ff = GetFFVal(ff);
    printf("state \"%s\": ", ff);

    tmp = Cudd_ReadOne(ddman);
    Cudd_Ref(tmp); 
    for (t = n_ff-1; t >= 0; --t) {
	if (ff[t] == '0') {
	    var = Cudd_Not(Cudd_bddIthVar(ddman, t));
	} else {
	    var = Cudd_bddIthVar(ddman, t);
	}
	Cudd_Ref(var); 

	tmp2 = tmp;
	tmp = Cudd_bddAnd(ddman, tmp2, var);
	Cudd_Ref(tmp);
	Cudd_Deref(var);
        Cudd_Deref(tmp2); /* QUESTA QUI CRASHA TUTTO ? */
    }

    if (Cudd_bddLeq(ddman, tmp, States)) {
	printf("woah!\n");
	LoopDetected = 1;
	StopSimulation();
    } else {
	tmp2 = States;
	States = Cudd_bddOr(ddman, tmp2, tmp);
	Cudd_Ref(States);
	Cudd_Deref(tmp2);
	Cudd_Deref(tmp);
	
	printf("added - mn=%.0f, f=", Cudd_CountMinterm(ddman, States, n_ff));
	Cudd_ApaPrintDensity(stdout, ddman, States, n_ff);
    }

    return F_ZERO;
}
