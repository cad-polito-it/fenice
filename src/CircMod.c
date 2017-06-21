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


#define EDF_VERSION "2.0"

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

#include <create.h>
#include "Fenice.h"
#include "SharedVars.h"
#include "CircuitMod.h"

/*
 * INTERNAL PROTOS
 */
static void     _internal_RemoveGate(int g);
static void     _internal_RemoveFromFrom(int gate, int from);
static void     _internal_RemoveFromTo(int gate, int to);
static void     _internal_findwire(long index, FILE * fp);
static void     _internal_writelib(FILE * fp, char *name, char n_in);
void            add_assoc(int nd, char *name);

int             GetFirstDeleted(void);
int             To2Pin(int g, int to);
int             From2Pin(int g, int from);

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

static int      CircuitModified = 0;
static int     *TranslationMatrix = NULL;

#ifdef DEBUG
static int      IndentLevel;
static char    *IndentSpaces = "    ";

/*#define         Indent() {register int _r; if(!IndentLevel)Print(stderr,"\n");for(_r=0;_r<IndentLevel;++_r) Print(stderr, "%s", IndentSpaces);}*/
void            Indent()
{
    register int    _r;

    if (!IndentLevel)
	Print(stderr, "\n");
    for (_r = 0; _r < IndentLevel; ++_r)
	Print(stderr, "%s", IndentSpaces);
}

#endif

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

/* 
 * Debug.c
 */
char           *Gate(int x);
char           *GetTypeName(int);
char           *GetAttrName(int);

int             To2Pin(int g, int to)
{
    int             t;

    for (t = 0; t < descr[g].fanout; ++t) {
	if (descr[g].to[t] == to) {
	    return t;
	}
    }
    return -1;
}
int             From2Pin(int g, int from)
{
    int             t;

    for (t = 0; t < descr[g].fanin; ++t) {
	if (descr[g].from[t] == from) {
	    return t;
	}
    }
    return -1;
}

int             CircuitMod_GetFreeGate(void)
{
    char           *_FunctionName = "CircuitMod_GetFreeGate";
    int             t;
    int             found = -1;

    for (t = 0; t < n_descr; ++t) {
	if (descr[t].attr == DELETED) {
	    found = t;
	    break;
	}
    }

    if (found == -1) {
/*      CheckTrue(descr = realloc(descr, (n_descr + 1) * sizeof(DESCRIPTOR))); */
	CheckTrue(Realloc(descr, (n_descr + 1) * sizeof(DESCRIPTOR)));
	found = n_descr++;
    }
    descr[found].type = BUF;
    descr[found].attr = DELETED;
    descr[found].fanin = descr[found].fanout = 0;
    descr[found].from = descr[found].to = NULL;

    return found;
}

void            _internal_RemoveGate(int g)
{
    char           *_FunctionName = "_internal_RemoveGate";

#ifdef DEBUG
    Indent();
    Print(stderr, "Zapping %s\n", Gate(g));
#endif
    CheckTrue(!descr[g].fanin && !descr[g].fanout);
    DCheckTrue(descr[g].attr != DELETED);
    descr[g].attr = DELETED;
    descr[g].type = BUF;
    if(descr[g].fanin)
	Free(descr[g].from);
    if(descr[g].fanout)
	Free(descr[g].to);
    Free(descr[g].name);
    Free(descr[g].to_name);

    CircuitModified = 1;
}

void            CircuitMod_RemoveGate(int g)
{
#ifdef DEBUG
    Indent();
    Print(stderr, "%BRemoving gate:%N %s\n", Gate(g));
    ++IndentLevel;
#endif
    while (descr[g].fanin)
	CircuitMod_RemoveNet(*descr[g].from, g);
    while (descr[g].fanout)
	CircuitMod_RemoveNet(g, *descr[g].to);
    _internal_RemoveGate(g);

    CircuitModified = 1;
#ifdef DEBUG
    --IndentLevel;
#endif
}

void            _internal_RemoveFromFrom(int gate, int from)
{
    char           *_FunctionName = "_internal_RemoveFromFrom";
    int             t, PinNumber;

#ifdef DEBUG
    Indent();
    Print(stderr, "Zapping fanin entry from %s\n", Gate(gate));
    ++IndentLevel;
#endif
    for (PinNumber = -1, t = 0; PinNumber == -1 && t < descr[gate].fanin; ++t)
	if (descr[gate].from[t] == from)
	    PinNumber = t;
    CheckTrue(PinNumber != -1);
    for (t = PinNumber; t < descr[gate].fanin - 1; ++t)
	descr[gate].from[t] = descr[gate].from[t + 1];
    if (!--descr[gate].fanin)
	Free(descr[gate].from);

    CircuitModified = 1;
#ifdef DEBUG
    --IndentLevel;
#endif
}

