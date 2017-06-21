/***************************************************************************\
*                         *                                                 *
*  Giovanni A. Squillero  *   LungoPo Antonelli 203 - 10153 TORINO - ITALY  *
*                         *   +39-11-8994677  (Roma: +39-6-68307390)        *
*           (!)           *   charme5@athena.polito.it  (POLCLU::CAD6)      *
*                         *                                                 *
\***************************************************************************/

/**
 * REVISION HISTORY
 * 
 * When       Who          What
 * 
 * 27-05-95   Fulvio       
 * 
 * xx-12-95   Alex         Added support for FF with Set and Preset 
 *                         signals: the reset state can be read from
 *                         a file. Modified gatto.c for allocation and
 *                         file reading, sub_64oo.c:mreset() to load
 *                         the reset state into FF
 */

#ifdef DEBUG
#define CHECK "YES"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data2.h"
#include "mydata.h"

/* In qualche *.h e` definito, ma quale sia ESATTAMENTE esattamente questo
 * header varia da compilatore a compilatore... */
#ifndef max
#define max(X, Y)       ( (X>Y) ? X : Y )
#endif

/* un po' di leggibilita` raramente guasta (FC, 27/5/94) */

#define BIT(t) ((unsigned long)(1UL<<(t)))
/* NOTA x Reba: Alpha ha un barrel shifter? se no, sarebbe + veloce se si
 * usasse una look-up table */

#define INCR(num,mod) (((num) == (mod-1)) ? ((num) = 0) : (num)++ )
/* oppure ((num+1)%(mod-1)) purche' Aplha abbia un divisore VELOCE */

/* utilizzata in mEvalDescr() */
#define GETVAL(gate, pi) \
		( (MyDescr[MyDescr[gate].from[pi]].GroupId == id) ? \
		   (MyDescr[MyDescr[gate].from[pi]].CurrentValue) : \
		   (MyDescr[MyDescr[gate].from[pi]].GoodValue) )

/* Fine modifiche (FC, 27/5/94) */

int             mGPlast = -1;

/* Per capire queste due variabili fate riferimento al file message.c */
extern char    *Message;

extern int      n_pi;		/* # di ingressi primari */
extern int      n_po;		/* # di uscite */
extern int      n_descr;	/* # totale di descrittori */
extern MYDESCRIPTOR *MyDescr;	/* i descrittori */
extern ERROR   *ErrorList;	/* gli errori */
extern int      max_level;	/* la profondita` del circuito */
extern int     *pi_array;	/* i descrittori ingressi */
extern int     *lat_array;	/* i descrittori ingressi */
extern int     *po_array;	/* i descrittori uscite */
extern int      num_fault;	/* # di guasti caricati */
extern int      num_ff;		/* # flip flop */
extern int     *ff_array;	/* i descrittori ff */

extern          saved;

unsigned long  *pi_val;		/* valori dei PI */
unsigned int    temp;		/* uso generico */
unsigned int    last;

/* Per queste due variabili fate riferimento alla procedura mSchedule */
int            *Event;
int            *FirstEvent;
unsigned int   *CurrentLevel;

/* Per queste variabili guardate la mChooseFault */
int             chosen[SQ];
int             OldFaultLeft, LastAFWritten, *array;

MYDESCRIPTOR   *temp1;		/* generica */
int             mStatus = 0;	/* x il debug... */

/* Per capire queste guardate mLoadFlipFlop & mStoreFlipFlop */
int            *MyMem, MyMemSize, LastMMWritten;
double          MMS_temp;

/* Abbiamo appena resettato */
int             ResetSignal;
int             count;

/* Vedi la mEvalDescr_E x capire questa variabile */
unsigned long   inputs[MAXIMUM_FANIN];

/* Vettore contenente lo stato di reset, allocato da gatto.c e inizializzato
 * con i valori letti dall'apposito file o con 0 se il file non e` specificato */
unsigned long  *FF_reset_value;

/* A scopi statistici memorizzo il numero di reset che faccio */
unsigned long   numero_resets = 0;

#ifdef CHECK
int             overflow;
#endif

/**
extern unsigned int *max_f;
extern unsigned int *corr_f;
*/
unsigned int    my_events;

/* i PROTOTIPI */
int             mGetPattern(char **s, unsigned long *pi);
void            mreset(int id);
void            mSchedule(int x);
void            mEvalLevel(int level, unsigned int id);
void            mEvalDescr(int current, unsigned int id);
int             mChooseFault(unsigned int *ActiveFault);
void            mInjectFault(void);
void            mRemoveFault(void);
unsigned long   mEvalDescr_E(int current, unsigned int id);
int            *mModifiedFF(void);
void            mLoadFlipFlop(unsigned int id);
void            mStoreFlipFlop(int *Detected);
/* void mVisit(unsigned int id); */

