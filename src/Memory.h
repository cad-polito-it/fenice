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

/* #define GC */
/* #define GC_DEBUG */

void *CheckRealloc(void *ptr, size_t size);
char *CheckStrdup(const char *s);
void *CheckCalloc(size_t nelem, size_t elsize);
void *CheckMalloc(size_t size);
void SafeMemCpy(void *dst, void *src, size_t size);

#define 	CheckFree(X) 	(_CheckFree(X), X=NULL)
void _CheckFree(void *ptr);
