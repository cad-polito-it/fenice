/*----------------------------------------------------------------------------
	create_supp.c

	Funzioni per la creazione della struttura dati.
*/

#define		STRONG_DEBUG	1

#define		VERBOSE		1
#define		TRACE		0

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include "create.h"
#include "library.h"

void init_create_memory();
void dump_descr();
void dump_netlist();
void dump_pinlist();
char get_descr_type();
int get_descr_pos(char *s);
void add_assoc();
int get_assoc(char *s);
char get_pin_type();
void create_netlist();
void find_clock_tree(void);

//extern        void init_library( void );

int compute_level(int n_pi, int n_ff, int *pi_array, int *ppi_array);
int Hfun(char *s, int tablen);
int find_clock_descr(void);
int find_reset_descr(void);
int find_preset_descr(void);

/*int    *pi_array;*/
/*int      *ppi_array;*/
/*int      n_ff;*/

int n_sub_descr = 0;            /* numero di descrittori eliminati */
int reset_pos = -1;             /* posizione del descrittore di reset */
int preset_pos = -1;            /* posizione del descrittore di preset */

char library_just_init = 0;     /* mi serve per inizializzare la libreria */

int n_pi;                       //create.h
int n_po;                       //create.h
int *pi_array;                  //create.h
int *ppi_array;                 //create.h
int *ppo_array;                 //create.h
int n_ff;                       //create.h
char *pin_name[5];              //library.h
char *pin_name_ffd[2];          //library.h
char *pin_name_ffr[3];          //library.h
char *pin_name_ffp[3];          //library.h
char *pin_name_ffrp[4];         //library.h
LIB_TYPE *library;              //library.h
int max_level;                  //create.h
int create_silent;

void init_library(void);
int get_pin_pos(char *pin_name);

void init_create_memory()
{
    int i;

#if TRACE == 1
    printf("[init_create_memory]\n");
#endif

    Descr_Def_Size = DESCR_DEF_SIZE;
    n_descr = 0;

    descr = (DESCRIPTOR *) malloc(sizeof(DESCRIPTOR) * Descr_Def_Size); //Descr_Def_Size=100

    /* quando realloca deve creare lo spazio per i nuovi dati */
    for (i = 0; i < Descr_Def_Size; i++) {
        descr[i].fanin = 0;
        descr[i].fanout = 0;
        descr[i].to = NULL;
        descr[i].from = malloc(sizeof(int) * MAX_FANIN);        //max_fanin=50 maximum limit of fanin
        if (descr[i].from == NULL) {
            printf("(create_init_memory) Memory error, quitting...\n");
            exit(1);
        }
    }

    n_pi = 0;
    n_po = 0;
    max_level = 0;

    /* OCCHIO */
    n_assoc = 0;                /* indica che non ci sono ancora associazioni
                                   nome componente -> descrittore there aren'n unions yet */

    list_of_assoc = (ASSOC *) malloc(sizeof(ASSOC) * MAXASSOC);
    if (list_of_assoc == NULL) {
        printf("(init_create_memory) Memory error, quitting...\n");
        return;
    }
    /* azzera la lista */
    for (i = 0; i < MAXASSOC; i++)
        list_of_assoc[i].name = NULL;

    l_net = 0;                  /* ultima net introdotta */
}

void dump_descr()
{
    int i, j;
    FILE *fp;

#if TRACE == 1
    printf("[dump_descr]\n");
#endif
    fp = fopen("result.log", "w");

    fprintf(fp, "\n\n");
    for (i = 0; i < n_descr; i++) {
        switch (descr[i].attr) {
        case PI:
            fprintf(fp, "%d:\tPI\t", i);
            break;
        case PO:
            fprintf(fp, "%d:\tPO\t", i);
            break;
        case INTERNAL:
            fprintf(fp, "%d:\tINTERNAL", i);
            switch (descr[i].type) {
            case FF:
                switch (descr[i].ff_type) {
                case FFD:
                    fprintf(fp, "(FF:D)\t");
                    break;
                case FFDR:
                    fprintf(fp, "(FF:DR)\t");
                    break;
                case FFDP:
                    fprintf(fp, "(FF:DP)\t");
                    break;
                case FFDRP:
                    fprintf(fp, "(FF:DRP)\t");
                    break;
                default:
                    fprintf(fp, "(FF:??)\t");
                    break;
                }
                break;
            case OR:
                fprintf(fp, "(OR)\t");
                break;
            case NOT:
                fprintf(fp, "(NOT)\t");
                break;
            case NAND:
                fprintf(fp, "(NAND)\t");
                break;
            case AND:
                fprintf(fp, "(AND)\t");
                break;
            case NOR:
                fprintf(fp, "(NOR)\t");
                break;
            case EXOR:
                fprintf(fp, "(EXOR)\t");
                break;
            case EXNOR:
                fprintf(fp, "(EXNOR)\t");
                break;
            case BUFF:
                fprintf(fp, "(BUFF)\t");
                break;
            case LOGIC0:
                fprintf(fp, "(LOGIC0)\t");
                break;
            case LOGIC1:
                fprintf(fp, "(LOGIC1)\t");
                break;
            default:
                fprintf(fp, "????\t");
                break;
            }
            break;
        default:
            break;
        }
        fprintf(fp, "%s\n\tlevel = %d\n", descr[i].name, descr[i].level);
        fprintf(fp, "\tfanout = %d : ", descr[i].fanout);
        for (j = 0; j < descr[i].fanout; j++)
            fprintf(fp, "%d, ", descr[i].to[j]);
        fprintf(fp, "\n");

        if (descr[i].fanout != 0)
            fprintf(fp, "\toutput net = %s\n", descr[i].to_name);

        fprintf(fp, "\tfanin = %d : ", descr[i].fanin);
        for (j = 0; j < descr[i].fanin; j++)
            fprintf(fp, "%d, ", descr[i].from[j]);
        fprintf(fp, "\n-------------------------------\n");
    }

    fclose(fp);
}