void            _internal_RemoveFromTo(int gate, int to)
{
    char           *_FunctionName = "_internal_RemoveFromTo";
    int             t, PinNumber;

#ifdef DEBUG
    Indent();
    Print(stderr, "Zapping fanout entry from %s\n", Gate(gate));
    ++IndentLevel;
#endif
    for (PinNumber = -1, t = 0; PinNumber == -1 && t < descr[gate].fanout; ++t)
	if (descr[gate].to[t] == to)
	    PinNumber = t;
    CheckTrue(PinNumber != -1);
    for (t = PinNumber; t < descr[gate].fanout - 1; ++t)
	descr[gate].to[t] = descr[gate].to[t + 1];
    if (!--descr[gate].fanout)
	Free(descr[gate].to);

    CircuitModified = 1;
#ifdef DEBUG
    --IndentLevel;
#endif
}

void            CircuitMod_RemoveNet(int from, int to)
{
#ifdef DEBUG
    Indent();
    Print(stderr, "%BRemoving net:%N");
    Print(stderr, "%s %B->%N", Gate(from));
    Print(stderr, "%s\n", Gate(to));
    ++IndentLevel;
#endif
    _internal_RemoveFromFrom(to, from);
    _internal_RemoveFromTo(from, to);

    CircuitModified = 1;
#ifdef DEBUG
    --IndentLevel;
#endif
}

void            CircuitMod_AddNet(int from, int to)
{
    char           *_FunctionName = "CircuitMod_AddNet";

#ifdef DEBUG
    Indent();
    Print(stderr, "%BAdding net:%N");
    Print(stderr, "%s%B ->%N", Gate(from));
    Print(stderr, "%s\n", Gate(to));
    ++IndentLevel;
#endif
    if (descr[from].fanout) {
	CheckTrue(Realloc(descr[from].to, (descr[from].fanout + 1) * sizeof(int)));
    } else {
	CheckTrue(Malloc(descr[from].to, sizeof(int)));
    }
    descr[from].to[descr[from].fanout++] = to;

    if (descr[to].fanin) {
	CheckTrue(Realloc(descr[to].from, (descr[to].fanin + 1) * sizeof(int)));
    } else {
	CheckTrue(Malloc(descr[to].from, sizeof(int)));
    }
    descr[to].from[descr[to].fanin++] = from;

    CircuitModified = 1;
#ifdef DEBUG
    --IndentLevel;
#endif
}

int             CircuitMod_AddGate(char *name, int type, int attr)
{
    char           *_FunctionName = "CircuitMod_AddGate";
    int             g;

#ifdef DEBUG
    Indent();
    Print(stderr, "%BAdding gate:%N \"%s\" (%s, %s)\n", name, GetTypeName(type), GetAttrName(attr));
    ++IndentLevel;
#endif
    g = CircuitMod_GetFreeGate();
    CheckTrue(descr[g].name = strdup(name));
    CheckTrue(descr[g].to_name = strdup(name));
    descr[g].type = type;
    descr[g].attr = attr;
    CircuitModified = 1;

#ifdef DEBUG
    --IndentLevel;
#endif
    return g;
}

int             GetFirstDeleted(void)
{
    int             t;

    for (t = 0; t < n_descr; ++t)
	if (descr[t].attr == DELETED)
	    return t;
    return -1;
}

