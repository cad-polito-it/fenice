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


char           *VERSION = "3.6";

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <termcap.h>
#include <Fenice.h>
#include <CircuitMod.h>

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 1024
#endif

#define TAIL(X, Y)	if(!strcmp(&X[strlen(X)-strlen(Y)], Y)) strcat(X, Y);

/* 
 * LOCAL PROTOS
 */
VALUE           hook(int x, VALUE * y);
VALUE           hook2(int x, VALUE * y);
VALUE           hook3(int x, VALUE * y);
void            GetSimulationSize(char **i);
void            StartBarGraph(char **i);
void            BarGraph(int force);
void            EndBarGraph(void);

char           *WriteDate(char *s);

int             ColsPerPage;
int             gs_sync;
int             TPatLen;
int             seqs;

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

int             opt_show_output = 0;
int             opt_random_pattern = 0, opt_random_sequences = 1;
int             opt_x_simulation = 0;
int             opt_cumulative = 0;
int             opt_kill_clock;
int             opt_initial_buffers;
int             opt_no_garbage_collect;
int             opt_last_po;
int             opt_silent_molokh = 0;
int             opt_log_stdout = 0;

char           *WriteDate(char *s)
{
    static char     date[64];
    char            a[8], b[8], c[8], d[16];

    sscanf(s, "%s %s %s %s", a, b, c, d);
    sprintf(date, "%s-%s-%s %s", b, a, c, d);

    return (date);
}

#define OPTIONS "or:R:xbgklLcCSB:u"

