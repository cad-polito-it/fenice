/*

	FILE: "message.c"

*/

 /************************************************************************\
 *                                                                        *
 *      Scritto nell'estate del 1993                                      *
 *      da                                                                *
 *                                                                        *
 *                              Marisa Divia`                             *
 *                             Stefano Terzolo                            *
 *                            Giovanni Squillero                          *
 *                                                                        *
 \************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "proto.h"
#include "myproto.h"
#include "data2.h"
#include "mydata.h"

extern char     *Context;
extern char     *Message;

/**************************************************************************
NOME:           WriteMessage
PARAMETRI:      che messaggio stampare
DESCRIZIONE:    Tentativo di rendere "coerente" e "modificabile" lo stile
		dei messaggi che il programma manda all'utente.
***************************************************************************/
void WriteMessage(int num)
{
	if(num==-1)
		printf("\n\n");

	printf("%s::", Context);

	switch(num) {
		case GENERAL:
			printf("GENERAL:\n");
			break;
		case WELCOME:
			printf("WELCOME:\n");
			printf("\tWelcome to PREPSIM, the fast, parallel, event-driven, PROOFS-like,\n");
			printf("\tdiagnostic fault simulator for sequential circuits.\n");
#ifdef DEBUG
			printf("\tFULL DEBUG: All internal checks active.\n");
#else
#ifdef CHECK
			printf("\tDEBUG: Circular buffer check active.\n");
#endif
#endif
			printf("\t(%s  -  Compiled: %s  %s)\n\n", VERSION, __DATE__, __TIME__);
			break;
		case IOERROR:
			printf("ERROR: fopen() failed.\n");
			printf("\tFile do not exist or not accessible.\n");
			break;
		case NOMEM:
			printf("ERROR: malloc() failed.\n");
			printf("\tNot enought memory available.\n");
			break;
		case BADFORMAT:
			printf("ERROR: Unknown file type.\n");
			printf("\tFile is not in the right format.\n");
			break;
		case SYNTAX:
			printf("ERROR: Syntax error.\n");
			printf("\tUnrecognized command, try \"?\" for help.\n");
			break;
		case NOARG:
			printf("ERROR: Syntax error.\n");
			printf("\tMissing argument, try \"?\" for help.\n");
			break;
		case BADFAULT:
			printf("ERROR: Error in fault list.\n");
			break;
		case NOFAULT:
			printf("WARNING: No fault to simulate\n");
			printf("\tFalt list not loaded or empty, or all faults were discarded.\n");
			printf("\tSimulating GOOD circuit only.\n\n");
			break;
		case CANNOT_ORDER:
			printf("ERROR: No fault to order\n");
			printf("\tFault list not loaded or empty\n");
			break;
		case BADREPSIM:
			printf("WARNING:\n");
			printf("\tResults may be incorrect due to REPSIM compatibility.\n");
			break;
		case CONNECT:
			printf("ERROR: Circuit is not well connected\n");
			printf("\tResults are likely to be incorrect\n");
			break;
		case UNDESCRIPTOR:
			printf("ERROR: Unknown descriptor type\n");
			printf("\tCannot evaluate circuit. Aborting.\n\n");
			break;
		case SAVEMEMORY:
			printf("WARNING:\n");
			printf("\tBe sure that you KNOW what you are doing.\n\n");
			break;
		case OVERFLOW:
			printf("OVERFLOW: Circular buffer full.\n");
			printf("\tResults are likely to be incorrect.\n");
			printf("\tModify current \"saved\" value and restart.\n");
			break;
		case FANIN:
			printf("FANIN: Fanin is too big.\n");
			printf("\tPRepsim cannot perform internal checks if a descriptor's fanin > %d,\n", MAXIMUM_FANIN);
			printf("\tModify current \"MAXIMUM_FANIN\" value and recompile.\n");
			break;
		default:
			printf("DISASTER: %s\n\tPlease contact the authors IMMEDIATELY.\n", Message);
			exit(-1);
			break;
	}
	return;
}
