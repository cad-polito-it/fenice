/*----------------------------------------------------------------------------
	
	celltable.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "keywords.h"
#include "types.h"
#include <sys/stat.h>
#include <ctype.h>
#include <math.h>

/* MVMV */
#include	"create.h"

extern int verbose;
extern int ncells;
extern int debug;
extern PARSETREE *parsetree;
extern int keywordlevelused;
extern int cell_externs;
extern char *keywords[];
extern char *cellname, *libraryname, *viewname;
extern int currentdir;
extern int get_geometry, unscale;
extern char *geometry_view;
extern int n_module_dirs;
extern char **module_dirs;

extern PARSETREE *get_array_size();
int rootcell = -1;
PARSETREE *rootview;

static void choose_views();
static void write_cell_data();
static void write_comment();
static void write_interface();
static void write_port();
static void write_contents();
static void write_page_inst();
static void write_page_net();
static void write_instance();
char *get_defname();
static void write_net();
static void write_joined();
static void write_unsup();
static void write_portref();
static void write_portlist();
static void convert_cell();

static int cells_done;
static int base_x, base_y;      /* lower bound  x,y cell coordinates */

char *realnameof(PARSETREE * p);        //tree_ops.c
char *nameof(PARSETREE * p);    //tree_ops.c
void add_net(char *s, int fp);  //create_supp.c
int reopen_cell_output(char *cname, int has_contents);
DESCRIPTOR *descr;
int n_descr;

void findcells()
{                               /* scan all cells for requested conversion */
    PARSETREE *cell, *lib, *locate();
    void report();
    char *nameof(), *realnameof(), *libname, *cname;

        /****** catalogue all cells and libraries ******/
    for (lib = parsetree->data.tree; lib; lib = lib->next) {
        if (!
            (lib->type == Klibrary
             || (lib->type == Kexternal && cell_externs)))
            continue;

        libname = nameof(lib);
        DEBUG(3) "findcells(): Library '%s' seen\n", libname);
        if (libraryname && (libraryname != libname))
            continue;           /* not the requested lib */

        for (cell = lib->data.tree; cell; cell = cell->next) {

            if (cell->type == Kcell) {
                cname = nameof(cell);
                if (cellname && (cellname != cname))
                    continue;   /* not the requested cell */
                DEBUG(2) "findcells(): cell '%s' lib '%s' entered\n",
                    cname, libname);
                choose_views(cell);

            }
        }
    }

    DEBUG(1) "%d EDIF cell%s translated.\n", cells_done,
        (cells_done == 1) ? "" : "s");
}

static void choose_views(cell)
PARSETREE *cell;
{

    static int net_view_warning = 0;
    static int geo_view_warning = 0;

    PARSETREE *view, *schem_view, *net_view, *geo_view;
    char *vname, *viewtype;
    int multiple_schem, multiple_net, multiple_geo;

    schem_view = net_view = geo_view = NULL;
    multiple_schem = multiple_net = multiple_geo = 0;

    for (view = cell->data.tree; view; view = view->next) {
        if (view->type != Kview)
            continue;

        vname = nameof(view);

        viewtype = nameof(view->data.tree->next);
        DEBUG(3) "    view '%s' type '%s' seen\n", vname, viewtype);

        if ((viewtype == keywords[KNETLIST] || viewtype == keywords[KMASKLAYOUT] || viewtype == keywords[KSYMBOLIC] || viewtype == keywords[KSCHEMATIC]) && (!viewname || !strcmp(viewname, vname))) {  /* consider this view for netlist info */
            if (viewtype == keywords[KNETLIST]) {
                if (!net_view)
                    net_view = view;
                else
                    multiple_net = 1;
            } else {
                if (!schem_view)
                    schem_view = view;
                else
                    multiple_schem = 1;
            }
        } else if (get_geometry
                   && (viewtype == keywords[KMASKLAYOUT]
                       || viewtype == keywords[KSCHEMATIC]
                       || viewtype == keywords[KSYMBOLIC]
                   ) && (!geometry_view || !strcmp(geometry_view, vname)
                   )) {         /* consider this view for geometric info */
            if (viewtype != keywords[KSCHEMATIC]) {
                if (!geo_view)
                    geo_view = view;
                else
                    multiple_geo = 1;
            } else {
                if (!schem_view)
                    schem_view = view;
                else
                    multiple_schem = 1;
            }
        }
    }

    if (multiple_net || (!net_view && multiple_schem)) {
        if (!net_view)
            net_view = schem_view;
        viewname = nameof(net_view);
        if (!net_view_warning) {
            DEBUG(0)
                "Multiple views available for netlist, choosing '%s'!\n",
                viewname);
            DEBUG(0) "Use option '-V viewname' to select otherwise.\n");
            net_view_warning = 1;
        }
    }
    if (multiple_geo || (!geo_view && multiple_schem)) {
        if (!geo_view)
            geo_view = schem_view;
        viewname = nameof(geo_view);
        if (!geo_view_warning) {
            DEBUG(0)
                "Multiple views available for geometry, choosing '%s'!\n",
                viewname);
            DEBUG(0) "Use option '-G viewname' to select otherwise.\n");
            geo_view_warning = 1;
        }
    }

    if (!net_view && schem_view)
        net_view = schem_view;
    if (net_view || geo_view)
        convert_cell(cell, net_view, geo_view);
}