int             main(int argc, char *argv[])
{
    char           *_FunctionName = "Molokh::main";
    char            circuit[256];
    char            fault[256];
    char            testpattern[256];
    extern int      create_silent;
    extern int      ClockDescr;
    char          **input;
    int            *coverage;
    int             totfault_col, detected_col;
    int             totfault_com, detected_com;
    int             totfault_col_testable, detected_col_testable;
    int             totfault_com_testable, detected_com_testable;
    int             showdetected;
    int             s, p, t, u;
    char            c;
    int             errors;
    int             timesimh, timesimm, timesims;
    extern char    *optarg;
    extern int      optind;
    char           *programname;
    char            hostname[MAXHOSTNAMELEN];
    int             sim_mday, sim_mon, sim_year, sim_hour, sim_min, sim_sec;
    time_t          sim_now;
    struct tm      *sim_tm;

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

    Print(stderr, "%BMolokh %N- "
	  "yet another parallel, event-driven fault simulator.\n"
	  "This is version %s, built at %s.\n"
	  "Using %UFenice%N version %s, built at %s.\n\n",
	  VERSION, WriteDate(__DATE__ " " __TIME__), GetFeniceVersion(), GetFeniceDate());

    SetSimulationType(DROP_FIRST_PO);

    gs_sync = 1;
    programname = *argv;
    errors = 0;
    showdetected = 0;
    opt_kill_clock = 1;
    opt_initial_buffers = opt_last_po = opt_no_garbage_collect = 0;
    opt_log_stdout = !isatty(fileno(stdout));
    while ((c = getopt(argc, argv, OPTIONS)) != -1)
	switch (c) {
	case 'o':
	    opt_show_output = 1;
	    break;
	case 'r':
	    opt_random_pattern = atoi(optarg);
	    break;
	case 'R':
	    opt_random_sequences = atoi(optarg);
	    if (!opt_random_pattern)
		opt_random_pattern = 1000;
	    break;
	case 'x':
	    opt_x_simulation = 1;
	    break;
	case 'u':
	    opt_cumulative = 1;
	    opt_log_stdout = 0;
	    break;
	case 'b':
	    opt_last_po = 1;
	    SetSimulationType(DROP_LAST_PO);
	    break;
	case 'g':
	    opt_no_garbage_collect = 1;
	    break;
	case 'k':
	    opt_kill_clock = 0;
	    break;
	case 'l':
	    opt_log_stdout = 1;
	    break;
	case 'L':
	    opt_log_stdout = 0;
	    break;
	case 'c':
	    showdetected = 1;
	    opt_log_stdout = 0;
	    break;
	case 'C':
	    showdetected = 2;
	    opt_log_stdout = 0;
	    break;
	case 'S':
	    opt_silent_molokh = 1;
	    break;
	case 'B':
	    opt_initial_buffers = atoi(optarg);
	    break;
	default:
	    errors = 1;
	}
    argv += optind;
    argc -= optind;

    if (errors || (argc != 1 && argc != 2 + !opt_random_pattern)) {
	Print(stderr, "\n%BUSAGE %N: %s [-" OPTIONS "] foo.edf bar.fau baz.inp\n"
	      "or: %s [-" OPTIONS "] foo\n\n", programname, programname);
	Print(stderr,
	      "\nPlease, send comments, bug reports and constructive criticisms to the author:\n"
	      "%BGiovanni Squillero%N <%Usquillero%N@%Upolito%N.%Uit%N>\n\n");
	Print(stderr, "\n%BNOTEZ BIEN:%N\n");
	Print(stderr, "\n    Fenice and the Molokh are provided 'as-is', without any express or "
	      "implied warranty. In no event will Giovanni Squillero or any member of the CAD Group be held "
	      "liable for any damages arising from the use of this software.\n");
	Print(stderr, "\n\n    Permission is granted to anyone to use Fenice and Molokh code for any purpose, "
	      "including commercial applications, and to alter it and redistribute it "
	      "freely, subject to the following restrictions:\n");

	Print(stderr, "\n\n 1. The origin of this software must not be misrepresented; you must not "
	      "claim that you wrote the original software. If you use my code "
	      "in a product, an acknowledgment in the product documentation would be "
	      "appreciated but is not required.\n");
	Print(stderr, "\n\n 2. Altered source versions must be plainly marked as such, and must not "
	      "be misrepresented as being the original software.\n");
	Print(stderr, "\n\n 3. This notice may not be removed or altered from any source "
	      "distribution.\n\n");
	exit(1);
    } else if (argc == 1) {
	strcpy(circuit, argv[0]), strcat(circuit, ".edf");
	strcpy(fault, argv[0]), strcat(fault, ".fau");
	strcpy(testpattern, argv[0]), strcat(testpattern, ".inp");
    } else {
	strcpy(circuit, argv[0]);
	strcpy(fault, argv[1]);
	if (!opt_random_pattern)
	    strcpy(testpattern, argv[2]);
    }

    if (opt_log_stdout)
	Print(stderr, "\nPrinting log information to stdout\n");

    if (!opt_silent_molokh)
	Print(stderr, "\nOpening description: \"%s\"\n", circuit);
    create_silent = 1;
    CheckFalse(create(circuit));
    CircuitMod_ParanoiaCheck();

    if (!opt_silent_molokh)
	Print(stderr, "\nOpening faultlist: \"%s\"\n", fault);
    if (!create_nfau(fault))
	create_tfau(fault);

    if (opt_kill_clock) {
	CircuitMod_RemoveClock();
	CircuitMod_Commit(faultlist, n_fault);
    } else {
	if (!opt_silent_molokh)
	    Print(stderr, "\nAssuming clock is descriptor %d (%s), just lowering n_pi\n",
		  pi_array[n_pi - 1], descr[pi_array[n_pi - 1]].name);
	--n_pi;
    }

    SetCircuit();
    SetFaults(faultlist, n_fault);

    if (!opt_silent_molokh) {
	PrintCircuitStats();
	PrintFaultStats();
    }
    if (opt_initial_buffers)
	AllocateBuffers(opt_initial_buffers);

    if (opt_random_pattern) {
	srand48(time(NULL));
	u = 0;
	CheckTrue(Malloc

		  (input, sizeof(char *) * (opt_random_sequences * (1 + opt_random_pattern) + 1)));
	for (s = 0; s < opt_random_sequences; ++s) {
	    input[u++] = "#";
	    for (p = 0; p < opt_random_pattern; ++p) {
		CheckTrue(Malloc(input[u], sizeof(char *) * (n_pi + 1)));

		for (t = 0; t < n_pi; ++t)
		    input[u][t] = '0' + (lrand48() % 10000 < 5000);
		++u;
	    }
	}
	input[u] = "";
    } else {
	input = SetInput(testpattern, NULL);
    }

    GetSimulationSize(input);
    if (ColsPerPage > 40) {
	StartBarGraph(input);
	SetHook(AFTER_SIMULATION_HOOK, &hook);
    }
    if (opt_show_output)
	SetHook(AFTER_SIMULATION_HOOK, &hook2);
    else if (opt_cumulative)
	SetHook(AFTER_SIMULATION_HOOK, &hook3);

    if (opt_x_simulation) {
	Print(stderr, "\nStarting with all flip-flops in the X state\n");
	SetInitialFFValue(F_ICS);
    } else {
	SetInitialFFValue(F_ZERO);
    }

    Simulation(input);
    fprintf(stderr, "\n");

    coverage = GetCoverage(NULL);
    totfault_com = detected_com = 0;
    totfault_col = detected_col = 0;
    totfault_com_testable = detected_com_testable = 0;
    totfault_col_testable = detected_col_testable = 0;
    for (t = 0; t < n_fault; ++t) {
	totfault_col += 1;
	totfault_com += faultlist[t].size;
	if (coverage[t]) {
	    detected_col += 1;
	    detected_com += faultlist[t].size;
	}

	if (faultlist[t].status != UNTESTABLE) {
	    totfault_col_testable += 1;
	    totfault_com_testable += faultlist[t].size;
	    if (coverage[t]) {
		detected_col_testable += 1;
		detected_com_testable += faultlist[t].size;
	    }
	}
    }

    timesimh = ((int)GetSimulationTime()) / (60 * 60);
    timesimm = (((int)GetSimulationTime()) - timesimh * 60 * 60) / (60);
    timesims = ((int)GetSimulationTime()) % 60;
    time(&sim_now), sim_tm = localtime(&sim_now);
    sim_sec = sim_tm->tm_sec;
    sim_min = sim_tm->tm_min;
    sim_hour = sim_tm->tm_hour;
    sim_mday = sim_tm->tm_mday;
    sim_mon = sim_tm->tm_mon;
    sim_year = sim_tm->tm_year;
    gethostname(hostname, MAXHOSTNAMELEN);
    Print(stderr, "\n");
    {
	FILE           *F[] = { stderr, stdout };
	int             f;

	if (opt_silent_molokh && opt_log_stdout) {
	    opt_log_stdout = 0;
	    F[0] = stdout;
	}

	for (f = 0; (f < 2 && opt_log_stdout) || f < 1; ++f) {
	    Print(F[f], "\n%BFault simulation performed on %s, %02d-%02d-%4d %02d:%02d:%02d%N\n",
		  hostname, sim_mday, 1 + sim_mon, 1900 + sim_year, sim_hour, sim_min, sim_sec);
	    Print(F[f], "\nCircuit file       : %s\n", circuit);
	    Print(F[f], "\nFault list file    : %s\n", fault);
	    Print(F[f], "\nTest pattern file  : %s\n",
		  opt_random_pattern ? "(random)" : testpattern);
	    Print(F[f], "\nTest pattern length: %d test vector%s in %d sequence%s\n", TPatLen,
		  TPatLen == 1 ? "" : "s", seqs, seqs == 1 ? "" : "s");
	    Print(F[f], "\nSimulation type    : %s\n",
		  opt_last_po ? "BIST-like (drop last po)" : "standard (drop first po)");
	    Print(F[f], "\nFault coverage     : %0.2f%% complete, %0.2f%% collapsed\n",
		  100.0 * detected_com / totfault_com, 100.0 * detected_col / totfault_col);
	    Print(F[f], "\nSimulation time    : %0.2fs (%d:%02d:%02d)\n", GetSimulationTime(),
		  timesimh, timesimm, timesims);
	    if (GetAllocatedBuffers() && opt_initial_buffers < GetAllocatedBuffers())
		Print(F[f], "\nFlip flop activity : %0.2f%% (allocated %d buffers)\n\n",
		      100.0 * GetAllocatedBuffers() / (n_ff * n_fault), GetAllocatedBuffers());
	    else
		Print(F[f], "\nFlip flop activity : unknown (allocated %d buffers)\n\n",
		      GetAllocatedBuffers());
	    fPrintSimulationResult(F[f]);
	}
    }

    if (showdetected == 1) {
	for (t = 0; t < n_fault; ++t)
	    if (coverage[t])
		printf("Fault n. %d detected at pattern n. %d\n", t + 1, coverage[t]);
	    else
		printf("Fault n. %d undetected\n", t + 1);
    } else if (showdetected == 2) {
	for (t = 0; t < n_fault; ++t) {
	    if (faultlist[t].pin == -1 && descr[faultlist[t].descr].type == FF)
		printf("%s/Q", descr[faultlist[t].descr].name);
	    else if (!faultlist[t].pin && descr[faultlist[t].descr].type == FF)
		printf("%s/D", descr[faultlist[t].descr].name);
	    else if (faultlist[t].pin == -1)
		printf("%s/O", descr[faultlist[t].descr].name);
	    else
		printf("%s/I%d", descr[faultlist[t].descr].name, 1 + faultlist[t].pin);
	    printf(" S-A-%d ", !!faultlist[t].val);
	    if (coverage[t])
		printf("DETECTED %d %d %d %d\n", t, faultlist[t].size, faultlist[t].descr,
		       coverage[t]);
	    else
		printf("UNDETECTED\n");
	}
    }

    if (!opt_no_garbage_collect) {
	if (!opt_silent_molokh)
	    Print(stderr, "\nGarbage collecting, wait... ");
	Free(faultlist);
	ReleaseCircuit();
	ReleaseFaults();
	Free(coverage);
	if (!opt_silent_molokh)
	    Print(stderr, "done	\n");
    }

    return 0;
}