/**************************************************************************
NOME:        FSim() - Fault simulation
PARAMETRI:   char **Seq: la sequenza di vettori da simulare
             unsigned int *ActiveFault: il vettore dei guasti attivi
             unsigned int FaultLeft: la dimensione di ActiveFault
             int *Detected: un vettore in cui scrivere il pattern a cui
                            un guasto e` stato detectato
             int *DNum: alla fine conterra` il numero di guasti detectati
DESCRIZIONE: almost PROOFS-like
***************************************************************************/
void            FSim(char **Seq, unsigned int *ActiveFault, unsigned int FaultLeft, int *Detected, int *DNum)
{
    int             inp;
    int             t, t2;
    unsigned int    id;
    unsigned long   good, bad;
    int             tmpFaultLeft, index, i, z;
    int             low, mid, high, found;

    tmpFaultLeft = FaultLeft;

    /* Reset input */
    *DNum = 0;

    count = 0;

    mGPlast = -1;
    /* Per riempire tutti e SQ i posti dell'ultimo "pacchetto" di guasti
     * abbiamo creato un guasto fittizio su di un descrittore inesistente: il
     * DUMMY_ERROR. (Il DUMMY_ERROR e` gia` stato trovato x default). Alcuni
     * guasti potrebbero avere il valore DUMMY_ERROR xche' sono stati scartati
     * dalla mReadFaultList */

    Detected[num_fault] = -1;


#ifdef DEBUG
    for (t = 0; t < n_descr; ++t) {
	Message = "Illegal from[] in circuit";
	for (t2 = 0; t2 < MyDescr[t].fanin; ++t2)
	    if (MyDescr[t].from[t2] >= n_descr || MyDescr[t].from[t2] < 0) {
		printf("Descriptor: %d : n_descr: %d\n", t, n_descr);
		WriteMessage(-1);
	    }
	Message = "Illegal to[] in circuit";
	for (t2 = 0; t2 < MyDescr[t].fanout; ++t2)
	    if (MyDescr[t].to[t2] >= n_descr || MyDescr[t].to[t2] < 0) {
		printf("Descriptor: %d : n_descr: %d\n", t, n_descr);
		WriteMessage(-1);
	    }
    }
#endif

    /* Inizializziamo le strutture di propagazione */
    for (t = 0; t <= max_level; ++t)
	FirstEvent[t] = LAST;	/* Azzerato primi eventi */
    for (t = 0; t < (2 + n_descr); ++t)
	Event[t] = UNSET;	/* Azzerato Eventi */

    /* Prima simulazione: valutiamo TUTTO, id=0 e infine resetta */
    for (t = 0; t < n_descr; ++t)
	if (MyDescr[t].type != CLOCK)
	    mSchedule(t);

#ifdef CHECK
    overflow = 0;
#endif

    mreset(id = 0);

    /* Finche' riusciamo a leggere dal file di input */
    while ((inp = mGetPattern(Seq, pi_val))) {
	++id;
	if (inp == -1) {	/* E` un RESET */
	    mreset(id);
	} else {

#ifdef DEBUG
	    for (t = 0; t < n_descr; ++t) {
		Message = "Illegal from[] in circuit";
		for (t2 = 0; t2 < MyDescr[t].fanin; ++t2)
		    if (MyDescr[t].from[t2] >= n_descr || MyDescr[t].from[t2] < 0) {
			printf("Descriptor: %d : n_descr: %d\n", t, n_descr);
			WriteMessage(-1);
		    }
		Message = "Illegal from[] in circuit";
		for (t2 = 0; t2 < MyDescr[t].fanout; ++t2)
		    if (MyDescr[t].to[t2] >= n_descr || MyDescr[t].to[t2] < 0) {
			printf("Descriptor: %d : n_descr: %d\n", t, n_descr);
			WriteMessage(-1);
		    }
	    }
#endif

	    mStatus = 0;
	    ++count;		/* # pattern letto */

	    /* Copiamo gli input e propaghiamo l'evento se il valore e` cambiato */
	    for (t = 0; t < n_pi; ++t) {
		temp = pi_array[t];
		if (MyDescr[temp].GoodValue != pi_val[t]) {
		    mSchedule(temp);
		}
	    }
	    /* Aggiorniamo i ff e propaghiamo se sono cambiati */
	    for (t = 0; t < num_ff; ++t) {
		temp1 = &MyDescr[ff_array[t]];
		if (ResetSignal || MyDescr[(*temp1).from[0]].GoodValue != (*temp1).GoodValue) {
		    mSchedule(ff_array[t]);
		}
	    }
	    /* A questo punto sappiamo quali eventi di livello 0 dobbiamo
	     * considerare: possiamo valutare tutto il circuito */
	    for (t = 0; t <= max_level; ++t)
		mEvalLevel(t, id);
	    /* Copiamo i valori appena calcolati (id corrente) in .GoodValue */
	    for (t = 0; t < n_descr; ++t) {
		temp1 = &MyDescr[t];
		if ((*temp1).GroupId == id)
		    (*temp1).GoodValue = (*temp1).CurrentValue;
	    }
	    /* E salviamo i valori */

#ifdef DEBUG
	    for (t = 0; t < n_descr; ++t)
		if (MyDescr[t].GoodValue != ZERO && MyDescr[t].GoodValue != UNO && MyDescr[t].type != CLOCK) {
		    Message = "Found a not ZERO-or-UNO value in good circuit";
		    WriteMessage(-1);
		}
#endif


	    /* Potremmo avere degli eventi sui ff, ma non ci interessano */
	    for (t = 0; t < num_ff; ++t)
		Event[(ff_array[t])] = UNSET;
	    FirstEvent[0] = LAST;

	    last = 0;

	    OldFaultLeft = FaultLeft;

	    /* Finche` ci sono guasti da considerare... la mChooseFault ritorna il
	     * pacchetto di SQ guasti da iniettare nel circuito */
	    while (mChooseFault(ActiveFault)) {

		mStatus = 1;
		++id;		/* l'id cambia sempre */
		/* Iniettiamo il pacchetto di SQ guasti */
		mInjectFault();

		my_events = 0;

		/* E aggiorniamo i ff */
		mLoadFlipFlop(id);
		/* Valutiamo tutto il circuito (quello che serve) NB: gli unici
		 * eventi che abbiamo adesso sono x ff cambiati e i guasti */
		for (t = 0; t <= max_level; ++t)
		    mEvalLevel(t, id);

		/* Adesso confrontiamo le uscite: se sono state valutate (id
		 * corrente) E sono diverse dal valore della macchina buona allora
		 * vediamo quali errori abbiamo beccato (sembra il sistema piu`
		 * rapido) */
		for (t2 = 0; t2 < n_po; ++t2) {
		    if (MyDescr[po_array[t2]].GroupId == id) {
			good = MyDescr[po_array[t2]].GoodValue;
			bad = MyDescr[po_array[t2]].CurrentValue;
			if (good != bad)
			    for (t = 0; t < SQ; ++t) {
				if (!(good) != !(bad & BIT(t)) && !Detected[chosen[t]]) {
				    Detected[chosen[t]] = count;
				    (*DNum)++;
				    --FaultLeft;
				}
			    }
		    }
		}

#ifdef DEBUG
		for (t = 0; t < n_descr; ++t)
		    if (MyDescr[t].GoodValue != UNO && MyDescr[t].GoodValue != ZERO && MyDescr[t].type != CLOCK) {
			printf("\ndescr[%d].GoodValue = %lx\n", t, MyDescr[t].GoodValue);
			Message = "Invalid GoodValue";
			WriteMessage(-1);
		    }
#endif

		/* E, infine, salviamo i ff */
		mStoreFlipFlop(Detected);
		/* Adesso possiamo rimuovere i SQ guasti */
		mRemoveFault();
	    }			/* while */
	    /* Un po' di fault dropping... */
	    for (LastAFWritten = 0, t = 0; t < OldFaultLeft; ++t)
		if (!Detected[ActiveFault[t]]) {
		    ActiveFault[LastAFWritten++] = ActiveFault[t];
		}
	    /* Se avevamo appena resettato */
	    if (ResetSignal)
		ResetSignal = 0;
	}			/* else del RESET */
    }
/****
  Cut 1
****/
#ifdef CHECK
    if (overflow) {
	printf("\n");
	WriteMessage(OVERFLOW);
    }
#endif

}

