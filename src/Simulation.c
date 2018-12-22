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

#include <sys/times.h>
#include "Fenice.h"
#include "SharedVars.h"
#include "DescrVal.h"
#include "Events.h"
#include "Simulation.h"
#include "Faults.h"

/*
 * LOCAL PROTO
 */
extern inline VALUE EvalRefGate(int gate);
inline void     ScheduleFanOut(int g);
static void     RefSimulation(void);
static void     PrepareLevelZERO(void);

#define MorePattern()	*_InputPattern[CurrIP]

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

#ifdef DEBUG
int             Fenice_VerboseState = 0;
struct tms      _cbs, _cbe;
double          _cbt;
#endif
static STATE   *InternalState = NULL;
struct tms      TimeStart, TimeEnd;

VALUE(*CallGateEvalFunction) (int, VALUE *);
VALUE(*AfterSimulationHook) (int, VALUE *);
VALUE(*AfterFaultDropping) (int, VALUE *);
VALUE(*EvaluateCallBackGate) (int, VALUE *);
int            *HotFFList;

unsigned long   ElapsedSimulationTime;

int            *SignatureFFList, SignatureFFNum;

DESCR_VAL      *DescrVal;
int             CurrIP = 0;
char          **_InputPattern;
int             GoodSimulation;

int             GoodActivity;

int             FD_TresholdFF = 1;

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/


/****************************************************************************/
/*                     2   V A L U E S   L O G I C                          */
/****************************************************************************/

inline VALUE    EvalRefGate(int gate)
{
#ifdef DEBUG
    char           *_FunctionName = "EvalRefGate";
#endif
    VALUE           result;
    register int    t;
    register DESCR_VAL *RD = DescrVal;
    register int   *from = descr[gate].from;
    VALUE           callbackpi[MAX_CALLBACK_INPUTS];

    result = RD[*from++].RefValue;

    switch (descr[gate].type) {
    case AND:			/* 0 */
	for (t = 1; t < descr[gate].fanin; ++t, ++from)
	    V_INC_AND(result, RD[*from].RefValue);
	break;
    case NAND:			/* 1 */
	for (t = 1; t < descr[gate].fanin; ++t, ++from)
	    V_INC_AND(result, RD[*from].RefValue);
	V_NOT(&result);
	break;

    case OR:			/* 2 */
	for (t = 1; t < descr[gate].fanin; ++t, ++from)
	    V_INC_OR(result, RD[*from].RefValue);
	break;
    case NOR:			/* 3 */
	for (t = 1; t < descr[gate].fanin; ++t, ++from)
	    V_INC_OR(result, RD[*from].RefValue);
	V_NOT(&result);
	break;

    case BUF:			/* 4 */
	break;

    case NOT:			/* 5 */
	V_NOT(&result);
	break;

    case EXOR:			/* 6 */
	if (V_EQ(F_ICS, result))
	    break;
	for (t = 1; t < descr[gate].fanin; ++t) {
	    if (V_EQ(F_ICS, RD[*from].RefValue)) {
		result = F_ICS;
		break;
	    }
	    V_INC_XOR(result, RD[*from].RefValue);
	    ++from;
	}
	break;
    case EXNOR:		/* 7 */
	if (V_EQ(F_ICS, result))
	    break;
	for (t = 1; t < descr[gate].fanin; ++t) {
	    if (V_EQ(F_ICS, RD[*from].RefValue)) {
		result = F_ICS;
		break;
	    }
	    V_INC_XOR(result, RD[*from].RefValue);
	    ++from;
	}
	V_NOT(&result);
	break;

    case LOGIC0:		/* 8 */
	result = F_ZERO;
	break;
    case LOGIC1:		/* 9 */
	result = F_UNO;
	break;

    case FF:			/* 10 */
	{
	    char           *_FunctionName = "EvalRefGate";

	    CheckFalse("TRYING TO EVALUATE A FLIP FLOP IN GOOD CIRCUIT");
	}
	break;

    case CALLBACK:		/* 11 */
#ifdef DEBUG
	times(&_cbs);
#endif
	for (t = 0; t < descr[gate].fanin; ++t)
	    callbackpi[t] = RD[descr[gate].from[t]].RefValue;
	DCheckTrue(CallGateEvalFunction);
	result = (*CallGateEvalFunction) (gate, callbackpi);
#ifdef DEBUG
	times(&_cbe);
	_cbt += (double)(_cbe.tms_utime + _cbe.tms_stime - _cbs.tms_utime - _cbs.tms_stime);
#endif
	break;

    default:
	{
	    char           *_FunctionName = "EvalRefGate";

	    CheckFalse("UNKNOWN GATE TYPE");
	}
    }

#ifdef DEBUG
    if (!(UNO_P(result) || ZERO_P(result) || ICS_P(result))) {
	PrintGate(gate);
	CheckTrue(UNO_P(result) || ZERO_P(result) || ICS_P(result));
    }
#endif
    return result;
}

