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

/*
 * NEED Fenice.h
 */

#ifndef DESCR_VAL_INCLUDED
#define DESCR_VAL_INCLUDED

typedef struct _DESCR_VAL {
    int             Id;
    VALUE           RefValue;
    VALUE           NewRefValue;
    VALUE           Value;
    int             Scheduled;
    char            dirty;
    int 	    Stats[2];
} DESCR_VAL;

extern DESCR_VAL *DescrVal;

#endif