/**************************************************************************
NOME:           mSchedule
PARAMETRI:      Il # di un descrittore
DESCRIZIONE:    Inserisce nella tabella dei descrittori "da rivalutare" il
		descrittore indicato.

		Un'esempio x capire la struttura dati:

            __Event__               __FirstEvent__

			0: UNSET                0: LAST
			1: LAST                 1: x

		       ...                     ...

		      x-1: UNSET    max_level-1: LAST
			x: 1          max_level: LAST
		      x+1: UNSET

		n_descr-1: UNSET
		  n_descr: UNSET
		n_descr+1: UNSET

	Nessun evento su descrittori di livello 0.
	Il primo descrittore da rivalutare del livello 1 e` il numero "x",
		nella posizione "x" di Event c'e` scritto "1". Allora
		il secondo da valutare e` il numero "1", in Event[1]
		troviamo LAST, allora il numero "1" e` anche l'ultimo.
	etc...
***************************************************************************/
void            mSchedule(int x)
{
    int             level;


#ifdef DEBUG
    if (x < 0 || x > (1 + n_descr)) {
	Message = "Event on a non-existing descriptor";
	WriteMessage(-1);
    }
#endif

    /* Dobbiamo sapere in che livello (profondita`) e` il descrittore x */
    level = MyDescr[x].level;

#ifdef DEBUG
    if (level > max_level) {
	Message = "Event at a non-existing level";
	WriteMessage(-1);
    }
#endif

    /* Se non ancora "Schedulato" lo concateniamo */
    if (Event[x] == UNSET) {
	Event[x] = FirstEvent[level];
	FirstEvent[level] = x;
    }
}

/**************************************************************************
NOME:           mEvalLevel
PARAMETRI:      Il # del livello e l'id corrente
DESCRIZIONE:    Chiama la mEvalDescr x tutti i descrittori "da valutare"
		del livello corrente.
	Inoltre resetta la struttura di propagazione (Event, FirstEvent).
	Il codice e` ovvio.
***************************************************************************/
void            mEvalLevel(int level, unsigned int id)
{
    int             current, vtemp;
    int             t, u;


    t = 0;
    if (level) {
	current = FirstEvent[level];
	while (current != LAST) {
	    vtemp = current;
	    CurrentLevel[t++] = vtemp;
	    current = Event[current];
	    Event[vtemp] = UNSET;
	}
	FirstEvent[level] = LAST;
    } else {
	for (u = 0; u < n_pi; ++u) {
	    if (Event[pi_array[u]] != UNSET) {
		CurrentLevel[t++] = pi_array[u];
		Event[pi_array[u]] = UNSET;
	    }
	}
	for (u = 0; u < num_ff; ++u) {
	    if (Event[ff_array[u]] != UNSET) {
		CurrentLevel[t++] = ff_array[u];
		Event[ff_array[u]] = UNSET;
	    }
	}
	FirstEvent[0] = LAST;

    }
    for (--t; t >= 0; --t)
	mEvalDescr(CurrentLevel[t], id);
}