static void convert_cell(cell, net_view, geo_view)
PARSETREE *cell, *net_view, *geo_view;
{
    PARSETREE *t, *contents, *net_intface, *geo_intface;

    DEBUG(2) "Convert_cell: cell=%s\n", nameof(cell));

    contents = net_intface = geo_intface = NULL;
    if (net_view) {
        for (t = net_view->data.tree; t; t = t->next) {
            if (t->type == Kcontents) {
                contents = t;
            } else if (t->type == Kinterface) {
                net_intface = t;
            }
        }
    }

    if (geo_view) {
        for (t = geo_view->data.tree; t; t = t->next) {
            if (t->type == Kinterface)
                geo_intface = t;
        }
    }

    if (!net_intface && !geo_intface) {
        DEBUG(0) "EDIF error: Cell '%s', view '%s' has no interface!\n",
            nameof(cell), nameof(net_view ? net_view : geo_view));
        return;
    }

    if (!contents && !cell_externs)
        return;

    if (reopen_cell_output(realnameof(cell), (contents != NULL))) {
        write_cell_data(cell, geo_intface, net_intface, contents);
    }
}

int reopen_cell_output(cname, has_contents)
char *cname;
int has_contents;
{
    char filename[1024], *dir;
    int i, r, l;
    struct stat buf;

    if (!currentdir) {          /* if no contents, check presence in read-only module directories */
        /* if found there, suppress overwriting by returning 0 */
        if (!currentdir && !has_contents && n_module_dirs > 1) {
            for (i = 1; i < n_module_dirs; i++) {
                strcpy(filename, module_dirs[i]);
                if (filename[strlen(filename) - 1] != '/')
                    strcat(filename, "/");
                strcat(filename, cname);

                r = stat(filename, &buf);
                if (!r && S_ISDIR(buf.st_mode)) {
                    DEBUG(1) "Not writing cell '%s': is in MODULEPATH\n",
                        cname);
                    return (0); /* already there */
                }
            }
        }

        /* generate cell in first directory of modulepath */
        l = strlen(module_dirs[0]);
        sprintf(filename, "%s%s%s", module_dirs[0],
                (module_dirs[0][l - 1] == '/') ? "" : "/", cname);

        r = stat(filename, &buf);
        /*
           if (r)       r = mkdir( filename,
           (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH));

           if (r)
           {    DEBUG(0) "Couldn't find/create cell directory '%s'!\n", filename);
           return(0);
           }

         */

        l = strlen(filename);
        sprintf(filename + l, "/%s.%s", cname,
                has_contents ? "min" : "io");

    } else
        sprintf(filename, "%s.%s", cname, has_contents ? "min" : "io");

    return (1);
}

static void write_cell_data(cell, geo_intface, net_intface, contents)
PARSETREE *cell, *geo_intface, *net_intface, *contents;
{
    cells_done++;

    if (net_intface || geo_intface) {
        write_interface(net_intface, geo_intface);
    }

    if (contents) {
        write_contents(contents);
    }

    fflush(stdout);
}

