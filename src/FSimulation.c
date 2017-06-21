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


#include <sys/times.h>
#include "Fenice.h"
#include "SharedVars.h"
#include "DescrVal.h"
#include "Events.h"
#include "Simulation.h"
#include "Faults.h"

/*
 * LOCAL PROTOS
 */
inline void     ScheduleFanOut(int g);

static inline int IsActive(F_FAULT * F);
static VALUE    EvalFGate(int gate);
static VALUE    EvalFault(int gate);
static void     RealFaultDropping(void);
static void     InjectFaults(void);
static void     StoreFlipFlop(void);
static int      ChooseFaults(void);

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

#ifdef DEBUG
int             Fenice_WarnOnGothca = 0;

extern struct tms _cbs, _cbe;
extern double   _cbt;
#endif
void            (*RunTimeFaultDropping) (void) = NULL;
void            (*LastFaultDropping) (void) = NULL;

F_FAULT        *InternalFaultList;
F_FAULT       **ActiveFaults;
int             FaultToExamine;
int             CurrentId;
F_FAULT_INFO    FaultInfo[WORD_SIZE + 1];
extern int      CurrIP;

/****************************************************************************/
/*                     E V A L   F U N C T I O N S                          */
/****************************************************************************/

VALUE           DebugGetVal(int num)
{
    char           *_FunctionName = "DebugGetVal2";

    VALUE           val =

	(DescrVal[num].Id == CurrentId ? DescrVal[num].Value : DescrVal[num].RefValue);
    return val;
}