void            CircuitMod_Commit(FAULT * flist, int fnum)
{
    char           *_FunctionName = "CircuitMod_Commit";
    int             t, u;
    int             target;
    int             ch, old_fau;
    int             old_n_descr;

#ifdef DEBUG
    Indent();
    Print(stderr, "%BCircuitMod_Commit()%N\n");
    ++IndentLevel;
#endif

    if (TranslationMatrix)
	Free(TranslationMatrix);
    CheckTrue(Malloc(TranslationMatrix, n_descr * sizeof(int)));

    for (t = 0; t < n_descr; ++t)
	TranslationMatrix[t] = t;

    for (t = 0; t < n_descr; ++t) {
	if (!descr[t].fanin)
	    descr[t].from = NULL;
	if (!descr[t].fanout)
	    descr[t].to = NULL;
    }

    /*
     * Kill wrong connection
     */
    for (t = 0; t < n_descr; ++t) {
	for (u = 0; u < descr[t].fanin; ++u) {
	    if (To2Pin(descr[t].from[u], t) == -1) {
		/* no to[] in parent descriptor */
		_internal_RemoveFromFrom(t, descr[t].from[u]);
	    }
	}
	for (u = 0; u < descr[t].fanout; ++u) {
	    if (From2Pin(descr[t].to[u], t) == -1) {
		/* no from[] in son descriptor */
		_internal_RemoveFromTo(t, descr[t].to[u]);
	    }
	}
    }

    for (t = 0; t < n_descr; ++t) {
	if (!descr[t].fanin && !descr[t].fanout && descr[t].attr != DELETED) {
	    _internal_RemoveGate(t);
	}
    }

    old_n_descr = n_descr;
    while ((target = GetFirstDeleted()) != -1) {
	--n_descr;
	if (target != n_descr) {
	    TranslationMatrix[n_descr] = target;
	    memcpy(&descr[target], &descr[n_descr], sizeof(DESCRIPTOR));
#ifdef DEBUG
	    Indent();
	    Print(stderr, "Moving gate: %s to %d\n", Gate(n_descr), target);
#endif
	}
    }

    for (t = 0; t < n_descr; ++t) {
	for (u = 0; u < descr[t].fanin; ++u) {
	    CheckTrue(descr[t].from[u] >= 0 && descr[t].from[u] < old_n_descr);
	    descr[t].from[u] = TranslationMatrix[descr[t].from[u]];
	}
	for (u = 0; u < descr[t].fanout; ++u) {
	    CheckTrue(descr[t].to[u] >= 0 && descr[t].to[u] < old_n_descr);
	    descr[t].to[u] = TranslationMatrix[descr[t].to[u]];
	}
    }

    /*
     * FAULTS
     */
#ifdef DEBUG
    Indent();
    Print(stderr, "Updating Fault list\n");
    ++IndentLevel;
#endif
    for (t = 0; t < fnum; ++t) {
	CheckTrue(TranslationMatrix[flist[t].descr] >= 0
		  && TranslationMatrix[flist[t].descr] < n_descr);
	flist[t].descr = TranslationMatrix[flist[t].descr];
	if (flist[t].pin == -1) {
	    flist[t].from = -1;
	} else {
	    CheckTrue(flist[t].pin < descr[flist[t].descr].fanin);
	    ch = 0;
	    old_fau = flist[t].from;
	    if (flist[t].from != descr[flist[t].descr].from[flist[t].pin])
		ch = 1, flist[t].from = descr[flist[t].descr].from[flist[t].pin];
#ifdef DEBUG
	    if (ch) {
		Indent();
		Print(stderr, "%BMoving fault%N %d: %d-*->%d to %d-*->%d\n",
		      t+1, old_fau, flist[t].descr, flist[t].from, flist[t].descr);
	    }
#endif
	}
	CheckTrue(flist[t].pin == -1 || descr[flist[t].descr].from[flist[t].pin] == flist[t].from);
    }
#ifdef DEBUG
    --IndentLevel;
#endif

    /*
     * HASH
     */
#ifdef DEBUG
    Indent();
    Print(stderr, "Rebuilding hash table\n");
#endif
    for (t = 0; t < n_assoc; ++t) {
	for (u = 0; u < list_of_assoc[t].n_el; ++u) {
	    Free(list_of_assoc[t].list[u].name);
	}
	Free(list_of_assoc[t].list);
	Free(list_of_assoc[t].name);
    }
    n_assoc = 0;
    for (t = 0; t < n_descr; ++t)
	add_assoc(t, descr[t].name);

    /*
     * Arrays'n'globals
     */
    Free(pi_array), CircuitMod_BuildPIArray();
    Free(po_array), CircuitMod_BuildPOArray();
    Free(ppi_array), CircuitMod_BuildFFArray();
    ClockDescr = ClockDescr >= 0 ? TranslationMatrix[ClockDescr] : ClockDescr;

    compute_level(n_pi, n_ff, pi_array, ppi_array);

    CircuitModified = 0;

#ifdef DEBUG
    --IndentLevel;
    Print(stderr, "\n%BCircuitMod_Commit() complete%N\n\n");
#endif
}

void            CircuitMod_SetTypeAttr(int gate, int type, int attr)
{
#ifdef DEBUG
    Indent();
    Print(stderr, "%BSetting%N %s to type %s, attr %s\n",
	  Gate(gate), GetTypeName(type), GetAttrName(attr));
#endif
    descr[gate].type = type;
    descr[gate].attr = attr;
}

/*
 * ARRAYS
 */
