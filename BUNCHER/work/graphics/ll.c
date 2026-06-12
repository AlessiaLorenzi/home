void
AttivaFinestra(w, client_data, call_data)
 Widget w;
 XtPointer client_data;
 XtPointer call_data;
{
    XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call_data;
char *input = (char *) client_data;
int index;

    Arg      args[256];
    Cardinal ac=0;
    Boolean  argok=False;

index = 0;
selezionato = 9999;
tipo = 9999;

for (index = 0 ; index < 20 ; index++)
	{	
	if (strcmp(input, nome_dispositivo[index])==0)
		{
		/* e' di tipo buncher o postchopper */
		if (strcmp(tipo_dispositivo[index], "buncher") == 0 ) tipo = 0;
		if (strcmp(tipo_dispositivo[index], "postchopper") == 0 ) tipo = 1;
		selezionato = index ;
		if (debug_verbose) printf ("dispositivo trovato in riga n. %d\n", index);
		if (debug_verbose) 
			{
			if (tipo == 0 )printf ("sottopagine %d e %d\n", sub_page_index[selezionato], sub_page_index[selezionato] + 1);
			if (tipo == 0 )printf ("etichetta 1 : %s etichetta 2 : %s\n", etichetta[sub_page_index[selezionato]], etichetta[sub_page_index[selezionato] + 1]);
			if (tipo == 1 )printf ("sottopagina  %d\n", sub_page_index[selezionato]);
			if (tipo == 1 )printf ("etichetta : %s\n", etichetta[sub_page_index[selezionato]]);
			}
		}
	}

/* dispositivo di tipo lefase o rivfase o chopper */

if (strcmp(input, "lefase") == 0 ) tipo = 2;
if (strcmp(input, "rivfase") == 0 ) tipo = 3;
if (strcmp(input, "chopper") == 0 ) tipo = 4;



if (debug_verbose) printf ("stringa : %s, tipo  = %d selezionato = %d\n", input, tipo, selezionato);
if (debug_verbose) printf ("stringa : %s, prec_tipo  = %d prec_selezionato = %d\n", input, prec_tipo, prec_selezionato);

/*

buncher     finestraFD
postchopper finestraFB

*/



switch ( tipo )
	{
	case 0:
	if (debug_verbose) printf ("** buncher ** \n\n");
	if ( tipo != prec_tipo )
		{
	        XtUnmanageChild(finestraFB);
	        XtUnmanageChild(finestraFF);
	        XtUnmanageChild(finestraFG);
	        XtUnmanageChild(finestraFH);
		XtManageChild(finestraFD);
		prec_tipo = tipo ;
		prec_selezionato = selezionato ;
		}
	else
		{
		if ( prec_selezionato != 9999 )
			{
			if ( prec_selezionato != selezionato )
				{
				if ( debug_verbose ) printf ("\nrefresh\n");
				prec_selezionato = selezionato ;
				}
			else
				{
				if ( debug_verbose ) printf ("\nchiudo tutto\n");
				XtUnmanageChild(finestraFD);
				prec_selezionato = 9999 ;
				}
			}

		else
			{
			XtManageChild(finestraFD);
			prec_selezionato = selezionato ;
			}
		}


	break;



	case 1:
	if (debug_verbose) printf ("** postchopper ** \n\n");
        if ( tipo != prec_tipo )
                {
                XtUnmanageChild(finestraFD);
                XtUnmanageChild(finestraFF);
                XtUnmanageChild(finestraFG);
                XtUnmanageChild(finestraFH);
                XtManageChild(finestraFB);
		prec_tipo = tipo ;
		prec_selezionato = selezionato ;
                }
        else
                {
		if ( prec_selezionato != 9999 )
			{
	                if ( prec_selezionato != selezionato )
       		                 {
               		         if ( debug_verbose ) printf ("\nrefresh\n");
				 prec_selezionato = selezionato ;
       		                 }
               		 else
                       		 {
                   		 if ( debug_verbose ) printf ("\nchiudo tutto\n");
                    		 XtUnmanageChild(finestraFB);
				 prec_selezionato = 9999 ;
                  		 }
			}
		else
			{
			XtManageChild(finestraFB);
			prec_selezionato = selezionato ;
			}

		}
	break;




	case 2:
	if (debug_verbose) printf ("** low energy phase ** \n\n");

        if (XtIsManaged(finestraFH))
                XtUnmanageChild(finestraFH);
        else
                XtManageChild(finestraFH);
        XtUnmanageChild(finestraFB);
        XtUnmanageChild(finestraFD);
        XtUnmanageChild(finestraFF);
        XtUnmanageChild(finestraFG);
	prec_tipo = tipo ;
	break;

	case 3:
	if (debug_verbose) printf ("** rivelatore di fase ** \n\n");
        if (XtIsManaged(finestraFG))
                XtUnmanageChild(finestraFG);
        else
                XtManageChild(finestraFG);
        XtUnmanageChild(finestraFB);
        XtUnmanageChild(finestraFD);
        XtUnmanageChild(finestraFF);
        XtUnmanageChild(finestraFH);
	prec_tipo = tipo ;
	break;

	case 4:
	if (debug_verbose) printf ("** chopper ** \n\n");
	if (XtIsManaged(finestraFF))
		XtUnmanageChild(finestraFF);
	else
		XtManageChild(finestraFF);
	XtUnmanageChild(finestraFB);
	XtUnmanageChild(finestraFD);
	XtUnmanageChild(finestraFG);
	XtUnmanageChild(finestraFH);
	prec_tipo = tipo ;
	break;

	case 9999:
	if (debug_verbose) printf ("\n** errore nella cofigurazione ** \n");
	break;

	default:
	if (debug_verbose) printf ("\n** errore nella cofigurazione ** tipo = %d \n", tipo);
	break;
	}


}