/**************************************************************************
NOME:           mEvalDescr
PARAMETRI:      Il # del descrittore e l'id corrente
DESCRIZIONE:    Valuta il descrittore, se il valore e` cambiato allora
		propaga le conseguenze.
***************************************************************************/
void            mEvalDescr(int current, unsigned int id)
{
    unsigned int   *to;
    int             t, fanin;
    unsigned long   result = 0;
    int             from;

#ifdef DEBUG
    if (id == -1) {
	printf("\nAbsurd Group Id\n");
	WriteMessage(-1);
    }
    if (current < 0 || current > (1 + n_descr)) {
	Message = "Evaluating a non-existing descriptor";
	WriteMessage(-1);
    }
#endif

    fanin = MyDescr[current].fanin;

#ifdef DEBUG
    for (t = 0; t < fanin; ++t) {
	from = MyDescr[current].from[t];
	if (MyDescr[from].GroupId == id)
	    inputs[t] = MyDescr[from].CurrentValue;
	else {
	    inputs[t] = MyDescr[from].GoodValue;
	    if (mStatus && inputs[t] != ZERO && inputs[t] != UNO) {
		printf("Found a non ZERO-or-UNO value on good circuit\n");
		WriteMessage(-1);
	    }
	}
    }
    for (t = 0; t < fanin; ++t)
	if (inputs[t] != ZERO && inputs[t] != UNO && !mStatus && !(ResetSignal && inputs[t] == 127)) {
	    printf("\nDescr: %d, input %d: %d (%d, 0x%08xh), Reset: %d", current, t, MyDescr[current].from[t], inputs[t], inputs[t], ResetSignal);
	    Message = "Input is not ZERO-or-UNO in good circuit";
	    WriteMessage(-1);
	}
    if (id == -1) {
	Message = "Group Id is absurd";
	WriteMessage(-1);
    }
#endif

    /* NOTE: 1- Se i padri (.from[]) hanno l'id corrente allora usa il
     * .CurrentValue (che e` stato appena calcolato). Altrimenti prende il
     * .GoodValue (il descrittore NON e` stato valutato: il suo valore e` lo
     * stesso del circuito buono). 2- Se il descrittore e` guasto, il suo type
     * e` stato modificato e nel default, viene chiamata la mEvalDescr_E. Cosi`
     * non si perde tempo nella valutazione del circuito GOOD */

    switch (MyDescr[current].type) {
    case AND:
	switch (fanin) {
	case 1:		/* IMPOSSIBILE */
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	case 2:
	    result =
		GETVAL(current, 0) &
		GETVAL(current, 1);
	    break;
	case 3:
	    result =
		GETVAL(current, 0) &
		GETVAL(current, 1) &
		GETVAL(current, 2);
	    break;
	case 4:
	    result =
		GETVAL(current, 0) &
		GETVAL(current, 1) &
		GETVAL(current, 2) &
		GETVAL(current, 3);
	    break;
	case 5:
	    result =
		GETVAL(current, 0) &
		GETVAL(current, 1) &
		GETVAL(current, 2) &
		GETVAL(current, 3) &
		GETVAL(current, 4);
	    break;
	default:
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	}
	break;
    case NAND:
	switch (fanin) {
	case 1:		/* IMPOSSIBILE */
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	case 2:
	    result = ~(
			  GETVAL(current, 0) &
			  GETVAL(current, 1));
	    break;
	case 3:
	    result = ~(
			  GETVAL(current, 0) &
			  GETVAL(current, 1) &
			  GETVAL(current, 2));
	    break;
	case 4:
	    result = ~(
			  GETVAL(current, 0) &
			  GETVAL(current, 1) &
			  GETVAL(current, 2) &
			  GETVAL(current, 3));
	    break;
	case 5:
	    result = ~(
			  GETVAL(current, 0) &
			  GETVAL(current, 1) &
			  GETVAL(current, 2) &
			  GETVAL(current, 3) &
			  GETVAL(current, 4));
	    break;
	default:
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	}
	break;
    case OR:
	switch (fanin) {
	case 1:		/* IMPOSSIBILE */
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	case 2:
	    result =
		GETVAL(current, 0) |
		GETVAL(current, 1);
	    break;
	case 3:
	    result =
		GETVAL(current, 0) |
		GETVAL(current, 1) |
		GETVAL(current, 2);
	    break;
	case 4:
	    result =
		GETVAL(current, 0) |
		GETVAL(current, 1) |
		GETVAL(current, 2) |
		GETVAL(current, 3);
	    break;
	case 5:
	    result =
		GETVAL(current, 0) |
		GETVAL(current, 1) |
		GETVAL(current, 2) |
		GETVAL(current, 3) |
		GETVAL(current, 4);
	    break;
	default:
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	}
	break;
    case NOR:
	switch (fanin) {
	case 1:		/* IMPOSSIBILE */
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	case 2:
	    result = ~(
			  GETVAL(current, 0) |
			  GETVAL(current, 1));
	    break;
	case 3:
	    result = ~(
			  GETVAL(current, 0) |
			  GETVAL(current, 1) |
			  GETVAL(current, 2));
	    break;
	case 4:
	    result = ~(
			  GETVAL(current, 0) |
			  GETVAL(current, 1) |
			  GETVAL(current, 2) |
			  GETVAL(current, 3));
	    break;
	case 5:
	    result = ~(
			  GETVAL(current, 0) |
			  GETVAL(current, 1) |
			  GETVAL(current, 2) |
			  GETVAL(current, 3) |
			  GETVAL(current, 4));
	    break;
	default:
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	}
	break;
    case BUF:
	result = GETVAL(current, 0);
	break;
    case NOT:
	result = ~GETVAL(current, 0);
	break;
    case EXOR:
	switch (fanin) {
	case 1:		/* IMPOSSIBILE */
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	case 2:
	    result =
		GETVAL(current, 0) ^
		GETVAL(current, 1);
	    break;
	case 3:
	    result =
		GETVAL(current, 0) ^
		GETVAL(current, 1) ^
		GETVAL(current, 2);
	    break;
	case 4:
	    result =
		GETVAL(current, 0) ^
		GETVAL(current, 1) ^
		GETVAL(current, 2) ^
		GETVAL(current, 3);
	    break;
	case 5:
	    result =
		GETVAL(current, 0) ^
		GETVAL(current, 1) ^
		GETVAL(current, 2) ^
		GETVAL(current, 3) ^
		GETVAL(current, 4);
	    break;
	default:
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	}
	break;
    case EXNOR:
	switch (fanin) {
	case 1:		/* IMPOSSIBILE */
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	case 2:
	    result = ~(
			  GETVAL(current, 0) ^
			  GETVAL(current, 1));
	    break;
	case 3:
	    result = ~(
			  GETVAL(current, 0) ^
			  GETVAL(current, 1) ^
			  GETVAL(current, 2));
	    break;
	case 4:
	    result = ~(
			  GETVAL(current, 0) ^
			  GETVAL(current, 1) ^
			  GETVAL(current, 2) ^
			  GETVAL(current, 3));
	    break;
	case 5:
	    result = ~(
			  GETVAL(current, 0) ^
			  GETVAL(current, 1) ^
			  GETVAL(current, 2) ^
			  GETVAL(current, 3) ^
			  GETVAL(current, 4));
	    break;
	default:
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	}
	break;
    case FF:
	if (ResetSignal) {
	    result = ZERO;
	} else
	    result = GETVAL(current, 0);
	break;
    case WIRE:

#ifdef DEBUG
	if (MyDescr[current].attr != PI) {
	    Message = "WIRE, but not PI";
	    WriteMessage(-1);
	}
#endif

	/* Tutti i WIRE sono (o meglio, dovrebbero essere) PI... ricopiamo il
	 * valore del pattern */
	/* printf("WARNING: secondo me (FC) qui non dovrebbe arrivare\n"); */
	/* for(t=0; t<n_pi; ++t) if(pi_array[t]==current) result = pi_val[t]; */
	result = pi_val[lat_array[current]];
	break;
    case CLOCK:
	/* Puo` essere utile nel debug... */
	result = ZERO;
	break;
	/* Il descrittore inesistente: fanin=0, fanout=0 */
    case DUMMY:
    case DUMMY_E:
	result = ZERO;
	break;
    default:
	/* Allora e` un descrittore guasto... */
	result = mEvalDescr_E(current, id);
    }

#ifdef DEBUG
    if (!mStatus && result != ZERO && result != UNO) {
	printf("\nFound a non ZERO-or-UNO value in good circuit\n");
	printf("Current Group Id: %d, mreset: %d, descriptor: %d, value: %d, ", id, ResetSignal, current, result);
	printf("type: %d", MyDescr[current].type);
	WriteMessage(-1);
    }
#endif

    /* Scriviamo il risultato in .CurrentValue, aggiorniamo il GroupId e (se
     * .CurrentValue != .GoodValue) propaghiamo il risultato */
    MyDescr[current].CurrentValue = result;
    MyDescr[current].GroupId = id;
    if (result != MyDescr[current].GoodValue || (MyDescr[current].type == FF && ResetSignal)) {
	my_events++;
	for (to = (unsigned int *) MyDescr[current].to, t = 0; t < MyDescr[current].fanout; ++t, ++to)
	    mSchedule(*to);
    }
}

