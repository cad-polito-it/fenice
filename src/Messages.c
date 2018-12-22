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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <stdarg.h>
//#include <varargs.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include "macros.h"

#include "Messages.h"

/*
 * INTERNAL PROTOS
 */




static void imsgShowTime(int l);
static char *imsgGetLevel(int l);

#define CHECKLEVEL(L) ((!(L&MSG_EXACT) && (L&~MSG_OPTIONMASK) >= (VerboseLevel&~MSG_OPTIONMASK)) || \
		       ((L&MSG_EXACT) && (L&~MSG_OPTIONMASK) == (VerboseLevel&~MSG_OPTIONMASK)))

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

static int VerboseLevel = MSG_INFO;
static int bgMaximum = 0;
static int bgCurrent = 0;

#define         bgMAX	40

/****************************************************************************/
/*                I N T E R N A L     F U N C T I O N S                     */
/****************************************************************************/


int min(int a, int b)
{
    if (a < b)
        return a;
    else
        return b;
}

static void imsgShowTime(int l)
{
    time_t _Allen;
    struct tm *_Ginsberg;

    if (l & MSG_NOTIME)
        return;
    if (!(VerboseLevel & MSG_SHOWTIME) && !(l & MSG_SHOWTIME))
        return;

    time(&_Allen);
    _Ginsberg = localtime(&_Allen);
    fprintf(stderr, "[%02d:%02d:%02d] ", _Ginsberg->tm_hour,
            _Ginsberg->tm_min, _Ginsberg->tm_sec);
}

static char *imsgGetLevel(int l)
{
    static char level[80];

    if (l < 0)
        l = VerboseLevel;

    *level = 0;

    if ((l & ~MSG_OPTIONMASK) == MSG_DEBUG)
        strcat(level, "DEBUG");
    else if ((l & ~MSG_OPTIONMASK) == MSG_VERBOSE)
        strcat(level, "VERBOSE");
    else if ((l & ~MSG_OPTIONMASK) == MSG_INFO)
        strcat(level, "INFO");
    else if ((l & ~MSG_OPTIONMASK) == MSG_WARNING)
        strcat(level, "WARNING");

    if (l & MSG_SHOWTIME)
        strcat(level, ", SHOWTIME");

    return level;
}

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

int msgSetLevel(int level)
{
    register int r;

    r = VerboseLevel;
    VerboseLevel = level;
    msgMessage(MSG_INFO, "Verbosity level set to %s", imsgGetLevel(level));
    return r;
}

int msgMessage(int level, const char *template, ...)
{
    va_list ap;
    static int LastLineIsBlank = 0;

    if (!CHECKLEVEL(level))
        return 0;

    if (!*template) {
        if (LastLineIsBlank) {
            return 0;
        } else {
            LastLineIsBlank = 1;
            fprintf(stderr, "\n");
            fflush(stderr);
            return -1;
        }
    } else {
        LastLineIsBlank = 0;
    }

    va_start(ap, template);
    imsgShowTime(level);
    vfprintf(stderr, template, ap);
    fprintf(stderr, "\n");
    fflush(stderr);
    va_end(ap);

    if (level & MSG_ABORT) {
        fprintf(stderr, "Dumping core (if possible). Wait...\n");
        fflush(stderr);
        abort();
    }

    return 1;
}

void msgBarGraphStart(int level, int max, const char *template, ...)
{
    va_list ap;

    bgCurrent = bgMaximum = -1;

    if (!CHECKLEVEL(level))
        return;

    va_start(ap, template);
    imsgShowTime(level);
    vfprintf(stderr, template, ap);
    fprintf(stderr, " ");
    fflush(stderr);
    va_end(ap);

    bgMaximum = max;
    bgCurrent = 0;
}




void msgBarGraph(int current)
{
    int t;

    if (bgMaximum < 0)
        return;

    if (current < 0)
        current = bgMaximum;
    else
        current = min(current, bgMaximum);

    if (!bgMaximum) {
        fprintf(stderr, "\n");
    } else {
        for (t = bgCurrent * bgMAX / bgMaximum;
             t < current * bgMAX / bgMaximum; ++t)
            fprintf(stderr, ".");
        if (!bgMaximum || current == bgMaximum)
            fprintf(stderr, "\n");
    }
    fflush(stderr);

    bgCurrent = current;
}

void msgBarGraphSpecial(char symbol)
{
    if (bgMaximum < 0)
        return;
    fprintf(stderr, "%c", symbol);
    fflush(stderr);
}
