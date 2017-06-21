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

#ifndef EVENTS_INCLUDED
#define EVENTS_INCLUDED

#include <DescrVal.h>
#include <SharedVars.h>

int             GetFrozenEvent(void);
void            FreezeLevel(int level);

extern int      FaultActivity;
int             __GetEvent1D(int level);
void            __SetEvent1D(int gate, int level);
void            __ScheduleFanOut1D(int gate, int level);

#ifdef DEBUG
void            SetEvent_deb(int gate);

#define         SetEvent(X) 		SetEvent_deb(X)
#else
static inline void SetEvent(register int X)
{
    if (!DescrVal[X].Scheduled) {
	DescrVal[X].Scheduled = 1;
	Events[descr[X].level][EventsCount[descr[X].level]++] = X;
    }
}
#endif

#ifdef DEBUG
int             GetEvent_deb(int level);

#define         GetEvent(X)		GetEvent_deb(X)
#else
#define         GetEvent(X)                                 \
    (EventsCount[X]?                                        \
        (DescrVal[Events[X][--EventsCount[X]]].Scheduled=0, \
	 Events[X][EventsCount[X]])                         \
    :                                                       \
	-1                                                  \
    )
#endif

#endif
