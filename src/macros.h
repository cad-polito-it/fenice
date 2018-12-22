#ifndef _INC_NICEMACROS_
#define _INC_NICEMACROS_


#define stateSet(p, state)          ((p)->dwState |=  (state))
#define stateToggle(p, state)       ((p)->dwState ^=  (state))
#define stateClear(p, state)        ((p)->dwState &= ~(state))
#define stateTest(p, state)         ((p)->dwState &   (state))

#define sysstateSet(p, state)       ((p)->dwSysState |=  (state))
#define sysstateToggle(p, state)    ((p)->dwSysState ^=  (state))
#define sysstateClear(p, state)     ((p)->dwSysState &= ~(state))
#define sysstateTest(p, state)      ((p)->dwSysState &   (state))

#define styleSet(p, style)          ((p)->dwStyle |=  (style))
#define styleToggle(p, style)       ((p)->dwStyle ^=  (style))
#define styleClear(p, style)        ((p)->dwStyle &= ~(style))
#define styleTest(p, style)         ((p)->dwStyle &   (style))

#define styleexSet(p, style)        ((p)->dwExStyle |=  (style))
#define styleexToggle(p, style)     ((p)->dwExStyle ^=  (style))
#define styleexClear(p, style)      ((p)->dwExStyle &= ~(style))
#define styleexTest(p, style)       ((p)->dwExStyle &   (style))


#define maskSet(p, mask)            ((p)->uiMask |=  (mask))
#define maskToggle(p, mask)         ((p)->uiMask ^=  (mask))
#define maskClear(p, mask)          ((p)->uiMask &= ~(mask))
#define maskTest(p, mask)           ((p)->uiMask &   (mask))

#define flagsSet(p, flags)          ((p)->dwFlags |=  (flags))
#define flagsToggle(p, flags)       ((p)->dwFlags ^=  (flags))
#define flagsClear(p, flags)        ((p)->dwFlags &= ~(flags))
#define flagsTest(p, flags)         ((p)->dwFlags &   (flags))

#define notifySet(p, ntfm)          ((p)->dwNotifyMask |=  (ntfm))
#define notifyToggle(p, ntfm)       ((p)->dwNotifyMask ^=  (ntfm))
#define notifyClear(p, ntfm)        ((p)->dwNotifyMask &= ~(ntfm))
#define notifyTest(p, ntfm)         ((p)->dwNotifyMask &   (ntfm))

#define sysnotifySet(p, ntfm)       ((p)->dwSysNotifyMask |=  (ntfm))
#define sysnotifyToggle(p, ntfm)    ((p)->dwSysNotifyMask ^=  (ntfm))
#define sysnotifyClear(p, ntfm)     ((p)->dwSysNotifyMask &= ~(ntfm))
#define sysnotifyTest(p, ntfm)      ((p)->dwSysNotifyMask &   (ntfm))

#define ctlstyleSet(p, style)       ((p)->dwCtlExStyle |=  (style))
#define ctlstyleToggle(p, style)    ((p)->dwCtlExStyle ^=  (style))
#define ctlstyleClear(p, style)     ((p)->dwCtlExStyle &= ~(style))
#define ctlstyleTest(p, style)      ((p)->dwCtlExStyle &   (style))


#ifdef __cplusplus
extern "C" {                    /* Assume C declarations for C++  */
#endif                          /* __cplusplus */


#ifdef _DELEAKER_ACTIVE_

    extern HNICEDELEAKER g_hndl;


    __inline LPVOID memAlloc(SIZE_T cbSize) {
        LPVOID lpvMem = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cbSize);
         NiceDeleaker_Alloc(g_hndl, cbSize, lpvMem);
         return (lpvMem);
    } __inline LPVOID memAllocNZ(SIZE_T cbSize) {
        LPVOID lpvMem = LocalAlloc(LMEM_FIXED, cbSize);
        NiceDeleaker_Alloc(g_hndl, cbSize, lpvMem);
        return (lpvMem);
    }


    __inline LPVOID memReAlloc(LPVOID lpvMem, SIZE_T cbSize) {
        LPVOID lpvMem1 =
            LocalReAlloc(lpvMem, cbSize, LMEM_MOVEABLE | LMEM_ZEROINIT);
        NiceDeleaker_Realloc(g_hndl, lpvMem, cbSize, lpvMem1);
        return (lpvMem1);
    }

    __inline LPVOID memReAllocNZ(LPVOID lpvMem, SIZE_T cbSize) {
        LPVOID lpvMem1 = LocalReAlloc(lpvMem, cbSize, LMEM_MOVEABLE);
        NiceDeleaker_Realloc(g_hndl, lpvMem, cbSize, lpvMem1);
        return (lpvMem1);
    }


    __inline BOOL memFree(LPVOID lpvMem) {
        LocalFree(lpvMem);
        NiceDeleaker_Free(g_hndl, lpvMem);
        return (TRUE);
    }


    __inline LPVOID memSizeBuf(LPVOID buf, SIZE_T cb) {
        if (buf)
            return (memReAlloc(buf, cb));
        else
            return (memAlloc(cb));
    }

    __inline LPVOID memSizeBufNZ(LPVOID buf, SIZE_T cb) {
        if (buf)
            return (memReAllocNZ(buf, cb));
        else
            return (memAllocNZ(cb));
    }

#else                           //_DELEAKER_ACTIVE_

#ifdef _WIN32


#ifndef LOCALALLOCSUPPORTED
#define LocalAlloc		LocalAlloc_Not_Supported
#define LocalReAlloc	LocalReAlloc_Not_Supported
#define LocalFree		LocalFree_Not_Supported
#endif

    extern HANDLE g_hHeap;


#define memAlloc(cb)            HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, (SIZE_T)(cb))
#define memAllocNZ(cb)          HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, (SIZE_T)(cb))
#define memReAlloc(p, cb)       HeapReAlloc(g_hHeap, HEAP_ZERO_MEMORY, (LPVOID)(p), (SIZE_T)(cb))
#define memReAllocNZ(p, cb)     HeapReAlloc(g_hHeap, HEAP_ZERO_MEMORY, (LPVOID)(p), (SIZE_T)(cb))
#define memFree(p)              HeapFree(g_hHeap, 0, (LPVOID)(p))


    __inline LPVOID memSizeBuf(LPVOID buf, SIZE_T cb) {
        if (buf)
            return (memReAlloc(buf, cb));
        else
            return (memAlloc(cb));
    } __inline LPVOID memSizeBufNZ(LPVOID buf, SIZE_T cb) {
        if (buf)
            return (memReAllocNZ(buf, cb));
        else
            return (memAllocNZ(cb));
    }

#else
#include <stdlib.h>
#define MemAlloc(cb)            malloc(cb)
#define MemReAlloc(p, cb)       realloc(p, (cb))
#define MemFree(p)              free((LPSTR)p)
#endif

#endif                          //_DELEAKER_ACTIVE_





// Rectangle helpers
#define rectWidth(rc)           ((rc).right - (rc).left)
#define rectHeight(rc)          ((rc).bottom - (rc).top)

#define rectWidthPtr(prc)       ((prc)->right - (prc)->left)
#define rectHeightPtr(prc)      ((prc)->bottom - (prc)->top)

// Gdi helpers
#define MoveTo(hdc, x, y)       MoveToEx(hdc, x, y, NULL)

// element size helper
#define lengthof(exp)   (sizeof(exp) / sizeof(exp[0]))

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          // _INC_NICEMACROS_
