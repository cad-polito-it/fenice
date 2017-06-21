/*****************************************************************-*-c-*-*\
*             *                                                           *
*   #####     *  (c) Copyright 2010, Giovanni Squillero                   *
*  ######     *  http://staff.polito.it/giovanni.squillero/               *
*  ###   \    *  giovanni.squillero@polito.it                             *
*   ##G  c\   *                                                           *
*   #     _\  *  This code is licensed under a BSD 2-clause license       *
*   |  _/     *  See <https://github.com/squillero/fenice> for details    *
*             *                                                           *
\*************************************************************************/


#include "Fenice.h"

#ifndef FENICE_VERSION
#warning Please, use Makefile supplied
#define FENICE_VERSION "unknown"
#endif

char           *GetFeniceVersion(void)
{
    return FENICE_VERSION 
#ifdef VALUE_2
	" [2-value]"
#endif
#ifdef VALUE_3
	" [3-value]"
#endif
#ifdef GARBAGE_COLLECTOR
	" [gc]"
#endif
	;
}

char           *GetFeniceDate(void)
{
    static char     date[64];
    char            a[8], b[8], c[8], d[16];

    sscanf(__DATE__ " " __TIME__, "%s %s %s %s", a, b, c, d);
    sprintf(date, "%s-%s-%s %s", b, a, c, d);

    return (date);
}
