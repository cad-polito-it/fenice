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


char           *VERSION = "3.0";

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <Fenice.h>

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
 * create_fau() variables
 */
int             n_fault;
FAULT          *faultlist;

int             main(int argc, char *argv[])
{
    char           *_FunctionName = "main";
    char            circuit[256];
    char            fault[256];
    extern int      create_silent;
    int             t;
    char            c;
    extern int      optind;
    int             equiv, error;

    Print(stderr, "%Bfau2nfau %N- converts fau (edf) fault list to nfau (2.0A).\n"
	  "Using %UFenice%N(r) version %s, built at %s.\n",
	  GetFeniceVersion(), GetFeniceDate());

    equiv = 0;
    error = 0;
    while ((c = getopt(argc, argv, "e")) != -1)
	switch (c) {
	  case 'e':
	      equiv = 1;
	      break;
	  default:
	      error = 1;
	}
    argv += optind;
    argc -= optind;

    if (!error && argc == 1) {
	strcpy(circuit, argv[0]), strcat(circuit, ".edf");
	strcpy(fault, argv[0]), strcat(fault, ".fau");
    } else if (!error && argc == 2) {
	strcpy(circuit, argv[0]);
	strcpy(fault, argv[1]);
    } else {
	Print(stderr, "\n\n%BUSAGE: %N\n"
	      "%s [-e] circuit\n"
	      "or %s [-e] circuit.edf flist.fau\n\n",
	      *argv, *argv);
	exit(1);
    }

    create_silent = 1;
    Print(stderr, "\n\nReading %s\n", circuit);
    CheckFalse(create(circuit));
    --n_pi;			/* CLOCK */
    SetCircuit();

    Print(stderr, "\nReading %s\n", fault);
    create_fau(fault);
    CheckTrue(n_fault > 0);

    printf("2.0A\n");
    for (t = 0; t < n_fault; ++t)
	printf("%d %d %d %d\n",
	       faultlist[t].val ? 128 : 0,
	       faultlist[t].descr,
	       faultlist[t].from,
	       equiv ? faultlist[t].size : 0
	    );

    Print(stderr, "\nConverted %d fault%s.\n", n_fault, n_fault == 1 ? "" : "s");

    return 0;
}
