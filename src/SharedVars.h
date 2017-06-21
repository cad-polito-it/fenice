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

#ifndef SHARED_VARS

#define SHARED_VARS

/* CIRCUIT */
extern int      n_descr;
extern DESCRIPTOR *descr;
extern int      n_pi;
extern int      n_po;
extern int      n_ff;
extern int      max_level;
extern int     *pi_array;
extern int     *po_array;
extern int     *ppi_array;
extern int     *ppo_array;

/* FAULTS */
extern int      n_fault;
extern FAULT   *faultlist;

/* INTERNAL CIRCUIT */
extern int     *ppi_array;
extern int     *LatArray;
extern VALUE   *InitialFFVal;
extern VALUE   *GoodFFBefore;
extern VALUE   *GoodFFAfter;
extern VALUE  *PIValues;

/* EVENTS */
extern int    **Events;
extern int     *EventsCount;

#endif