int             pat = 0;
int             oldp = -1;
int             olddots;
char           *megalomany = "(!) 1994-2001, Giovanni Squillero";
char            base[256], string1[256], string2[256];
int             MLen;
int             olddots = -1;
int             totFNum;

VALUE           hook(int x, VALUE * y)
{
    BarGraph(0);

    return (VALUE) {
    f_ZERO, f_UNO};
}

VALUE           hook2(int x, VALUE * y)
{
    static char    *tmp1;
    static char    *tmp2;

    printf("%s %s\n", tmp1 = GetPOVal(tmp1), tmp2 = GetFFVal(tmp2));

    return (VALUE) {
    f_ZERO, f_UNO};
}

VALUE           hook3(int x, VALUE * y)
{
    extern int      CurrIP;
    extern F_FAULT **ActiveFaults;
    static int      lastCurrIP = 0;
    static int      lastActive = -1;
    register int    Active;

    for (Active = 0; ActiveFaults[Active]; ++Active);

    if (lastActive < 0)
	lastActive = n_fault;

    for (++lastCurrIP; lastCurrIP < CurrIP; ++lastCurrIP)
	printf("%d\n", n_fault-lastActive);
    printf("%d\n", n_fault-Active);

    lastCurrIP = CurrIP;
    lastActive = Active;

    return (VALUE) {
    f_ZERO, f_UNO};
}