char get_pin_type(char *s)
/* char	*s;	nome del pin */
{
    /* determina il verso del pin sulla base del suo nome */

#if TRACE == 1
    printf("[get_pin_type]\n");
#endif

    if (strcmp(s, pin_name[0]) == 0)
        return (IN);

    if (strcmp(s, pin_name[1]) == 0)
        return (IN);

    if (strcmp(s, pin_name[2]) == 0)
        return (IN);

    if (strcmp(s, pin_name[3]) == 0)
        return (IN);

    if (strcmp(s, pin_name[4]) == 0)
        return (IN);

    if (strcmp(s, pin_name_ffd[0]) == 0)
        return (IN);

    if (strcmp(s, pin_name_ffd[1]) == 0)
        return (IN);

    if (strcmp(s, pin_name_ffr[2]) == 0)
        return (IN);

    if (strcmp(s, pin_name_ffp[2]) == 0)
        return (IN);

    if (strcmp(s, "LOGIC_0_PIN") == 0)
        return (OUT);

    if (strcmp(s, "LOGIC_1_PIN") == 0)
        return (OUT);

    if (strcmp(s, "O") == 0)
        return (OUT);

    if (strcmp(s, "Q") == 0)
        return (OUT);

    printf("(get_pin_type) Unresolved reference, quitting...\n");
    exit(1);
}

char get_descr_type(s)
char *s;                        /* nome del componente */
{
    /* determina il tipo del componente sulla base del suo nome */

#if TRACE == 1
    printf("[get_descr_type]\n");
#endif

    if (!library_just_init) {
        init_library();
        library_just_init = 1;
    }

    if (strcmp(s, library[AND2].name) == 0)
        return AND;
    if (strcmp(s, library[AND3].name) == 0)
        return AND;
    if (strcmp(s, library[AND4].name) == 0)
        return AND;
    if (strcmp(s, library[AND5].name) == 0)
        return AND;

    if (strcmp(s, library[NAND2].name) == 0)
        return NAND;
    if (strcmp(s, library[NAND3].name) == 0)
        return NAND;
    if (strcmp(s, library[NAND4].name) == 0)
        return NAND;
    if (strcmp(s, library[NAND5].name) == 0)
        return NAND;

    if (strcmp(s, library[OR2].name) == 0)
        return OR;
    if (strcmp(s, library[OR3].name) == 0)
        return OR;
    if (strcmp(s, library[OR4].name) == 0)
        return OR;
    if (strcmp(s, library[OR5].name) == 0)
        return OR;

    if (strcmp(s, library[NOR2].name) == 0)
        return NOR;
    if (strcmp(s, library[NOR3].name) == 0)
        return NOR;
    if (strcmp(s, library[NOR4].name) == 0)
        return NOR;
    if (strcmp(s, library[NOR5].name) == 0)
        return NOR;

    if (strcmp(s, library[XOR2].name) == 0)
        return EXOR;
    if (strcmp(s, library[XOR3].name) == 0)
        return EXOR;
    if (strcmp(s, library[XOR4].name) == 0)
        return EXOR;
    if (strcmp(s, library[XOR5].name) == 0)
        return EXOR;

    if (strcmp(s, library[XNOR2].name) == 0)
        return EXNOR;
    if (strcmp(s, library[XNOR3].name) == 0)
        return EXNOR;
    if (strcmp(s, library[XNOR4].name) == 0)
        return EXNOR;
    if (strcmp(s, library[XNOR5].name) == 0)
        return EXNOR;

    if (strcmp(s, library[INV].name) == 0)
        return NOT;

    if (strcmp(s, library[BUFF].name) == 0)
        return BUF;

    if (strcmp(s, library[FFDG].name) == 0)
        return FF;

    if (strcmp(s, library[FFDRG].name) == 0)
        return FF;

    if (strcmp(s, library[FFDPG].name) == 0)
        return FF;

    if (strcmp(s, library[FFDRPG].name) == 0)
        return FF;

    if (strcmp(s, library[LOGIC_0].name) == 0)
        return LOGIC0;

    if (strcmp(s, library[LOGIC_1].name) == 0)
        return LOGIC1;

    printf("(get_descr_type) Unresolved reference %s, quitting...\n", s);
    exit(1);
}

void add_assoc(int nd, char *name)
/* int	nd;	numero di descrittore */
/* char	*name;	nome del componente */
{
    int pos;

#if TRACE == 1
    printf("[add_assoc]\n");
#endif

    /* calcola la posizione in cui mettere il nuovo elemento */
    pos = Hfun(name, MAXASSOC);

    if (list_of_assoc[pos].name == NULL) {
        /* non c'e` collisione */
        list_of_assoc[pos].name = strdup(name);
        list_of_assoc[pos].n_descr = nd;
        list_of_assoc[pos].list = NULL;
        list_of_assoc[pos].n_el = 0;
    } else {
        /* c'e` collisione */

        if (list_of_assoc[pos].n_el == 0) {
            /* prima collisione */
            list_of_assoc[pos].list =
                malloc(sizeof(LASSOC) * (list_of_assoc[pos].n_el + 1));
            if (list_of_assoc[pos].list == NULL) {
                printf("(add_assoc) Memory error, quitting...\n");
                exit(1);
            }
            list_of_assoc[pos].list[0].name = strdup(name);
            list_of_assoc[pos].list[0].n_descr = nd;
            list_of_assoc[pos].n_el++;
        } else {
            /* altra collisione */
            list_of_assoc[pos].list =
                realloc(list_of_assoc[pos].list,
                        sizeof(LASSOC) * (list_of_assoc[pos].n_el + 1));
            if (list_of_assoc[pos].list == NULL) {
                printf("(add_assoc) Memory error, quitting...\n");
                exit(1);
            }
            list_of_assoc[pos].list[list_of_assoc[pos].n_el].name =
                strdup(name);
            list_of_assoc[pos].list[list_of_assoc[pos].n_el].n_descr = nd;
            list_of_assoc[pos].n_el++;
        }
    }
}

