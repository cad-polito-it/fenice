
void dump_descr()
{
    int i, j;
    FILE *fp;

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
                fprintf(fp, "(FF)\t");
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
            default:
                fprintf(fp, "????\t");
                break;
            }
            break;
        default:
            break;
        }
        fprintf(fp, "%s\, level = %d\n", descr[i].name, descr[i].leve l);
        fprintf(fp, "\tfanout = %d : ", descr[i].fanout);
        for (j = 0; j < descr[i].fanout; j++)
            fprintf(fp, "%d, ", descr[i].to[j]);
        fprintf(fp, "\n");
        fprintf(fp, "\tfanin = %d : ", descr[i].fanin);
        for (j = 0; j < descr[i].fanin; j++)
            fprintf(fp, "%d, ", descr[i].from[j]);
        fprintf(fp, "\n-------------------------------\n");
    }
}