void            CircuitMod_BuildPIArray(void)
{
    char           *_FunctionName = "CircuitMod_BuildPIArray";
    int             t;
    int            *P;

    Free(pi_array);
    for (n_pi = t = 0; t < n_descr; ++t)
	n_pi += descr[t].attr == PI;
    CheckTrue(Malloc(pi_array, MAX(1, n_pi) * sizeof(int)));

    for (P = pi_array, t = 0; t < n_descr; ++t)
	if (descr[t].attr == PI)
	    *P++ = t;
}
void            CircuitMod_BuildPOArray(void)
{
    char           *_FunctionName = "CircuitMod_BuildPOArray";
    int             t;
    int            *P;

    Free(po_array);
    for (n_po = t = 0; t < n_descr; ++t)
	n_po += descr[t].attr == PO;
    CheckTrue(Malloc(po_array, MAX(1, n_po) * sizeof(int)));

    for (P = po_array, t = 0; t < n_descr; ++t)
	if (descr[t].attr == PO)
	    *P++ = t;
}
void            CircuitMod_BuildFFArray(void)
{
    char           *_FunctionName = "CircuitMod_BuildFFArray";
    int             t;
    int            *P;

    Free(ppi_array);
    for (n_ff = t = 0; t < n_descr; ++t)
	n_ff += descr[t].type == FF;
    CheckTrue(Malloc(ppi_array, MAX(1, n_ff) * sizeof(int)));

    for (P = ppi_array, t = 0; t < n_descr; ++t)
	if (descr[t].type == FF)
	    *P++ = t;
}

int             CircuitMod_Translate(int x)
{
    char           *_FunctionName = "CircuitMod";

    CheckTrue(TranslationMatrix);
    return TranslationMatrix[x];
}

int             CircuitMod_TranslateVector(int *v)
{
    int             t;

    for (t = 0; v[t] != -1; ++t)
	v[t] = CircuitMod_Translate(v[t]);
    return t;
}

int             CircuitMod_ChangeFFData(int ff, int newdata)
{
    char           *_FunctionName = "CircuitMod_ChangeFFData";
    int             old;

#ifdef DEBUG
    Indent();
    Print(stderr, "%BChanging ff data input:%N "
	  "ff: \"%s\"; new data: %d\n", descr[ff].name, newdata);
#endif
    CheckTrue(descr[ff].type == FF);

    if (descr[newdata].fanout) {
	CheckTrue(Realloc(descr[newdata].to, (descr[newdata].fanout + 1) * sizeof(int)));
    } else {
	CheckTrue(Malloc(descr[newdata].to, sizeof(int)));
    }
    descr[newdata].to[descr[newdata].fanout++] = ff;

    if (descr[ff].fanin) {
	old = *descr[ff].from;
    } else {
	old = -1;
	descr[ff].fanin = 1;
	CheckTrue(Malloc(descr[ff].from, 2 * sizeof(int)));
    }
    *descr[ff].from = newdata;

    CircuitModified = 1;
    return old;
}

int             CircuitMod_ChangeFFClock(int ff, int newclock)
{
    char           *_FunctionName = "CircuitMod_ChangeFFClock";
    int             old;

#ifdef DEBUG
    Indent();
    Print(stderr, "%BChanging ff clock input:%N "
	  "ff: \"%s\"; old: %d, new: %d\n", descr[ff].name, descr[ff].from[1], newclock);
#endif
    CheckTrue(descr[ff].type == FF);

    old = descr[ff].from[1];
    descr[ff].from[1] = newclock;
    return old;
}

void            CircuitMod_ScanFF(int ff, FAULT * flist, int fnum)
{
    char           *_FunctionName = "CircuitMod_ScanFF";
    int             new_pi;
    char            name[128];
    int             t;

    CircuitModified = 1;

#ifdef DEBUG
    Indent();
    Print(stderr, "%BConverting to scan%N %s\n", Gate(ff));
    ++IndentLevel;
#endif

    CheckTrue(descr[ff].type == FF);

    /*
     * aggiungo un buffer come PI e copio il fanout
     *
     *   ---FF
     *        \___
     *        /
     *      PI
     */
    sprintf(name, "%s_SCAN_IN", descr[ff].name);
    new_pi = CircuitMod_AddGate(name, BUF, PI);
    for (t = 0; t < descr[ff].fanout; ++t)
	CircuitMod_AddNet(new_pi, descr[ff].to[t]);

    /*
     * trasformo il ff in PO e cancello il fanout
     *
     *   ---PO
     *         ___
     *        /
     *      PI
     */
    sprintf(name, "%s_SCAN_OUT", descr[ff].name);
    Free(descr[ff].name);
    descr[ff].name = strdup(name);
    descr[ff].to_name = strdup(name);
    CircuitMod_SetTypeAttr(ff, BUF, PO);
    CircuitMod_RemoveNet(descr[ff].from[1], ff);
    while (descr[ff].fanout)
	CircuitMod_RemoveNet(ff, *descr[ff].to);


    /*
     * aggiorna la flist.
     */
    for (t = 0; t < fnum; ++t) {
	if (flist[t].descr == ff && flist[t].from == -1) {
#ifdef DEBUG
	    Indent();
	    Print(stderr, "%BMoving fault%N %d: %d-*=> to %d-*=>\n", t + 1, ff, new_pi);
#endif
	    flist[t].descr = new_pi;
	}
    }

#ifdef DEBUG
    --IndentLevel;
#endif
}

