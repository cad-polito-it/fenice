/*****************************************************************-*-c-*-*\
*             *                                                           *
*   #####     *  Copyright (c) 2000 Giovanni Squillero                    *
*  ######     *  http://staff.polito.it/giovanni.squillero/               *
*  ###   \    *  giovanni.squillero@polito.it                             *
*   ##G  c\   *                                                           *
*   #     _\  *  This code is licensed under a BSD license.               *
*   |  _/     *  See <https://github.com/squillero/fenice> for details    *
*             *                                                           *
\*************************************************************************/

#ifndef FENICE_HEADER_ALREADY_INCLUDED

#define FENICE_HEADER_ALREADY_INCLUDED Yes

#if !defined(VALUE_3) && !defined(VALUE_2)
#warning "Assuming VALUE_3 (you should specify VALUE_2 or VALUE_3)"
#define VALUE_3 		"Blues Rules"
#endif

#if defined(VALUE_2) && defined(VALUE_3)
#error "Both VALUE_2 and VALUE_3 defined"
#endif

#ifdef GARBAGE_COLLECTOR
#ifdef DEBUG
#define GC_DEBUG
#endif
#include <gc.h>
#endif

#include <data2.h>
#include <fault2.h>
#include <gutils.h>
/* #include <Check.h> included by gutils */

#define  DELETED -1
#define MAX_CALLBACK_INPUTS 512

#ifdef  __MSDOS__
#define times(X) (X)->tms_utime=time(NULL),(X)->tms_stime=0
#endif

/*
 * SIMULATION RUN TIME FLAGS
 * Use with the SetSimulationType().
 */
#define FLAGS_MASK		0x0000000f	/* flags mask */
#define RESET_ALL_FLAGS		0x00000003	/* clear all flags */
																	  /* #define SUNRISE_COMPATIBLE   0x00000003 *//* sunrise compatible */
#define UNUSED_1		0x00000005
#define UNUSED_2		0x00000009

#define VALUE_MASK      	0x000000f0
																	  /* #define VALUE_2              0x00000010 *//* 0-1 simulation */
																	  /* #define VALUE_3              0x00000020 *//* 0-1-X simulation */

#define DROP_MASK       	0x00000f00	/* kinda of fault droppin' */
#define DROP_FIRST_PO   	0x00000100	/* first different po */
#define DROP_LAST_PO    	0x00000200	/* different po's at last pattern */
#define DROP_FIRST_FF   	0x00000300	/* first different ff */
#define DROP_LAST_FF    	0x00000400	/* different ff's at last pattern */
#define DROP_LAST_FF_UD    	0x00000500	/* different ff's (user defined) at last pattern */
#define DROP_TRESHOLD_FF    	0x00000600	/* different ff's at last pattern */
#define DROP_FIRST_GATE		0x00000700	/* controlled faults */

#define RESET_MASK      	0x0000f000	/* kinda of reset used */
																	  /* #define RESET_SYNC           0x00001000 *//* synchronous reset (like PRepSim & old molokhs) */
																	  /* #define RESET_ASYNC          0x00002000 *//* asynchronous reset */

/*
 * Compat.c - functions to ensure conmpatibility with previous version
 */
int             create_nfau(const char *name);
void            FSim(char **Seq, unsigned int *ActiveFault, unsigned int FaultLeft, int *Detected, int *DNum);
void            InitFSim(void);

/*
 * TRANSIENT FAULTS
 */
int             create_tfau(const char *name);

/*
 * A few 'useful' macros and defines
 */
typedef struct _VALUE {
    unsigned long int A;
#ifdef VALUE_3
    unsigned long int B;
#endif
} VALUE;

#define WORD_SIZE  32

#define BIT(X)     (1lu<<(X))
#define BITCOUNT(X) (((_BC_(X)+(_BC_(X)>>4))&0x0F0F0F0F)%255)
#define _BC_(X)     ((X)-(((X)>>1)&0x77777777)-(((X)>>2)&0x33333333)-(((X)>>3)&0x11111111))

/*
 * HOOKS - The only hook currently  supported is AFTER_SIMULATION, but
 * new hooks will be added soon.  
 */
enum {
    AFTER_SIMULATION_HOOK, USER_CALLBACK_GATE, AFTER_FAULT_DROPPING_HOOK
};
VALUE           (*SetHook(int, VALUE (*func) (int, VALUE *))) (int, VALUE *);

#define f_ZERO     0x00000000ul
#define f_UNO      0xfffffffful
#define f_UNKNOWN  0x5a5a5a5aul

#ifdef VALUE_3

#define F_ZERO	   (VALUE){f_ZERO, f_UNO}
#define F_UNO	   (VALUE){f_UNO, f_ZERO}
#define F_ICS	   (VALUE){f_ZERO, f_ZERO}

#define ZERO_P(X) (X.A==f_ZERO&&X.B==f_UNO)
#define UNO_P(X) (X.A==f_UNO&&X.B==f_ZERO)
#define ICS_P(X) (X.A==f_ZERO&&X.B==f_ZERO)
#define V_INC_AND(X, Y) X.A&=Y.A, X.B|=Y.B
#define V_INC_OR(X, Y) X.A|=Y.A, X.B&=Y.B
/*
 * Notez bien: only works for 0 or 1s
 */