/**************************************************************************
NOME:           mChooseFault
PARAMETRI:      -
DESCRIZIONE:    Sceglie SQ guasti e li mette in chosen[].
		Ritorna 1 se ne ha scelto ALMENO 1
***************************************************************************/
int             mChooseFault(unsigned int *ActiveFault)
{
    unsigned int    t;
    int             num;
    ERROR          *fast;

    num = 0;
    /* Evitiamo di mettere uno STACK-AT-UNO su di un filo gia` ad UNO etc...
     * NOTA: Se il guasto i-esimo ha modificato flip flop noi non possiamo
     * sapere il valore dei descrittori nel circuito prima di valutarli, allora
     * iniettiamo il guasto e basta (primo if) */
    for (t = last; t < OldFaultLeft && num < SQ; ++t) {
	fast = &ErrorList[ActiveFault[t]];
	if (MyMem[fast->Modified] != -1) {
	    chosen[num++] = ActiveFault[t];
	} else if (fast->Type == -1 &&
		   ((fast->Value == STUCK_AT_UNO && MyDescr[fast->NumDesc].GoodValue == ZERO) ||
		    (fast->Value == STUCK_AT_ZERO && MyDescr[fast->NumDesc].GoodValue == UNO))) {
	    chosen[num++] = ActiveFault[t];
	} else if ((fast->Type != -1 &&
		    ((fast->Value == STUCK_AT_UNO && MyDescr[MyDescr[fast->NumDesc].from[fast->Type]].GoodValue == ZERO) ||
		     (fast->Value == STUCK_AT_ZERO && MyDescr[MyDescr[fast->NumDesc].from[fast->Type]].GoodValue == UNO)))
		   || MyDescr[fast->NumDesc].type == FF) {
	    chosen[num++] = ActiveFault[t];
	} else {
	    /* Per evitare noie con il buffer circolare: la posizione MyMemSize
	     * contiene un "-1" e non viene mai scritta */
	    ErrorList[ActiveFault[t]].Modified = MyMemSize;
	}

#ifdef DEBUG
	if (ActiveFault[t] >= num_fault) {
	    printf("%d: %d: %d\n", t, ActiveFault[t], num_fault);
	    Message = "Active fault greater than num_fault";
	    WriteMessage(-1);
	}
	/* if (Detected[ActiveFault[t]]) { Message = "Active fault on dropped
	 * fault"; WriteMessage(-1); } */
#endif
    }
    last = t;			/* Aggiorniamo last */

    /* Se non ne abbiamo scelto nessuno ritorna 0 */
    if (!num)
	return (0);

    /* Riempi il pacchetto (fino al SQ-esimo) con dei DUMMY_ERROR */
    for (; num < SQ; ++num)
	chosen[num] = num_fault;

    return (1);
}

