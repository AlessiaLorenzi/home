void
load_cbk(w, client_data, call_data)
 Widget w;
 XtPointer client_data;
 XtPointer call_data;
{
    XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call_data;
int cnt;
FILE *configfile;


strcpy(file_da_scrivere, "parametri");

configfile = fopen ( file_da_scrivere, "r" );
if ( configfile == NULL )
        {
        printf ( "leggiconf : error opening data file\n");
        fflush ( stdout );
        exit ( 2 );
        }
else
        if (open_verbose) printf ("\n   ----  configurazione in %s\n", file_da_scrivere);



for (cnt = 0 ; cnt < max_sp; cnt++)
        {
        if (open_verbose) printf ("loading %s\n", etichetta[cnt]);
        if (sottop_tipo[cnt] == 0) fscanf (configfile, "%f %d %d %d %d %d\n", &(field[cnt]),
                        &(loop[cnt]), &(power[cnt]), &(attenuation[cnt]), &(a_gain[cnt]), &(f
_gain[cnt]));
        else fscanf (configfile, "%f %d %d %d %d\n", &(field[cnt]), &(loop[cnt]), &(power[cnt
]),
                        &(a_gain[cnt]), &(f_gain[cnt]));
        carica_controllers(cnt);
        pdr = cnt;
        refresh();
        }


}
:q


