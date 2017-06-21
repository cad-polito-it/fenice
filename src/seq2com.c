/*****************************************************************-*-c-*-*\
*             *                                                           *
*   #####     *  (c) Copyright 2000, Giovanni Squillero                   *
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

int             n_fault;
FAULT          *faultlist;

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

int            main(int argc, char *argv[])
{
    char           *_FunctionName = "main";
    int             t;
    char            edf[1024], fau[1024];
    FILE           *FAU;
    int             ColsPerPage;

#ifdef  __MSDOS__
    ColsPerPage = 80;
#else
    if (isatty(fileno(stderr)) && getenv("TERM")) {
	tgetent(NULL, getenv("TERM"));
	if ((ColsPerPage = tgetnum("co")) <= 1)
	    ColsPerPage = 80;
	SetWidth(stdout, ColsPerPage);
	SetWidth(stderr, ColsPerPage);
    } else {
	ColsPerPage = 0;
    }
#endif

    while (*++argv) {
	fprintf(stderr, "%s\n", *argv);
	strcpy(edf, *argv), strcat(edf, ".edf");
	strcpy(fau, *argv), strcat(fau, ".fau");
	CheckFalse(create(edf));
	if(FAU=fopen(fau, "r")) {
	    fclose(FAU);
	    if (!create_nfau(fau))
		create_fau(fau);
	}
	for (t = 0; t < n_ff; ++t)
	    CircuitMod_ScanFF(ppi_array[t], faultlist, n_fault);
	CircuitMod_Commit(faultlist, n_fault);
	
	strcpy(edf, *argv), strcat(edf, "_C.edf");
	CircuitMod_WriteEDF(edf, "zeebo");

	if(faultlist) {
	    strcpy(fau, *argv), strcat(fau, "_C.fau");
	    CheckTrue(FAU = fopen(fau, "w"));
	    for (t = 0; t < n_fault; ++t) {
		fprintf(FAU, "%s/", descr[faultlist[t].descr].name);
		if (faultlist[t].from == -1)
		    fprintf(FAU, "O");
		else
		    fprintf(FAU, "I%d", faultlist[t].pin + 1);
		fprintf(FAU, " S-A-%d", !!faultlist[t].val);
		fprintf(FAU, " UNDETECTED (UNTESTED) (0 0), INJECTION 0 0 0\n");
	    }
	    fclose(FAU);
	}
    }
    return 0;
}