/**************************************************************************
NOME:           mInjectFault
PARAMETRI:      -
DESCRIZIONE:    Inietta i SQ errori in chosen[].
		L'errore e` iniettato modificando un bit del type del
	descrittore.

	X X X X X X X * X X X X X X X X
	~~~~~~~~~~~~~~^ ~~~~~~~|~~~~~~~
		      |   il type della create() e` su 8 bit
	   questo e` il bit di errore (il nostro type e` su 16 bit)

***************************************************************************/
void            mInjectFault(void)
{
    int             t;
    ERROR          *zufo;

    for (t = 0; t < SQ; ++t) {
	/* Nota: possiamo avere piu` di un guasto su un unico descrittore.
	 * Abbiamo una catena di guasti linkata... */
	zufo = &(ErrorList[chosen[t]]);
	if (MyDescr[(*zufo).NumDesc].type & ERRORFLAG)
	    (*zufo).Next = MyDescr[(*zufo).NumDesc].ErrorId;
	else {
	    (*zufo).Next = chosen[t];
	    mSchedule((*zufo).NumDesc);
	    MyDescr[(*zufo).NumDesc].type |= ERRORFLAG;
	}

	MyDescr[(*zufo).NumDesc].ErrorId = chosen[t];
    }
    return;
}

/**************************************************************************
NOME:           mRemoveFault
PARAMETRI:      -
DESCRIZIONE:    Rimuove gli errori (e resetta chosen[] )
***************************************************************************/
void            mRemoveFault(void)
{
    int             t;
    int             cfval;
    MYDESCRIPTOR   *Giovanni;
    for (t = 0; t < SQ; ++t) {
	Giovanni = &MyDescr[ErrorList[chosen[t]].NumDesc];
	(*Giovanni).type &= ~ERRORFLAG;
	(*Giovanni).ErrorId = -1;
/***Gatto?
	if (max_f[chosen[t]] < cfval)
	    max_f[chosen[t]] = cfval; 
*/
	chosen[t] = num_fault;

    }
    return;
}

