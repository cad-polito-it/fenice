/*------------------------------------------------------------------------------
	descr.h

	File di configurazione per la create.
	Massimo Violante
*/

#include "data2.h"

int		Descr_Def_Size;


/* le definizioni che seguono mi servono per gestire l'associazione
	nome componente-> numero di descrittore
*/

#define		PEDANTIC	0

#define		NET_MEAN_FANOUT	3	/* fanout medio di una net */

#define		DESCR_DEF_SIZE	100	/* numero minimo di componenti
					   allocati in una malloc */
#define		MAXASSOC	100003
#define		IN	0
#define		OUT	1

/*
extern DESCRIPTOR	*descr;			
extern int		n_descr;	
extern int		n_pi;	
extern int		n_po;
*/

extern int		max_level;	

/*extern int	*pi_array;
extern int	*ppi_array;
extern int	*ppo_array;
extern int	n_ff;
*/

int		n_descr1;

typedef struct {
	char	*name;
	int	n_descr;
} LASSOC;

typedef struct {
	char	*name;
	int 	n_descr;
	LASSOC	*list;	/* lista delle collisioni */
	int	n_el;	/* numero di elementi nella lista */
} ASSOC;

typedef struct {
	int	n_descr;	/* descrittore del componente */
	char	dir;		/* direzione del pin */	
	char	*pin_name;	/* nome del pin */
	int	net_index;	/* numero della net corrispondente */
} PIN;

typedef	struct {
	char	*netname;	/* nome della net */
	int	list;		/* indice del primo descrittore */
				/* nella lista delle associazioni */
				/* descrittore ->verso del pin */
	int	n_info;		/* numero di elementi */
} NET;					   

/* DA METTERE A POSTO */
int	n_assoc;	/* numero di associazioni */
ASSOC	*list_of_assoc;

/*--------------------*/

NET	*netlist;	/* vettore contenente la netlist */
int	n_net;		/* numero di net indentificate */
int	l_net;		/* ultima net della lista */

PIN	*pinlist;	/* vettore contenente le associazioni
				   descrittore componente -> verso del pin */
int	n_pin;		/* numero di descrittori */
int	pinlist_size;

char	*ResetDescrName;	/* nome del descrittore di reset */
char	*PresetDescrName;	/* nome del descrittore di preset */
int	ClockDescr;		/* numero del descrittore del clock */