int get_assoc(char *s)
{
    int i, pos;

#if TRACE == 1
    printf("[get_assoc]\n");
#endif
    pos = Hfun(s, MAXASSOC);

    if (list_of_assoc[pos].name == NULL)
        return (-1);

    if (strcmp(list_of_assoc[pos].name, s) == 0)
        return (list_of_assoc[pos].n_descr);
    else {
        for (i = 0; i < list_of_assoc[pos].n_el; i++)
            if (strcmp(list_of_assoc[pos].list[i].name, s) == 0)
                return (list_of_assoc[pos].list[i].n_descr);
    }

    return (-1);
}

int get_descr_pos(char *s)
{
    int i;

    i = get_assoc(s);

    if (i == -1)
        return (-1);

    if (reset_pos != -1 && i >= reset_pos) {
        if (preset_pos != -1 && i >= preset_pos)
            return (i - 2);
        else
            return (i - 1);
    }

    if (preset_pos != -1 && i >= preset_pos)
        return (i - 1);

    /*
       for( i = 0; i < n_descr; i++ )
       if( strcmp( s, descr[i].name ) == 0 )
       return( i );

       return( -1 );
     */

}

void add_net(s, fp)
char *s;                        /* nome della netlist */
int fp;                         /* primo pin della lista */
{
    /* aggiunge una net al vettore */

#if TRACE == 1
    printf("[add_net]\n");
#endif
    netlist[l_net].netname = strdup(s);
    netlist[l_net].n_info = 0;
    netlist[l_net].list = fp;

    l_net++;
}

void add_net_info(char *nn, char *in, char *pn)
/* char	*nn;	nome della net */
/* char	*in;	nome instanza del componente */
/* char	*pn;	nome del pin */
{
#if TRACE == 1
    printf("[add_net_info]\n");
#endif
    if (*in == '-') {           /* ho un PI o un PO */
        pinlist[n_pin].n_descr = get_assoc(pn);

        if (descr[pinlist[n_pin].n_descr].attr == PI)
            pinlist[n_pin].dir = OUT;
        else
            pinlist[n_pin].dir = IN;
    } else {                    /* ho un instanza di un componente */
        pinlist[n_pin].n_descr = get_assoc(in);
        pinlist[n_pin].dir = get_pin_type(pn);
    }

    /* salva il nome del pin */

    pinlist[n_pin].pin_name = strdup(pn);

    /* collego il pin con la net corrispondente */

    pinlist[n_pin].net_index = l_net - 1;

    /* identifico se e` il pin di reset */

    netlist[l_net - 1].n_info++;
    n_pin++;

    if (n_pin >= pinlist_size) {
        if (!create_silent)
            printf("(add_net_info) Stretching pinlist\n");
        pinlist_size *= 2;
        pinlist = realloc(pinlist, sizeof(PIN) * pinlist_size);
    }
}

void dump_pinlist()
{
    int i;

#if TRACE == 1
    printf("[dump_pinlist]\n");
#endif
    printf("\nPIN found\n");
    for (i = 0; i < n_pin; i++) {
        printf("%d: %d, ", i, pinlist[i].n_descr);
        if (pinlist[i].dir == OUT)
            printf("OUT\n");
        else
            printf("IN\n");
    }
}

void dump_netlist()
{
    int i;
    FILE *fp;

#if TRACE == 1
    printf("[dump_netlist]\n");
#endif
    fp = fopen("netlist.log", "w");

    fprintf(fp, "\nNET identificate\n");
    for (i = 0; i < n_net; i++) {
        fprintf(fp, "%d: %s\t", i, netlist[i].netname);
        fprintf(fp, "n_info = %d\t", netlist[i].n_info);
        fprintf(fp, "s = %d\n", netlist[i].list);
    }

    fclose(fp);
}

void check_netlist(void)
{
    int i, j, k, found, err;

    if (!create_silent)
        printf("\nCecking netlist\n");
    if (!create_silent)
        printf("----------------\n");

    err = 0;

    for (i = 0; i < n_descr; i++) {
        for (j = 0; j < descr[i].fanout; j++) {
            found = 0;
            for (k = 0; k < descr[descr[i].to[j]].fanin; k++)
                if (descr[descr[i].to[j]].from[k] == i)
                    found = 1;
            if (!found && descr[i].attr != PO) {
                err++;
                printf
                    ("(check_netlist) WARNING checking from field: %d, %s \n",
                     i, descr[i].name);
            }
        }
    }

    if (!create_silent)
        printf("Fanout: check ");
    if (err == 0)
        if (!create_silent)
            printf("passed\n");
        else if (!create_silent)
            printf("failed, %d warning\n", err);

    err = 0;

    for (i = 0; i < n_descr; i++) {
        for (j = 0; j < descr[i].fanin; j++) {
            found = 0;
            for (k = 0; k < descr[descr[i].from[j]].fanout; k++)
                if (descr[descr[i].from[j]].to[k] == i)
                    found = 1;

            if (!found && descr[i].attr != PI) {
                err++;
                printf("(check_netlist) ERROR checking to field: %d, %s\n",
                       i, descr[i].name);
            }
        }
    }

    if (!create_silent)
        printf("Fanin check: ");
    if (err == 0)
        if (!create_silent)
            printf("passed\n");
        else if (!create_silent)
            printf("failed, %d errors found\n", err);
}

void check_level(void)
{
    int i, j, err;

    /* Faccio alcuni controlli sul livello */
    if (!create_silent)
        printf("\nChecking level\n");
    if (!create_silent)
        printf("---------------\n");

    err = 0;
    for (i = 0; i < n_descr; i++)
        for (j = 0; j < descr[i].fanin; j++)
            if ((descr[i].level <= descr[descr[i].from[j]].level
                 && descr[i].type != FF) && descr[i].attr != PI) {
                printf
                    ("(check_level) Gate %d (%s) has level %d, lower than gate %d (%s) level %d",
                     i, descr[i].name, descr[i].level, descr[i].from[j],
                     descr[descr[i].from[j]].name,
                     descr[descr[i].from[j]].level);

                /*MVMV */
                printf("Repaired\n");
                descr[i].level++;

                err++;
            }

    if (!create_silent)
        printf("%d errors found\n\n", err);
}

