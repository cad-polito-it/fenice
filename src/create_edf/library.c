/*gcc 
	library.c

	Descrizione della libreria utilizzata dalla create()

	Massimo Violante, 1995
*/

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "library.h"

int n_comp = 32;
LIB_TYPE *library;

char *pin_name[5];
char *pin_name_ffd[2];
char *pin_name_ffr[3];
char *pin_name_ffp[3];
char *pin_name_ffrp[4];

void init_library(void)
{
    int i;

    library = (LIB_TYPE *) malloc(sizeof(LIB_TYPE) * n_comp);

    if (library == NULL) {
        printf("(init_library) Memory error, quitting...\n");
        exit(1);
    }

    pin_name[0] = strdup("I1");
    pin_name[1] = strdup("I2");
    pin_name[2] = strdup("I3");
    pin_name[3] = strdup("I4");
    pin_name[4] = strdup("I5");

    pin_name_ffd[0] = strdup("D");
    pin_name_ffd[1] = strdup("CK");

    pin_name_ffr[0] = strdup("D");
    pin_name_ffr[1] = strdup("CK");
    pin_name_ffr[2] = strdup("RESET");

    pin_name_ffp[0] = strdup("D");
    pin_name_ffp[1] = strdup("CK");
    pin_name_ffp[2] = strdup("PRESET");

    pin_name_ffrp[0] = strdup("D");
    pin_name_ffrp[1] = strdup("CK");
    pin_name_ffrp[2] = strdup("RESET");
    pin_name_ffrp[3] = strdup("PRESET");

    library[0].name = strdup("AND_GATE");
    library[0].n_fanin = 2;
    library[0].fanin_list = pin_name;
    library[1].name = strdup("NAND_GATE");
    library[1].n_fanin = 2;
    library[1].fanin_list = pin_name;
    library[2].name = strdup("OR_GATE");
    library[2].n_fanin = 2;
    library[2].fanin_list = pin_name;
    library[3].name = strdup("INV_GATE");
    library[3].n_fanin = 1;
    library[3].fanin_list = pin_name;
    library[4].name = strdup("BUF_GATE");
    library[4].n_fanin = 1;
    library[4].fanin_list = pin_name;
    library[5].name = strdup("NOR_GATE");
    library[5].n_fanin = 2;
    library[5].fanin_list = pin_name;
    library[6].name = strdup("XOR_GATE");
    library[6].n_fanin = 2;
    library[6].fanin_list = pin_name;
    library[7].name = strdup("XNOR_GATE");
    library[7].n_fanin = 2;
    library[7].fanin_list = pin_name;
    library[8].name = strdup("XOR3_GATE");
    library[8].n_fanin = 3;
    library[8].fanin_list = pin_name;
    library[9].name = strdup("XNOR3_GATE");
    library[9].n_fanin = 3;
    library[9].fanin_list = pin_name;
    library[10].name = strdup("XOR4_GATE");
    library[10].n_fanin = 4;
    library[10].fanin_list = pin_name;
    library[11].name = strdup("XNOR4_GATE");
    library[11].n_fanin = 4;
    library[11].fanin_list = pin_name;
    library[12].name = strdup("XOR5_GATE");
    library[12].n_fanin = 5;
    library[12].fanin_list = pin_name;
    library[13].name = strdup("XNOR5_GATE");
    library[13].n_fanin = 5;
    library[13].fanin_list = pin_name;
    library[14].name = strdup("NAND3_GATE");
    library[14].n_fanin = 3;
    library[14].fanin_list = pin_name;
    library[15].name = strdup("AND3_GATE");
    library[15].n_fanin = 3;
    library[15].fanin_list = pin_name;
    library[16].name = strdup("OR3_GATE");
    library[16].n_fanin = 3;
    library[16].fanin_list = pin_name;
    library[17].name = strdup("NOR3_GATE");
    library[17].n_fanin = 3;
    library[17].fanin_list = pin_name;
    library[18].name = strdup("NAND4_GATE");
    library[18].n_fanin = 4;
    library[18].fanin_list = pin_name;
    library[19].name = strdup("AND4_GATE");
    library[19].n_fanin = 4;
    library[19].fanin_list = pin_name;
    library[20].name = strdup("OR4_GATE");
    library[20].n_fanin = 4;
    library[20].fanin_list = pin_name;
    library[21].name = strdup("NOR4_GATE");
    library[21].n_fanin = 4;
    library[21].fanin_list = pin_name;
    library[22].name = strdup("NAND5_GATE");
    library[22].n_fanin = 5;
    library[22].fanin_list = pin_name;
    library[23].name = strdup("AND5_GATE");
    library[23].n_fanin = 5;
    library[23].fanin_list = pin_name;
    library[24].name = strdup("OR5_GATE");
    library[24].n_fanin = 5;
    library[24].fanin_list = pin_name;
    library[25].name = strdup("NOR5_GATE");
    library[25].n_fanin = 5;
    library[25].fanin_list = pin_name;
    library[26].name = strdup("FLIP_FLOP_D");
    library[26].n_fanin = 2;
    library[26].fanin_list = pin_name_ffd;
    library[27].name = strdup("FLIP_FLOP_D_RESET");
    library[27].n_fanin = 3;
    library[27].fanin_list = pin_name_ffr;
    library[28].name = strdup("FLIP_FLOP_D_PRESET");
    library[28].n_fanin = 3;
    library[28].fanin_list = pin_name_ffp;
    library[29].name = strdup("FLIP_FLOP_D_RESET_PRESET");
    library[29].n_fanin = 4;
    library[29].fanin_list = pin_name_ffrp;

    library[30].name = strdup("LOGIC_0");
    library[30].n_fanin = 0;
    library[30].fanin_list = pin_name;
    library[31].name = strdup("LOGIC_1");
    library[31].n_fanin = 0;
    library[31].fanin_list = pin_name;

    for (i = 0; i < 26; i++)
        library[i].fanout = strdup("O");
    /* le uscite dei gate si chiamano tutte O */
    for ( /*NOP*/; i < 30; i++)
        library[i].fanout = strdup("Q");
    /* le uscite dei FF si chiamano tutte Q */

    library[30].fanout = strdup("O");
    library[31].fanout = strdup("O");
}

int get_pin_pos(char *s)
{
    int i;

    for (i = 0; i < 5; i++)
        if (strcmp(s, pin_name[i]) == 0)
            return (i);

    for (i = 0; i < 2; i++)
        if (strcmp(s, pin_name_ffd[i]) == 0)
            return (i);

    for (i = 0; i < 3; i++)
        if (strcmp(s, pin_name_ffr[i]) == 0)
            return (i);

    for (i = 0; i < 3; i++)
        if (strcmp(s, pin_name_ffp[i]) == 0)
            return (i);

    for (i = 0; i < 4; i++)
        if (strcmp(s, pin_name_ffrp[i]) == 0)
            return (i);

    return (-1);
}