/****************************************************************************/
/*                             F F ,   P I                                  */
/****************************************************************************/

static void     PrepareLevelZERO(void)
{
#ifdef DEBUG
    char           *_FunctionName = "PrepareLevelZERO";
#endif
    VALUE           val[3] = { F_ZERO, F_UNO, F_ICS };
    register int    r;
    register const char *P = _InputPattern[CurrIP];
    
    /*
     * copy ff
     */
    memcpy(GoodFFBefore, GoodFFAfter, n_ff * sizeof(VALUE));

    /*
     * Set PI & generate events (where needed)
     */
    if (*P == '#') {
	/* SoftResetFlipFlops();        /* NB: softreset modifies GoodFFBefore */
	memcpy(GoodFFBefore, InitialFFVal, n_ff * sizeof(VALUE));
	for (r = 0; r < n_ff; ++r)
	    GoodFFAfter[r] = F_ICS;
	for (r = 0; r < InternalFNum; ++r) {
	    if (InternalFaultList[r].modified) {
		DropBufferChain(InternalFaultList[r].modified);
		InternalFaultList[r].modified = NULL;
	    }
	}

	while (*P == '#')
	    P = _InputPattern[++CurrIP];	/* old-style rest, jam multiple resets */
    }

    for (r = 0; r < n_pi && *P; ++P, ++r) {
#ifdef VALUE_3
	DCheckTrue(*P == '0' || *P == '1' || *P == '2');
#else
	DCheckTrue(*P == '0' || *P == '1');
#endif
	PIValues[r] = val[*P - '0'];
	if (V_NE(DescrVal[pi_array[r]].RefValue, PIValues[r])) {
	    DescrVal[pi_array[r]].RefValue = PIValues[r];
	    ScheduleFanOut(pi_array[r]);
	}
    }
    ++CurrIP;

    /*
     * And now: ff
     */
    for (r = 0; r < n_ff; ++r) {
#ifdef DEBUG
	CheckFalse(!ZERO_P(GoodFFBefore[r]) && !UNO_P(GoodFFBefore[r]) && !ICS_P(GoodFFBefore[r]));
#endif
	DescrVal[ppi_array[r]].RefValue = GoodFFBefore[r];
	ScheduleFanOut(ppi_array[r]);
    }
}

inline void     SaveGoodFF(void)
{
    register int    r;

    for (r = 0; r < n_ff; ++r) {
	GoodFFAfter[r] = DescrVal[*descr[ppi_array[r]].from].RefValue;
	/*
	 * (!)gs20000607
	 * si poteva fare in modo + "intelligente", con la get event... e non
	 * sarebbe stato necessario azzerare DescrVal[r].
	 *
	 * chissa` xche' l'ho tolto...
	 *
	 ** while ((r = GetEvent(0)) != -1)
	 **     GoodFFAfter[LatArray[r]] = DescrVal[*descr[r].from].RefValue;
	*/
	DescrVal[ppi_array[r]].Scheduled = 0;	
    }
}

/****************************************************************************/
/*                         S I M U L A T I O N                              */
/****************************************************************************/