static void write_comment(comment)
PARSETREE *comment;
{
}

static PARSETREE *scan4bb(parent)
PARSETREE *parent;
{
    PARSETREE *bb;

    for (bb = parent->data.tree; bb; bb = bb->next) {
        if (bb->type == KboundingBox)
            return (bb);
    }
    return (NULL);
}

static PARSETREE *find_bb(intface)
PARSETREE *intface;
{
    PARSETREE *t;
    PARSETREE *symbol = NULL;
    PARSETREE *bb = NULL;

    if (!intface)
        return (NULL);

    for (t = intface->data.tree; t; t = t->next) {
        if (t->type == KprotectionFrame) {
            bb = scan4bb(t);
            if (bb)
                break;
        } else if (t->type == Ksymbol) {
            symbol = t;
        }
    }

    if (!bb && symbol)
        bb = scan4bb(symbol);

    return (bb);
}

static void write_bb(bb)
PARSETREE *bb;
{
    int x1, x2, y1, y2, h;
    PARSETREE *pt1, *pt2;

    pt1 = bb->data.tree->data.tree;

    if (!pt1 || pt1->type != Kpt) {
        DEBUG(0) "Line %d: '(pt ..' expected!\n", bb->line);
        return;
    }
    pt2 = pt1->next;
    if (!pt2 || pt2->type != Kpt) {
        DEBUG(0) "Line %d: '(pt ..' expected!\n", bb->line);
        return;
    }

    x1 = pt1->data.tree->data.ival;
    y1 = pt1->data.tree->next->data.ival;
    x2 = pt2->data.tree->data.ival;
    y2 = pt2->data.tree->next->data.ival;

    if (x1 > x2)
        h = x1, x1 = x2, x2 = h;
    if (y1 > y2)
        h = y1, y1 = y2, y2 = h;

    base_x = x1;
    base_y = y1;
}

static void write_interface(net_intface, geo_intface)
PARSETREE *net_intface, *geo_intface;
{
    PARSETREE *t, *bb, *intface;

    bb = find_bb(net_intface);
    if (!bb)
        bb = find_bb(geo_intface);

    if (bb)
        write_bb(bb);

    intface = net_intface ? net_intface : geo_intface;

    /* riempie la struttura dati con le informazioni sulle porte */
    for (t = intface->data.tree; t; t = t->next) {
        if (t->type == Kport) {
            write_port(t);
        } else if (t->type == Kcomment)
            write_comment(t);
    }
}

/*----------------------------------------------------------------------------
	Scrive nel descrittore le porte di ingress e quelle di
	uscita
*/
static void write_port(port)
PARSETREE *port;
{
    PARSETREE *t, *dir;
    int i;

    extern void add_assoc();

    dir = NULL;
    for (t = port->data.tree; t; t = t->next)
        if (t->type == Kdirection)
            dir = t;

    if (dir) {
        if (dir->data.tree->data.token == keywords[KINPUT]) {
            descr[n_descr].attr = PI;
            descr[n_descr].fanin = 0;
            descr[n_descr].type = BUF;
        } else if (dir->data.tree->data.token == keywords[KOUTPUT]) {
            descr[n_descr].type = BUF;
            descr[n_descr].attr = PO;
            descr[n_descr].fanin = 0;
        }
        descr[n_descr].name = strdup(realnameof(port));

        /* MVMV 
           printf( "(write_port) Aggiungo %d, %s, ", n_descr, descr[n_descr].name );
           if( descr[n_descr].attr == PI )
           printf( "PI\n" );
           else
           printf( "PO\n" );
         */

        /* aggiunge l'associazione nome->numero descrittore */
        add_assoc(n_descr, descr[n_descr].name);

        n_descr++;
    }

    if (n_descr >= Descr_Def_Size) {
        if (!create_silent)
            printf("(write_port)Resize descr to %d\n", 2 * Descr_Def_Size);
        Descr_Def_Size *= 2;
        descr = realloc(descr, sizeof(DESCRIPTOR) * Descr_Def_Size);

        /* quando realloca deve creare lo spazio per i nuovi dati */
        for (i = +Descr_Def_Size / 2; i < Descr_Def_Size; i++) {
            descr[i].fanin = 0;
            descr[i].fanout = 0;
            descr[i].to = NULL;
            descr[i].from = malloc(sizeof(int) * MAX_FANIN);
        }
    }

}

