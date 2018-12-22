#include <stdio.h>
#include <signal.h>
#include "defines.h"
#include <stdlib.h>

/* MVMV */
#include	"create.h"

#ifdef DEBUG
#undef DEBUG
#endif

/** export global variables ***/
char *myname;                   /* name of this executable */
char *cellname;                 /* name of cell to extract */
char *viewname;                 /* name of principal view to evaluate */
char *libraryname;
char **module_dirs;             /* vector of module dirs from MODULEPATH */
char *geometry_view;            /* name of view to extract geometry info */
int n_module_dirs;
int cell_externs;
int debug;                      /* debug level: 0=silent, 1=verbose, 2=more... */
int currentdir;
int get_geometry, unscale;
FILE *edifp;

int create_silent = 1;

extern char *optarg;            /* silently set by getopt() */
extern int optind, opterr;      /* silently set by getopt() */

/*
	se mod = 0 stampo output
	se mod = 1 non stampo nulla
*/

char *mycalloc(int n, int size);
char *new_string(char *c);
void terminate(int err);
void create_netlist(int mode);

int create(char *argv)
{
    int mode = 1;

    void get_args(), signal_setup();
    extern void parse_init(), parse_edif(), simplify();
    extern void findcells();

    /* MVMV */
    extern void init_create_memory();
    extern void dump_netlist();
    extern void dump_pinlist();

    if (!mode)
        printf("Leggo la descrizione del circuito...\n");

    init_create_memory();

    get_args(2, argv);
    parse_init();
    parse_edif();
    findcells();
    fclose(edifp);

    create_netlist(mode);

    /* MVMV */
    if (PEDANTIC == 1) {
        /*      dump_netlist(); */
        /*      dump_pinlist(); */
    }

    terminate(0);

    return (0);
}

static void usage()
{
    fprintf(stderr, "Usage: %s %s%s\n", myname,
            "[-v | -d debuglevel][-c][-e][(-g | -G viewname)[-u]][-C cellname]",
            "[-V viewname][-L libraryname][infile]");
    exit(1);
}

void get_args(argc, argv)
int argc;
char *argv;
{
    int i, n, l, opt;
    char *str_hash(), *arg, *filename = NULL;
    char *modpath, *c;

    /*** reading command line options ***/

    myname = argv;

    /**** still arguments left ... */
    edifp = fopen(argv, "r");
    if (!edifp) {
        fprintf(stderr, "Cannot open input file '%s'!\n", argv);
        exit(1);
    } else if (debug)
        fprintf(stderr, "Opened '%s' for reading.\n", argv);

    if (!currentdir) {
        /* use MODULEPATH environment variable */
        char buf[512];

        modpath = getenv("MODULEPATH");
        if (!modpath)
            modpath = ".";

        /* split MODULEPATH into a vector of dirnames */
        n = 1;
        for (c = modpath; *c; c++)
            if (*c == ':')
                n++;
        n_module_dirs = n;

        module_dirs = (char **) mycalloc(n + 1, sizeof(char *));
        module_dirs[0] = new_string(modpath);
        i = 1;
        for (c = module_dirs[0]; *c; c++) {
            if (*c == ':') {
                module_dirs[i++] = c + 1;
                *c = '\0';
            }
        }
        for (i = 0; i < n_module_dirs; i++) {
            if (module_dirs[i][0] == '~') {
                sprintf(buf, "%s%s", getenv("HOME"), module_dirs[i] + 1);
                module_dirs[i] = new_string(buf);
            }
        }
    }

    if (debug > 1 && module_dirs) {
        fprintf(stderr, "MODULEPATH =");
        for (i = 0; i < n_module_dirs; i++) {
            fprintf(stderr, " '%s'", module_dirs[i]);
        }
        fprintf(stderr, "\n");
    }
}

void terminate(err)             /* terminate program */
int err;
{
    int i, j;

    extern void create_hash_stats(), mem_stats();

    if (debug > 1) {
        create_hash_stats();
        mem_stats();
        fprintf(stderr, "Return value of the program is %d.\n", err);
    }

    /* libera la memoria allocata durante la create */

    free(pinlist);

    for (i = 0; i < n_net; i++)
        free(netlist[i].netname);
    free(netlist);

    /*
       for( i = 0; i < MAXASSOC; i++ )
       if( list_of_assoc[i].name != 0 )
       {
       for( j = 0; j < list_of_assoc[i].n_el; j++ )
       free( list_of_assoc[i].list[j].name );
       if( list_of_assoc[i].n_el != 0 )
       free( list_of_assoc[i].list );
       free( list_of_assoc[i].name );
       }

       free( list_of_assoc );
     */
}
