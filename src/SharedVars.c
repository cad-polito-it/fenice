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

int           **Events;
int            *EventsCount;

/* CIRCUIT */
int            *LatArray;
VALUE         *GoodFFBefore;
VALUE         *GoodFFAfter;
VALUE         *GoodFFAfter;
VALUE         *InitialFFVal;
VALUE         *PIValues;
int             InternalFNum;
