/****************************************************************************\
*                         *                                                  *
*  Giovanni A. Squillero  *   LungoPo Antonelli 203 - 10153 TORINO - ITALY   *
*                         *   +39-11-8994677  (Roma: +39-6-68307390)         *
*           (!)           *   charme5@athena.polito.it  (POLCLU::CAD6)       *
*                         *                                                  *
\****************************************************************************/

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include "data2.h"
#include "mydata.h"
#include "myfault.h"

/*   In qualche *.h e` definito, ma quale sia ESATTAMENTE esattamente
 * questo header varia da compilatore a compilatore... 
 */
#ifndef max
#define max(X, Y)       ( (X>Y) ? X : Y )
#endif

/* Variabili globali inizializzate da create() */
DESCRIPTOR     *descr, *tmp_descr;
MYDESCRIPTOR   *MyDescr;
ERROR          *ErrorList;
int             n_descr;	/* numero totale di descrittori               */
int             n_pi;		/* numero totale di ingressi primari          */
int             n_po;		/* numero totale di uscite primarie           */
int             n_ff;		/* numero totale di Flip Flop           */
int             max_level;

/* Altre variabili globali */
int             num_fault;
int             leftfault;
char            net_name[MAX_FILENAME_LEN];	/* nome del circuito */
char            exp_name_dee[MAX_FILENAME_LEN + 4];
char           *Context;
char           *Message;

/* Variabili globali x descrivere il circuito */
int            *pi_array, *po_array, *lat_array;
int            *ff_array, *ppi_array;
int             num_ff = 0;

/* Confronta proofs() */
int             saved = SAVED;
unsigned int   *ActiveFault;
int            *detected;
int             out_max;
int            *out_visited;

/* PROTO */
void            ReadFaultList(char *name);

int             TrueNumFault;

int             main(int argc, char *argv[])
{
    char            exp_name[MAX_LINE], dee_name[MAX_LINE], out_name[MAX_LINE],
                    fau_name[MAX_LINE];
    char          **TestPattern;
    FILE           *in;
    int             t;
    int             result;

    Context = "main";

    strcpy(exp_name, argv[1]);
    strcpy(dee_name, exp_name);
    strcat(dee_name, ".edf");

    strcpy(fau_name, exp_name);
    strcat(fau_name, ".rfau");

    strcpy(out_name, exp_name);
    strcat(out_name, ".ftj");

    create(dee_name);

#ifdef GS_DEBUGGING
    /* (!)gs040396 */
    { 
	FILE *F;
	F = fopen("fenice.fau", "w");
	for(t=0; t<TrueNumFault; ++t)
	    if(ErrorList[t].Type==-1)
		fprintf(F, "%s /%d\n",
		       descr[ErrorList[t].NumDesc].name,
		       !!ErrorList[t].Value);
	    else
		fprintf(F, "%s<%s /%d\n",
			descr[ErrorList[t].NumDesc].name,
			descr[MyDescr[ErrorList[t].NumDesc].from[ErrorList[t].Type]].name,
			!!ErrorList[t].Value);
	fclose(F);
    }
#endif

    /*
     * Legge il Test Pattern
     */
    if(!(TestPattern=malloc((sizeof(char *)*5000))))
	WriteMessage(NOMEM), exit(-1);
    for(t=0; t<5000; ++t) 
	if(!(TestPattern[t]=malloc(100)))
	    WriteMessage(NOMEM), exit(-1);
    if (!(in = fopen(out_name, "r")))
	WriteMessage(IOERROR), exit(-1);
    else
	for (t = 0; fgets(TestPattern[t], 100, in); ++t);
    *TestPattern[t] = 0;

    /**
     * Questa era la vecchia sequenza di chiamata:
     *
    SwapData();
    ReadFaultList(fau_name);
    InitFSim();
    FSim(TestPattern, ActiveFault, TrueNumFault, detected, &result);
     *
     * Adesso basta chiamare InitFSim e FSim(stessi parametri di prima)
     * Senza nessun bisogno della SwapData.
     *
     * NB: E` solo apparente... E` la InitFSim che chiama la SwapData se serve...
     * Questo vuol dire: 
     * SE MODIFICATE IL CIRCUITO RUN-TIME DOVETE ARRANGIARVI
     */
    ReadFaultList(fau_name);

    /*
     * Annula il vettore dei detected. Questo va fatto *DOPO* la
     * ReadFaultList, se non non sappiamo quanti sono i fault
     */
    for(t=0; t<num_fault; ++t)
	detected[t]=0;

    InitFSim();
    FSim(TestPattern, ActiveFault, TrueNumFault, detected, &result);
    printf("Detected: %d faults.\n", result);
    for(result=t=0; t<num_fault; ++t) {
	    result += detected[t]!=0;
	    printf("Fault %d -> %d %d\n", t, detected[t], ErrorList[t].Gotcha);
	}
    printf("Detected: %d faults.\n", result);
    return (0);
}