void destroy_reset(int reset_d, int preset_d)
{

    DESCRIPTOR *new_descr;
    int i, j, new_n_descr;
    char *temp;

#if TRACE == 1
    printf("[destroy_reset]\n");
#endif

    if ((reset_d == -1) && (preset_d == -1))
        return;

    if (!create_silent) {       /* [gs, 27-11-96] added line */
        printf("Removing reset and preset\n");
        printf("-------------------------\n");
        if (reset_d != -1)
            printf("reset = %s\n", descr[reset_d].name);
        else
            printf("reset not present\n");
        if (preset_d != -1)
            printf("preset = %s\n", descr[preset_d].name);
        else
            printf("preset not present\n");
        printf("\n");
    }
    /* [gs, 27-11-96] added line */
    new_n_descr = n_descr;      /* non ho ancora cancellato nulla */

    /* alloco la memoria per il nuovo descrittore */
    if (((reset_d != -1) && (preset_d != -1))
        && (reset_d != preset_d)) {
        if (!create_silent)
            printf("(destroy_reset) Lowering descr by 2\n");
        new_descr =
            (DESCRIPTOR *) malloc(sizeof(DESCRIPTOR) * (n_descr - 2));
    } else {
        if (!create_silent)
            printf("(destroy_reset) Lowering descr by 1\n");
        new_descr =
            (DESCRIPTOR *) malloc(sizeof(DESCRIPTOR) * (n_descr - 1));
    }

    if (new_descr == NULL) {
        printf("(destroy_reset) Memory error, quitting...\n");
        exit(1);
    }

    j = 0;
    for (i = 0; i < n_descr; i++)
        if ((i != reset_d) && (i != preset_d)) {        /* salta il descrittore del reset */
            new_descr[j].attr = descr[i].attr;
            new_descr[j].type = descr[i].type;
/*			new_descr[j].espl = descr[i].espl;*/
            new_descr[j].fanin = descr[i].fanin;
            new_descr[j].fanout = descr[i].fanout;
            new_descr[j].level = descr[i].level;
            new_descr[j].to = descr[i].to;
            new_descr[j].from = descr[i].from;
            new_descr[j].name = descr[i].name;
            new_descr[j].to_name = descr[i].to_name;
            new_descr[j].ff_type = descr[i].ff_type;
            new_descr[j].gate_id = descr[i].gate_id;
            j++;
        } else if (!create_silent)
            printf("(destroy_reset) Skipping %d, %s\n", i, descr[i].name);

    /* setto il corretto numero di descrittori e PI */

    if (((reset_d != -1) && (preset_d != -1))
        && (reset_d != preset_d)) {
        if (!create_silent)
            printf("(destroy_reset) Removed 2 PI\n");
        new_n_descr -= 2;
    } else {
        if (reset_d == preset_d)
            preset_d = -1;      /* cancello solo uno dei due */

        if (reset_d != -1)
            if (!create_silent)
                printf("(destroy_reset) Removed 1 PI\n");

        if (preset_d != -1)
            if (!create_silent)
                printf("(destroy_reset) Removed 1 PI");
        new_n_descr -= 1;
    }

    if (!create_silent)
        printf("(destroy_reset) Resizing FF fanin\n");

    for (i = 0; i < n_descr; i++)
        if (descr[i].type == FF)
            descr[i].fanin = 2;

    for (i = 0; i < new_n_descr; i++)
        if (new_descr[i].type == FF)
            new_descr[i].fanin = 2;     /* fissa a 2 il fanin dei FF */

    /* se c'e RESET riduci di uno */
    if (reset_d != -1) {
        n_sub_descr++;
        reset_pos = reset_d;
        if (preset_d != -1)
            preset_pos = preset_d;

        if (!create_silent)
            printf("(destroy_reset) Adjusting for reset...\n");

        for (i = 0; i < new_n_descr; i++) {
            for (j = 0; j < new_descr[i].fanin; j++)
                if (new_descr[i].from[j] >= reset_d)
                    if (new_descr[i].from[j] >= preset_d && preset_d != -1)
                        new_descr[i].from[j] -= 2;
                    else
                        new_descr[i].from[j]--;

            for (j = 0; j < new_descr[i].fanout; j++)
                if (new_descr[i].to[j] >= reset_d)
                    if (new_descr[i].to[j] >= preset_d && preset_d != -1)
                        new_descr[i].to[j] -= 2;
                    else
                        new_descr[i].to[j]--;
        }
    }

    /* se c'e PRESET riduci di uno */
    if (preset_d != -1 && reset_d == -1) {
        n_sub_descr++;
        preset_pos = preset_d;
        if (!create_silent)
            printf("(destroy_reset) Adjusting for preset...\n");

        for (i = 0; i < new_n_descr; i++) {
            for (j = 0; j < new_descr[i].fanin; j++)
                if (new_descr[i].from[j] >= preset_d)
                    new_descr[i].from[j]--;
            for (j = 0; j < new_descr[i].fanout; j++)
                if (new_descr[i].from[j] >= preset_d)
                    new_descr[i].to[j]--;
        }
    }

    /* libero la memoria occupata dal vecchio descr */

    free(descr);

    descr = new_descr;
    n_descr = new_n_descr;

}

/*
int	find_clock_descr( void )
{
	int	i, j;
	int	found;	

	found = -1;

	for( i = 0; i < n_net; i++ )
		for( j = 0; j < netlist[i].n_info; j++ )
			if( !strcmp( pinlist[j+netlist[i].list].pin_name, pin_name_ffd[1] )  )
			{
				found = i;
				break;
			}

	if( found == -1 )
		return( -1 );

	for( i = 0; i < n_descr; i++ )
		if( descr[i].attr == PI )
			if( !strcmp( netlist[found].netname, descr[i].name ) )
				return( i );

	return( -1 );
}
*/

