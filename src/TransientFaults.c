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


#include <sys/times.h>
#include "Fenice.h"
#include "SharedVars.h"
#include "DescrVal.h"
#include "Events.h"
#include "Simulation.h"
#include "Faults.h"

#include <library.h>

#define TFL_TAG "#TFL1.0"
#define TRUE (1==1)
#define FALSE (1==0)


/* variabili esterne inizializzate da create_fau */
extern FAULT   *faultlist;
extern int      n_fault;

/* variabili esterne inizializzate da create */
extern LIB_TYPE *library;
extern char    *pin_name[5], *pin_name_ffd[2], *pin_name_ffr[3], *pin_name_ffp[3],

    *pin_name_ffrp[4];
extern int      n_comp;

extern DESCRIPTOR *descr;
extern int      nfop;		/* numero di punti di fanout nel circuito     */
extern int      n_descr;	/* numero totale di descrittori               */
extern int      n_pi;		/* numero totale di ingressi primari          */
extern int      n_po;		/* numero totale di uscite primarie           */
extern int      max_fanout;	/* fanout massimo nella rete                  */
extern int      max_fanin;
extern int      max_level;
extern int      n_gate;

extern int      create_silent;

/* funzioni esterne utilizzate da create_tfau */
extern int      get_pin_pos(char *s);
extern int      get_descr_pos(char *);