static void write_contents(cont)
PARSETREE *cont;
{
    PARSETREE *t;

    for (t = cont->data.tree; t; t = t->next) {
        if (t->type == Kinstance)
            write_instance(t);
        else if (t->type == Kpage)
            write_page_inst(t);
        else if (t->type == Kcomment)
            write_comment(t);
    }

    /* PASSO1: calcola il numero di net nel circuito */

    n_net = 0;                  /* numero di net identificate */
    l_net = 0;                  /* indice dell'ultima net *///point of the last net

    for (t = cont->data.tree; t; t = t->next) {
        //printf("write_contents %s \n",nameof(t));             
        if (t->type == Knet) {
            n_net++;            /* aggiunge la net nuova net */
        } else {
            if (t->type == Kpage)
                write_page_net(t, 1);
        }
    }

    if (PEDANTIC) {
        printf("(write_contents) Identificate %d NET\n", n_net);
        printf("(write_contents) Alloco la memoria\n");
        printf("(write_contents) Alloco memoria per %d PIN\n",
               (NET_MEAN_FANOUT * n_net));
    }

    /* alloco la memoria per i descrittori di net */
    netlist = malloc(sizeof(NET) * n_net);
    if (netlist == NULL) {
        printf("(write_contents) Errore di memoria NET\n");
        return;
    }

    /* alloco la memoria per i descrittori di pin */
    pinlist = malloc(sizeof(PIN) * (NET_MEAN_FANOUT * n_net));
    n_pin = 0;                  /* nessun pin identificato */
    pinlist_size = n_net * NET_MEAN_FANOUT;
    if (pinlist == NULL) {
        printf("(write_contents) Errore di memoria PIN\n");
        return;
    }

    /* PASSO2: scrive la lista di PIN */
    for (t = cont->data.tree; t; t = t->next) {
        if (t->type == Knet)
            write_net(t);
        else if (t->type == Kpage)
            write_page_net(t, 0);
    }
}

static void write_page_inst(page)
PARSETREE *page;
{
    PARSETREE *t;

    for (t = page->data.tree; t; t = t->next) {
        if (t->type == Kinstance)
            write_instance(t);
        else if (t->type == Kcomment)
            write_comment(t);
    }
}

static void write_page_net(page, p)
PARSETREE *page;
int p;                          /* p = 1 conta le net, 0 altrimenti */
{
    PARSETREE *t;

    if (p)
        for (t = page->data.tree; t; t = t->next) {
            if (t->type == Knet)
                n_net++;        /* aggiunge la net */
            else if (t->type == Kcomment)
                write_comment(t);
    } else
        for (t = page->data.tree; t; t = t->next) {
            if (t->type == Knet)
                write_net(t);
            else if (t->type == Kcomment)
                write_comment(t);
        }
}

/*----------------------------------------------------------------------------
	Crea un nuovo elemento in descr, per ogni porta istanziata
	nella net-list.
*/
static void write_instance(inst)
PARSETREE *inst;
{
    PARSETREE *t;
    char *name, *defname;
    int i;

    extern char get_descr_type();
    extern void add_assoc();

    name = realnameof(inst);
    defname = "";
    for (t = inst->data.tree; t; t = t->next) {
        if (t->type == Kcomment)
            write_comment(t);
        else if (t->type == KviewRef)
            defname = get_defname(name, t);
        else if (t->type == KviewList)
            write_unsup(t);
    }

    /* MVMV inserisce la porta nella lista dei descrittori */
    descr[n_descr].attr = INTERNAL;
    descr[n_descr].type = get_descr_type(defname);

    descr[n_descr].name = strdup(name);

    /* aggiunge l'associazione nome->numero descrittore */
    add_assoc(n_descr, name);

    /* se i componenti sono piu` del minimo prestabilito, fa una
       realloc */

    n_descr++;

    if (n_descr >= Descr_Def_Size) {
        if (!create_silent)
            printf("(write_instance)Resize descr to %d\n",
                   2 * Descr_Def_Size);
        Descr_Def_Size *= 2;
        descr = realloc(descr, sizeof(DESCRIPTOR) * Descr_Def_Size);

        /* quando realloca deve creare lo spazio per i nuovi dati */
        for (i = Descr_Def_Size / 2; i < Descr_Def_Size; i++) {
            descr[i].fanin = 0;
            descr[i].fanout = 0;
            descr[i].to = NULL;
            descr[i].from = malloc(sizeof(int) * MAX_FANIN);
        }
    }
}

