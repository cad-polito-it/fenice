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

extern int      CircuitModified;

int             compute_level(int, int, int *, int *);

void            CircuitMod_Commit(FAULT * flist, int fnum);
int             CircuitMod_TranslateVector(int *v);
int             CircuitMod_Translate(int x);

int             CircuitMod_GetFreeGate(void);
void            CircuitMod_SetTypeAttr(int gate, int type, int attr);

void            CircuitMod_RemoveGate(int g);
int             CircuitMod_AddGate(char *name, int type, int attr);
void            CircuitMod_RemoveNet(int from, int to);
void            CircuitMod_AddNet(int from, int to);

int             CircuitMod_ChangeFFClock(int ff, int newclock);
int             CircuitMod_ChangeFFData(int ff, int newdata);

void            CircuitMod_BuildFFArray(void);
void            CircuitMod_BuildPIArray(void);
void            CircuitMod_BuildPOArray(void);
#define         CircuitMod_BuildArrays() \
                CircuitMod_BuildFFArray(), CircuitMod_BuildPIArray(), CircuitMod_BuildPOArray()

long            CircuitMod_WriteEDF(char *filename, char *entity_name);
void            CircuitMod_ScanFF(int ff, FAULT * flist, int fnum);

int             CircuitMod_RemoveClock(void);

int             CircuitMod_ParanoiaCheck(void);