int find_clock_descr(void)
{
    int i;
    int found;

    found = -1;

    if (!create_silent)
        printf("(find_clock_descr) Looking for clock PI...\n");

    for (i = 0; i < n_pin; i++) {
        if (descr[pinlist[i].n_descr].type == FF)
            if (!strcmp(pinlist[i].pin_name, pin_name_ffd[1])) {
                /*
                   printf( "%s, %s\n", pinlist[i].pin_name, pin_name_ffd[1] );
                 */

                found = i;
                break;
            }
    }

    if (found == -1)
        return (-1);

    if (descr[pinlist[found].n_descr].type != FF) {
        printf("(find_clock_descr) ERROR: clock gating\n");
        return (-1);
    }

    for (i = 0; i < n_descr; i++)
        if (descr[i].attr == PI)
            if (!strcmp
                (netlist[pinlist[found].net_index].netname,
                 descr[i].name)) {
                if (!create_silent)
                    printf("(find_clock_descr) found descriptor %d, %s\n",
                           i, descr[i].name);

                return (i);
            }

    printf("(find_clock_descr) ERROR: clock is not a PI, quitting...\n");
    exit(1);
}

int find_reset_descr(void)
{
    int i;
    int found;

    found = -1;

    /* cerco un pin che si chiami come pin_name_ffr[2] */

    ResetDescrName = NULL;

    for (i = 0; i < n_pin; i++)
        if (!strcmp(pinlist[i].pin_name, pin_name_ffr[2])
            && descr[pinlist[i].n_descr].type == FF) {
            found = i;
            break;
        }

    if (found == -1)
        return (-1);

    if (descr[pinlist[found].n_descr].type != FF)
        return (-1);

    /* controllo che il reset sia un PI */

    for (i = 0; i < n_descr; i++)
        if (descr[i].attr == PI)
            if (!strcmp
                (netlist[pinlist[found].net_index].netname,
                 descr[i].name)) {
                ResetDescrName = strdup(descr[i].name);
                return (i);
            }

    return (-1);
}

int find_preset_descr(void)
{
    int i;
    int found;

    found = -1;

    /* cerco un pin che si chiami come pin_name_ffp[2] */

    PresetDescrName = NULL;

    for (i = 0; i < n_pin; i++)
        if (!strcmp(pinlist[i].pin_name, pin_name_ffp[2])
            && descr[pinlist[i].n_descr].type == FF) {
            found = i;
            break;
        }

    if (found == -1)
        return (-1);

    if (descr[pinlist[found].n_descr].type != FF)
        return (-1);

    /* controllo che il reset sia un PI */

    for (i = 0; i < n_descr; i++)
        if (descr[i].attr == PI)
            if (!strcmp
                (netlist[pinlist[found].net_index].netname,
                 descr[i].name)) {
                PresetDescrName = strdup(descr[i].name);
                return (i);
            }

    return (-1);
}

/*
int	find_preset_descr( void )
{
	int	i, j;
	int	found;	

	found = -1;

	for( i = 0; i < n_net; i++ )
		for( j = 0; j < netlist[i].n_info; j++ )
			if( !strcmp( pinlist[j+netlist[i].list].pin_name, pin_name_ffp[2] )  )
			{
				found = i;
				break;
			}

	if( found == -1 )
		return( -1 );

	for( i = 0; i < n_descr; i++ )
		if( descr[i].attr == PI )
			if( !strcmp( netlist[found].netname, descr[i].name ) )
				return( i );

	return( -1 );
}
*/

void check_ff_fanin(void)
{
    int i, err;

    if (!create_silent)
        printf("\nChecking ff fanin\n");
    if (!create_silent)
        printf("-----------------\n");
    err = 0;
    for (i = 0; i < n_descr; i++) {
        if (descr[i].type == FF) {
            if ((descr[i].fanin < 2) || (descr[i].fanin > 4)) {
                err++;
                printf("ERROR on %d, %s, fanin = %d\n", i, descr[i].name,
                       descr[i].fanin);
            }
        }
    }
    if (!create_silent)
        printf("%d error found\n\n", err);

    if (err != 0) {
        printf("Exiting...\n");
        exit(1);
    }
}

void find_clock_tree(void)
{
    int i;

    for (i = 0; i < n_descr; i++)
        if ((descr[i].attr == INTERNAL) && (descr[i].type == FF)) {
            ClockDescr = descr[i].from[1];

            return;
        }
}