#define V_INC_XOR(X, Y) X.A^=Y.A, X.B=~X.A
static __inline__ int V_EQ(VALUE X, VALUE Y) { return X.A==Y.A && X.B==Y.B; }
static __inline__ int V_NE(VALUE X, VALUE Y) { return X.A!=Y.A || X.B!=Y.B; }

#else

#define F_ZERO	   (VALUE){f_ZERO, f_UNO}
#define F_UNO	   (VALUE){f_UNO, f_ZERO}
#define F_ICS	   (VALUE){f_UNKNOWN, f_UNKNOWN}

#define ZERO_P(X) (X.A==f_ZERO)
#define UNO_P(X) (X.A==f_UNO)
#define ICS_P(X) (0==1)
#define V_INC_AND(X, Y) X.A&=Y.A
#define V_INC_OR(X, Y) X.A|=Y.A
#define V_INC_XOR(X, Y) X.A^=Y.A
static __inline__ int V_EQ(VALUE X, VALUE Y) { return X.A==Y.A; }
static __inline__ int V_NE(VALUE X, VALUE Y) { return X.A!=Y.A; }

#endif

/*
 * STRUCTs
 */
typedef struct _FF_BUFFER {
    int             num;
    int             val;
    struct _FF_BUFFER *next;    
} FF_BUFFER;

/***
 * in faults2.h
 * #define FAULT_MASK_PERMANENT 0x0001
 * #define FAULT_MASK_TRANSIENT 0x0002
 * #define FAULT_MASK_STUCK_AT  0x0010
 * #define FAULT_MASK_BIT_FLIP  0x0020
 * 
 * #define FAULT_UNKNOWN   0x0000
 * #define FAULT_PERMANENT_STUCK_AT (FAULT_MASK_PERMANENT | FAULT_MASK_STUCK_AT)
 * #define FAULT_TRANSIENT_STUCK_AT (FAULT_MASK_TRANSIENT | FAULT_MASK_STUCK_AT)
 * #define FAULT_TRANSIENT_BIT_FLIP (FAULT_MASK_TRANSIENT | FAULT_MASK_BIT_FLIP)
 * 
 */

typedef struct _F_FAULT {
    int             num;
    int             gotcha;
    int             value;
    int             descr;
    int             from;
    int             size;
    int             pin;
    FF_BUFFER      *modified;
    int		    activity;
    int		    type;
    int		    activation;
} F_FAULT;

typedef struct _F_FAULT_INFO {
    F_FAULT        *fault;
    VALUE          store;
    int             Jazz;
} F_FAULT_INFO;

typedef struct _STATE {
    int	       	    NFault;
    VALUE	   *GoodFF;
    F_FAULT        *IFList;
} STATE;

/*
 * Get info from fenice
 */
char           *GetFeniceVersion(void);
char           *GetFeniceDate(void);

/* 
 * Set and Get. Controls various flags. Get simulation results
 */
void            SetDropFFTreshold(int t);

STATE 	       *SetState(STATE *S);
STATE 	       *GetState(STATE *S);

unsigned long   my_clock();

int             Get_fenice_internal_AllocatedBuffers(void);
double          GetSimulationTime(void);
int            *GetHotFFList(void);

void            AddFault(FAULT f);
void            SpecialAddFault(FAULT f, int n);
int             AddHotFF(int num);
int            *GetCoverage(int *buf);
int             GetCurrentUndetectedFaultNumber(void);
int             GetAllocatedBuffers(void);
char           *GetFFVal(char *str);
char           *GetFFVal2(char *str);
char           *GetPIVal(char *str);
char           *GetPOVal(char *str);
void            ReleaseCircuit(void);
void            ReleaseFaults(void);
void            SetCircuit(void);
void            SetFaults(const FAULT * flist, int nfault);
void            SetInitialFFValue(VALUE val);
void            SetInitialFFValues(VALUE *val);
char          **SetInput(const char *file, char **ptr);
void            SetSignatureFF(int num, int *list);
void            SetSimulationType(unsigned int flags);

enum { FAULT_LIST_COMPLETE = 0x3, FAULT_LIST_DETECTED = 0x2, FAULT_LIST_UNDETECTED = 0x1 } ;
void PrintFaultList(FILE *strem, int type);

/*
 * Circuit maintenance
 */
int             CheckCircuit(void);
void            PrintCircuitStats(void);
void            PrintFaultStats(void);
void            fPrintCircuitStats(FILE *);
void            fPrintFaultStats(FILE *);
void            PrintSimulationResult(void);
void            fPrintSimulationResult(FILE * out);

/*
 * Simulation. Allocate buffers for faults and start simulation.
 */
void            AllocateBuffers(int num);
int             Simulation(char **inp);
void            StopSimulation(void);

/*
 * DEBUG
 */
void            PrintGate(int g);

#endif

/****************************************************************************\
*                                                                            *
*   (in italiano, questa volta)                                              *
*                                                                            *
*                                                                            *
*   ... perche' cio che  veramente non ha  in se' una ragione  di esistere   *
*                                                                            *
*   non ha vita, e non puo` esser grande, ne' diventar grande                *
*                                                                            *
*                                                                            *
*                           - Johann Wolfang Goethe, "Italienische Reise"    *
*                                                                            *
*                                                                            *
\****************************************************************************/