int             Simulation(char **inp)
{
    char           *_FunctionName = "Simulation";
    int             t;
    int             u;
    int             found;
    int             coverage;
    FF_BUFFER      *B1;
    FF_BUFFER      *B2;

    ++CurrentId;		/* per max */

#ifdef DEBUG
    _cbt = 0.0;
#endif
    _InputPattern = inp;
    CurrIP = 0;
    for (t = 0; t < n_descr; ++t) {
	/*** AAARGH!!! ***/
	DescrVal[t].Scheduled = 0;
	if (descr[t].type == LOGIC0)
	    DescrVal[t].RefValue = F_ICS;
	else if (descr[t].type == LOGIC1)
	    DescrVal[t].RefValue = F_UNO;
	else
	    DescrVal[t].RefValue = F_ICS;

	DescrVal[t].Stats[0] = DescrVal[t].Stats[1] = 0;
    }

/*     DCheckTrue(InternalFNum); */

/*     if(!InternalFNum) */
/*      return 0; */

    /*  
     * CLEAN
     */
    for (t = 0; t < InternalFNum; ++t) {
	if (InternalFaultList[t].modified) {
	    DropBufferChain(InternalFaultList[t].modified);
	    InternalFaultList[t].modified = NULL;
	}
	InternalFaultList[t].gotcha = 0;
	InternalFaultList[t].activity = 0;
    }
    memcpy(GoodFFAfter, InitialFFVal, n_ff * sizeof(VALUE));

    /*
     * STORED STATE
     */
    if (InternalState) {
#ifdef DEBUG
	if (Fenice_VerboseState)
	    Print(stdout, ":LS Loading state %p\n", InternalState);
#endif
	for (t = 0; t < InternalFNum; ++t) {
	    found = 0;
	    for (u = 0; !found && u < InternalState->NFault; ++u) {
		if (
/* 		   InternalFaultList[t].num==InternalState->IFList[u].num && */
		       InternalFaultList[t].value == InternalState->IFList[u].value &&
		       InternalFaultList[t].descr == InternalState->IFList[u].descr &&
		       InternalFaultList[t].from == InternalState->IFList[u].from &&
		       InternalFaultList[t].size == InternalState->IFList[u].size &&
		       InternalFaultList[t].pin == InternalState->IFList[u].pin) {
		    found = 1;
		    InternalFaultList[t].gotcha = InternalState->IFList[u].gotcha;
#ifdef DEBUG
		    if (Fenice_VerboseState)
			Print(stdout, ":LS F:%s/%d/%s: ",
			      descr[InternalFaultList[t].descr].name,
			      InternalFaultList[t].from, InternalFaultList[t].value ? "1" : "0");
#endif
		    InternalFaultList[t].modified = NULL;
		    for (B1 = InternalState->IFList[u].modified; B1; B1 = B1->next) {
			TakeBuffer(B2);
			B2->next = InternalFaultList[t].modified;
			InternalFaultList[t].modified = B2;
			B2->num = B1->num;
			B2->val = B1->val;
#ifdef DEBUG
			if (Fenice_VerboseState)
			    Print(stdout, " %d", B2->num);
#endif
		    }
#ifdef DEBUG
		    if (Fenice_VerboseState)
			Print(stdout, "\n");
#endif
		}
	    }
	    CheckTrue(found);
	}
	/*
	 * And now: ff
	 */
	memcpy(GoodFFAfter, InternalState->GoodFF, n_ff * sizeof(VALUE));
	for (t = 0; t < n_descr; ++t) {
	    DescrVal[t].Value = DescrVal[t].RefValue = F_ICS;
	    DescrVal[t].Id = -1;
	}

#ifdef DEBUG
	if (Fenice_VerboseState) {
	    static char    *_x_;

	    Print(stdout, ":LS FF: %s\n", _x_ = GetFFVal(_x_));
	}
#endif
    }

    /*
     * ACTIVE FAULTS
     */
    for (u = 0, t = 0; t < InternalFNum; ++t)
	if (!InternalFaultList[t].gotcha)
	    ActiveFaults[u++] = &InternalFaultList[t];
    ActiveFaults[u] = NULL;

    times(&TimeStart);
    while (MorePattern()) {
	GoodSimulation = 1;
	RefSimulation();
	GoodSimulation = 0;
	FaultSimulation();

	if (AfterSimulationHook)
	    (*AfterSimulationHook) (CurrIP, NULL);
    }

    if (LastFaultDropping)
	(*LastFaultDropping) ();
    times(&TimeEnd);

    for (coverage = 0, t = 0; t < InternalFNum; ++t)
	if (InternalFaultList[t].gotcha)
	    ++coverage;

#ifdef DEBUG
    _cbt /= CLOCKS_PER_SEC;
    if (_cbt != 0.0)
	Print(stderr, "\nSimtime: %.2fs - callback: %.2fs (%.2f%%)\n",
	      GetSimulationTime(), _cbt, 100.0 * _cbt / GetSimulationTime());
#endif

    return coverage;
}