char *get_defname(instname, viewref)
char *instname;
PARSETREE *viewref;
{
    PARSETREE *cellref;

    cellref = viewref->data.tree->next;
    if (!cellref) {
        DEBUG(0) "Missing '(cellRef ...' at instance '%s'!\n", instname);
        return ("");
    } else
        return (realnameof(cellref));
}

static void write_net(net)
PARSETREE *net;
{
    PARSETREE *t, *arraydim;
    char *netname;

    netname = realnameof(net);
    arraydim = get_array_size(net);

    /* MVMV */
    add_net(netname, n_pin);    /*aggiunge la net alla lista */

    if (PEDANTIC)
        printf("(write_net) Aggiunta NET %s\n", netname);

    for (t = net->data.tree; t; t = t->next) {
        if (t->type == Kjoined)
            write_joined(netname, arraydim, t);
        else if (t->type == Kcomment)
            write_comment(t);
    }
}

static void write_joined(netname, arraydim, join)
char *netname;
PARSETREE *arraydim, *join;
{
    PARSETREE *t;

    for (t = join->data.tree; t; t = t->next) {
        if (t->type == KportRef)
            write_portref(netname, arraydim, t);
        else if (t->type == KportList) {
            char buf[YYLMAX];

            strcpy(buf, netname);
            write_portlist(buf, arraydim, t);
        } else
            write_unsup(t);
    }
}

static void write_unsup(t)
PARSETREE *t;
{
    DEBUG(0) "line %d: %d  '(%s ...' still unsupported :-(\n", t->line,
        t->type, keywords[t->type]);
}

static void write_portlist(netname, arraydim, portlist)
char *netname;
PARSETREE *arraydim, *portlist;
{
    PARSETREE *p;
    int i, l, n;

    l = strlen(netname);
    n = arraydim->data.ival;

    for (i = 0, p = portlist->data.tree; p; p = p->next, i++) {
        sprintf(netname + l, "[%d]", i);
        if (p->type == KportList)
            write_portlist(netname, arraydim->next, p);
        else if (p->type == KportRef)
            write_portref(netname, arraydim->next, p);
        else
            write_unsup(p);
    }

    if (i != n) {
        fprintf(stderr,
                "Warning: Edif error: mismatch in array dimension\n");
        fprintf(stderr, "      Line %d: declared %d, used %d\n",
                arraydim->line, n, i);
    }
}

static void write_portref(netname, arraydim, portref)
char *netname;
PARSETREE *arraydim, *portref;
{
    PARSETREE *instref, *member;
    char *portname, *instname, *dimbuf[256];
    char *c;

    extern void add_net_info();

    if (arraydim) {
        for (c = dimbuf; arraydim; arraydim = arraydim->next) {
            sprintf(c, "[0:%d]", arraydim->data.ival - 1);
            while (*c)
                c++;
        }
    } else
        dimbuf[0] = '\0';

    portname = strdup(realnameof(portref));
    instref = portref->data.tree->next;
    member = NULL;
    if (instref && (instref->type != KinstanceRef)) {
        write_unsup(instref);
        instname = "???";
    } else
        instname = instref ? realnameof(instref) : "-";

    /* MVMV 
       netname = nome del filo o della porta
       dimbuf = dimensione del bus
       instname = nome del componente instanziato
     */

    /* MVMV
       portname = nome della porta del componente
       instaziato a cui si collega la net
     */

    add_net_info(netname, instname, portname);  /* aggiunge la connessione */

    if (PEDANTIC)
        printf("(write_portref) Aggiunto PIN %s, %s\n", instname,
               portname);
    free(portname);
}
