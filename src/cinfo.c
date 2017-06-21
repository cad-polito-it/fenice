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
#include <sys/ddi.h>

#include <Fenice.h>
#include <CircuitMod.h>

/*
 * INTERNAL PROTOS
 */
int             GetTerminalWidth(FILE * F);
int             AreaGate(int g);

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

/*
 * create_fau(), create_nfau() variables
 */
int             n_fault;
FAULT          *faultlist;

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

int             AreaGate(int g)
{
    char           *_FunctionName = "AreaGate";
    switch (descr[g].type) {
      case AND:
	  return descr[g].fanin;
      case NAND:
	  return descr[g].fanin;
      case OR:
	  return descr[g].fanin;
      case NOR:
	  return descr[g].fanin;
      case BUF:
	  return 1;
      case NOT:
	  return 1;
      case EXOR:
	  switch(descr[g].fanin) {
	  case 2: return 4;
	  case 3: return 5;
	  case 4: return 6;
	  default: CheckTrue(descr[g].type == EXOR && descr[g].fanin < 4);
	  }
      case EXNOR:
	  switch(descr[g].fanin) {
	  case 2: return 5;
	  case 3: return 6;
	  case 4: return 7;
	  default: CheckTrue(descr[g].type == EXNOR && descr[g].fanin < 4);
	  }
      case LOGIC0:
	  return 0;
      case LOGIC1:
	  return 0;
      case FF:
	  return 10;
      case CALLBACK:
	  CheckFalse(descr[g].type == CALLBACK);
      default:
	  CheckFalse("UNKNOWN TYPE");
    }
}

void            main(int argc, char *argv[])
{
    char           *_FunctionName = "cinfo::main";
    char            circuit[2048];
    char            fault[2048];
    char           *programname;
    FILE           *F;
    int             t;
    int             area;

    char            c;
    extern char    *optarg;
    extern int      optind;

    int             opt_preserve_clock = 0;
    int		    opt_only_flip_flop = 0;
    int		    opt_bist = 0;
    int		    opt_only;

    int             min_fanin;
    int             max_fanin;
    int             sum_fanout;
    int             min_fanout;
    int             max_fanout;
    int             sum_fanin;
    int             internal_gates;

    SetWidth(stdout, GetTerminalWidth(stdout));
    SetWidth(stderr, GetTerminalWidth(stderr));

    Print(stderr, "%Bcinfo %N- "
	  "circuit information\n"
	  "Using %UFenice%N version %s, built at %s.\n\n",
	  GetFeniceVersion(), GetFeniceDate());

    CheckTrue(programname = strdup(argv[0]));

    while ((c = getopt(argc, argv, "fcB")) != -1)
	switch (c) {
	  case 'B':
	      opt_bist = 1;
	      break;
	  case 'f':
	      opt_only_flip_flop = 1;
	      break;
	  case 'c':
	      opt_preserve_clock = 1;
	      break;
	  default:
	      exit(1);
	}
    argv += optind;
    argc -= optind;

    if(opt_only_flip_flop)
	opt_only = 1;
    else 
	opt_only = 0;

    CheckTrue(argc == 1 || argc == 2);

    strcpy(circuit, *argv);
    if (F = fopen(circuit, "r")) {
	fclose(F);
    } else {
	strcat(circuit, ".edf");
    }

    if (argc == 2) {
	strcpy(fault, argv[1]);
    } else {
	strcpy(fault, *argv);
	strcat(fault, ".fau");
    }

    if(!opt_only)
	Print(stdout, "\nCircuit     : %s\n", circuit);
    create_silent = 0;
    CheckFalse(create(circuit));
    SetCircuit();
    CircuitMod_ParanoiaCheck();

    if (!opt_only && (F = fopen(fault, "r"))) {
	fclose(F);
	Print(stdout, "\nFaultlist   : %s\n", fault);
	if (!create_nfau(fault))
	    create_fau(fault);
	SetFaults(faultlist, n_fault);
    }
    if (!opt_only && !opt_preserve_clock) {
	Print(stdout, "\n");
	CircuitMod_RemoveClock();
	CircuitMod_Commit(faultlist, n_fault);
    }
    if(!opt_only)
	fPrintCircuitStats(stdout);

    for (t = 0; t < n_descr; ++t) {
	if (descr[t].attr == INTERNAL) {
	    min_fanout = max_fanout = descr[t].fanout;
	    min_fanin = max_fanin = descr[t].fanin;
	    break;
	}
    }
    sum_fanin = sum_fanout = internal_gates = 0;
    for (area = 0, t = 0; t < n_descr; ++t) {
	if (descr[t].attr == INTERNAL) {
	    min_fanin = min(min_fanin, descr[t].fanin);
	    max_fanin = max(max_fanin, descr[t].fanin);
	    sum_fanout += descr[t].fanout;
	    min_fanout = min(min_fanout, descr[t].fanout);
	    max_fanout = max(max_fanout, descr[t].fanout);
	    sum_fanin += descr[t].fanin;
	    ++internal_gates;
	}
    }

    if(!opt_only) {
	Print(stdout, "\nFanin stats : min=%d, max=%d, average=%.2f\n",
	      min_fanin, max_fanin, 1.0 * sum_fanin / n_descr);
	Print(stdout, "\nFanout stats: min=%d, max=%d, average=%.2f\n",
	      min_fanout, max_fanout, 1.0 * sum_fanout / n_descr);
	
	for (area = 0, t = 0; t < n_descr; ++t)
	    area += AreaGate(t);
	Print(stdout, "\nCircuit area: %d (using static pdt.genlib data)\n", area);
	if(opt_bist) {
	    area += (n_pi+n_po)*10; 		/* FF */;
	    area += (n_pi+n_po+n_ff)*2*1;	/* NOT */;
	    area += (n_pi+n_po+n_ff)*3*2;	/* AND2 */;
	    area += (n_pi+n_po+n_ff)*3;		/* OR3 */;
	    Print(stdout, "\nExpected bist area: %d (using the violante hypothesis)\n", area);
	}

	if (faultlist)
	    fPrintFaultStats(stdout);
    } else if(opt_only_flip_flop) {
	Print(stdout, "%d\n", n_ff);
    }
}

int             GetTerminalWidth(FILE * F)
{
#ifdef  __MSDOS__
    return 80;
#else
    if (!isatty(fileno(F)))
	return 0;		/* e` un file... */

    if (!getenv("TERM"))
	return 80;		/* terminale non definito */

    tgetent(NULL, getenv("TERM"));
    return tgetnum("co");
#endif
}
