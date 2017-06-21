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

/*
 * NEED Fenice.h
 */

enum { FF_UNO, FF_ZERO, FF_ICS };

FF_BUFFER      *funcTakeBuffer(void);
void            AllocateBuffers(int num);
void            DropBufferChain(FF_BUFFER * ptr);
void            ReleaseBuffers(void);

#define TRUETYPE(X)  ((X)>0?(X):-(X))
extern F_FAULT *InternalFaultList;
extern F_FAULT **ActiveFaults;
extern F_FAULT_INFO FaultInfo[WORD_SIZE + 1];
extern int      InternalFNum;


#ifdef DEBUG

#define         TakeBuffer(B)  		(B = funcTakeBuffer())

#else

extern FF_BUFFER *_fenice_internal_FreeBufferList;
extern int      _fenice_internal_AllocatedBuffers;
#ifdef GARBAGE_COLLECTOR
#define MEM(X)	GC_MALLOC(X)
#else
#define MEM(X)	malloc(X)
#endif
#define         TakeBuffer(B)						                \
{                                                                                       \
    if (!_fenice_internal_FreeBufferList) {						\
	_fenice_internal_FreeBufferList = calloc(sizeof(FF_BUFFER), 1);        		\
	++_fenice_internal_AllocatedBuffers;						\
    }                                                                                   \
    B = _fenice_internal_FreeBufferList;					        \
    _fenice_internal_FreeBufferList = B->next;						\
    B->next = NULL;                                                                     \
}

#endif
