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

#define MSG_ABORT 	0x1000

#define MSG_OPTIONMASK	0x000f
#define MSG_SHOWTIME 	0x0001
#define MSG_NOTIME 	0x0004
#define MSG_EXACT 	0x0008

#define MSG_DEBUG 	0x0010
#define MSG_VERBOSE 	0x0020
#define MSG_INFO 	0x0040
#define MSG_NONE 	0x0100
#define MSG_WARNING 	0x0400
#define MSG_FORCESHOW 	0x0800

#define MSG_ERROR 	(MSG_ABORT | MSG_FORCESHOW | MSG_SHOWTIME)

void msgBarGraphStart(int level, int max, const char *template, ...);
void msgBarGraph(int current);
void msgBarGraphSpecial(char symbol);

int msgSetLevel(int level);
int msgMessage(int level, const char *template, ...);
