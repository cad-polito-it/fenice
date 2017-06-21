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

#ifndef CHECK_HEADER_ALREADY_INCLUDED

#define CHECK_HEADER_ALREADY_INCLUDED Yes
/* 
 * CHECK MACROS. Really USEFUL
 */
#define PANIC "%BPANIC %Ncontact author imediately"
#define WARNING(X, Y) Print(stderr, "%BWARNING:" X "::%N \n" Y "\n")
#define BANNER(X)                                                             \
        { time_t _Allen; struct tm *_Ginsberg;                                \
          time(&_Allen);                                                      \
          _Ginsberg = localtime(&_Allen);                                     \
          Print(X, "\n    [%02d:%02d:%02d] in function ",                     \
                  _Ginsberg->tm_hour, _Ginsberg->tm_min, _Ginsberg->tm_sec);  \
          Print(X, "%U%s%N (" __FILE__ ":%d)\n",                              \
                  _FunctionName, __LINE__);                                   \
        }
#define CheckFalse(ex)                                                        \
        { long      _Kerouac;                                                 \
          if( (_Kerouac=(long) (ex)) ) {                                      \
                Print(stderr,                                                 \
                      "\n\n%BERROR::CHECK FAILED:"                            \
                      " expression is true (0x%lx) %N\n"                      \
                      "(" #ex ")\n", _Kerouac);                               \
                BANNER(stderr);                                               \
                Print(stderr, "\n\nDumping core (if possible). Wait...\n");   \
                abort();                                                      \
        } }

#define CheckTrue(ex)                                                         \
        { if((ex)==0) {                                                       \
                Print(stderr,                                                 \
                      "\n\n%BERROR::CHECK FAILED:"                            \
                      " expression is false (zero) %N\n"                      \
                      "(" #ex ")\n");                                         \
                BANNER(stderr);                                               \
                Print(stderr, "\n\nDumping core (if possible). Wait...\n");   \
                abort();                                                      \
        } }

#ifdef DEBUG
#define DCheckTrue(X)    CheckTrue(X)
#define DCheckFalse(X)   CheckFalse(X)
#else
#define DCheckTrue(X)    0
#define DCheckFalse(X)   0
#endif

#endif