static VALUE    EvalFGate(int gate)
{
#ifdef DEBUG
    char           *_FunctionName = "EvalFfGate";
#endif
    VALUE           result;
    register int    t;
    register DESCR_VAL *RD = DescrVal;
    register int   *from = descr[gate].from;
    VALUE           callbackpi[MAX_CALLBACK_INPUTS];
    register int    _cId = CurrentId;

    result = GETVAL(_cId, RD, *from), ++from;

    switch (descr[gate].type) {
    case AND:
	for (t = 1; t < descr[gate].fanin; ++t) {
	    V_INC_AND(result, GETVAL(_cId, RD, *from));
	    ++from;
	}
	break;
    case NAND:
	for (t = 1; t < descr[gate].fanin; ++t) {
	    V_INC_AND(result, GETVAL(_cId, RD, *from));
	    ++from;
	}
	V_NOT(&result);
	break;

    case OR:
	for (t = 1; t < descr[gate].fanin; ++t) {
	    V_INC_OR(result, GETVAL(_cId, RD, *from));
	    ++from;
	}
	break;
    case NOR:
	for (t = 1; t < descr[gate].fanin; ++t) {
	    V_INC_OR(result, GETVAL(_cId, RD, *from));
	    ++from;
	}
	V_NOT(&result);
	break;

    case BUF:
	if (descr[gate].attr == PI)
	    result = GETVAL(_cId, RD, gate);
	break;

    case NOT:
	V_NOT(&result);
	break;

    case EXOR:
	if (V_EQ(F_ICS, result))
	    break;
	for (t = 1; t < descr[gate].fanin; ++t) {
	    if (V_EQ(F_ICS, GETVAL(_cId, RD, *from))) {
		result = F_ICS;
		break;
	    } else {
		V_INC_XOR(result, GETVAL(_cId, RD, *from));
	    }
	    ++from;
	}
	break;
    case EXNOR:
	if (V_EQ(F_ICS, result))
	    break;
	for (t = 1; t < descr[gate].fanin; ++t) {
	    if (V_EQ(F_ICS, GETVAL(_cId, RD, *from))) {
		result = F_ICS;
		break;
	    } else {
		V_INC_XOR(result, GETVAL(_cId, RD, *from));
	    }
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

    case FF:
	break;

    case CALLBACK:
#ifdef DEBUG
	times(&_cbs);
#endif
	for (t = 0; t < descr[gate].fanin; ++t)
	    callbackpi[t] = GETVAL(_cId, RD, descr[gate].from[t]);
	DCheckTrue(CallGateEvalFunction);
	result = (*CallGateEvalFunction) (gate, callbackpi);
#ifdef DEBUG
	times(&_cbe);
	_cbt += (double)(_cbe.tms_utime + _cbe.tms_stime - _cbs.tms_utime - _cbs.tms_stime);
#endif
	break;

    default:
	result = EvalFault(gate);
    }

    return result;
}

/* 
 * Evaluating a faulty descriptor...
 */
static VALUE    EvalFault(int gate)
{
    char           *_FunctionName = "EvalFault";
    int             type;
    F_FAULT        *F;
    VALUE           ormask = { f_ZERO, f_UNO };
    VALUE           andmask = { f_UNO, f_ZERO };
    register int    index;
    int             fanin;
    int             firsttype = descr[gate].type;
    VALUE           result;

    for (type = firsttype; type < 0; type = FaultInfo[index].Jazz) {
	/*      
	 * index: the bit we are reffering to (type is <0).
	 */
	index = -type;
	F = FaultInfo[index].fault;

	DCheckTrue(F->descr == gate);	/* paranoia check */
	DescrVal[gate].Id = CurrentId;	/* current gate */
	if (F->pin == -1) {
	    /* 
	     * FANOUT FAULT: just update andmask & ormask
	     */
	    if (F->type & FAULT_MASK_STUCK_AT && F->value) {
		ormask.A |= BIT(index - 1);
#ifdef VALUE_3
		ormask.B &= ~BIT(index - 1);
#endif
	    } else if (F->type & FAULT_MASK_STUCK_AT && !F->value) {
		andmask.A &= ~BIT(index - 1);
#ifdef VALUE_3
		andmask.B |= BIT(index - 1);
#endif
	    } else if (F->type & FAULT_MASK_BIT_FLIP && UNO_P(DescrVal[gate].RefValue)) {
		andmask.A &= ~BIT(index - 1);
#ifdef VALUE_3
		andmask.B |= BIT(index - 1);
#endif
	    } else if (F->type & FAULT_MASK_BIT_FLIP && ZERO_P(DescrVal[gate].RefValue)) {
		ormask.A |= BIT(index - 1);
#ifdef VALUE_3
		ormask.B &= ~BIT(index - 1);
#endif
	    } else {
		CheckFalse("PANIK!");
	    }
	} else {
	    /* 
	     * FANIN FAULT: Copy from in .store
	     */
	    fanin = descr[F->descr].from[F->pin];
	    FaultInfo[index].store = DescrVal[fanin].Value = GETVAL(CurrentId, DescrVal, fanin);
	    DescrVal[fanin].Id = CurrentId;
	    /*
	     * patch descr value
	     */
	    if (F->type & FAULT_MASK_STUCK_AT && F->value) {
		DescrVal[fanin].Value.A |= BIT(index - 1);
#ifdef VALUE_3
		DescrVal[fanin].Value.B &= ~BIT(index - 1);
#endif
	    } else if (F->type & FAULT_MASK_STUCK_AT && !F->value) {
		DescrVal[fanin].Value.A &= ~BIT(index - 1);
#ifdef VALUE_3
		DescrVal[fanin].Value.B |= BIT(index - 1);
#endif
	    } else if (F->type & FAULT_MASK_BIT_FLIP) {
		DescrVal[fanin].Value.A ^= BIT(index - 1);
#ifdef VALUE_3
		DescrVal[fanin].Value.B ^= BIT(index - 1);
#endif
	    } else {
		CheckFalse("PANIK!");
	    }
	    DescrVal[fanin].dirty = 1;
	}
    }
    /*
     * set true gate type & eval
     */
    descr[gate].type = type;
    result = EvalFGate(gate);
    V_INC_AND(result, andmask);
    V_INC_OR(result, ormask);

    /*
     * set froms' true values
     */
    for (type = firsttype; type < 0; type = FaultInfo[index].Jazz) {
	index = -type;
	F = FaultInfo[index].fault;

	if (F->pin >= 0) {
	    fanin = descr[F->descr].from[F->pin];
	    if (DescrVal[fanin].dirty) {
		DescrVal[fanin].Value = FaultInfo[index].store;
		DescrVal[fanin].dirty = 0;
	    }
	}
    }

    return result;
}

/****************************************************************************/
/*                      2 / 3   F U N C T I O N S                           */
/****************************************************************************/

static inline int IsActive(register F_FAULT * F)
{
#ifdef DEBUG
    char           *_FunctionName = "IsActive";
#endif

    DCheckFalse(F->gotcha);

    if (F->modified)
	return 1;

    if (F->type & FAULT_MASK_TRANSIENT && F->activation != CurrIP) {
	DCheckTrue(F->type == FAULT_TRANSIENT_STUCK_AT || F->type == FAULT_TRANSIENT_BIT_FLIP);
	return 0;
    }

    if(F->type == FAULT_TRANSIENT_BIT_FLIP)
	return 1;

    if (F->pin == -1) {
	if (F->value)
	    return !UNO_P(DescrVal[F->descr].RefValue);
	else
	    return !ZERO_P(DescrVal[F->descr].RefValue);
    }

    if (F->pin != -1) {
	VALUE           temp;
	register        r;

	if (descr[F->descr].type == FF)
	    return 1;
	
	DescrVal[F->from].Id = ++CurrentId;
	if (F->value) {
	    DescrVal[F->from].Value = F_UNO;
	} else {
	    DescrVal[F->from].Value = F_ZERO;
	}
	temp = EvalFGate(F->descr);
	DescrVal[F->from].Id = ++CurrentId;
	/* 	if(V_EQ(temp, DescrVal[F->descr].RefValue)) */
	/* 	    fprintf(stderr, "Skipping fault %d\n", F->num); */
	return V_NE(temp, DescrVal[F->descr].RefValue);
    }
}

static void     InjectFaults(void)
{
#ifdef DEBUG
    char           *_FunctionName = "InjectFaults";
#endif
    register int    r;
    FF_BUFFER      *B;

    /*
     * Flip Flops
     */
    for (r = 0; r < FaultInfo->Jazz; ++r) {
	if (FaultInfo[r + 1].fault->modified) {
	    for (B = FaultInfo[r + 1].fault->modified; B; B = B->next) {
		if (DescrVal[B->num].Id != CurrentId) {
		    DescrVal[B->num].Value = GoodFFBefore[LatArray[B->num]];
		    DescrVal[B->num].Id = CurrentId;
		    ScheduleFanOut(B->num);
		}
		/*
		 * Change values (DescrVal[B->num].Value ^= BIT(r))
		 */
		switch (B->val) {
		case FF_ZERO:
		    DescrVal[B->num].Value.A &= ~BIT(r);
#ifdef VALUE_3
		    DescrVal[B->num].Value.B |= BIT(r);
#endif
		    break;
		case FF_UNO:
		    DescrVal[B->num].Value.A |= BIT(r);
#ifdef VALUE_3
		    DescrVal[B->num].Value.B &= ~BIT(r);
#endif
		    break;
		case FF_ICS:
		    DescrVal[B->num].Value.A &= ~BIT(r);
#ifdef VALUE_3
		    DescrVal[B->num].Value.B &= ~BIT(r);
#endif
		    break;
#ifdef DEBUG
		default:
		    CheckFalse("PANIK");
#endif
		}

	    }
	    DropBufferChain(FaultInfo[r + 1].fault->modified);
	    FaultInfo[r + 1].fault->modified = NULL;
	}
    }

    /* 
     * Faults
     */
    for (r = 1; r <= FaultInfo->Jazz; ++r) {
	if (FaultInfo[r].fault->type & FAULT_MASK_TRANSIENT && FaultInfo[r].fault->activation != CurrIP) {
	    continue;
	} else if (!descr[FaultInfo[r].fault->descr].level && FaultInfo[r].fault->pin == -1) {
	    /* corrent value (PI or FF) */
	    DescrVal[FaultInfo[r].fault->descr].Value =
		GETVAL(CurrentId, DescrVal, FaultInfo[r].fault->descr);
	    DescrVal[FaultInfo[r].fault->descr].Id = CurrentId;
	    if (FaultInfo[r].fault->type & FAULT_MASK_STUCK_AT && FaultInfo[r].fault->value) {
		DescrVal[FaultInfo[r].fault->descr].Value.A |= BIT(r - 1);
#ifdef VALUE_3
		DescrVal[FaultInfo[r].fault->descr].Value.B &= ~BIT(r - 1);
#endif
	    } else if (FaultInfo[r].fault->type & FAULT_MASK_STUCK_AT && !FaultInfo[r].fault->value) {
		DescrVal[FaultInfo[r].fault->descr].Value.A &= ~BIT(r - 1);
#ifdef VALUE_3
		DescrVal[FaultInfo[r].fault->descr].Value.B |= BIT(r - 1);
#endif
	    } else if (FaultInfo[r].fault->type & FAULT_MASK_BIT_FLIP) {
		DescrVal[FaultInfo[r].fault->descr].Value.A ^= BIT(r - 1);
#ifdef VALUE_3
		DescrVal[FaultInfo[r].fault->descr].Value.B ^= BIT(r - 1);
#endif
	    }
	    ScheduleFanOut(FaultInfo[r].fault->descr);
	} else {
	    /*
	     * just invert descr type...
	     */
	    SetEvent(FaultInfo[r].fault->descr);
	    /* link faults */
	    FaultInfo[r].Jazz = descr[FaultInfo[r].fault->descr].type;
	    descr[FaultInfo[r].fault->descr].type = -r;
	}
    }

    return;
}

static void     StoreFlipFlop(void)
{
#ifdef DEBUG
    char           *_FunctionName = "StoreFlipFlop";
#endif
    register int    t;
    int             ff;
    register unsigned long xoredA;

#ifdef VALUE_3
    register unsigned long xoredB;
#endif
    VALUE           V1, V2;
    FF_BUFFER      *B;

#ifdef VALUE_3
    while ((ff = GetEvent(0)) != -1) {
	V1 = EvalFGate(ff);
	V2 = GoodFFAfter[LatArray[ff]];
	xoredA = V1.A ^ V2.A;
	xoredB = V1.B ^ V2.B;
	for (t = 0; t < FaultInfo->Jazz; ++t) {
	    if ((xoredA & BIT(t)) || (xoredB & BIT(t))) {
		/* 
		 * fault t modifies ff
		 */
		TakeBuffer(B);
		B->num = ff;
		B->next = FaultInfo[t + 1].fault->modified;
		/*
		 * if V1.A==1 is UNO!
		 * dumb version:
		 * if((V1.A & BIT(t)) && !(V1.B & BIT(t)))
		 * B->val = FF_UNO;
		 * else if(!(V1.A & BIT(t)) && (V1.B & BIT(t)))
		 * B->val = FF_ZERO;
		 * else if(!(V1.A & BIT(t)) && !(V1.B & BIT(t)))
		 * B->val = FF_ICS;
		 */
		if (V1.A & BIT(t))
		    B->val = FF_UNO;
		else if (V1.B & BIT(t))
		    B->val = FF_ZERO;
		else
		    B->val = FF_ICS;
		FaultInfo[t + 1].fault->modified = B;
	    }
	}
    }
#else
    while ((ff = GetEvent(0)) != -1) {
	V1 = EvalFGate(ff);
	V2 = GoodFFAfter[LatArray[ff]];
	xoredA = V1.A ^ V2.A;
	for (t = 0; t < FaultInfo->Jazz; ++t) {
	    if (xoredA & BIT(t)) {
		TakeBuffer(B);
		B->num = ff;
		B->next = FaultInfo[t + 1].fault->modified;
		if (V1.A & BIT(t))
		    B->val = FF_UNO;
		else
		    B->val = FF_ZERO;
		FaultInfo[t + 1].fault->modified = B;
	    }
	}
    }
#endif

}

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

static int      ChooseFaults(void)
{
    register int    r = 1;

    while (ActiveFaults[FaultToExamine]) {
	if (IsActive(ActiveFaults[FaultToExamine])) {
	    FaultInfo[r].fault = ActiveFaults[FaultToExamine];

	    ++FaultToExamine;
	    if (++r > WORD_SIZE)
		break;
	} else {
	    ++FaultToExamine;
	}
    }
    --r;
    FaultInfo->Jazz = r;
    return r;
}

void            FaultSimulation(void)
{
    char           *_FunctionName = "FaultSimulation";
    int             level;
    register int    event;
    register int    r;

    static int dbg;

    FaultToExamine = 0;
    while (ChooseFaults()) {
	++CurrentId;
	InjectFaults();
	FaultActivity = 0;

	/*
	 * NO NEED TO SIMULATE LEVEL ZERO (EVER)
	 */
	for (level = 1; level <= max_level; ++level) {
	    while ((event = GetEvent(level)) != -1) {
		DescrVal[event].Value = EvalFGate(event);
		if (V_NE(DescrVal[event].Value, DescrVal[event].RefValue)) {
		    /* new value! */
		    ++FaultActivity;
		    DescrVal[event].Id = CurrentId;
		    ScheduleFanOut(event);
		}
	    }
	}
	StoreFlipFlop();
	if (RunTimeFaultDropping)
	    (*RunTimeFaultDropping) ();
	if (AfterFaultDropping)
	    (*AfterFaultDropping) (CurrIP, NULL);
    }
    if (RunTimeFaultDropping)
	RealFaultDropping();
}


void            PrintPacket(void)
{
#ifdef DEBUG
    int             r;

    for (r = 0; r < FaultInfo->Jazz; ++r)
	printf("%d\n", FaultInfo[r + 1].fault->num);
#else
    Print(stderr, "\nOnly available in debug library\n");
#endif
}

/****************************************************************************/
/*                     F A U L T   D R O P P I N G                          */
/****************************************************************************/

static void     RealFaultDropping(void)
{
    int             r, r2;

    for (r2 = r = 0; ActiveFaults[r]; ++r) {
#ifdef DEBUG
	if (Fenice_WarnOnGothca && ActiveFaults[r]->gotcha)
	    Print(stderr, ":D DETECTED FAULT %s/%d/%s AT %d\n",
		  descr[ActiveFaults[r]->descr].name,
		  ActiveFaults[r]->from,
		  ActiveFaults[r]->value ? "1" : "0", ActiveFaults[r]->gotcha);
#endif
	if (!ActiveFaults[r]->gotcha) {
	    ActiveFaults[r2++] = ActiveFaults[r];
	} else if (ActiveFaults[r]->modified) {
	    DropBufferChain(ActiveFaults[r]->modified);
	    ActiveFaults[r]->modified = NULL;
	}
    }
    ActiveFaults[r2] = NULL;
}

void            FaultDroppingPO(void)
{
    register int    r;
    register int    r2;
    register unsigned long xoredA;

#ifdef VALUE_3
    register unsigned long xoredB;
#endif

    for (r = 0; r < n_po; ++r) {
#ifdef VALUE_3
	if (DescrVal[po_array[r]].Id == CurrentId) {
	    xoredA = DescrVal[po_array[r]].RefValue.A ^ DescrVal[po_array[r]].Value.A;
	    xoredB = DescrVal[po_array[r]].RefValue.B ^ DescrVal[po_array[r]].Value.B;
	    for (r2 = 0; r2 < FaultInfo->Jazz; ++r2)
		if ((xoredA & BIT(r2)) && (xoredB & BIT(r2)))	/* no pot detected */
		    FaultInfo[r2 + 1].fault->gotcha = CurrIP;
	}
#else
	if (DescrVal[po_array[r]].Id == CurrentId) {
	    xoredA = DescrVal[po_array[r]].RefValue.A ^ DescrVal[po_array[r]].Value.A;
	    for (r2 = 0; r2 < FaultInfo->Jazz; ++r2)
		if (xoredA & BIT(r2))
		    FaultInfo[r2 + 1].fault->gotcha = CurrIP;
	}
#endif
    }
}

void            __FaultDroppingPO_1D(void)
{
    register int    r;
    register int    r2;
    register unsigned long xoredA;
    register unsigned long xoredB;

#ifndef VALUE_3
    CheckFalse("Unit-delay simulation supports only 3-valued faults simulation");
#endif
    for (r = 0; r < n_po; ++r) {
	if (DescrVal[po_array[r]].Id == CurrentId) {
	    xoredA = DescrVal[po_array[r]].RefValue.A ^ DescrVal[po_array[r]].Value.A;
	    xoredB = DescrVal[po_array[r]].RefValue.B ^ DescrVal[po_array[r]].Value.B;
	    for (r2 = 0; r2 < FaultInfo->Jazz; ++r2)
		if ((xoredA & BIT(r2)) && (xoredB & BIT(r2)))	/* no pot detected */
		    FaultInfo[r2 + 1].fault->gotcha = CurrIP;
	}
    }
}

void            FaultDroppingLastPO(void)
{
    register int    r;
    register int    r2;
    register unsigned long xoredA, xoredB;
    extern char   **_InputPattern;

    if (*_InputPattern[CurrIP])
	return;

    for (r = 0; r < n_po; ++r) {
	if (DescrVal[po_array[r]].Id == CurrentId) {
	    xoredA = DescrVal[po_array[r]].RefValue.A ^ DescrVal[po_array[r]].Value.A;
#ifdef VALUE_3
	    xoredB = DescrVal[po_array[r]].RefValue.B ^ DescrVal[po_array[r]].Value.B;
#else
	    xoredB = f_ZERO;
#endif
	    for (r2 = 0; r2 < FaultInfo->Jazz; ++r2)
		if ((xoredA & BIT(r2)) && (xoredB & BIT(r2)))
		    FaultInfo[r2 + 1].fault->gotcha = CurrIP;
	}
    }
}

void            FaultDroppingFF(void)
{
    register int    r;

    for (r = 0; r < FaultInfo->Jazz; ++r)
	if (FaultInfo[r + 1].fault->modified)
	    FaultInfo[r + 1].fault->gotcha = CurrIP;
}

void            FaultDroppingTresholdFF(void)
{
    register int    r;
    extern int      FD_TresholdFF;

    for (r = 0; r < FaultInfo->Jazz; ++r)
	if (FaultInfo[r + 1].fault->modified)
	    if (++FaultInfo[r + 1].fault->activity > FD_TresholdFF)
		FaultInfo[r + 1].fault->gotcha = CurrIP;
}

void            FaultDroppingLastFF(void)
{
    register int    r, s;
    FF_BUFFER      *B;

#ifdef DEBUG
    char           *_FunctionName = "FaultDroppingLastFF";
    static int      noHotList;

    if (HotFFList) {
	for (r = 0; HotFFList[r] != -1; ++r) {
	    CheckTrue(descr[HotFFList[r]].type == FF);
	}
    }
#endif

    if (!HotFFList) {
#ifdef DEBUG
	if (!noHotList)
	    WARNING("FAULT DROPPING",
		    "HotFFList is NULL, checking all flip flops\n"
		    "(you should use the AddHotFF() function to choose flip flops)\n"
		    "subsequent warning of the same type will be ignored");
	noHotList = 1;
#endif
	for (r = 0; ActiveFaults[r]; ++r)
	    if (ActiveFaults[r]->modified)
		ActiveFaults[r]->gotcha = CurrIP;
    } else {
	for (r = 0; ActiveFaults[r]; ++r)
	    for (B = ActiveFaults[r]->modified; B; B = B->next)
		for (s = 0; HotFFList[s] != -1; ++s)
		    if (B->num == HotFFList[s])
			ActiveFaults[r]->gotcha = CurrIP;
    }
}

/****************************************************************************/
/*                                I N F O                                   */
/****************************************************************************/

int            *GetHotFFList(void)
{
    return HotFFList;
}

void            PrintFaultStats(void)
{
    fPrintFaultStats(stderr);
}

void            fPrintFaultStats(FILE * F)
{
    int             t;
    int             tot;

    for (tot = 0, t = 0; t < InternalFNum; ++t)
	tot += InternalFaultList[t].size;

    Print(F, "\n\nComplete fault list contains %d faults, collapsed list contains %d faults\n\n",
	  tot, InternalFNum);
}

/**
void PrintFaultList(FILE *stream, int type)
{
    int t;
    int u;

    for(t=0; t<n_fault; ++t) {
	if(((type & FAULT_LIST_UNDETECTED) && !InternalFaultList[t].gotcha)
	   || ((type & FAULT_LIST_DETECTED) && InternalFaultList[t].gotcha)) {

	    if (InternalFaultList[t].pin == -1 && descr[InternalFaultList[t].descr].type == FF)
		fprintf(stream, "%s/Q", descr[InternalFaultList[t].descr].name);
	    else if (!InternalFaultList[t].pin && descr[InternalFaultList[t].descr].type == FF)
		fprintf(stream, "%s/D", descr[InternalFaultList[t].descr].name);
	    else if (InternalFaultList[t].pin == -1)
		fprintf(stream, "%s/O", descr[InternalFaultList[t].descr].name);
	    else
		fprintf(stream, "%s/I%d", descr[InternalFaultList[t].descr].name, 1 + InternalFaultList[t].pin);
	    fprintf(stream, " S-A-%d ", !!InternalFaultList[t].value);
	    if (InternalFaultList[t].gotcha)
		fprintf(stream, "DETECTED SOLID_DETECT) AT VECTOR -1, FDATA %d %d %d %d\n", 
		       t, InternalFaultList[t].size, InternalFaultList[t].descr, InternalFaultList[t].gotcha);
	    else
		fprintf(stream, "UNDETECTED (UNTESTED)\n");
	    for(u=1; u<InternalFaultList[t].size; ++u) {
		fprintf(stream, "= DUMMY/O S-A-0\n");
	    }
	}
    }
}
 **/
