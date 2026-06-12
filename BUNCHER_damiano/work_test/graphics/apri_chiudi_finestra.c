void
apri_chiudi_finestra(w, client_data, call_data)
 Widget w;
 XtPointer client_data;
 XtPointer call_data;
{
    XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call_data;

sottop_selez = sub_page_index[selezionato] ;


if ( debug_verbose ) printf ("\nsottop_selez = %d di tipo %d , prec_sottop_selez = %d\n", sottop_selez, sottop_tipo[sottop_selez], prec_sottop_selez);
switch (tipo)
	{
	case 1:
	/* voglio aprire una sottopagina di postchopper */
	if ( prec_sottop_selez == 9999 )
		{
		/* non era aperta nessuna pagina */
                XtManageChild(bulletinBoard1);
		pdr = sottop_selez;
		refresh();
		XtAppAddTimeOut(app, 5, display_error, NULL);
		if ( debug_verbose ) printf ("\napro sottopagina postchopper\n");
		/* apro pagina postchopper */
		prec_sottop_selez = sottop_selez ;
		}
	else
		if ( prec_sottop_selez == sottop_selez )
			{
			/* era aperta la medesima pagina */
                	XtUnmanageChild(bulletinBoard1);
			prec_sottop_selez = 9999 ;
			if ( debug_verbose ) printf ("\nchiudo sottopagina postchopper\n");
			}

		else
			{
			/* era aperta una pagina diversa */
			if ( debug_verbose ) printf ("\nera aperta una sottopagina diversa\n");
			if ( sottop_tipo[prec_sottop_selez] == 1 )
				{
				if ( debug_verbose ) printf ("\nera aperta una sottopagina postchopper ==> refresh\n");
		pdr = sottop_selez;
		refresh();
				}
			else
				{
				if ( debug_verbose ) printf ("\nera aperta una sottopagina buncher\n");
                		XtUnmanageChild(bulletinBoard12);
                		XtManageChild(bulletinBoard1);
		pdr = sottop_selez;
		refresh();
				}
                	prec_sottop_selez = sottop_selez ;
			}


	break;

	case 0:
	/* voglio aprire una sottopagina di buncher */
        if ( prec_sottop_selez == 9999 )
                {
                /* non era aperta nessuna pagina */
                XtManageChild(bulletinBoard12);
		pdr = sottop_selez;
		refresh();
		XtAppAddTimeOut(app, 5, display_error, NULL);
                if ( debug_verbose ) printf ("\napro sottopagina buncher\n");
                /* apro pagina buncher */
                prec_sottop_selez = sottop_selez ;
                }
        else
                if ( prec_sottop_selez == sottop_selez )
                        {
                        /* era aperta la medesima pagina */
                        XtUnmanageChild(bulletinBoard12);
                        prec_sottop_selez = 9999 ;
                        if ( debug_verbose ) printf ("\nchiudo sottopagina buncher\n");
                        }

                else
                        {
                        /* era aperta una pagina diversa */
                        if ( debug_verbose ) printf ("\nera aperta una sottopagina diversa\n");
                        if ( sottop_tipo[prec_sottop_selez] == 0 )
                                {
                                if ( debug_verbose ) printf ("\nera aperta una sottopagina buncher ==> refresh\n");
		pdr = sottop_selez;
		refresh();
                                }
                        else
                                {
                                if ( debug_verbose ) printf ("\nera aperta una sottopagina postchopper\n");
                                XtUnmanageChild(bulletinBoard1);
                                XtManageChild(bulletinBoard12);
		pdr = sottop_selez;
		refresh();
                                }
                	prec_sottop_selez = sottop_selez ;
                        }


	break;
	}




}
