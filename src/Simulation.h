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

#include "Events.h"

/****************************************************************************/
/*                             F A U L T S                                  */
/****************************************************************************/

extern int      CurrentId;

/****************************************************************************/
/*                            C I R C U I T                                 */
/****************************************************************************/

/*
 * MACROS
 */
#ifdef DEBUG
VALUE        DebugGetVal(int num);
#define       GETVAL(ID, Y, X)	     DebugGetVal(X)
#else
#define       GETVAL(ID, Y, X)          (Y[X].Id==ID ? Y[X].Value : Y[X].RefValue)
#endif

#define         SoftResetFlipFlops() memcpy(GoodFFBefore, InitialFFVal, n_ff*sizeof(VALUE))
#define         HardResetFlipFlops() memset(GoodFFBefore, 0, n_ff*sizeof(VALUE))
#define         ResetEvents()        memset(EventsCount, 0, (1+max_level)*sizeof(int))

static inline void ScheduleFanOut(register int g)
{
    extern int      GoodActivity;
    register int    r;

    GoodActivity += descr[g].fanout;
    for (r = 0; r < descr[g].fanout; ++r)
	SetEvent(descr[g].to[r]);
}

static inline void V_NOT(VALUE *X)
{
#ifdef VALUE_3
    register long t = X->A;
    X->A = X->B;
    X->B = t;
#else
    X->A = ~X->A;
#endif
}

/* FAULT-SIMULATION */
void            FaultSimulation(void);
extern int     *SignatureFFList, SignatureFFNum;
extern int     *HotFFList;
extern int      GoodActivity;

/* HOOKS */
extern VALUE    (*CallGateEvalFunction) (int, VALUE *);
extern VALUE    (*AfterSimulationHook) (int, VALUE *);
extern VALUE    (*AfterFaultDropping) (int, VALUE *);
extern VALUE    (*EvaluateCallBackGate) (int, VALUE *);

/* RUN-TIME FLAGS */
extern void     (*RunTimeFaultDropping) (void);
extern void     (*LastFaultDropping) (void);
void            __FaultDroppingPO_1D(void);
void            FaultDroppingPO(void);
void            FaultDroppingLastPO(void);
void            FaultDroppingFF(void);
void            FaultDroppingLastFF(void);
void            FaultDroppingTresholdFF(void);