void            StartBarGraph(char **i)
{
    int             t;
    int             WHITE = 7;

    for (t = 0; t < (ColsPerPage - strlen(megalomany) - WHITE) >> 1; ++t)
	strcat(base, " ");
    strcat(base, megalomany);
    for (t = 0; t < (ColsPerPage - strlen(megalomany) - WHITE) >> 1; ++t)
	strcat(base, " ");

    MLen = strlen(base);
    Print(stderr, "\n\nSimulating %d test vector%s in %d sequence%s%s\n",
	  TPatLen, TPatLen == 1 ? "" : "s", seqs, seqs == 1 ? "" : "s",
	  opt_last_po ? " (no fault dropping)" : "");
}

void            EndBarGraph(void)
{
    BarGraph(1);
}

void            BarGraph(int force)
{
    register int    newp;
    register int    dots, t;
    extern char    *tcap_reverse, *tcap_normal;

    ++pat;
    newp = 100.0 * pat / TPatLen;
    dots = MLen * newp / 100.0;

    if (dots == olddots)
	return;

    oldp = newp;
    olddots = dots;
    for (t = 0; t < dots; ++t)
	string1[t] = *tcap_reverse ? base[t] : '#';
    string1[t] = 0;
    for (t = dots; t < MLen; ++t)
	string2[t - dots] = base[t];
    string2[t - dots] = 0;

    fprintf(stderr, "%3d%%|%s%s%s%s|\r", newp, tcap_reverse, string1, tcap_normal, string2);
}

void            GetSimulationSize(char **i)
{
    int             t;
    int             lines;

    for (totFNum = 0, t = 0; t < n_fault; ++t)
	totFNum += faultlist[t].size;

    for (lines = 0; *i[lines]; ++lines);
    for (TPatLen = lines, t = 0; *i[t]; ++t)
	TPatLen -= (gs_sync || !t) && !strcmp(i[t], "#");

    for (seqs = 0, t = 0; *i[t] && *i[t] == '#'; ++t);
    while (*i[t]) {
	++seqs;
	for (; *i[t] && *i[t] != '#'; ++t);
	for (; *i[t] && *i[t] == '#'; ++t);
    }

}
