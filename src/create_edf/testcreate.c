#include <stdio.h>
#include "data2.h"

DESCRIPTOR *descr;
int n_pi, n_po, n_ff;
int n_descr;
int *pi_array, *po_array, *ppi_array;
int max_level;

main(int argc, char **argv)
{
    create(argv[1]);
    dump_descr();
    exit(0);
}