void create_netlist(int mode)
{
    int i, j, out_el, k;
    int fanout;
    int *dummy_p;
    int dummy_l;

    int clock_descr;
    int reset_descr;
    int preset_descr;

    int max_len;
    int n_elem;

    int p1, p2, p3, p4;
    int pin_pos;

    extern int get_pin_pos();   //i added the int

    /*
       init_library();
     */

#if TRACE == 1
    printf("[create_netlist]\n");
#endif
    if (!mode)
        printf("Building netlist...\n");

    /*
       if( VERBOSE )
       dump_netlist();
     *//* creo il file netlist.log */

    /*
       if( VERBOSE )
       dump_descr();
     */

    for (i = 0; i < n_net; i++) {       /* cerca il nodo di uscita */
        out_el = -1;            /* descrittore dell'elemeto di uscita */
        fanout = netlist[i].n_info - 1;

        for (j = 0; j < netlist[i].n_info; j++)
            if (pinlist[j + netlist[i].list].dir == OUT)
                out_el = pinlist[j + netlist[i].list].n_descr;

        if (out_el == -1) {
            printf("ERROR 7: Output descriptor not found, net %d\n", i);
            exit(1);
        }

        /* alloca la memoria per il descrittore corrente */
        if (fanout != 0)
            descr[out_el].to_name = strdup(netlist[i].netname);

        descr[out_el].fanout = fanout;

        descr[out_el].to = malloc(sizeof(int) * fanout);
        if (descr[out_el].to == NULL) {
            printf("(create_netlist) Memory error, quitting...\n");
            exit(1);
        }

        /* scrive il campo to della porta di output */
        k = 0;
        for (j = 0; j < netlist[i].n_info; j++)
            if (pinlist[j + netlist[i].list].dir == IN)
                descr[out_el].to[k++] =
                    pinlist[j + netlist[i].list].n_descr;

        /* Aggiunge la porta di uscita nel campo from di ogni porta
           connessa.
           Questa parte puo` dare dei problemi nel caso che il fanin
           sia maggiore di MAX_FANIN */
        for (j = 0; j < netlist[i].n_info; j++)
            if (pinlist[j + netlist[i].list].dir == IN) {
                /* identifica il descrittore a cui modificare 
                   il fanin */
                k = pinlist[j + netlist[i].list].n_descr;

                /* identifica la posizione in cui scrivere
                   il nuovo elemento in from, sulla base del
                   nome del pin a cui ci si connette:
                   l'ordine in from corrisponde all'ordine
                   dei pin nella libreria */
                pin_pos =
                    get_pin_pos(pinlist[j + netlist[i].list].pin_name);

                if (descr[k].attr != PO && pin_pos == -1) {
                    printf("(create_netlist) pin name not known\n");
                    printf("descr %d, name=%s\n", k,
                           pinlist[j + netlist[i].list].pin_name);
                    exit(1);
                }

                if (descr[k].fanin >= MAX_FANIN) {
                    printf("(create_netlist) Fanin over MAX_FANIN\n");
                    exit(1);
                }

                if (descr[k].attr == PO)
                    descr[k].from[descr[k].fanin++] = out_el;
                else {
                    descr[k].from[pin_pos] = out_el;
                    descr[k].fanin++;
                }

                if (descr[k].fanin >= MAX_FANIN) {
                    printf("(create_netlist) Fanin too high\n");
                    return;
                }
            }
    }

    /* fisso la dimensione corretta del descrittore */
    descr = realloc(descr, n_descr * sizeof(DESCRIPTOR));

    /* fisso la dimensione corretta del campo descr[i].from */
    for (i = 0; i < n_descr; i++)
        if (descr[i].fanin < MAX_FANIN) {
            dummy_l = descr[i].fanin;
            dummy_p = realloc(descr[i].from, dummy_l * sizeof(int));
            descr[i].from = dummy_p;
        }

    /* calcolo il numero di PI, PO e FF */
    n_pi = n_po = 0;

    clock_descr = find_clock_descr();
    reset_descr = find_reset_descr();
    preset_descr = find_preset_descr();

    if (VERBOSE) {
        if (reset_descr != -1)
            if (!create_silent)
                printf("(create_netlist) Circuit with RESET: %s\n",
                       descr[reset_descr].name);
        if (preset_descr != -1)
            if (!create_silent)
                printf("(create_netlist) Circuit with PRESET: %s\n",
                       descr[preset_descr].name);
    }

    if (clock_descr == -1)
        if (!create_silent)
            printf
                ("(create_netlist) Clock not found: combinatorial network\n");

    /* controlla il fanin dei FF */
    check_ff_fanin();

    for (i = 0; i < n_descr; i++) {
        switch (descr[i].attr) {
        case PI:
            n_pi++;
            break;
        case PO:
            n_po++;
            break;
        case INTERNAL:
            descr[i].ff_type = FFD;
            if (descr[i].type == FF) {
                n_ff++;

                /* controlla che il fanin dei FF
                   sia 2, 3, 4 */
                if (descr[i].fanin < 2 || descr[i].fanin > 4) {
                    printf
                        ("(create_netlist) ERROR 1: %d, %s fanin FF lower than 2 or bigger than 4\n",
                         i, descr[i].name);
                    exit(1);
                }

                /*
                   Mette a posto l'ordine di clk,
                   dato:
                   from[0] = dato
                   from[1] = clock 
                   from[2] = reset o preset (se esiste)
                   from[3] = preset (se esiste)

                   p1 = descrittore clock
                   p2 = descrittore reset
                   p3 = descrittore preset
                   p4 = descrittore dato
                 */

                p1 = p2 = p3 = p4 = -1;
                for (j = 0; j < descr[i].fanin; j++) {
                    if (descr[i].from[j] == clock_descr)
                        p1 = clock_descr;
                    else if (descr[i].from[j] == reset_descr)
                        p2 = reset_descr;
                    else if (descr[i].from[j] == preset_descr)
                        p3 = preset_descr;
                    else
                        p4 = descr[i].from[j];

                }

/*
	printf( "FF %s, descr = %d, fanin = %d, p1 = %d, p2 = %d, p3 = %d, p4 = %d\n", descr[i].name, i, descr[i].fanin, p1, p2, p3, p4 );
*/

                /* fisso il campo from */
                descr[i].from[0] = p4;

                /*
                   printf( "(create_netlist) FF %s, clock = %s\n", descr[i].name, descr[clock_descr].name );
                 */

                /*
                   printf( "*****************\n" );
                   printf( "clock_descr = %d\n", clock_descr );
                 */
                descr[i].from[1] = clock_descr;

                /* se il fanin e` 3, registra il
                   reset o il preset */
                if (descr[i].fanin == 3) {
                    if (p2 == -1 && p3 == -1) {
                        printf
                            ("(create_netlist) ERROR: %d, %s, fanin = 3, but RESET nor PRESET are present\n",
                             i, descr[i].name);
                        dump_descr();
                        exit(1);
                    }
                    descr[i].from[2] = (p2 == -1 ? p3 : p2);
                }

                /* se il fanin e` 4, no problem */
                if (descr[i].fanin == 4) {
                    descr[i].from[2] = p2;
                    descr[i].from[4] = p3;
                }

                /* FF con reset */
                if ((p2 != -1) && (p3 == -1))
                    descr[i].ff_type = FFDR;

                /* FF con preset */
                if ((p2 == -1) && (p3 != -1))
                    descr[i].ff_type = FFDP;

                /* FF con reset e preset */
                if ((p2 != -1) && (p3 != -1))
                    descr[i].ff_type = FFDRP;
            }
            break;
        }
    }

    if (n_ff != 0 && clock_descr == -1) {
        printf("(create_netlist) Clock not found\n");

        dump_descr();

        exit(1);
    }

        /*-------------------------------*/
    /* controlla che il CK sia un PI */
    for (i = 0; i < n_descr; i++)
        if (descr[i].type == FF)
            if ((descr[descr[i].from[1]].attr != PI)
                || (descr[i].from[1] != clock_descr)) {
                printf("(create_netlist) ERROR 2: gated clock .\n");
                exit(1);
            }

        /*--------------------------------------------------------*/
    /* controlla che i segnali di reset e preset siano dei PI */
    for (i = 0; i < n_descr; i++)
        if (descr[i].type == FF)
            if (descr[i].fanin > 2) {
                if (descr[descr[i].from[2]].attr != PI) {
                    printf
                        ("(create_netlist) ERROR 3: descr[%d].from[2] isn't a PI\n",
                         i);
                    exit(1);
                }

                if (descr[i].fanin > 3)
                    if (descr[descr[i].from[3]].attr != PI) {
                        printf
                            ("(create_netlist) ERROR 4: descr[%d].from[3] isn't a PI\n",
                             i);
                        exit(1);
                    }
            }

        /*---------------------------------------------------------------*/
    /* controlla che il segnale di reset e preset siano RESET/PRESET */
    for (i = 0; i < n_descr; i++)
        if (descr[i].type == FF)
            if (descr[i].fanin > 2) {
                if ((descr[i].from[2] != reset_descr)
                    && (descr[i].from[2] != preset_descr)) {
                    printf
                        ("(create_netlist) ERROR 5: reset/preset descriptor not correct\n");
                    exit(1);
                }

                if (descr[i].fanin > 3)
                    if ((descr[i].from[3] != reset_descr)
                        && (descr[i].from[3] != preset_descr)) {
                        printf
                            ("(create_netlist) ERROR 6: reset/preset descriptor not correct\n");
                        exit(1);
                    }
            }

        /*-----------------------------------------------------------
		ATTENZIONE: a questo punto descr contiene anche 
		il segnale di reset e/o preset, la chiamata a questa 
		procedura elima il segale di reset
	*/

    destroy_reset(reset_descr, preset_descr);

    if (VERBOSE) {
        if (!create_silent)
            printf("(create_netlist) Create found:\n");
        if (!create_silent)
            printf("%d PI\n%d PO\n%d FF\n", n_pi, n_po, n_ff);
    }

    /* alloca la memoria per i vettori contenenti PI, PO e FF */

    if (reset_descr != -1) {
        if (VERBOSE)
            if (!create_silent)
                printf
                    ("(create_netlist) Lowering n_pi, there is a reset to skip over\n");

        n_pi--;
    }
    if (preset_descr != -1) {
        if (VERBOSE)
            if (!create_silent)
                printf
                    ("(create_netlist) Lowering n_pi, there is a preset to skip over\n");
        n_pi--;
    }

    pi_array = (int *) malloc(sizeof(int) * n_pi);
    if (pi_array == NULL) {
        printf("(create_netlist) Memory error, quitting...\n");
        exit(1);
    }

    ppi_array = (int *) malloc(sizeof(int) * n_ff);
    if (ppi_array == NULL) {
        printf("(create_netlist) Memory error, quitting...\n");
        exit(1);
    }

    /* scrive la lista dei descrittori dei PI */
    j = 0;
    k = 0;
    for (i = 0; i < n_descr; i++)
        switch (descr[i].attr) {
        case PI:
            pi_array[j++] = i;
            break;
        case INTERNAL:
            if (descr[i].type == FF)
                ppi_array[k++] = i;
            break;
        }

    max_len = 0;
    n_elem = 0;
    for (i = 0; i < MAXASSOC; i++)
        if (list_of_assoc[i].name != 0) {
            if (list_of_assoc[i].n_el > max_len)
                max_len = list_of_assoc[i].n_el;
            n_elem += list_of_assoc[i].n_el;
        }

    /* collego descr alla libreria */

    for (i = 0; i < n_descr; i++)
        switch (descr[i].type) {
        case LOGIC0:
            descr[i].gate_id = LOGIC_0;
            descr[i].level = 0;
            break;
        case LOGIC1:
            descr[i].gate_id = LOGIC_1;
            descr[i].level = 0;
            break;
        case NAND:
            if (descr[i].fanin == 2)
                descr[i].gate_id = NAND2;
            if (descr[i].fanin == 3)
                descr[i].gate_id = NAND3;
            if (descr[i].fanin == 4)
                descr[i].gate_id = NAND4;
            if (descr[i].fanin == 5)
                descr[i].gate_id = NAND5;
            break;
        case AND:
            if (descr[i].fanin == 2)
                descr[i].gate_id = AND2;
            if (descr[i].fanin == 3)
                descr[i].gate_id = AND3;
            if (descr[i].fanin == 4)
                descr[i].gate_id = AND4;
            if (descr[i].fanin == 5)
                descr[i].gate_id = AND5;
            break;
        case OR:
            if (descr[i].fanin == 2)
                descr[i].gate_id = OR2;
            if (descr[i].fanin == 3)
                descr[i].gate_id = OR3;
            if (descr[i].fanin == 4)
                descr[i].gate_id = OR4;
            if (descr[i].fanin == 5)
                descr[i].gate_id = OR5;
            break;
        case NOR:
            if (descr[i].fanin == 2)
                descr[i].gate_id = NOR2;
            if (descr[i].fanin == 3)
                descr[i].gate_id = NOR3;
            if (descr[i].fanin == 4)
                descr[i].gate_id = NOR4;
            if (descr[i].fanin == 5)
                descr[i].gate_id = NOR5;
            break;
        case EXOR:
            if (descr[i].fanin == 2)
                descr[i].gate_id = XOR2;
            if (descr[i].fanin == 3)
                descr[i].gate_id = XOR3;
            if (descr[i].fanin == 4)
                descr[i].gate_id = XOR4;
            if (descr[i].fanin == 5)
                descr[i].gate_id = XOR5;
            break;
        case EXNOR:
            if (descr[i].fanin == 2)
                descr[i].gate_id = XNOR2;
            if (descr[i].fanin == 3)
                descr[i].gate_id = XNOR3;
            if (descr[i].fanin == 4)
                descr[i].gate_id = XNOR4;
            if (descr[i].fanin == 5)
                descr[i].gate_id = XNOR5;
            break;
        case NOT:
            descr[i].gate_id = INV;
            break;
        case BUF:
            descr[i].gate_id = BUFF;
            break;
        case FF:
            if (descr[i].ff_type == FFD)
                descr[i].gate_id = FFDG;
            if (descr[i].ff_type == FFDR)
                descr[i].gate_id = FFDRG;
            if (descr[i].ff_type == FFDP)
                descr[i].gate_id = FFDPG;
            if (descr[i].ff_type == FFDRP)
                descr[i].gate_id = FFDRPG;

                                /******/
            if (descr[i].gate_id == 0) {
                printf
                    ("(create_netlist) Fatal error in library binding, exit.\n");
                exit(1);
            }
            break;
        default:
            printf("(create_netlist) Unknown descriptor: %s, %d\n",
                   descr[i].name, descr[i].type);
            exit(1);
        }

    if (!mode) {
        if (!create_silent)
            printf("HASH chain max size = %d\n", max_len);
        if (max_len != 0.0)
            if (!create_silent)
                printf("HASH average size =%f\n",
                       (double) n_elem / max_len);
            else if (!create_silent)
                printf("HASH average size = 0\n");

        /* calcola il livello */
        if (!create_silent)
            printf("Start computing level, ");
    }

    if (!create_silent)
        printf("\n%d descriptor found\n", n_descr);

#if STRONG_DEBUG
    check_netlist();
#endif

    compute_level(n_pi, n_ff, pi_array, ppi_array);

    /* controllo che il livello sia corretto */
#if STRONG_DEBUG
    check_level();
#endif

    if (!mode)
        if (!create_silent)
            printf("end computing level.\n");

    /* determina il descrittore del clock, questa deve essere l'ultima
       operazione fatta perche` nei passi precedenti posso aver modificato
       il descrittore cancellando i segnali di reset/preset */

    find_clock_tree();

    /*if( VERBOSE ) */
    /*dump_descr(); */
}