static void     RefSimulation(void)
{
    /*
     * ATTENZIONE: LA FUNZIONE E` STATA PACIOCCATA IN MODO ORRENDO
     */
    int             level;
    register int    event;
    VALUE           value;
    int             list[n_descr];
    int             listpos;

    ++CurrentId;
    ResetEvents();		/* Paranoia. It ``should be'' useless */
    GoodActivity = 0;

    PrepareLevelZERO();

    for (level = 1; level <= max_level; ++level) {
	listpos = 0;
	while ((event = GetEvent(level)) != -1) {
	    value = EvalRefGate(event);
	    if (V_NE(value, DescrVal[event].RefValue)) {
		/* value has changed! */
		list[listpos++] = event;
		DescrVal[event].NewRefValue = value;
		ScheduleFanOut(event);

		/* stats! */
#ifdef VALUE_3
		++DescrVal[event].Stats[value.B&0x1];
#else
		++DescrVal[event].Stats[value.A?1:0];
#endif
	    }
	}
	for (--listpos; listpos >= 0; --listpos)
	    DescrVal[list[listpos]].RefValue = DescrVal[list[listpos]].NewRefValue;
    }
    SaveGoodFF();
}

void            StopSimulation(void)
{
    for (; *_InputPattern[CurrIP]; ++CurrIP);
}

/****************************************************************************/
/*                         G E T   S T A T U S                              */
/****************************************************************************/

int             GetCurrentUndetectedFaultNumber(void)
{
    register int    u;
    register F_FAULT **F;

    for (u = 0, F = ActiveFaults; *F; ++F)
	u += (*F)->size;
    return u;
}

double          GetSimulationTime(void)
{
    long int        start = TimeStart.tms_utime + TimeStart.tms_stime;
    long int        end = TimeEnd.tms_utime + TimeEnd.tms_stime;

#if defined(CLK_TCK)
    return ((double)(end - start)) / (double)CLK_TCK;
#elif defined(CLOCKS_PER_SEC)
    return ((double)(end - start)) / (double)CLOCKS_PER_SEC;
#elif defined(HZ)
    return ((double)(end - start)) / (double)HZ;
#else
    return ((double)(end - start)) / 60.0;
#endif
}

STATE          *GetState(STATE * S)
{
    char           *_FunctionName = "GetState";
    FF_BUFFER      *B1;
    FF_BUFFER      *B2;
    int             t;

    if (S) {
#ifdef DEBUG
	if (Fenice_VerboseState)
	    Print(stdout, ":GS Getting state %p\n", InternalState);
#endif
	CheckTrue(S->NFault == InternalFNum);
	for (t = 0; t < InternalFNum; ++t)
	    if (S->IFList[t].modified)
		DropBufferChain(S->IFList[t].modified);
    } else {
#ifdef DEBUG
	if (Fenice_VerboseState)
	    Print(stdout, ":GS Getting new state\n");
#endif
	CheckTrue(Malloc(S, sizeof(STATE)));
	CheckTrue(Malloc(S->GoodFF, n_ff * sizeof(VALUE)));
	CheckTrue(Malloc(S->IFList, InternalFNum * sizeof(F_FAULT)));
    }

#ifdef DEBUG
    if (Fenice_VerboseState) {
	int             _t_;

	for (_t_ = 0; *_InputPattern[_t_]; ++_t_)
	    Print(stdout, ":GS #%s\n", _InputPattern[_t_]);
    }
#endif

    memcpy(S->GoodFF, GoodFFAfter, n_ff * sizeof(VALUE));
    S->NFault = InternalFNum;
    memcpy(S->IFList, InternalFaultList, InternalFNum * sizeof(F_FAULT));

    for (t = 0; t < InternalFNum; ++t) {
#ifdef DEBUG
	if (Fenice_VerboseState)
	    Print(stdout, ":GS F:%s/%d/%s: ",
		  descr[InternalFaultList[t].descr].name,
		  InternalFaultList[t].from, InternalFaultList[t].value ? "1" : "0");
#endif
	S->IFList[t].modified = NULL;
	for (B1 = InternalFaultList[t].modified; B1; B1 = B1->next) {
	    TakeBuffer(B2);
	    B2->next = S->IFList[t].modified;
	    S->IFList[t].modified = B2;
	    B2->num = B1->num;
	    B2->val = B1->val;
#ifdef DEBUG
	    if (Fenice_VerboseState)
		Print(stdout, " %d", B2->num);
#endif
	}
#ifdef DEBUG
	if (Fenice_VerboseState)
	    Print(stdout, "\n");
#endif
    }
#ifdef DEBUG
    if (Fenice_VerboseState) {
	static char    *_x_;

	Print(stdout, ":GS FF: %s\n", _x_ = GetFFVal(_x_));
    }
#endif

    return S;
}

STATE          *SetState(STATE * S)
{
    STATE          *OS;

    OS = InternalState;
    InternalState = S;
    return OS;
}
