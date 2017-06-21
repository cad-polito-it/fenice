/*
 * @(#)data.h   1.1
 * $Id: data2.h,v 1.17 1999/02/22 17:59:08 violante Exp $
 *
 * $Log: data2.h,v $
 * Revision 1.17  1999/02/22 17:59:08  violante
 * Allargato a 2048 MAX_LINE.
 *
 * Revision 1.16  1998/01/23 15:30:33  squiller
 * TYPES with LSB==1 are NEGATED
 *
 * Revision 1.15  1998/01/23  15:07:10  squiller
 * FF set to 10, for compatibility...
 *
 * Revision 1.14  1998/01/23  14:57:54  squiller
 * added LOGIC0 & LOGIC1. tr/define/enum/
 *
 * Revision 1.13  1996/10/11  14:48:24  corno
 * 'type' converted to 'signed char'
 *
 * Revision 1.12  1996/10/09  09:41:32  corno
 * Added CALLBACK gate type
 * Addes some #ifdefs to make Giovanni happier
 *
 * Revision 1.11  1996/05/21  15:44:43  corno
 * Allargati MAX_LINE e MAX_LEN_ID
 *
 * Revision 1.10  1996/03/19  22:25:39  corno
 * Added ClockDescr
 *
 * Revision 1.9  1996/03/08  15:31:06  corno
 * added {Reset|Preset}DescrName
 *
 * Revision 1.8  1996/01/30  16:52:45  corno
 * added create_silent
 *
 * Revision 1.7  1996/01/25  13:21:41  corno
 * added max_level
 *
 * Revision 1.6  1996/01/10  22:42:14  corno
 * added get_descr_pos
 *
 * Revision 1.5  1995/11/24  12:55:27  corno
 * Version 2.00 create() from Edif:
 * now handles VDD, GND, generic Flip-Flops (through the .ff_type field),
 * pin names (see library.h and get_pin_pos(), accessible trough the .date_id
 * field).
 *
 * Revision 1.4  1995/11/24  12:52:50  corno
 * cleaned up definitions
 *
 */

#ifndef __DATA2_H
#define __DATA2_H

/* OBSOLETE codes DO NOT USE */
#define MMAND     0     /* codici per la lettura del .DEE */
#define MMNAND    1
#define MMOR      2
#define MMNOR     3
#define MMBUF     4
#define MMNOT     5
#define MMEXOR    30
#define MMEXNOR   31
#define MMFF      44

/* GATE TYPES. INTERNAL CODES */
/* TYPES with LSB==1 are NEGATED (ask corno@polito.it) */
/* TYPES must be CONSECUTIVE (ask squillero@polito.it) */
enum {
    AND = 0,	/* 0 */
    NAND,	/* 1 */
    OR,		/* 2 */
    NOR,	/* 3 */
    BUF,	/* 4 */
    NOT,	/* 5 */
    EXOR,	/* 6 */
    EXNOR,	/* 7 */
    LOGIC0,	/* 8 */
    LOGIC1, 	/* 9 */
    FF = 10,	/* 10 */
    CALLBACK	/* 11 */
};

/* Extend FF type */
#define	FFD	0
#define	FFDR	1
#define	FFDP	2
#define	FFDRP	3

/* gate ATTRIBUTES */
#define INTERNAL   0
#define PI     1
#define PO         2

#ifndef MAX
#define MAX(a,b)  (a>b)? a:b
#endif
#ifndef MIN
#define MIN(c,d)  (c<d)? c:d
#endif
#define MOZ_VERSION20   20
#define MOZ_VERSION22   22
#define MOZ_VERSION23   23

#define MAX_FILENAME_LEN  80    /* lunghezza massima nome file di input      */
#define MAX_FANOUT_BRANCH 400    /* massimo fanout consentito nella rete      */
#define MAX_FANIN      50    /* massimo fanin consentito nella rete       */
#define MAX_LONG   0xffffffff   /* massimo numero intero senza segno su 32
				 * bit */
#define MAX_DOUBLE 1.7E+30  /* massimo numero in floating point su 8 byte */
#define MAX_LINE      2048
#define MAX_LEN_ID        80    /* maximum length of an identifier */

/* costanti per la struttura di propagazione */
#define FINE        -1
#define PRESENT     1
#define ABSENT      0

/* valori assunti dai descritori */
#define ICS 1
#define ZERO    0
#define UNO -1

#ifndef NULL
#define NULL    0
#endif

/* struttura singolo descrittore */
typedef struct
{
	char	attr;    /* PI, PO, INTERNAL */
	signed char type;    /* AND NAND .... FF */
	int	fanin, fanout; /* number of gate inputs/outputs */
	char	*to_name;	/* name of the output net */
	int	level;		/* distance from PIs or PPIs */
	int	*to, *from;	/* pointers to arrays of gate indexes */
	char        *name;	/* gate instance name */

	char	ff_type;	/* FFD   = FF
				   FFDR  = FF with reset 
				   FFDP  = FF with preset
				   FFDRP = FF with reset and preset */
	int	gate_id;	/* index to a structure describing library */
}DESCRIPTOR;

/* prototype della create() */
int create (char *name) ;

int	get_descr_pos( char *s) ;

/* global variables */

extern int n_descr ;	/* number of gates */
extern DESCRIPTOR       *descr;       /* gate structures */

extern int              n_pi;
extern int              n_po;
extern int              n_ff;

extern int              max_level;

extern int      *pi_array;
extern int      *po_array;
extern int      *ppi_array;
extern int      *ppo_array;

extern int create_silent ;

/* exported by the create library */
extern	char	*ResetDescrName;
extern	char	*PresetDescrName;
extern  int     ClockDescr;

#endif
