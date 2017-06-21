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
int             SwapData(void);

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

    /*
     * La ReadFaultList utilizza la descrizione del circuito, quindi i descr devono essere ricopiati
     * in MyDescr *PRIMA* di chiamarla 
     */
    SwapData();
    /*
     * Bisogna assegnare i giusti valori alle variabili num_fault, num_ff e saved
     * prima di chiamare la InitFSim
     */
    ReadFaultList(fau_name);

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
NOME:           SwapData
PARAMETRI:      -
DESCRIZIONE:    Come detto la struttura dati DESCRIPTOR ritornata dalla
    create() ci sta stretta. Allochiamo la nostra struttura MYDESCRIPTOR,
    copiamo i dati e liberiamo la prima.
***************************************************************************/
int             SwapData(void)
{
    int             t, t2, j, i;
    unsigned int    clock_descr;
    int             found, good;
    int             maxl;

    Context = "SwapData";

    /* Allochiamo memoria per la struttura interna */
    if ((MyDescr = malloc((n_descr + 2) * sizeof(MYDESCRIPTOR))) == NULL) {
	WriteMessage(NOMEM);
	return (1);
    }
    /* Copia copia */
    for (j = t2 = t = 0; t < n_descr; ++t) {
	MyDescr[t].level = descr[t].level;
	MyDescr[t].type = descr[t].type;
	MyDescr[t].attr = descr[t].attr;
	if (MyDescr[t].attr == PI)
	    MyDescr[t].type = WIRE;
	MyDescr[t].fanin = descr[t].fanin;
	MyDescr[t].fanout = descr[t].fanout;
	MyDescr[t].from = descr[t].from;
	MyDescr[t].to = descr[t].to;
	MyDescr[t].GoodValue = 127;
	MyDescr[t].CurrentValue = 127;
	MyDescr[t].GroupId = -1;
	MyDescr[t].ErrorId = -1;
    }
    /* Inizializziamo il descrittore DUMMY (non c'era prima, lo creiamo
     * noi).Inizializzo il puntatore al clock (tanto poi lo calcola giusto). */
    clock_descr = n_descr + 1;
    MyDescr[n_descr + 1].fanin = 0;
    MyDescr[n_descr + 1].fanout = 0;
    MyDescr[n_descr + 1].level = 0;
    MyDescr[n_descr + 1].type = DUMMY;

    /* Un po' di valori di default (utili x il debugging) */
    MyDescr[n_descr].GoodValue = 127;
    MyDescr[n_descr].CurrentValue = 127;
    MyDescr[n_descr].ErrorId = -1;
    MyDescr[n_descr].GroupId = -1;
    MyDescr[n_descr + 1].GoodValue = 127;
    MyDescr[n_descr + 1].CurrentValue = 127;
    MyDescr[n_descr + 1].ErrorId = -1;
    MyDescr[n_descr + 1].GroupId = -1;

    /* Contiamo i Flip Flop */
    for (num_ff = t = 0; t < n_descr; ++t)
	if (MyDescr[t].type == FF)
	    num_ff++;
    /* e allochiamo l'array */
    if ((ff_array = (unsigned int *) malloc((num_ff + 1) * sizeof(unsigned int))) == NULL) {
	WriteMessage(NOMEM);
	return (1);
    }
    /* i primary inputs e i primary outputs */
    if ((po_array = (unsigned int *) malloc(n_po * sizeof(unsigned int))) == NULL) {
	WriteMessage(NOMEM);
	return (1);
    }
    if ((pi_array = (unsigned int *) malloc(n_pi * sizeof(unsigned int))) == NULL) {
	WriteMessage(NOMEM);
	return (1);
    }
    if ((lat_array = (unsigned int *) malloc(n_descr * sizeof(unsigned int))) == NULL) {
	WriteMessage(NOMEM);
	return (1);
    }
    /* Adesso cerchiamo il descrittore corrispondente al CLOCK */
    for (j = 0; j < n_descr; j++)
	if (MyDescr[j].type == FF) {
	    clock_descr = MyDescr[j].from[1];
	    break;
	}
    n_pi--;			/* per tener conto del clock */
    /* Possiamo scrivere i PI */
    for (j = i = 0; i < n_descr; ++i)
	if ((MyDescr[i].attr == PI) && (i != clock_descr))
	    pi_array[j++] = i;
    /* E i PO */
    for (j = i = 0; i < n_descr; ++i)
	if (MyDescr[i].attr == PO) {
	    po_array[j++] = i;
	}
    /* Last but not least i FF */
    for (j = i = 0; i < n_descr; ++i)
	if (MyDescr[i].type == FF) {
	    /* printf("ff[%d]:%d\n",j,i); */
	    ff_array[j++] = i;
	    MyDescr[i].fanin = 1;
	    MyDescr[MyDescr[i].from[1]].type = CLOCK;
	    MyDescr[MyDescr[i].from[1]].level = 0;
	}
    for (i = 0; i < n_pi; i++)
	lat_array[pi_array[i]] = i;

    /* Buttiamo via i vecchi dati */
    /* free(descr); (!)gs040396 */

    /* Un po' di controlli... */

#ifdef DEBUG
    for (good = 1, t = 0; t < n_descr; ++t)
	if (MyDescr[t].fanin > MAXIMUM_FANIN)
	    good = 0;
    if (!good) {
	printf("\n");
	WriteMessage(FANIN);
    }
#endif

    /* Controlliamo che a tutti i from[] corrisponda un to[] */
    good = 1;
    for (t = 0; t < n_descr; ++t) {
	for (t2 = 0; t2 < MyDescr[t].fanin; ++t2) {
	    for (found = 0, i = 0; i < MyDescr[MyDescr[t].from[t2]].fanout; ++i) {
		if (MyDescr[MyDescr[t].from[t2]].to[i] == t) {
		    found = 1;
		}
	    }
	    if (!found)
		good = 0;
	}
    }
    /* Controlliamo che a tutti i to[] corrisponda un from[] */
    for (t = 0; t < n_descr; ++t) {
	if (MyDescr[t].type != CLOCK) {
	    for (t2 = 0; t2 < MyDescr[t].fanout; ++t2) {
		for (found = 0, i = 0; i < MyDescr[MyDescr[t].to[t2]].fanin; ++i) {
		    if (MyDescr[MyDescr[t].to[t2]].from[i] == t) {
			found = 1;
		    }
		}
		if (!found)
		    good = 0;
	    }
	}
    }
    /* Controlliamo che i livelli siano coerenti */
    for (t = 0; t < n_descr; ++t) {
	for (t2 = 0; t2 < MyDescr[t].fanin; ++t2)
	    if (MyDescr[t].level <= MyDescr[MyDescr[t].from[t2]].level && MyDescr[t].type != FF)
		good = 0;
    }
    for (t = 0; t < n_descr; ++t) {
	for (t2 = 0; t2 < MyDescr[t].fanout; ++t2) {
	    if (MyDescr[t].level >= MyDescr[MyDescr[t].to[t2]].level && MyDescr[MyDescr[t].to[t2]].type != FF) {
		printf("\n%d %d;", t, t2);
		printf(" ...... %d %d \n", MyDescr[t].level, MyDescr[MyDescr[t].to[t2]].level);
		good = 0;
	    }
	}
    }

    if (!good) {
	printf("\n");
	WriteMessage(CONNECT);
    }
    /* Controlliamo che a tutti i descrittori siano di tipo noto */
    for (good = 1, t = 0; t < n_descr; ++t)
	if (MyDescr[t].type != AND && MyDescr[t].type != NAND && MyDescr[t].type != OR && MyDescr[t].type != NOR &&
	    MyDescr[t].type != BUF && MyDescr[t].type != NOT && MyDescr[t].type != EXOR && MyDescr[t].type != EXNOR &&
	    MyDescr[t].type != FF && MyDescr[t].type != WIRE && MyDescr[t].type != CLOCK)
	    good = 0;

    if (!good) {
	printf("\n");
	WriteMessage(UNDESCRIPTOR);
	return (1);
    }
    for (t = 0; t < num_ff; ++t)
	MyDescr[ff_array[t]].ErrorId = 0;
    maxl = 0;
    for (t = 0; t < num_ff; ++t) {
	for (found = 0, t2 = ff_array[t]; MyDescr[t2].type == FF; t2 = MyDescr[t2].from[0]) {
	    ++found;
	    if (maxl < found)
		maxl = found;
	    if (MyDescr[t2].ErrorId < found)
		MyDescr[t2].ErrorId = found;
	}
    }

#ifdef DEBUG2
    for (t = 0; t < num_ff; ++t)
	if (MyDescr[ff_array[t]].ErrorId > 1)
	    printf("%ld) %ld -> %ld\n", t, ff_array[t], MyDescr[ff_array[t]].ErrorId);
#endif

    t = 0;
    for (j = maxl; j >= 0; --j) {
	/* for(j=0; j<=maxl; ++j) { */
	for (i = 0; i < n_descr; ++i) {
	    if (MyDescr[i].type == FF && MyDescr[i].ErrorId == j) {
		ff_array[t++] = i;
	    }
	}
    }

#ifdef DEBUG2
    for (t = 0; t < num_ff; ++t)
	printf("%ld) %ld -> %ld\n", t, ff_array[t], MyDescr[ff_array[t]].ErrorId);
#endif

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
		for (t = 0; t < MyDescr[numdesc].fanin; ++t)
		    if (MyDescr[numdesc].from[t] == type)
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
	if (!anyerror && MyDescr[numdesc].fanin >= MAXIMUM_FANIN) {
	    anyerror = 1;
	    if (warnings < 2) {
		WriteMessage(BADFAULT);
		printf("\tIn line %d descriptor %ld STUCK-AT-%s\n", 2 + lines, numdesc, (value > 127) ? "UNO" : "ZERO");
		printf("\tbut descriptor's fanin is %ld and PRepsim cannot handle it.\n", MyDescr[numdesc].fanin);
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
