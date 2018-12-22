/*
 * library.h
 * $Id: library.h,v 1.2 1997/01/23 16:00:34 violante Exp $
 *
 * Definizione delle costanti.
 *
 * I numeri sono le posizioni dei componenti nella library.
 *
 * $Log: library.h,v $
 * Revision 1.2  1997/01/23  16:00:34  violante
 * Aggiunti LOGIC_0 e LOGIC_1
 *
 * Revision 1.1  1995/11/24  12:58:49  corno
 * Initial revision
 *
 */

#define		AND2	0
#define		AND3	15
#define		AND4	19
#define		AND5	23
#define		NAND2	1
#define		NAND3	14
#define		NAND4	18
#define		NAND5	22
#define		OR2	2
#define		OR3	16
#define		OR4	20
#define		OR5	24
#define		NOR2	5
#define		NOR3	17
#define		NOR4	21
#define		NOR5	25
#define		XOR2	6
#define		XOR3	8
#define		XOR4	10
#define		XOR5	12
#define		XNOR2	7
#define		XNOR3	9
#define		XNOR4	11
#define		XNOR5	13
#define		INV	3
#define		BUFF	4
#define		FFDG	26
#define		FFDRG	27
#define		FFDPG	28
#define		FFDRPG	29
#define		LOGIC_0	30
#define		LOGIC_1	31

typedef struct T {
    char *name;
    int n_fanin;
    char **fanin_list;
    char *fanout;
} LIB_TYPE;

extern int n_comp;
extern LIB_TYPE *library;

/* these are implementation-dependent. Please do not use */
extern char *pin_name[5];
extern char *pin_name_ffd[2];
extern char *pin_name_ffr[3];
extern char *pin_name_ffp[3];
extern char *pin_name_ffrp[4];

/* Function Prototypes */
void init_library(void);
int get_pin_pos(char *pin_name);
