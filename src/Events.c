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

#include "Fenice.h"
#include "SharedVars.h"
#include "Simulation.h"
#include "DescrVal.h"

int FaultActivity = 0;

void            SetEvent_deb(int gate)
{
    if (DescrVal[gate].Scheduled)
	return;

    DescrVal[gate].Scheduled = 1;
    Events[descr[gate].level][EventsCount[descr[gate].level]++] = gate;
}

int             GetEvent_deb(int level)
{
    register int    r;

    if (!EventsCount[level])
	return -1;

    r = Events[level][--EventsCount[level]];
    DescrVal[r].Scheduled = 0;
    return r;
}

void            FreezeLevel(int lev)
{
    EventsCount[max_level + 1] = EventsCount[lev];
    memcpy(Events[max_level + 1], Events[lev], EventsCount[lev] * sizeof(int));
    EventsCount[lev] = 0;
}

int             GetFrozenEvent(void)
{
    register int    r;

    if (!EventsCount[max_level + 1])
	return -1;

    r = Events[max_level + 1][--EventsCount[max_level + 1]];
    DescrVal[r].Scheduled = 0;
    return r;
}