long            CircuitMod_WriteEDF(char *filename, char *entity_name)
{
    char           *_FunctionName = "CircuitMod_WriteEDF";
    int             t, u;
    int             error;
    FILE           *fp;
    char            type_of_comp[10];
    extern char    *ResetDescrName;

    /*
     * rimettiamo a posto gli array
     */
    CircuitMod_BuildArrays();

    CheckTrue(ResetDescrName && *ResetDescrName);
    /*
     * apro il file
     */
    CheckTrue(fp = fopen(filename, "w"));
    /*
     * scrivo header del file
     */
    error = 0;
    fprintf(fp, "(edif PoliTO_EDIF (edifVersion 2 0 0 ) (edifLevel 0)\n");
    fprintf(fp, " (keywordMap (keywordLevel 0))\n");
    fprintf(fp, " ( status\n  (written \n");
    fprintf(fp, "   (timeStamp 1 1 1 1 1 1 )\n");
    fprintf(fp, "   (program \"writedif\"\n");
    fprintf(fp, "    ( Version \"%s\" ))\n", EDF_VERSION);
    fprintf(fp, "   (author \"designer\")\n  )\n )\n");
    fprintf(fp, " (external my_class (edifLevel 0)");
    fprintf(fp, "  (technology (numberDefinition))\n");

    /*
     * scrivo i riferimenti alla libreria
     */
    fprintf(fp, "  (cell Flip_Flop_D_reset (cellType GENERIC)\n");
    fprintf(fp, "   (view Netlist_representation (viewType NETLIST )\n");
    fprintf(fp, "    (interface \n     (port D (direction INPUT))\n");
    fprintf(fp,
	    "     (port CK (direction INPUT))\n     (port RESET (direction INPUT))\n     (port Q (direction OUTPUT))\n");
    fprintf(fp, "    )\n   )\n  )\n");
    _internal_writelib(fp, "And3_gate", 3);
    _internal_writelib(fp, "And4_gate", 4);
    _internal_writelib(fp, "And5_gate", 5);
    _internal_writelib(fp, "Nand3_gate", 3);
    _internal_writelib(fp, "Nand4_gate", 4);
    _internal_writelib(fp, "Nand5_gate", 5);
    _internal_writelib(fp, "Or3_gate", 3);
    _internal_writelib(fp, "Or4_gate", 4);
    _internal_writelib(fp, "Or5_gate", 5);
    _internal_writelib(fp, "Nor3_gate", 3);
    _internal_writelib(fp, "Nor4_gate", 4);
    _internal_writelib(fp, "Nor5_gate", 5);
    _internal_writelib(fp, "Xor3_gate", 3);
    _internal_writelib(fp, "Xor4_gate", 4);
    _internal_writelib(fp, "Xor5_gate", 5);
    _internal_writelib(fp, "Xnor3_gate", 3);
    _internal_writelib(fp, "Xnor4_gate", 4);
    _internal_writelib(fp, "Xnor5_gate", 5);
    _internal_writelib(fp, "And_gate", 2);
    _internal_writelib(fp, "Or_gate", 2);
    _internal_writelib(fp, "Nand_gate", 2);
    _internal_writelib(fp, "Nor_gate", 2);
    _internal_writelib(fp, "Xor_gate", 2);
    _internal_writelib(fp, "Xnor_gate", 2);
    _internal_writelib(fp, "Buf_gate", 1);
    _internal_writelib(fp, "Inv_gate", 1);
    _internal_writelib(fp, "Logic_0", 0);
    _internal_writelib(fp, "Logic_1", 0);
    fprintf(fp, " )\n");

    /*
     * costruisco l'interfaccia esterna del circuito
     */
    /*
     * comincio scrivendo il nome
     */
    fprintf(fp, " (library DESIGNS (edifLevel 0)\n");
    fprintf(fp, "  (technology (numberDefinition))\n");
    fprintf(fp, "  (cell %s (cellType GENERIC)\n", entity_name);
    fprintf(fp, "   (view Netlist_representation (viewType NETLIST)\n");
    fprintf(fp, "    (interface\n");

    /*
     * scrivo i PI
     */
    for (t = 0; t < n_pi; t++) {
	fprintf(fp, "     (port %s (direction INPUT ))\n", descr[pi_array[t]].name);
	if (!strcmp(ResetDescrName, descr[pi_array[t]].name)) {
	    fprintf(stderr, "ERRORE. Un PI (%d) ha lo stesso nome del reset (%s)\n",
		    pi_array[t], descr[pi_array[t]].name);
	    error = 1;
	}
    }

    /*
     * scrivo l' ingresso relativo al RESET del FF
     * Prima pero' controllo che esistano dei FF
     */
    if (n_ff) {
	fprintf(fp, "     (port %s (direction INPUT ))\n", ResetDescrName);
    }

    /*
     * scrivo i PO
     */
    for (t = 0; t < n_po; t++) {
	if (!strcmp(descr[po_array[t]].name, ResetDescrName)) {
	    fprintf(stderr, "ERRORE. un PO (%d) ha lo stesso nome del reset (%s)\n",
		    po_array[t], descr[po_array[t]].name);
	    error = 1;
	}
	fprintf(fp, "     (port %s (direction OUTPUT ))\n", descr[po_array[t]].name);
    }
    fprintf(fp, "    )\n    (contents\n");

    /*
     * scrivo le istanziazioni dei componenti
     */
    for (t = 0; t < n_descr; t++) {
	if ((descr[t].attr != PI)
	    && !((descr[t].attr == PO) && (descr[t].type == BUF))) {
	    /*
	     * printf(" %d\n ",descr[t].type);
	     */
	    if (!strcmp(descr[t].name, ResetDescrName)) {
		fprintf(stderr, "ERRORE. Un gate (%d) ha lo stesso nome del reset (%s)\n",
			t, descr[t].name);
		error = 1;
	    }
	    switch (descr[t].type) {
	    case NOT:{
		    strcpy(type_of_comp, "Inv");
		    break;
		}
	    case BUF:{
		    strcpy(type_of_comp, "Buf");
		    break;
		}
	    case AND:{
		    strcpy(type_of_comp, "And");
		    break;
		}
	    case NAND:{
		    strcpy(type_of_comp, "Nand");
		    break;
		}
	    case OR:{
		    strcpy(type_of_comp, "Or");
		    break;
		}
	    case NOR:{
		    strcpy(type_of_comp, "Nor");
		    break;
		}
	    case EXOR:{
		    strcpy(type_of_comp, "Xor");
		    break;
		}
	    case EXNOR:{
		    strcpy(type_of_comp, "Xnor");
		    break;
		}
	    case FF:{
		    strcpy(type_of_comp, "Flip_Flop");
		    break;
		}

	    case LOGIC0:{
		    strcpy(type_of_comp, "Logic_0");
		    break;
		}
	    case LOGIC1:{
		    strcpy(type_of_comp, "Logic_1");
		    break;
		}
	    }
	    /* scrivo sul file comp_name e type of comp */
	    fprintf(fp, "     (instance %s\n      (viewref Netlist_representation\n",
		    descr[t].name);
	    fprintf(fp, "       (cellRef %s", type_of_comp);
	    if (descr[t].fanin >= 3) {
		fprintf(fp, "%d", descr[t].fanin);
	    }
	    if (descr[t].type == FF) {
		fprintf(fp, "_D_reset (libraryRef my_class))\n");
	    } else if (descr[t].type == LOGIC0) {
		fprintf(fp, " (libraryRef my_class))\n");
	    } else if (descr[t].type == LOGIC1) {
		fprintf(fp, " (libraryRef my_class))\n");
	    } else {
		fprintf(fp, "_gate (libraryRef my_class))\n");
	    }
	    fprintf(fp, "      )\n     )\n");
	}
    }

    /*
     * scrivo le connessioni dei componenti
     */
    for (t = 0; t < n_descr; t++) {
	switch (descr[t].attr) {
	case PI:		/*
				 * scrivo il nome del PI _wire
				 */
	    fprintf(fp, "     (net %s\n", descr[t].name);
	    fprintf(fp, "      (joined \n");
	    fprintf(fp, "       (portRef %s)\n", descr[t].name);
	    _internal_findwire(t, fp);
	    fprintf(fp, "      )\n     )\n");
	    break;
	case INTERNAL:		/*
				 * scrivo il nome del componente_out
				 */
	    if (descr[t].to_name != NULL)
		fprintf(fp, "     (net %s\n      (joined\n", (descr[t].to_name));
	    else
		fprintf(fp, "     (net %s_OUT\n      (joined\n", (descr[t].name));
	    if (descr[t].type == FF) {
		fprintf(fp, "       (portRef Q (instanceRef %s))\n", descr[t].name);
	    } else {
		fprintf(fp, "       (portRef O (instanceRef %s))\n", descr[t].name);
	    }
	    _internal_findwire(t, fp);
	    fprintf(fp, "      )\n     )\n");
	    break;
	case PO:
	    if ((descr[t].attr == PO) && (descr[t].type != BUF)) {
		fprintf(fp, "     (net %s_out\n", descr[t].name);
		fprintf(fp, "      (joined \n");
		fprintf(fp, "       (portRef %s)\n", descr[t].name);
		if (descr[t].type == FF) {
		    fprintf(fp, "       (portRef Q (instanceRef %s))\n", descr[t].name);
		} else {
		    fprintf(fp, "       (portRef O (instanceRef %s))\n", descr[t].name);
		}
		_internal_findwire(t, fp);
		fprintf(fp, "      )\n     )\n");
	    }
	    break;
	}
    }
    if (n_ff) {
	fprintf(fp, "     (net %s\n", ResetDescrName);
	fprintf(fp, "      (joined \n");
	fprintf(fp, "       (portRef %s)\n", ResetDescrName);
	for (t = 0; t < n_descr; t++) {
	    if (descr[t].type == FF)
		fprintf(fp, "       (portRef RESET (instanceRef %s))\n", descr[t].name);
	}
	fprintf(fp, "      )\n     )\n");
    }
    fprintf(fp, "    )\n   )\n  )\n )\n");
    fprintf(fp, " (design PoliTO_EDIF(cellRef %s (libraryRef DESIGNS)))\n)\n", entity_name);
    fclose(fp);

    for (t = 0; t < n_descr; ++t) {
	for (u = 0; u < descr[t].fanin; ++u) {
	    CheckTrue(descr[t].from[u] < 0);
	    descr[t].from[u] = -(descr[t].from[u] + 1);
	}
    }
/*     CircuitMod_RemoveGate(reset); */
/*     CircuitMod_Commit(NULL, 0); */

    return 1;
}