/**************************************************************************
NOME:           mEvalDescr_E
PARAMETRI:      il # del descrittore da valutare e l'id corrente
DESCRIZIONE:    Valuta un descrittore guasto.
***************************************************************************/
unsigned long   mEvalDescr_E(int current, unsigned int id)
{
    unsigned int    errorid, temperrorid;
    unsigned long   result = 0;
    int             fanin, t;
    int             bit;
    int             finito;


#ifdef DEBUG
    if (id == -1) {
	printf("\nAbsurd Group Id\n");
	WriteMessage(-1);
    }
#endif

    fanin = MyDescr[current].fanin;
    errorid = MyDescr[current].ErrorId;

    /* Copiamo gli ingressi nel vettore input */
    for (t = 0; t < fanin; ++t) {
	if (MyDescr[MyDescr[current].from[t]].GroupId == id)
	    inputs[t] = MyDescr[MyDescr[current].from[t]].CurrentValue;
	else
	    inputs[t] = MyDescr[MyDescr[current].from[t]].GoodValue;
    }

    finito = 0;
    temperrorid = errorid;
    /* Correggiamo il vettore input se abbiamo degli STUCK in ingresso */
    while (!finito) {
	if (ErrorList[temperrorid].Type != -1) {
	    for (bit = 0; chosen[bit] != temperrorid; ++bit);
	    if (!ErrorList[temperrorid].Value)	/* ie. STUCK_AT_ZERO */
		inputs[ErrorList[temperrorid].Type] &= ~BIT(bit);
	    else		/* ie. STUCK_AT_ZERO */
		inputs[ErrorList[temperrorid].Type] |= BIT(bit);
	}
	if (temperrorid == ErrorList[temperrorid].Next)
	    finito = 1;
	else
	    temperrorid = ErrorList[temperrorid].Next;
    }

    /* Stessa procedura x valutare... */
    switch (MyDescr[current].type) {
    case AND_E:
	switch (fanin) {
	case 1:		/* IMPOSSIBILE */
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	case 2:
	    result =
		inputs[0] &
		inputs[1];
	    break;
	case 3:
	    result =
		inputs[0] &
		inputs[1] &
		inputs[2];
	    break;
	case 4:
	    result =
		inputs[0] &
		inputs[1] &
		inputs[2] &
		inputs[3];
	    break;
	case 5:
	    result =
		inputs[0] &
		inputs[1] &
		inputs[2] &
		inputs[3] &
		inputs[4];
	    break;
	default:
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	}
	break;
    case NAND_E:
	switch (fanin) {
	case 1:		/* IMPOSSIBILE */
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	case 2:
	    result = ~(
			  inputs[0] &
			  inputs[1]);
	    break;
	case 3:
	    result = ~(
			  inputs[0] &
			  inputs[1] &
			  inputs[2]);
	    break;
	case 4:
	    result = ~(
			  inputs[0] &
			  inputs[1] &
			  inputs[2] &
			  inputs[3]);
	    break;
	case 5:
	    result = ~(
			  inputs[0] &
			  inputs[1] &
			  inputs[2] &
			  inputs[3] &
			  inputs[4]);
	    break;
	default:
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	}
	break;
    case OR_E:
	switch (fanin) {
	case 1:		/* IMPOSSIBILE */
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	case 2:
	    result =
		inputs[0] |
		inputs[1];
	    break;
	case 3:
	    result =
		inputs[0] |
		inputs[1] |
		inputs[2];
	    break;
	case 4:
	    result =
		inputs[0] |
		inputs[1] |
		inputs[2] |
		inputs[3];
	    break;
	case 5:
	    result =
		inputs[0] |
		inputs[1] |
		inputs[2] |
		inputs[3] |
		inputs[4];
	    break;
	default:
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	}
	break;
    case NOR_E:
	switch (fanin) {
	case 1:		/* IMPOSSIBILE */
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	case 2:
	    result = ~(
			  inputs[0] |
			  inputs[1]);
	    break;
	case 3:
	    result = ~(
			  inputs[0] |
			  inputs[1] |
			  inputs[2]);
	    break;
	case 4:
	    result = ~(
			  inputs[0] |
			  inputs[1] |
			  inputs[2] |
			  inputs[3]);
	    break;
	case 5:
	    result = ~(
			  inputs[0] |
			  inputs[1] |
			  inputs[2] |
			  inputs[3] |
			  inputs[4]);
	    break;
	default:
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	}
	break;
    case BUF_E:
	result = inputs[0];
	break;
    case NOT_E:
	result = ~inputs[0];
	break;
    case EXOR_E:
	switch (fanin) {
	case 1:		/* IMPOSSIBILE */
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	case 2:
	    result =
		inputs[0] ^
		inputs[1];
	    break;
	case 3:
	    result =
		inputs[0] ^
		inputs[1] ^
		inputs[2];
	    break;
	case 4:
	    result =
		inputs[0] ^
		inputs[1] ^
		inputs[2] ^
		inputs[3];
	    break;
	case 5:
	    result =
		inputs[0] ^
		inputs[1] ^
		inputs[2] ^
		inputs[3] ^
		inputs[4];
	    break;
	default:
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	}
	break;
    case EXNOR_E:
	switch (fanin) {
	case 1:		/* IMPOSSIBILE */
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	case 2:
	    result = ~(
			  inputs[0] ^
			  inputs[1]);
	    break;
	case 3:
	    result = ~(
			  inputs[0] ^
			  inputs[1] ^
			  inputs[2]);
	    break;
	case 4:
	    result = ~(
			  inputs[0] ^
			  inputs[1] ^
			  inputs[2] ^
			  inputs[3]);
	    break;
	case 5:
	    result = ~(
			  inputs[0] ^
			  inputs[1] ^
			  inputs[2] ^
			  inputs[3] ^
			  inputs[4]);
	    break;
	default:
	    printf("Wrong fanin\n");
	    WriteMessage(-1);
	    break;
	}
	break;
    case FF_E:
	result = inputs[0];
	if (ResetSignal)
	    result = ZERO;
	break;
    case WIRE_E:
	/* printf("WARNING: secondo me (FC) qui non dovrebbe arrivare\n"); */
	/* for(t=0; t<n_pi; ++t) if(pi_array[t]==current) result = pi_val[t]; */
	result = pi_val[lat_array[current]];
	break;
    default:
	Message = "Unknown descriptor type";
	WriteMessage(-1);
    }

    /* Come per gli input, dobbiamo "guastare" l'output */
    finito = 0;
    while (!finito) {
	if (ErrorList[errorid].Type == -1) {
	    for (bit = 0; chosen[bit] != errorid; ++bit);
	    if (!ErrorList[errorid].Value)	/* ie. STUCK_AT_ZERO */
		result &= ~BIT(bit);
	    else		/* ie. STUCK_AT_UNO */
		result |= BIT(bit);
	}
	if (errorid == ErrorList[errorid].Next)
	    finito = 1;
	else
	    errorid = ErrorList[errorid].Next;
    }

    return (result);
}

/**************************************************************************
NOME:           mreset
PARAMETRI:      l'id corrente
DESCRIZIONE:    RESET! e propaga le conseguenze
***************************************************************************/
void            mreset(int id)
{
    int             t, t2;
    unsigned int    zufo;

    /* A scopi statistici conto il numero di chiamate */
    numero_resets++;

    /* Test purpose only...
     * for (t=0; t < num_ff; ++t) printf("%lu ",FF_reset_value[t]);
     * printf("\n");
     */

    for (t = 0; t < num_ff; ++t) {
	/*   Acquisisce il descrittore del padre del FF */
	zufo = MyDescr[ff_array[t]].from[0];
	/*   Setta il padre del FF al valore di reset (caricato all'inizio) */
	MyDescr[zufo].GoodValue = ZERO;
	/*   Setta il FF al valore di reset */
	MyDescr[ff_array[t]].GoodValue = ZERO;
	MyDescr[zufo].GroupId = id;
	/*   Rivalutiamo il padre del FF */
	mSchedule(zufo);
	/*   Rivalutiamo tutti i figli del padre del FF (ie. i fratelli del FF) */
	for (t2 = 0; t2 < MyDescr[zufo].fanout; ++t2)
	    mSchedule(MyDescr[zufo].to[t2]);
    }

    /* Test purpose only...
     * for (t=0; t < num_ff; ++t) printf("%lu ",MyDescr[ff_array[t]].GoodValue);
     * printf("\n");
     */


    /* Resettiamo il buffer circolare con i ff modificati dai guasti */
    LastMMWritten = 0;

#ifdef CHECK
    for (t = 0; t < MyMemSize; ++t)
	MyMem[t] = -1;
#endif

    for (t = 0; t < num_fault; ++t)
	ErrorList[t].Modified = MyMemSize;

    ResetSignal = 1;
}