int             create_tfau(const char *name)
{
    char           *_FunctionName = "create_tfau";

    FILE           *ffau;

    char            line[MAX_LINE], tmp1[50], tmp2[50], fault[80], type_fault[20], fine;
    char            status_fault[20];
    char            transient_info[20];
    int             activation;

    int             num_equiv, error, ignored, num_line, actual_fault, untestable, k, i, j;

    int 	    type;

    error = 0;
    untestable = 0;
    CheckTrue(ffau = fopen(name, "r"));

    if (!create_silent)
	printf("aperto ffau\n");
    n_fault = 0;
    num_line = 0;
    ignored = 0;
    error = 0;
    while (fgets(line, MAX_LINE, ffau) != NULL) {
	num_line++;
	n_fault++;
	if (line[0] == '=') {
	    n_fault--;
	} else {
	    sscanf(line, "%s", fault);

	    for (i = 0; (fault[i] != '\0') && (fault[i] != '/'); i++)
		fault[i] = toupper(fault[i]);
	    if ((!strcmp(&(fault[i + 1]), pin_name_ffd[1])) ||
		(!strcmp(&(fault[i + 1]), pin_name_ffr[1])) ||
		(!strcmp(&(fault[i + 1]), pin_name_ffp[1])) ||
		(!strcmp(&(fault[i + 1]), pin_name_ffrp[1]))) {
		ignored++;
		n_fault--;
		Print(stderr, "\n%BWARNING:%N ignored fault at line %d, %s\n", num_line, fault);
	    } else if ((!strcmp(&(fault[i + 1]), pin_name_ffr[2])) ||
		       (!strcmp(&(fault[i + 1]), pin_name_ffp[2])) ||
		       (!strcmp(&(fault[i + 1]), pin_name_ffrp[2])) ||
		       (!strcmp(&(fault[i + 1]), pin_name_ffrp[3]))) {
		error++;
		Print(stderr, "\n%BERROR:%N bad pin at line %d, %s\n", num_line, fault);
	    }
	}
    }
    fclose(ffau);

    if (!error)
	faultlist = (FAULT *) malloc(sizeof(FAULT) * n_fault);
    if (error) {
	fprintf(stderr, "Create_tfau stopped!.\n");
    } else if (!faultlist) {
	Print(stderr, "\n%BERROR:%N Not enough memory for faultlist\n");
	error = 2;
    } else {
	CheckTrue(ffau = fopen(name, "r"));
	actual_fault = 0;
	while (fgets(line, MAX_LINE, ffau) != NULL) {
	    j = strlen(line);
	    for (i = 0; i < j; i++)
		line[i] = toupper(line[i]);
	    if (line[0] == '=') {
		(faultlist[actual_fault - 1].size)++;
	    } else {
		if(sscanf(line, "%s %s %s %*s %s", fault, type_fault, status_fault, transient_info) == 4) {
		    if (sscanf(transient_info, "ACTIVE@%d", &activation) != 1) 
			activation = 0;
		} else {
		    activation = 0;
		}
		for (i = 0; (fault[i] != '\0') && (fault[i] != '/'); i++)
		    /* NOP */ ;
		if (fault[i] == '\0') {
		    Print(stderr, "\n%BERRORE:%N Errato formato del .tfau\n  %s\n", fault);
		    exit(1);
		}
		if ((strcmp(&(fault[i + 1]), pin_name_ffd[1])) &&
		    (strcmp(&(fault[i + 1]), pin_name_ffr[1])) &&
		    (strcmp(&(fault[i + 1]), pin_name_ffp[1])) &&
		    (strcmp(&(fault[i + 1]), pin_name_ffrp[1]))) {
		    for (j = 0; (fault[j] != '/') && (fault[j] != '\0'); j++);
		    if (fault[j] == '\0') {
			Print(stderr, "\n%BERRORE:%N Formato errato del .tfau.\n  %s\n", fault);
			exit(1);
		    }
		    fault[j] = '\0';
		    fine = FALSE;
		    i = GetAssoc(fault);
		    if (i == -1) {
			fprintf(stderr, "Non trovato gate %s.\n", fault);
		    } else {
			if (!strcmp(status_fault, "UNDETECTED")) {
			    faultlist[actual_fault].status = UNDETECTED;
			} else if (!strcmp(status_fault, "UNTESTABLE")) {
			    ++untestable;
			    faultlist[actual_fault].status = UNTESTABLE;
			} else if (!strcmp(status_fault, "DETECTED")) {
			    faultlist[actual_fault].status = DETECTED;
			} else {
			    Print(stderr, "\n%BERROR:%N Unrecognized fault status %s\n",
				  status_fault);
			    faultlist[actual_fault].status = UNDETECTED;
			}

			if (!strcmp(type_fault, "S-A-0\0")) {
			    faultlist[actual_fault].val = STUCK_AT_0;
			    type = activation ? FAULT_TRANSIENT_STUCK_AT : FAULT_PERMANENT_STUCK_AT;
			} else if (!strcmp(type_fault, "S-A-1\0")) {
			    faultlist[actual_fault].val = STUCK_AT_1;
			    type = activation ? FAULT_TRANSIENT_STUCK_AT : FAULT_PERMANENT_STUCK_AT;
			} else if (!strcmp(type_fault, "BIT-FLIP\0")) {
			    faultlist[actual_fault].val = 0;
			    CheckTrue(activation);
			    type = FAULT_TRANSIENT_BIT_FLIP;
			} else {
			    Print(stderr, "\n%BERROR:%N Unrecognized fault type %s\n", type_fault);
			}

			faultlist[actual_fault].activation = activation;	/* (!)GG20000906 */
			faultlist[actual_fault].type = type;
			faultlist[actual_fault].descr = i;
			faultlist[actual_fault].size = 1;

			faultlist[actual_fault].pin = get_pin_pos(&(fault[j + 1]));
			if (faultlist[actual_fault].pin == -1) {
			    strcpy(tmp1, &(fault[j + 1]));
			    k = descr[i].gate_id;
			    strcpy(tmp2, library[k].fanout);
			    if (!strcmp(tmp1, tmp2)) {
				faultlist[actual_fault].pin = -1;
			    } else {
				faultlist[actual_fault].pin = -2;
				Print(stderr, "\n%BERRORE:%N Non trovato pin %s.\n",
				      &(fault[j + 1]));
			    }
			}
			if (faultlist[actual_fault].pin < 0) {
			    faultlist[actual_fault].from = faultlist[actual_fault].pin;
			} else {
			    k = faultlist[actual_fault].pin;
			    k = descr[i].from[k];
			    faultlist[actual_fault].from = k;
			}
			actual_fault++;
			fine = TRUE;
		    }
		}
	    }
	}
	fclose(ffau);
	/*print_faultlist(); */
    }

    if (!create_silent)
	Print(stderr, "\nn_fault = %d (%d untestable)\n", n_fault, untestable);
    if (!create_silent)
	Print(stderr, "%d error, ignored %d fault\n", error, ignored);

    return n_fault;
}