/* struttura locale, la uso per il calcolo del livello */
struct fifo {
    int desc;
    struct fifo *link;
};
struct fifo *mytail, myhead;    /* NB: myhead e` un nodo dummy */
int *visited;

/**
 ** routines per gestione coda FIFO
 **/
int insert(int desc);
int extract(int *pdesc);

int insert(int desc)
{
    struct fifo *p1;

#if TRACE == 1
    printf("[insert]\n");
#endif
    if (NULL == (p1 = (struct fifo *) malloc(sizeof(struct fifo))))
        return 1;
    p1->desc = desc;
    p1->link = NULL;
    mytail = (mytail->link = p1);
    return 0;
}                               /* end of insert() */

int extract(int *pdesc)
{
    struct fifo *p1;

#if TRACE == 1
    printf("[extract]\n");
#endif
    p1 = myhead.link;
    if (!p1)
        return 1;
    *pdesc = p1->desc;
    myhead.link = p1->link;
    if (mytail == p1)
        mytail = &myhead;
    free(p1);
    return 0;
}

/* calcola il livello di ogni descrittore a partire da PI e FF */
int compute_level(int n_pi, int n_ff, int *pi_array, int *ppi_array)
{
    /* Uso una lista FIFO di #descr */
    int this_desc, next_level, next_desc;
    int i;
    DESCRIPTOR *p, *p1;

#if TRACE == 1
    printf("[compute_level]\n");
#endif
    /* inizializza la lista */
    myhead.link = NULL;
    mytail = &myhead;
    visited = (int *) malloc(n_descr * sizeof(int));
    if (!visited)
        return (1);

    /* inizializzo tutti i descrittori a livello nullo, che poi incrementero` */
    for (i = 0, p = descr; i < n_descr; i++, p++) {
        p->level = 0L;
        visited[i] = 0;
    }
    max_level = 0;

    /* PI & FF & LOGIC_1 & LOGIC_0: gia` giustamente a livello 0 */

    for (i = 0; i < n_descr; i++)
        if (descr[i].attr == INTERNAL
            && (descr[i].type == LOGIC0 || descr[i].type == LOGIC1))
            insert(i);

    for (i = 0; i < n_pi; ++i) {
        /*printf("PI-%d\n",pi_array[i]); */
        if (insert(pi_array[i]))
            return 1;
    }
    for (i = 0; i < n_ff; ++i) {
        /*printf("FF-%d\n",ppi_array[i]); */
        if (insert(ppi_array[i]))
            return 1;
    }

    /* vai a esplorare il grafo calcolando il MAX livello, escludendo i FF */
    while (!extract(&this_desc)) {
        /*printf("EXT-%d\n",this_desc); */
        p = &descr[this_desc];
        next_level = p->level + 1;
        /* scandisci i gate a livello successivo */
        for (i = 0; i < p->fanout; i++) {
            next_desc = p->to[i];
            p1 = &descr[next_desc];
            visited[next_desc]++;
            if ((p1->level < next_level) && (p1->type != FF)) {
                if (visited[next_desc] < p1->fanin) {
                    /*printf("Delaying %s\n", p1->name); */
                    /*printf("DEL-%d\n",next_desc) ; */
                } else if (visited[next_desc] == p1->fanin) {
                    /*printf("Setting %s to %d\n", p1->name, next_level); */
                    /*printf("INS-%d\n",next_desc); */
                    p1->level = next_level;
                    if (next_level > max_level)
                        max_level = next_level;
                    if (insert(next_desc))
                        return 1;
                } else {
                    printf("ERROR: Asynchronous loop detected\n");
                    printf
                        ("\t(net %s from gate %s to gate %s is a feedback net)\n",
                         p->to_name, p->name, p1->name);

#if VERBOSE
                    dump_descr();
#endif

                    exit(1);
                }
            }
        }
    }
    free(visited);
    visited = NULL;
    return 0;
}                               /* end of compute_level() */

int Hfun(char *s, int tablen)
{
    char *p;
    unsigned int h = 0, g;
    int i;

#if TRACE == 1
    printf("[Hfun]\n");
#endif
    for (p = s, i = strlen(s); i; p++, i--) {
        h <<= 4;
        h += *p;
        if ((g = (h & 0xF0000000L)))
            h ^= g ^ (g >> 24);
    }
    return h % tablen;
}
