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


#include <time.h>
#include "Fenice.h"
#include "SharedVars.h"
#include "Simulation.h"
#include "DescrVal.h"
#include "Faults.h"

/* PROTOS */
void            PrintGate(int g);
char           *Gate(int x);
char           *GetAttrName(int n);
char           *GetTypeName(int n);


char           *GetTypeName(int n)
{
    static char     buf[16];

    if (n == AND)
	return "and";
    if (n == NAND)
	return "nand";
    if (n == OR)
	return "or";
    if (n == NOR)
	return "nor";
    if (n == BUF)
	return "buf";
    if (n == NOT)
	return "not";
    if (n == EXOR)
	return "exor";
    if (n == EXNOR)
	return "exnor";
    if (n == CALLBACK)
	return "callback";
    if (n == FF)
	return "ff";
    if (n == LOGIC0)
	return "LOGIC0";
    if (n == LOGIC1)
	return "LOGIC1";

    sprintf(buf, "UNKNOWN(%d)", n);
    return buf;
}

char           *GetAttrName(int n)
{
    static char     buf[16];

    if (n == PI)
	return "pi";
    if (n == PO)
	return "po";
    if (n == INTERNAL)
	return "internal";

    sprintf(buf, "UNKNOWN(%d)", n);
    return buf;
}

void            PrintGate(int g)
{
    int             t;

    Print(stdout, "%s level %d", Gate(g), descr[g].level);

    Print(stdout, "\n\n%BFROM: %N");
    for (t = 0; t < descr[g].fanin; ++t)
	Print(stdout, "%s/n:%d/l:%d",
	      descr[descr[g].from[t]].name, descr[g].from[t], descr[descr[g].from[t]].level);

    Print(stdout, "\n\n%BTO  : %N");
    for (t = 0; t < descr[g].fanout; ++t)
	Print(stdout, "%s/n:%d/l:%d",
	      descr[descr[g].to[t]].name, descr[g].to[t], descr[descr[g].to[t]].level);

    Print(stdout, " \n\n\n");
}

char           *Gate(int x)
{
    static char     b[256];

    sprintf(b, "gate %d \"%s\" (%s, %s)",
	    x, descr[x].name, GetAttrName(descr[x].attr), GetTypeName(descr[x].type));
    return b;
}

void            Debug(void)
{
    int             t, u;

    Print(stderr, "- Circuits stats:");
    PrintCircuitStats();

    Print(stderr, "- CALLBACK GATES:\n");
    for (t = 0; t < n_descr; ++t) {
	if (descr[t].type == CALLBACK) {
	    Print(stderr, "    descr[%d]: %s, level %d, attr %s. Connected to:\n",
		  t, descr[t].name, descr[t].type, GetAttrName(descr[t].attr));
	    for (u = 0; u < descr[t].fanout; ++u)
		Print(stderr, "        %d (%s), level %d, type %s, attr %s\n",
		      descr[t].to[u], descr[descr[t].to[u]].name,
		      descr[descr[t].to[u]].level,
		      GetTypeName(descr[descr[t].to[u]].type),
		      GetAttrName(descr[descr[t].to[u]].attr));
	}
    }
}

/****************************************************************************/
/*                                I N F O                                   */
/****************************************************************************/

void            PrintCircuitStats(void)
{
    fPrintCircuitStats(stderr);
}

void            fPrintCircuitStats(FILE * out)
{
    int             gate[256];
    int             t;
    int             d, i;
    int             f, l = -1;

    memset(gate, 0, 256 * sizeof(int));

    for (d = i = 0, t = 0; t < n_descr; ++t) {
	if (descr[t].attr == INTERNAL && descr[t].type != FF) {
	    ++i;
	    ++gate[descr[t].type];
	} else if (descr[t].attr == DELETED) {
	    ++d;
	}
    }
    for (f = 0; !gate[f]; ++f);
    for (t = f; t < 256; ++t)
	if (gate[t])
	    l = t;

    Print(out, "\n\nCircuit contains %d gates: %d pi, %d po, %d ff and %d internal gates",
	  n_descr - d, n_pi, n_po, n_ff, i);
    if (l > f) {
	Print(out, "(%d %s,", gate[f], GetTypeName(f));
	for (t = f + 1; t < l; ++t)
	    if (gate[t])
		Print(out, "%d %s,", gate[t], GetTypeName(t));
	Print(out, "%d %s)\n", gate[l], GetTypeName(l));
    } else {
	Print(out, "(%d %s)", gate[f], GetTypeName(f));
    }
    if (d)
	Print(out, "descr[] array contains %d deleted gate%s\n", d, d == 1 ? "" : "s");
    Print(out, " \n");
}

#define FAULTS_CATEGORIES 3

void            PrintSimulationResult(void)
{
    fPrintSimulationResult(stdout);
}


void            fPrintSimulationResult(FILE * out)
{
    char           *_FunctionName = "fPrintSimulationResult";
    int             Faults[FAULTS_CATEGORIES][4];
    int            *coverage;
    int             t;
    int             x, y;
    char           *FaultCategories[] = { "Permanent Single Bit-Stuck-At",
					  "Transient Single Bit-Stuck-At",
					  "Transient Single Bit-Flip (SEU)"
    };

    /* 
     * Fults[x][y]
     *
     * y=0 : tot collapsed
     * y=1 : tot complete
     * y=2 : detected collapsed
     * y=3 : detected complete
     */

    for (x = 0; x < FAULTS_CATEGORIES; ++x)
	for (y = 0; y < 4; ++y)
	    Faults[x][y] = 0;
    coverage = GetCoverage(NULL);
    for (t = 0; t < n_fault; ++t) {
	switch(faultlist[t].type) {
	case FAULT_PERMANENT_STUCK_AT:
	    x = 0;
	    break;
	case FAULT_TRANSIENT_STUCK_AT:
	    x = 1;
	    break;
	case FAULT_TRANSIENT_BIT_FLIP:
	    x = 2;
	    break;
	default:
	    CheckFalse("PANIK: UNKNOWN FAULT TYPE");
	}
	
	Faults[x][0] += 1;	/* collapsed */
	Faults[x][1] += faultlist[t].size;	/* complete */
	if (coverage[t]) {
	    Faults[x][2] += 1;	/* collapsed */
	    Faults[x][3] += faultlist[t].size;	/* complete */
	}
    }

    fprintf(out,
	    "                                 |   COLL    COMP|   DCOL    FC%%|   DCOM    FC%%\n");

    for (x = 0; x < FAULTS_CATEGORIES; ++x) {
	if (Faults[x][0])
	    fprintf(out, "%-32s |%7d %7d|%7d %6.2f|%7d %6.2f\n",
		    FaultCategories[x],
		    Faults[x][0],
		    Faults[x][1],
		    Faults[x][2],
		    100.0 * Faults[x][2] / Faults[x][0],
		    Faults[x][3], 
		    100.0 * Faults[x][3] / Faults[x][1]);
	else
	    fprintf(out, "%-32s |%7d %7d|%7d %6s|%7d %6s\n",
		    FaultCategories[x],
		    Faults[x][0], Faults[x][1], Faults[x][2], "-", Faults[x][3], "-");
    }

    free(coverage);
}


int            *GetCoverage(int *buf)
{
    char           *_FunctionName = "GetCoverage";
    int             t;

    if (!buf) {
	CheckTrue(Malloc(buf, sizeof(int) * (InternalFNum + 1)));
    }
    for (t = 0; t < InternalFNum; ++t)
	buf[t] = InternalFaultList[t].gotcha;
    buf[t] = -1;

    return buf;
}
