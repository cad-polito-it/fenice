/****************************************************************************\
*                         *                                                  *
*  Giovanni A. Squillero  *   LungoPo Antonelli 203 - 10153 TORINO - ITALY   *
*                         *   +39-11-8994677  (Roma: +39-6-68307390)         *
*           (!)           *   charme5@athena.polito.it  (POLCLU::CAD6)       *
*                         *                                                  *
\****************************************************************************/

#define VERSION "Version 4.2b"

#define SQ (8*sizeof(int))
#define ERRORFLAG 0x100
#define AND_E     AND   | ERRORFLAG
#define NAND_E    NAND  | ERRORFLAG
#define OR_E      OR    | ERRORFLAG
#define NOR_E     NOR   | ERRORFLAG
#define BUF_E     BUF   | ERRORFLAG
#define NOT_E     NOT   | ERRORFLAG
#define EXOR_E    EXOR  | ERRORFLAG
#define EXNOR_E   EXNOR | ERRORFLAG
#define WIRE      255
#define WIRE_E    WIRE  | ERRORFLAG
#define FF_E      FF    | ERRORFLAG
#define CLOCK     11    /*********************/
#define DUMMY     12    /*********************/
#define DUMMY_E   DUMMY | ERRORFLAG

#ifdef ZERO
#undef ZERO
#endif
#define ZERO    ((unsigned)(0))

#ifdef UNO
#undef UNO
#endif
#define UNO    (~ZERO)

/*  Nostra struttura di un singolo descrittore */
typedef struct {
	char            attr;
	short int       type;
	unsigned int            fanin, fanout;
	int             level, *to, *from;
	unsigned long           GoodValue;
	unsigned long           CurrentValue;
	unsigned int            GroupId;
	unsigned int            ErrorId;
} MYDESCRIPTOR;


/*  Un fault e` fatto cosi` */
typedef struct {
	unsigned int   NumDesc;
	unsigned char  Value;
	unsigned int   Type;
	unsigned int   Next;
	unsigned int   Gotcha;
	int            Modified;
	char			*Output;
} ERROR;


typedef struct {
	int fitness;
	int lung_individuo;
        char **pattern;
} POPOLAZIONE;

/* TIPI DI ERRORI */
#define STUCK_AT_UNO    0xff
#define STUCK_AT_ZERO   0x00
#define DUMMY_ERROR     0x02

/*PER SCHEDULARE */
#define LAST    -1
#define UNSET   -2

/* TIPI DI MESSAGGI */
#define GENERAL         0
#define IOERROR         1
#define NOMEM           2
#define BADFORMAT       3
#define SYNTAX          4
#define BADFAULT        5
#define WELCOME         6
#define NOFAULT         8
#define NOFAULT2        9
#define CANNOT_ORDER    11
#define BADREPSIM       12
#define CONNECT         13
#define UNDESCRIPTOR    14
#define SAVEMEMORY      15
#define NOARG           16
#define OVERFLOW        17
#define FANIN		18

/*      SAVED:  SAVED e` il valore di default della variabile saved che
    indica quanta memoria risparmiamo rispetto ad allocare staticamente
    una matrice di dimensione
			num_ff * num_fault
    per salvare i valori dei flip flop.
    ie. Noi allochiamo un vettore di dimensione
		      num_ff * num_fault * saved / 1000

	Notez Bien: Quando la memoria e` insufficiente il programma produce
    risultati sbagliati MA NON DA` NESSUN ERRORE.
	Se compilate con #define DEBUG il programma invece se ne accorge
    e vi avverte... */

#define SAVED 100 

#define  MAX_PATTERN               700
#define  PERC_COMPL_A_1_BIT          /*  1      */ 10
#define  PERC_MUT_INDIV              /*  80     */ 90

#define MAXIMUM_FANIN	512