/**************************************************************************
NOME:           mModifiedFF
PARAMETRI:      -
DESCRIZIONE:    ritorna un puntatore ad un array con scritti sopra
		i ff modificati (quelli schedulati)
***************************************************************************/
int            *
                mModifiedFF(void)
{
    unsigned int    t, vtemp, i;

    for (i = 0, t = FirstEvent[0]; t != LAST; t = vtemp) {
	vtemp = Event[t];
	Event[t] = UNSET;
	array[i++] = t;
    }
    array[i] = -1;
    FirstEvent[0] = LAST;

    return (array);
}

/**************************************************************************
NOME:           mStoreFlipFlop
PARAMETRI:      ...
DESCRIZIONE:    Assegna ad ogni fault di chosen[] l'elenco dei ff in cui
		la macchina guasta ha un valore diverso dalla macchina
	buona. Usiamo un buffer circolare MyMem, ogni errore ha nel
	campo .Modified l'indirizzo della prima cella di MyMem dove
	sono memorizzati i ff che ha modificato. L'elenco termina con
	-1.
***************************************************************************/
void            mStoreFlipFlop(int *Detected)
{
    int            *list;
    int             t, t2;
    int             zufo;
    int             incro;

    list = mModifiedFF();
    for (t = 0; t < SQ; ++t) {
	if (!Detected[chosen[t]]) {
	    ErrorList[chosen[t]].Modified = LastMMWritten;
	    for (t2 = 0; list[t2] != -1; ++t2) {
		zufo = MyDescr[list[t2]].from[0];
		if ((MyDescr[zufo].GoodValue & BIT(t)) !=
		    (MyDescr[zufo].CurrentValue & BIT(t))) {

#ifdef CHECK
		    if (MyMem[LastMMWritten] != -1)
			overflow = 1;
#endif
		    MyMem[LastMMWritten] = list[t2];
		    LastMMWritten = (LastMMWritten+1)%MyMemSize;
		}
	    }
	    MyMem[LastMMWritten] = -1;
	    INCR(LastMMWritten, MyMemSize);
	}
    }
}

/**************************************************************************
NOME:           mLoadFlipFlop
PARAMETRI:      l'id corrente
DESCRIZIONE:    x ogni guasto di chosen[] va a leggere la lista dei ff
		modificati e complementa il bit opportuno del
	GoodValue.
***************************************************************************/
void            mLoadFlipFlop(unsigned int id)
{
    unsigned int    t, addr;
    int             Marisa;

    /* PRIMO  : Copiamo nei padri dei ff i valori che avevano i ff */
    for (t = 0; t < num_ff; ++t) {
	Marisa = MyDescr[ff_array[t]].from[0];
	MyDescr[Marisa].CurrentValue = MyDescr[ff_array[t]].GoodValue;
	MyDescr[Marisa].GroupId = id;
	/* SECONDO: Rivalutiamo padre */
	mSchedule(Marisa);
	/* NB: Qui non occorre rivalutare i "fratelli" del ff come nel mreset
	 * xche' modifichiamo il .CurrentValue e NON il .GoodValue */
    }

    /* TERZO  : Complementiamo dove occorre */
    for (t = 0; t < SQ && chosen[t] < num_fault; ++t) {
	for (addr = ErrorList[chosen[t]].Modified; MyMem[addr] != -1; INCR(addr, MyMemSize)) {
	    mSchedule(MyMem[addr]);
	    if (MyDescr[MyMem[addr]].GoodValue == UNO)
		MyDescr[MyDescr[MyMem[addr]].from[0]].CurrentValue &= ~BIT(t);
	    else if (MyDescr[MyMem[addr]].GoodValue == ZERO)
		MyDescr[MyDescr[MyMem[addr]].from[0]].CurrentValue |= BIT(t);
	    else
		WriteMessage(-1);

#ifdef CHECK
	    MyMem[addr] = -1;
#endif
	}
    }
}

int             mGetPattern(char **s, unsigned long *pi)
{
    char           *c;

    ++mGPlast;

    if (!*s[mGPlast])
	return (0);		/* Finito! */
    else if (*s[mGPlast] == '#')
	return (-1);		/* Reset */

    for (c = s[mGPlast]; *c; ++c)
	*(pi++) = *c == '0' ? ZERO : UNO;
    return (1);
}

void            InitFSim(void)
{
#ifdef CHECK
    int t;
#endif

    /* Allochiamo tutto cio` che ci occorre */
    MMS_temp = 1.0 * num_fault * (num_ff + 1) * saved;
    MMS_temp /= 1000;
    MyMemSize = max((unsigned int) MMS_temp, 1023);
    if ((MyMem = (int *) malloc(sizeof(int) * (MyMemSize + 1))) == NULL) {
	WriteMessage(NOMEM);
	return;
    }
#ifdef CHECK
    for (t = 0; t < MyMemSize; ++t)
	MyMem[t] = -1;
#endif

    MyMem[MyMemSize] = -1;
    if ((pi_val = (unsigned long *) malloc(sizeof(unsigned long) * n_pi)) == NULL) {
	WriteMessage(NOMEM);
	return;
    }
    if ((array = (int *) malloc(sizeof(int) * num_ff + 1)) == NULL) {
	WriteMessage(NOMEM);
	return;
    }
    if ((Event = (int *) malloc(sizeof(int) * (n_descr + 2))) == NULL) {
	WriteMessage(NOMEM);
	return;
    }
    if ((FirstEvent = (int *) malloc(sizeof(int) * (max_level + 1))) == NULL) {
	WriteMessage(NOMEM);
	return;
    }
    if ((CurrentLevel = (unsigned int *) malloc(sizeof(unsigned int) * (n_descr + 2))) == NULL) {
	WriteMessage(NOMEM);
	return;
    }
}