void            _internal_writelib(FILE * fp, char *name, char n_in)
{
    char            i = 1;

    fprintf(fp,
	    "  (cell %s (cellType GENERIC)\n   (view Netlist_representation (viewType NETLIST )\n",
	    name);
    fprintf(fp, "    (interface\n");
    while (i <= n_in) {
	fprintf(fp, "     (port I%d (direction INPUT))\n", i++);
    }
    fprintf(fp, "     (port O (direction OUTPUT))\n    )\n   )\n  )\n");
}

void            _internal_findwire(long index, FILE * fp)
{
    int             i, j;
    long            to;
    char            finded;

    for (i = 0; i < descr[index].fanout; i++) {
	to = descr[index].to[i];
	finded = 0;
	for (j = 0; j < descr[to].fanin; j++) {
	    if (descr[to].from[j] == index) {
		if (descr[to].type == FF) {
		    /*
		     * scrivo nome filo e nome componente
		     */
		    switch (j) {
		    case 0:
			fprintf(fp, "       (portRef D (instanceRef %s))\n", descr[to].name);
			break;
		    case 1:
			fprintf(fp, "       (portRef CK (instanceRef %s))\n", descr[to].name);
			break;
		    }
		} else {
		    if ((descr[to].attr == PO) && (descr[to].type == BUF)) {
			/*
			 * scrivo nome PO
			 */
			fprintf(fp, "       (portRef %s)\n", descr[to].name);
		    } else {
			/*
			 * scrivo nome componente semplice
			 */
			fprintf(fp, "       (portRef I%d (instanceRef %s))\n", j + 1,
				descr[to].name);
		    }
		}
		finded = 1;

		descr[to].from[j] = -(descr[to].from[j] + 1);

		break;
	    }
	}
	if (!finded) {		/*
				 * Errore filo Flottante
				 */
	    fprintf(stderr, "\nErrore filo Flottante %s\n", descr[to].name);
	}
    }
}

