        case 8:

        /* prova a fare un passo */
        sprintf(stringa,"!%1d, %d, %d",motor[sottop_selez], tu_direction * steps, 400);


     /* write to server */
        if (  write(za_fildes[sottop_selez],stringa,sizeof(stringa)) < 0 )
            perror( "\nclient: error writing to s streamsocket\n" ) ;
        if (net_verbose) printf (" scritto %s su   %d\n", stringa, za_fildes[sottop_selez]);

     /* listen to server */
        if ( read( za_fildes[sottop_selez], frame_to_r, nb_to_r ) < 0 )
            perror( "\nclient: error reading from s streamsocket\n" ) ;
        if (net_verbose) printf (" letto %s da   %d\n", frame_to_r, za_fildes[sottop_selez]);




        if ( strncmp(frame_to_r, "Start position", 14) == 0 )


        {
        if ( strncmp(frame_to_r, "Start position CW", 17) == 0 )
                {
                if (auto_verbose) printf ("ho raggiunto il fine corsa CW -> cambio direzione \n");
                tu_direction = tu_direction * -1;
                }

        if ( strncmp(frame_to_r, "Start position CCW", 18) == 0 )
                {
                if (auto_verbose) printf ("ho raggiunto il fine corsa CCW -> cambio direzione \n");
                tu_direction = tu_direction * -1;
                }

        to_do = 8 ;
        }
       else

        {


        /* rileggo errore di frequenza */

        strcpy(cmd,"!FREQ?");

         /* write to server */
         if (  write(rf_fildes[sottop_selez],cmd,sizeof(cmd)) < 0 )
                 perror( "\nclient: error writing to s streamsocket\n" ) ;
                 if (net_verbose) printf (" scritto %s su   %d\n", cmd, rf_fildes[sottop_selez]);

         /* listen to server */
         if ( read( rf_fildes[sottop_selez], frame_to_r, nb_to_r ) < 0 )
                 perror( "\nclient: error reading from s streamsocket\n" ) ;
         if (net_verbose) printf (" letto %s da   %d\n", frame_to_r, rf_fildes[sottop_selez]);

         old_frequency_error = frequency_error ;
         frequency_error = new_frequency_error ;
         new_frequency_error = 2048 - atoi(frame_to_r);

        if ( auto_verbose ) printf ("automagico: old err. di freq = %d  %f KHz\n", old_frequency_error
, old_frequency_error * 100.0 / 2048.0);
        if ( auto_verbose ) printf ("automagico:     err. di freq = %d  %f KHz\n", frequency_error, fr
equency_error * 100.0 / 2048.0);
        if ( auto_verbose ) printf ("automagico: new err. di freq = %d  %f KHz\n", new_frequency_error
, new_frequency_error * 100.0 / 2048.0);

        ok_1 = 0;
        ok_2 = 0;
        if (old_frequency_error - frequency_error > 0 ) ok_1 = 1;
        if (frequency_error - new_frequency_error >= 0 ) ok_2 = 1;

        if ( auto_verbose ) printf ("automagico: ok_1 = %d ok_2 = %d\n", ok_1, ok_2);



        if (new_frequency_error * 100.0 / 2048.0 < 5.0 )
                {
                steps = tu_step_slow[sottop_selez];
                }
        else
                {
                steps = tu_step_fast[sottop_selez];
                }
        if (new_frequency_error * 100.0 / 2048.0 < end_fr_corr[sottop_selez])
                {
                /* ho raggiunto la soglia */
                if ( auto_verbose ) printf ("automagico: ho raggiunto la soglia\n");
                to_do = 9 ;
                }



        if ( ok_1 == 1 && ok_2 == 0 )
                {
                /* ho oltrepassato il minimo */
                if ( auto_verbose ) printf ("automagico: ho oltrepassato il minimo\n");
                /*tu_direction = tu_direction * -1;

                steps = tu_step_slow[sottop_selez];
                to_do = 8 ;*/
                }

        if ( ok_1 == 0 && ok_2 == 0 )
                {
                /* ho sbagliato direzione */
                if ( auto_verbose ) printf ("automagico: ho sbagliato direzione\n");
                tu_direction = tu_direction * -1;
                to_do = 8 ;
                }

        }



