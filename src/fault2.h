/*
 * $Id: fault2.h, v 0.1 19995/12/14 Bolla
 *
 * $Log: fault2.h,v $
 * Revision 1.4  2000/06/16 13:20:30  squiller
 * *** empty log message ***
 *
 * Revision 1.3  2000/06/09 12:48:36  squiller
 * *** empty log message ***
 *
 * Revision 1.2  1996/08/28 13:27:56  corno
 * Adapted for 64bit - Massimo, Flavio
 *
 * Revision 1.1  1996/03/08  15:32:43  corno
 * Initial revision
 *
 *
 * define new fault list
 */

#define STUCK_AT_0 0
#define STUCK_AT_1 -1

#define FAULT_MASK_PERMANENT 0x0001
#define FAULT_MASK_TRANSIENT 0x0002
#define FAULT_MASK_STUCK_AT  0x0010
#define FAULT_MASK_BIT_FLIP  0x0020

#define FAULT_UNKNOWN   0x0000
#define FAULT_PERMANENT_STUCK_AT (FAULT_MASK_PERMANENT | FAULT_MASK_STUCK_AT)
#define FAULT_TRANSIENT_STUCK_AT (FAULT_MASK_TRANSIENT | FAULT_MASK_STUCK_AT)
#define FAULT_TRANSIENT_BIT_FLIP (FAULT_MASK_TRANSIENT | FAULT_MASK_BIT_FLIP)

enum {
    UNTESTABLE = -1, UNDETECTED = 0, DETECTED = 1
};

/* fault descriptor */

typedef struct fd {
    int             descr;	/* descrittore del gate */

    int             from;	/* -1 : guasto sull'uscita 
				 * #  : descrittore del gate padre */

    int             pin;	/* -1 : guasto sull'uscita
				 * #  : numero del pin di ingresso */

    int             val;	/* 0  : stuck at 0
				 * -1 : stack at 1 */

    int             size;	/* #  : numero di guasti equivalenti */

    int             status;	/* UNTESTABLE, UNDETECTED or DETECTED */

    int 	    type;	/* tipo del guasto: SAx, SEU... */
    int 	    activation;	/* numero della riga in cui il guasto e` attivo
				 * == 0 se permanent stuck-at */
} FAULT;

/* function prototype */
int             create_fau(char *);
void            write_faultlist(char *);