int             CircuitMod_RemoveClock(void)
{
    char           *_FunctionName = "CircuitMod_RemoveClock";
    int             clock;
    extern int      ClockDescr;
    int             t;

    if (!n_ff) {
	Print(stderr, "\nCombinatorial circuit, assuming no clock\n");
	return -1;
    }
    clock = descr[*ppi_array].from[1];
    CheckTrue(ClockDescr == clock);
    for (t = 0; t < n_ff; ++t)
	CheckTrue(descr[ppi_array[t]].from[1] == clock);
    Print(stderr, "\nRemoving clock: %s\n", Gate(clock));
    CircuitMod_RemoveGate(clock);
    CircuitMod_BuildArrays();
    ClockDescr = -1;
    return clock;
}

int             CircuitMod_ParanoiaCheck(void)
{
    int             t, u, v;
    int             multiple_pin;
    int             problem = 0;

    for (t = 0; t < n_descr; ++t) {
	if (!descr[t].fanout && !descr[t].fanin) {
	    ++problem;
	    Print(stderr,
		  "%BWARNING::DANGLING GATE:%N"
		  " %d \"%s\" (%s)\n", t, descr[t].name, GetTypeName(descr[t].type));
	}
    }
    for (t = 0; t < n_descr; ++t) {
	if (!descr[t].fanout && descr[t].fanin && descr[t].attr == INTERNAL) {
	    ++problem;
	    Print(stderr,
		  "%BWARNING::NO FANOUT:%N"
		  " %d \"%s\" (%s)\n", t, descr[t].name, GetTypeName(descr[t].type));
	}
    }
    for (t = 0; t < n_descr; ++t) {
	if (descr[t].fanout && !descr[t].fanin && descr[t].attr == INTERNAL) {
	    ++problem;
	    Print(stderr,
		  "%BWARNING::NO FANIN:%N"
		  " %d \"%s\" (%s)\n", t, descr[t].name, GetTypeName(descr[t].type));
	}
    }


    for (t = 0; t < n_descr; ++t) {
	for (u = 0; u < descr[t].fanout; ++u) {
	    if (From2Pin(descr[t].to[u], t) == -1) {
		problem += 1000;
		Print(stderr,
		      "%BERROR::NO CONNECTION:%N"
		      "  %d \"%s\" pin(%d) -> %d \"%s\"\n",
		      t, descr[t].name, u, descr[t].to[u], descr[descr[t].to[u]].name);
	    }
	}
    }
    for (t = 0; t < n_descr; ++t) {
	for (u = 0; u < descr[t].fanin; ++u) {
	    if (To2Pin(descr[t].from[u], t) == -1) {
		problem += 1000;
		Print(stderr,
		      "%BERROR::NO CONNECTION:%N"
		      " %d \"%s\" pin(%d) <- %d \"%s\"\n",
		      t, descr[t].name, u, descr[t].from[u], descr[descr[t].from[u]].name);
	    }
	}
    }

    for (t = 0; t < n_descr; ++t) {
	for (u = 0; u < descr[t].fanout; ++u) {
	    multiple_pin = 0;
	    for (v = u + 1; v < descr[t].fanout; ++v)
		if (descr[t].to[u] == descr[t].to[v])
		    ++multiple_pin;
	    for (v = 0; v < u; ++v)
		if (descr[t].to[u] == descr[t].to[v])
		    multiple_pin = 0;
	    if (multiple_pin) {
		++problem;
		Print(stderr,
		      "%BWARNING::MULTI CONNECTION:%N"
		      " %d \"%s\" (%s) =%d=> %d \"%s\" (%s)\n",
		      t, descr[t].name, GetTypeName(descr[t].type),
		      1 + multiple_pin,
		      descr[t].to[u], descr[descr[t].to[u]].name,
		      GetTypeName(descr[descr[t].to[u]].type));
	    }
	}
    }

    for(t=0; t < n_descr; ++t) {
	if(descr[t].type == CALLBACK && descr[t].fanin >= MAX_CALLBACK_INPUTS)
		Print(stderr,
		      "%BERROR::CALLBACK FANIN >%d:%N"
		      " %d \"%s\" fanin: %d\n",
		      MAX_CALLBACK_INPUTS,
		      t, descr[t].name, descr[t].fanin);
    }

    return problem;
}

int             GetAssoc(char *name)
{
    int             t;

    t = get_assoc(name);
    if (t >= 0 && t < n_descr && !strcmp(name, descr[t].name)) {
	return t;
    }

    t = get_assoc(name) - 1;
    if (t >= 0 && t < n_descr && !strcmp(name, descr[t].name)) {
#ifdef DEBUG
	Print(stderr, "%BWARNING:%N get_assoc patched (-1)\n");
#endif	
	return t;
    }

    t = get_assoc(name) + 1;
    if (t >= 0 && t < n_descr && !strcmp(name, descr[t].name)) {
#ifdef DEBUG
	Print(stderr, "%BWARNING:%N get_assoc patched (+1)\n");
#endif	
	return t;
    }

    for (t = 0; t < n_descr; ++t) {
	if (!strcmp(name, descr[t].name))
	    return t;
#ifdef DEBUG
	Print(stderr, "%BWARNING:%N get_assoc patched (%+d)\n", get_assoc(name)-t);
#endif	
    }
    return -1;
}
