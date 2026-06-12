mag_page_refresh()
{
char string[30];
Arg args[9] ;



    if ( refresh_onoff == 1 )
        {
        XtSetArg( args[0], XtNlabel, "REFRESH  ON" ) ;
        XtSetArg( args[1], XtNbackground, pixel_orange ) ;
        XtSetValues(ma_refr_onoff_but , args, 2 ) ;
        }
    else /* refresh_onoff == 0 */
        {
        XtSetArg( args[0], XtNlabel, "REFRESH  OFF" ) ;
        XtSetArg( args[1], XtNbackground, pixel_grey ) ;
        XtSetValues(ma_refr_onoff_but , args, 2 ) ;
        }

    /* aggiornamento label inc/dec */
    sprintf( string, "%d", ma_step_key ) ;
    XtSetArg( args[0], XtNlabel, string ) ;
    switch ( button[bottone_premuto].type )
        {
        case 1:
        XtSetValues( ma_incdec_a, args, 1 ) ;
        break;

        case 2:
        case 4:
        XtSetValues( ma_incdec_a, args, 1 ) ;
        XtSetValues( ma_incdec_b, args, 1 ) ;
        break;

        case 3:
        case 5:
        case 6:
        XtSetValues( ma_incdec_a, args, 1 ) ;
        XtSetValues( ma_incdec_b, args, 1 ) ;
        XtSetValues( ma_incdec_c, args, 1 ) ;
        break;

        }

    switch ( button[bottone_premuto].type )
        {
        case 1:

        XawScrollbarSetThumb( ma_cur_ruler_a,
        supply[magnet[button[numero_bottone].obj2].alim1].current/
        supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent, 0.01);

        XtSetArg( args[0], XtNlabel, magnet[numero_bottone].n_alim1 ) ;
        XtSetValues(ma_name_label_a , args, 1 ) ;

        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj2].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_a, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj2].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_a, args,1);


        if ( supply[magnet[button[numero_bottone].obj2].alim1].on_off == ON )
            {
            XtSetArg( args[0], XtNlabel, "ON" ) ;
            XtSetArg( args[1], XtNbackground, pixel_orange ) ;
            XtSetValues(ma_onoff_but_a , args, 2 ) ;
            }
        if ( supply[magnet[button[numero_bottone].obj2].alim1].on_off == OFF )
            {
            XtSetArg( args[0], XtNlabel, "OFF" ) ;
            XtSetArg( args[1], XtNbackground, pixel_grey ) ;
            XtSetValues(ma_onoff_but_a , args, 2 ) ;
            }

        if ( supply[magnet[button[numero_bottone].obj2].alim1].on_off == STUB )
            {
            XtSetArg( args[0], XtNlabel, "STUB" ) ;
            XtSetArg( args[1], XtNbackground, pixel_white ) ;
            XtSetArg( args[2], XtNforeground, pixel_black ) ;
            XtSetValues(ma_onoff_but_a , args, 3 ) ;
            XtSetArg( args[0], XtNbackground, pixel_black ) ;
            XtSetValues(ma_box_a , args, 1 ) ;
            }



        if ( supply[magnet[button[numero_bottone].obj2].alim1].rem_loc
             == REMOTE)
            {
            XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
            XtSetValues(ma_remloc_but_a , args, 1 ) ;
            }
        else /* LOCAL */
            {
            XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
            XtSetValues(ma_remloc_but_a , args, 1 ) ;
            }

        XtSetArg( args[0], XtNlabel, "        " ) ;
        XtSetArg( args[1], XtNbackground, pixel_grey);

        XtSetValues(ma_status_lab_1_a, args, 2 ) ;
        XtSetValues(ma_status_lab_2_a, args, 2 ) ;
        XtSetValues(ma_status_lab_3_a, args, 2 ) ;
        XtSetValues(ma_status_lab_4_a, args, 2 ) ;
        XtSetValues(ma_status_lab_5_a, args, 2 ) ;
        XtSetValues(ma_status_lab_6_a, args, 2 ) ;
        XtSetValues(ma_status_lab_7_a, args, 2 ) ;
        XtSetValues(ma_status_lab_8_a, args, 2 ) ;
        XtSetValues(ma_status_lab_9_a, args, 2 ) ;
        XtSetValues(ma_status_lab_10_a, args, 2 ) ;
        XtSetValues(ma_status_lab_11_a, args, 2 ) ;
        XtSetValues(ma_status_lab_12_a, args, 2 ) ;
        XtSetValues(ma_status_lab_13_a, args, 2 ) ;
        XtSetValues(ma_status_lab_14_a, args, 2 ) ;
        XtSetValues(ma_status_lab_15_a, args, 2 ) ;
        XtSetValues(ma_status_lab_16_a, args, 2 ) ;

        break;



        case 2:
        case 4:

        XawScrollbarSetThumb( ma_cur_ruler_a,
        supply[magnet[button[numero_bottone].obj2].alim1].current/
        supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent, 0.01);

        XtSetArg( args[0], XtNlabel, magnet[numero_bottone].n_alim1 ) ;
        XtSetValues(ma_name_label_a , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj2].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_a, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj2].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_a, args,1);


        switch (supply[magnet[button[numero_bottone].obj2].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_a , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_a , args, 1 ) ;
                break;
                }






        switch (supply[magnet[button[numero_bottone].obj2].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
                break;
                }



                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_a, args, 2 ) ;
                XtSetValues(ma_status_lab_2_a, args, 2 ) ;
                XtSetValues(ma_status_lab_3_a, args, 2 ) ;
                XtSetValues(ma_status_lab_4_a, args, 2 ) ;
                XtSetValues(ma_status_lab_5_a, args, 2 ) ;
                XtSetValues(ma_status_lab_6_a, args, 2 ) ;
                XtSetValues(ma_status_lab_7_a, args, 2 ) ;
                XtSetValues(ma_status_lab_8_a, args, 2 ) ;
                XtSetValues(ma_status_lab_9_a, args, 2 ) ;
                XtSetValues(ma_status_lab_10_a, args, 2 ) ;
                XtSetValues(ma_status_lab_11_a, args, 2 ) ;
                XtSetValues(ma_status_lab_12_a, args, 2 ) ;
                XtSetValues(ma_status_lab_13_a, args, 2 ) ;
                XtSetValues(ma_status_lab_14_a, args, 2 ) ;
                XtSetValues(ma_status_lab_15_a, args, 2 ) ;
                XtSetValues(ma_status_lab_16_a, args, 2 ) ;


        XawScrollbarSetThumb( ma_cur_ruler_b,
        supply[magnet[button[numero_bottone].obj2].alim2].current/
        supply[magnet[button[numero_bottone].obj2].alim2].maxcurrent, 0.01);

        XtSetArg( args[0], XtNlabel, magnet[numero_bottone].n_alim2 ) ;
        XtSetValues(ma_name_label_b , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj2].alim2].current * 100.0 /
        supply[magnet[button[numero_bottone].obj2].alim2].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_b, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj2].alim2].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_b, args,1);


        switch (supply[magnet[button[numero_bottone].obj2].alim2].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_b , args, 3 ) ;
                   XtSetArg( args[0], XtNbackground, pixel_black ) ;
                   XtSetValues(ma_box_b , args, 1 ) ;
                break;
                }





        switch (supply[magnet[button[numero_bottone].obj2].alim2].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_b , args, 1 ) ;
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_b , args, 1 ) ;
                break;
                }


                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_b, args, 2 ) ;
                XtSetValues(ma_status_lab_2_b, args, 2 ) ;
                XtSetValues(ma_status_lab_3_b, args, 2 ) ;
                XtSetValues(ma_status_lab_4_b, args, 2 ) ;
                XtSetValues(ma_status_lab_5_b, args, 2 ) ;
                XtSetValues(ma_status_lab_6_b, args, 2 ) ;
                XtSetValues(ma_status_lab_7_b, args, 2 ) ;
                XtSetValues(ma_status_lab_8_b, args, 2 ) ;
                XtSetValues(ma_status_lab_9_b, args, 2 ) ;
                XtSetValues(ma_status_lab_10_b, args, 2 ) ;
                XtSetValues(ma_status_lab_11_b, args, 2 ) ;
                XtSetValues(ma_status_lab_12_b, args, 2 ) ;
                XtSetValues(ma_status_lab_13_b, args, 2 ) ;
                XtSetValues(ma_status_lab_14_b, args, 2 ) ;
                XtSetValues(ma_status_lab_15_b, args, 2 ) ;
                XtSetValues(ma_status_lab_16_b, args, 2 ) ;


        break;


        case 3:


        XawScrollbarSetThumb( ma_cur_ruler_a,
        supply[magnet[button[numero_bottone].obj1].alim1].current/
        supply[magnet[button[numero_bottone].obj1].alim1].maxcurrent, 0.01);

        XtSetArg( args[0], XtNlabel,
        magnet[button[numero_bottone].obj1].n_alim1 ) ;
        XtSetValues(ma_name_label_a , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj1].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj1].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_a, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj1].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_a, args,1);


        switch (supply[magnet[button[numero_bottone].obj1].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;
                
                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_a , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_a , args, 1 ) ;
                break;
                }



        switch (supply[magnet[button[numero_bottone].obj1].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
                break;
                }


                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_a, args, 2 ) ;
                XtSetValues(ma_status_lab_2_a, args, 2 ) ;
                XtSetValues(ma_status_lab_3_a, args, 2 ) ;
                XtSetValues(ma_status_lab_4_a, args, 2 ) ;
                XtSetValues(ma_status_lab_5_a, args, 2 ) ;
                XtSetValues(ma_status_lab_6_a, args, 2 ) ;
                XtSetValues(ma_status_lab_7_a, args, 2 ) ;
                XtSetValues(ma_status_lab_8_a, args, 2 ) ;
                XtSetValues(ma_status_lab_9_a, args, 2 ) ;
                XtSetValues(ma_status_lab_10_a, args, 2 ) ;
                XtSetValues(ma_status_lab_11_a, args, 2 ) ;
                XtSetValues(ma_status_lab_12_a, args, 2 ) ;
                XtSetValues(ma_status_lab_13_a, args, 2 ) ;
                XtSetValues(ma_status_lab_14_a, args, 2 ) ;
                XtSetValues(ma_status_lab_15_a, args, 2 ) ;
                XtSetValues(ma_status_lab_16_a, args, 2 ) ;




        XawScrollbarSetThumb( ma_cur_ruler_b,
        supply[magnet[button[numero_bottone].obj2].alim1].current/
        supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent, 0.01);

        XtSetArg( args[0], XtNlabel,
        magnet[button[numero_bottone].obj2].n_alim1 ) ;
        XtSetValues(ma_name_label_b , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj2].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_b, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj2].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_b, args,1);


        switch (supply[magnet[button[numero_bottone].obj2].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_b , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_b , args, 1 ) ;
                break;
                }


        switch (supply[magnet[button[numero_bottone].obj2].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_b , args, 1 ) ;
                break;
                }


                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_b, args, 2 ) ;
                XtSetValues(ma_status_lab_2_b, args, 2 ) ;
                XtSetValues(ma_status_lab_3_b, args, 2 ) ;
                XtSetValues(ma_status_lab_4_b, args, 2 ) ;
                XtSetValues(ma_status_lab_5_b, args, 2 ) ;
                XtSetValues(ma_status_lab_6_b, args, 2 ) ;
                XtSetValues(ma_status_lab_7_b, args, 2 ) ;
                XtSetValues(ma_status_lab_8_b, args, 2 ) ;
                XtSetValues(ma_status_lab_9_b, args, 2 ) ;
                XtSetValues(ma_status_lab_10_b, args, 2 ) ;
                XtSetValues(ma_status_lab_11_b, args, 2 ) ;
                XtSetValues(ma_status_lab_12_b, args, 2 ) ;
                XtSetValues(ma_status_lab_13_b, args, 2 ) ;
                XtSetValues(ma_status_lab_14_b, args, 2 ) ;
                XtSetValues(ma_status_lab_15_b, args, 2 ) ;
                XtSetValues(ma_status_lab_16_b, args, 2 ) ;




        XawScrollbarSetThumb( ma_cur_ruler_c,
        supply[magnet[button[numero_bottone].obj3].alim1].current/
        supply[magnet[button[numero_bottone].obj3].alim1].maxcurrent, 0.01);

        XtSetArg( args[0], XtNlabel,
        magnet[button[numero_bottone].obj3].n_alim1 ) ;
        XtSetValues(ma_name_label_c , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj3].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj3].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_c, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj3].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_c, args,1);


        switch (supply[magnet[button[numero_bottone].obj3].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_c , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_c , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_c , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_c , args, 1 ) ;
                break;
                }





        switch (supply[magnet[button[numero_bottone].obj3].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_c , args, 1 ) ;
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_c , args, 1 ) ;
                break;
                }


                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_c, args, 2 ) ;
                XtSetValues(ma_status_lab_2_c, args, 2 ) ;
                XtSetValues(ma_status_lab_3_c, args, 2 ) ;
                XtSetValues(ma_status_lab_4_c, args, 2 ) ;
                XtSetValues(ma_status_lab_5_c, args, 2 ) ;
                XtSetValues(ma_status_lab_6_c, args, 2 ) ;
                XtSetValues(ma_status_lab_7_c, args, 2 ) ;
                XtSetValues(ma_status_lab_8_c, args, 2 ) ;
                XtSetValues(ma_status_lab_9_c, args, 2 ) ;
                XtSetValues(ma_status_lab_10_c, args, 2 ) ;
                XtSetValues(ma_status_lab_11_c, args, 2 ) ;
                XtSetValues(ma_status_lab_12_c, args, 2 ) ;
                XtSetValues(ma_status_lab_13_c, args, 2 ) ;
                XtSetValues(ma_status_lab_14_c, args, 2 ) ;
                XtSetValues(ma_status_lab_15_c, args, 2 ) ;
                XtSetValues(ma_status_lab_16_c, args, 2 ) ;



        break;


        case 5:



        XawScrollbarSetThumb( ma_cur_ruler_a,
        supply[magnet[button[numero_bottone].obj1].alim1].current/
        supply[magnet[button[numero_bottone].obj1].alim1].maxcurrent, 0.01);

        XtSetArg( args[0], XtNlabel,
                  magnet[button[numero_bottone].obj1].n_alim1 ) ;
        XtSetValues(ma_name_label_a , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj1].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj1].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_a, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj1].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_a, args,1);


        switch (supply[magnet[button[numero_bottone].obj1].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_a , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_a , args, 1 ) ;
                break;
                }




        switch (supply[magnet[button[numero_bottone].obj1].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
                break;
                }



                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_a, args, 2 ) ;
                XtSetValues(ma_status_lab_2_a, args, 2 ) ;
                XtSetValues(ma_status_lab_3_a, args, 2 ) ;
                XtSetValues(ma_status_lab_4_a, args, 2 ) ;
                XtSetValues(ma_status_lab_5_a, args, 2 ) ;
                XtSetValues(ma_status_lab_6_a, args, 2 ) ;
                XtSetValues(ma_status_lab_7_a, args, 2 ) ;
                XtSetValues(ma_status_lab_8_a, args, 2 ) ;
                XtSetValues(ma_status_lab_9_a, args, 2 ) ;
                XtSetValues(ma_status_lab_10_a, args, 2 ) ;
                XtSetValues(ma_status_lab_11_a, args, 2 ) ;
                XtSetValues(ma_status_lab_12_a, args, 2 ) ;
                XtSetValues(ma_status_lab_13_a, args, 2 ) ;
                XtSetValues(ma_status_lab_14_a, args, 2 ) ;
                XtSetValues(ma_status_lab_15_a, args, 2 ) ;
                XtSetValues(ma_status_lab_16_a, args, 2 ) ;



        XawScrollbarSetThumb( ma_cur_ruler_b,
        supply[magnet[button[numero_bottone].obj1].alim2].current/
        supply[magnet[button[numero_bottone].obj1].alim2].maxcurrent, 0.01);

        XtSetArg( args[0], XtNlabel,
        magnet[button[numero_bottone].obj1].n_alim2 ) ;
        XtSetValues(ma_name_label_b , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj1].alim2].current * 100.0 /
        supply[magnet[button[numero_bottone].obj1].alim2].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_b, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj1].alim2].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_b, args,1);


        switch (supply[magnet[button[numero_bottone].obj1].alim2].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_b , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_b , args, 1 ) ;
                break;
                }



        switch (supply[magnet[button[numero_bottone].obj1].alim2].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_b , args, 1 ) ;
                break;
                }

                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_b, args, 2 ) ;
                XtSetValues(ma_status_lab_2_b, args, 2 ) ;
                XtSetValues(ma_status_lab_3_b, args, 2 ) ;
                XtSetValues(ma_status_lab_4_b, args, 2 ) ;
                XtSetValues(ma_status_lab_5_b, args, 2 ) ;
                XtSetValues(ma_status_lab_6_b, args, 2 ) ;
                XtSetValues(ma_status_lab_7_b, args, 2 ) ;
                XtSetValues(ma_status_lab_8_b, args, 2 ) ;
                XtSetValues(ma_status_lab_9_b, args, 2 ) ;
                XtSetValues(ma_status_lab_10_b, args, 2 ) ;
                XtSetValues(ma_status_lab_11_b, args, 2 ) ;
                XtSetValues(ma_status_lab_12_b, args, 2 ) ;
                XtSetValues(ma_status_lab_13_b, args, 2 ) ;
                XtSetValues(ma_status_lab_14_b, args, 2 ) ;
                XtSetValues(ma_status_lab_15_b, args, 2 ) ;
                XtSetValues(ma_status_lab_16_b, args, 2 ) ;


        XawScrollbarSetThumb( ma_cur_ruler_c,
        supply[magnet[button[numero_bottone].obj2].alim1].current/
        supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent, 0.01);

        XtSetArg( args[0], XtNlabel,
        magnet[button[numero_bottone].obj2].n_alim1 ) ;
        XtSetValues(ma_name_label_c , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj2].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_c, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj2].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_c, args,1);


        switch (supply[magnet[button[numero_bottone].obj2].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_c , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_c , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_c , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_c , args, 1 ) ;
                break;
                }




        switch (supply[magnet[button[numero_bottone].obj2].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_c , args, 1 ) ;
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_c , args, 1 ) ;
                break;
                }


                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_c, args, 2 ) ;
                XtSetValues(ma_status_lab_2_c, args, 2 ) ;
                XtSetValues(ma_status_lab_3_c, args, 2 ) ;
                XtSetValues(ma_status_lab_4_c, args, 2 ) ;
                XtSetValues(ma_status_lab_5_c, args, 2 ) ;
                XtSetValues(ma_status_lab_6_c, args, 2 ) ;
                XtSetValues(ma_status_lab_7_c, args, 2 ) ;
                XtSetValues(ma_status_lab_8_c, args, 2 ) ;
                XtSetValues(ma_status_lab_9_c, args, 2 ) ;
                XtSetValues(ma_status_lab_10_c, args, 2 ) ;
                XtSetValues(ma_status_lab_11_c, args, 2 ) ;
                XtSetValues(ma_status_lab_12_c, args, 2 ) ;
                XtSetValues(ma_status_lab_13_c, args, 2 ) ;
                XtSetValues(ma_status_lab_14_c, args, 2 ) ;
                XtSetValues(ma_status_lab_15_c, args, 2 ) ;
                XtSetValues(ma_status_lab_16_c, args, 2 ) ;


        break;



        case 6:

        XawScrollbarSetThumb( ma_cur_ruler_a,
        supply[magnet[button[numero_bottone].obj1].alim1].current/
        supply[magnet[button[numero_bottone].obj1].alim1].maxcurrent, 0.01);

        XtSetArg( args[0], XtNlabel,
                  magnet[button[numero_bottone].obj1].n_alim1 ) ;
        XtSetValues(ma_name_label_a , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj1].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj1].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_a, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj1].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_a, args,1);


        switch (supply[magnet[button[numero_bottone].obj1].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_a , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_a , args, 1 ) ;
                break;
                }





        switch (supply[magnet[button[numero_bottone].obj1].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
                break;
                }



                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_a, args, 2 ) ;
                XtSetValues(ma_status_lab_2_a, args, 2 ) ;
                XtSetValues(ma_status_lab_3_a, args, 2 ) ;
                XtSetValues(ma_status_lab_4_a, args, 2 ) ;
                XtSetValues(ma_status_lab_5_a, args, 2 ) ;
                XtSetValues(ma_status_lab_6_a, args, 2 ) ;
                XtSetValues(ma_status_lab_7_a, args, 2 ) ;
                XtSetValues(ma_status_lab_8_a, args, 2 ) ;
                XtSetValues(ma_status_lab_9_a, args, 2 ) ;
                XtSetValues(ma_status_lab_10_a, args, 2 ) ;
                XtSetValues(ma_status_lab_11_a, args, 2 ) ;
                XtSetValues(ma_status_lab_12_a, args, 2 ) ;
                XtSetValues(ma_status_lab_13_a, args, 2 ) ;
                XtSetValues(ma_status_lab_14_a, args, 2 ) ;
                XtSetValues(ma_status_lab_15_a, args, 2 ) ;
                XtSetValues(ma_status_lab_16_a, args, 2 ) ;



        XawScrollbarSetThumb( ma_cur_ruler_b,
        supply[magnet[button[numero_bottone].obj2].alim1].current/
        supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent, 0.01);

        XtSetArg( args[0], XtNlabel,
                  magnet[button[numero_bottone].obj2].n_alim1 ) ;
        XtSetValues(ma_name_label_b , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj2].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj1].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_b, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj2].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_b, args,1);


        switch (supply[magnet[button[numero_bottone].obj2].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_b , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_b , args, 1 ) ;
                break;
                }




        switch (supply[magnet[button[numero_bottone].obj2].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
                break;
                }



                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_b, args, 2 ) ;
                XtSetValues(ma_status_lab_2_b, args, 2 ) ;
                XtSetValues(ma_status_lab_3_b, args, 2 ) ;
                XtSetValues(ma_status_lab_4_b, args, 2 ) ;
                XtSetValues(ma_status_lab_5_b, args, 2 ) ;
                XtSetValues(ma_status_lab_6_b, args, 2 ) ;
                XtSetValues(ma_status_lab_7_b, args, 2 ) ;
                XtSetValues(ma_status_lab_8_b, args, 2 ) ;
                XtSetValues(ma_status_lab_9_b, args, 2 ) ;
                XtSetValues(ma_status_lab_10_b, args, 2 ) ;
                XtSetValues(ma_status_lab_11_b, args, 2 ) ;
                XtSetValues(ma_status_lab_12_b, args, 2 ) ;
                XtSetValues(ma_status_lab_13_b, args, 2 ) ;
                XtSetValues(ma_status_lab_14_b, args, 2 ) ;
                XtSetValues(ma_status_lab_15_b, args, 2 ) ;
                XtSetValues(ma_status_lab_16_b, args, 2 ) ;



        XawScrollbarSetThumb( ma_cur_ruler_c,
        supply[magnet[button[numero_bottone].obj2].alim2].current/
        supply[magnet[button[numero_bottone].obj2].alim2].maxcurrent, 0.01);

        XtSetArg( args[0], XtNlabel,
                  magnet[button[numero_bottone].obj2].n_alim2 ) ;
        XtSetValues(ma_name_label_c , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj2].alim2].current * 100.0 /
        supply[magnet[button[numero_bottone].obj2].alim2].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_c, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj2].alim2].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_c, args,1);


        switch (supply[magnet[button[numero_bottone].obj2].alim2].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_c , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_c , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_c , args, 3 ) ;
                XtSetArg( args[0], XtNbackground, pixel_black ) ;
                XtSetValues(ma_box_c , args, 1 ) ;
                break;
                }




        switch (supply[magnet[button[numero_bottone].obj2].alim2].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_c , args, 1 ) ;
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_c , args, 1 ) ;
                break;
                }


                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_c, args, 2 ) ;
                XtSetValues(ma_status_lab_2_c, args, 2 ) ;
                XtSetValues(ma_status_lab_3_c, args, 2 ) ;
                XtSetValues(ma_status_lab_4_c, args, 2 ) ;
                XtSetValues(ma_status_lab_5_c, args, 2 ) ;
                XtSetValues(ma_status_lab_6_c, args, 2 ) ;
                XtSetValues(ma_status_lab_7_c, args, 2 ) ;
                XtSetValues(ma_status_lab_8_c, args, 2 ) ;
                XtSetValues(ma_status_lab_9_c, args, 2 ) ;
                XtSetValues(ma_status_lab_10_c, args, 2 ) ;
                XtSetValues(ma_status_lab_11_c, args, 2 ) ;
                XtSetValues(ma_status_lab_12_c, args, 2 ) ;
                XtSetValues(ma_status_lab_13_c, args, 2 ) ;
                XtSetValues(ma_status_lab_14_c, args, 2 ) ;
                XtSetValues(ma_status_lab_15_c, args, 2 ) ;
                XtSetValues(ma_status_lab_16_c, args, 2 ) ;

        break;

        }

}