/**************************************************************************
NOME:           ReadFaultList
PARAMETRI:      Il nome della fault list
DESCRIZIONE:    Carica la fault list in memoria
***************************************************************************/
void            ReadFaultList(char *name)
{
    FILE           *inp;
    char            line[256];
    int             lines;
    long            value, numdesc, type, zero;
    long            t;
    int             warnings;
    int             anyerror = 0;

    Context = "READ_FAULT_LIST";

/*   Apriamo il file, controlliamo la versione e contiamo le linee */
    if ((inp = fopen(name, "r")) == NULL) {
	WriteMessage(IOERROR);
	return;
    }
    if (fgets(line, 255, inp) == NULL || strncmp(line, "2.0A", 4)) {
	WriteMessage(BADFORMAT);
	return;
    }
    for (lines = 0; fgets(line, 255, inp); ++lines);
    num_fault = lines;

/*   Non e` molto elegante, ma funziona */
    fclose(inp);
    inp = fopen(name, "r");
    fgets(line, 255, inp);

/*   Scarichiamo altre (eventuali) fault list in memoria e allochiamo la
 * nostra */
    if (ErrorList != NULL) {
	free(ErrorList);
	ErrorList = NULL;
    }
    if ((ErrorList = (ERROR *) malloc(max((1 + num_fault) * sizeof(ERROR), 128))) == NULL) {
	WriteMessage(NOMEM);
	return;
    }
/*   Inizializziamo il DUMMY_ERROR, l'errore inesistente */
    ErrorList[num_fault].Value = DUMMY_ERROR;
    ErrorList[num_fault].Type = -1;
    ErrorList[num_fault].NumDesc = n_descr + 1;
    ErrorList[num_fault].Next = num_fault;
    ErrorList[num_fault].Gotcha = -1;

/*   E leggiamo...
 * I vari controlli di BADFORMAT si sono resi necessari xche' ci avevano
 * dato delle fault list meta` in formato ISCAS e meta` in formato MOZART.
 * Il programma si piantava e nessuno capiva perche'...
 * (notti insonni x scoprirlo) */

    warnings = 0;
    for (lines = 0; lines < num_fault; ++lines) {
	anyerror = 0;
	if (sscanf(fgets(line, 255, inp), "%ld %ld %ld %ld",
		   &value, &numdesc, &type, &zero) != 4) {
	    WriteMessage(BADFORMAT);
	    return;
	}
	{
	    ErrorList[lines].Value = (value > 127) ? STUCK_AT_UNO : STUCK_AT_ZERO;
	    ErrorList[lines].NumDesc = numdesc;
	    ErrorList[lines].Next = lines;
	    ErrorList[lines].Gotcha = 0;
	    if (ErrorList[lines].NumDesc >= n_descr) {
		anyerror = 1;
		if (warnings < 2) {
		    WriteMessage(BADFAULT);
		    printf("\tIn line %d non-existing descriptor %ld STUCK-AT-%s.\n", 2 + lines, numdesc, (value > 127) ? "UNO" : "ZERO");
		}
	    } else if (type != -1) {
		ErrorList[lines].Type = -1;
		for (t = 0; t < descr[numdesc].fanin; ++t)
		    if (descr[numdesc].from[t] == type)
			ErrorList[lines].Type = t;
		if (ErrorList[lines].Type == -1) {
		    anyerror = 1;
		    if (warnings < 2) {
			WriteMessage(BADFAULT);
			printf("\tIn line %d descriptor %ld STUCK-AT-%s from descriptor %ld\n", 2 + lines, numdesc, (value > 127) ? "UNO" : "ZERO", type);
			printf("\tbut descriptors are not connected.\n");
		    }
		}
	    } else
		ErrorList[lines].Type = -1;
	}
	if (!anyerror && descr[numdesc].fanin >= MAXIMUM_FANIN) {
	    anyerror = 1;
	    if (warnings < 2) {
		WriteMessage(BADFAULT);
		printf("\tIn line %d descriptor %ld STUCK-AT-%s\n", 2 + lines, numdesc, (value > 127) ? "UNO" : "ZERO");
		printf("\tbut descriptor's fanin is %ld and PRepsim cannot handle it.\n", descr[numdesc].fanin);
	    }
	}
	if (anyerror) {
	    ++warnings;
	    if (warnings <= 2)
		printf("\tFault will be ignored.\n\n");
	    if (warnings == 2) {
		WriteMessage(GENERAL);
		printf("\tFurther warnings will be suppressed.\n\n");
	    }
	    ErrorList[lines].Value = DUMMY_ERROR;
	    ErrorList[lines].Type = -1;
	    ErrorList[lines].NumDesc = n_descr + 1;
	    ErrorList[lines].Next = num_fault;
	    ErrorList[lines].Gotcha = -1;
	}
    }
    fclose(inp);

/*   Inizializziamo ActiveFault (ie. i fault attivi) */
    if ((ActiveFault = (unsigned int *) malloc((num_fault) * sizeof(unsigned int))) == NULL) {
	WriteMessage(NOMEM);
	return (1);
    }
    if ((detected = (unsigned int *) malloc((num_fault) * sizeof(unsigned int))) == NULL) {
	WriteMessage(NOMEM);
	return (1);
    }
    TrueNumFault = 0;
    for (t = 0; t < num_fault; ++t)
	if (!ErrorList[t].Gotcha)
	    ActiveFault[TrueNumFault++] = t;

/*   Le classiche statistiche finali */
    printf("Read %ld faults", num_fault);
    if (warnings)
	printf("  (%d ignored)", warnings);
    printf(".\n");
    return;

}
