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
    extern int      ClockDescr;
    extern char    *ResetDescrName;
    extern int      create_silent;
    int             t;

    create_silent = 1;
    create(argv[1]);
    SetCircuit();

    for (t = 0; t < n_po; ++t)
	printf("%s\n", descr[po_array[t]].name);

    return n_po;
}
