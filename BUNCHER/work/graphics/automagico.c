#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/BulletinB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include <Xm/ArrowB.h>
#include <sys/types.h>l
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#include "creation-c.h"
#include "defs.h"
#include "w_defs.h"


oscillazione()
{

Arg args[10];
Boolean argok;
char cmd[20];
char frame_to_s[1024], frame_to_r[1024] ;
int nb_to_s, nb_to_r ;
int ind;

int value;
int      slider_size;
int      increment;
int      page_increment;
Boolean  notify;


char stringa[64];
char stringa1[64];
char namestring[64];




nb_to_s = sizeof( frame_to_s ) ;
nb_to_r = sizeof( frame_to_r ) ;

if ( auto_verbose ) printf ("automagico: stato = %d  loop = %d\n", to_do, loop_counter);
switch (to_do)
	{
	case 0:
	/* accendere l'amplificatore */
	if ( auto_verbose ) printf ("automagico: stato = %d  loop = %d\n", to_do, loop_counter);
if ( sottop_tipo[sottop_selez] == 0 )
        {
        amplifier_onoff[sottop_selez]=ACCESO;
        bit_set(za_ampli_fildes[sottop_selez], za_par_io_port[sottop_selez], za_ampl_bit[sottop_selez]);

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton59, "red", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton59, "black", XmRPixel, 0, &argok));
        XtSetArg(args[0], XmNlabelString, XmStringCreate("amplif.:ON", XmFONTLIST_DEFAULT_TAG));
	}

if ( sottop_tipo[sottop_selez] == 0 )
        {
        /* BUNCHER */
        XtSetValues(pushButton35, args, 3);
        }

#ifdef HOLAMPLIFICATORE
else
        {
        /* POSTCHOPPER */
        XtSetValues(pushButton59, args, 3);
        }
#endif

if ( auto_verbose ) printf ("automagico: ho acceso l'ampli\n");


	/* accendere loop power */

	lp_onoff[sottop_selez]=ACCESO;

        strcpy(cmd,"!POWER=1");
        XtSetArg(args[2], XmNbackground, CONVERT(pushButton29, "red", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton29, "black", XmRPixel, 0, &argok));
        XtSetArg(args[0], XmNlabelString, XmStringCreate("loop pow.:ON", XmFONTLIST_DEFAULT_TAG));

	if ( sottop_tipo[sottop_selez] == 0 )
	        {
	        /* BUNCHER */
	        XtSetValues(pushButton29, args, 3);
	        }

	else
	        {
	        /* POSTCHOPPER */
	        XtSetValues(pushButton58, args, 3);
	        }


     /* write to server */
        if (  write(rf_fildes[sottop_selez],cmd,sizeof(cmd)) < 0 )
            {
            perror( "\nclient: error writing to s streamsocket\n" ) ;
            }
        if (net_verbose) printf (" scritto %s su   %d\n", cmd, rf_fildes[sottop_selez]);

     /* listen to server */
        if ( read( rf_fildes[sottop_selez], frame_to_r, nb_to_r ) < 0 )
            {
            perror( "\nclient: error reading from s streamsocket\n" ) ;
            }
        if (net_verbose) printf (" letto %s da   %d\n", frame_to_r, rf_fildes[sottop_selez]);


	sleep(2);

if ( auto_verbose ) printf ("automagico: ho acceso loop power\n");


        strcpy(cmd,"!AMP?");

     /* write to server */
        if (  write(rf_fildes[sottop_selez],cmd,sizeof(cmd)) < 0 )
            {
            perror( "\nclient: error writing to s streamsocket\n" ) ;
            }
        if (net_verbose) printf (" scritto %s su   %d\n", cmd, rf_fildes[sottop_selez]);

     /* listen to server */
        if ( read( rf_fildes[sottop_selez], frame_to_r, nb_to_r ) < 0 )
            {
            perror( "\nclient: error reading from s streamsocket\n" ) ;
            }
        if (net_verbose) printf (" *letto %s da   %d\n", frame_to_r, rf_fildes[sottop_selez]);

        amplitude_error = atoi(frame_to_r);

        to_do = 1;

        if ( amplitude_error < 4000 ) /* l'errore di ampiezza non e' in saturazione */
                {
                if ( auto_verbose ) printf ("*automagico: amplitude error = %d  OSCILLA !!!!\n", amplitude_error);
                to_do = 1;
                }

        if ( amplitude_error < 20 ) /* l'errore di ampiezza e' in saturazione */
					/* campo troppo alto */	
                {
                if ( auto_verbose ) printf ("**automagico: amplitude error = %d  OSCILLA !!!!\n", amplitude_error);
		next_to_do = 1;
                to_do = 5;
                }




	if (auto_oscill_onoff)
        oscill_interval = XtAppAddTimeOut(XtWidgetToApplicationContext(bulletinBoard), 500, oscillazione, NULL);

	loop_counter++;
        break;



	case 1:
	
	/*termino il loop */
        auto_oscill_cbk();
	if ( auto_verbose ) printf ("automagico: prima procedura terminata\n");



	default:





	break;

	case 2:
	/* ricerca dell'oscillazione */
	if ( auto_verbose ) printf ("automagico: stato = %d  loop = %d\n", to_do, loop_counter);

	loop[sottop_selez] = a_loop_ph ;

if ( sottop_tipo[sottop_selez] == 0 )
        {
        /* BUNCHER */
        XmScrollBarSetValues (scrollBar16, a_loop_ph, 19, 1, 10, TRUE);
	if ( auto_verbose ) printf ("automagico: ho impostato ph = %d\n", a_loop_ph);
        }

else
        {
        /* POSTCHOPPER */
	/*XmScrollBarSetValues (widget, value, slider_size, increment, page_increment, notify)*/
        XmScrollBarSetValues (scrollBar7, a_loop_ph, 19, 1, 10, TRUE);
	if ( auto_verbose ) printf ("automagico: ho impostato ph = %d\n", a_loop_ph);
        }



        strcpy(cmd,"!AMP?");

     /* write to server */
        if (  write(rf_fildes[sottop_selez],cmd,sizeof(cmd)) < 0 )
            {
            perror( "\nclient: error writing to s streamsocket\n" ) ;
            }
        if (net_verbose) printf (" scritto %s su   %d\n", cmd, rf_fildes[sottop_selez]);

     /* listen to server */
        if ( read( rf_fildes[sottop_selez], frame_to_r, nb_to_r ) < 0 )
            {
            perror( "\nclient: error reading from s streamsocket\n" ) ;
            }
        if (net_verbose) printf (" letto %s da   %d\n", frame_to_r, rf_fildes[sottop_selez]);

	amplitude_error  = new_amplitude_error;

        new_amplitude_error = atoi(frame_to_r);

	if ( auto_verbose ) printf ("automagico:     ampl. err. = %d\n", amplitude_error);
	if ( auto_verbose ) printf ("automagico: new ampl. err. = %d  \n", new_amplitude_error);


        if ( new_amplitude_error < 20 ) /* sono andato in saturazione */
                {
		next_to_do = 2;
		to_do = 5;
		loop_for_min = a_loop_ph;
		index_for_min = ph_index;
		min_from_sat = 1;
		oscill_is_on = 1;
		}
	else
		{	
		ampl_err[ph_index] = new_amplitude_error ;
		loop_phase[ph_index] = a_loop_ph;
		ph_max_index = ph_index;
		ph_index++;
		a_loop_ph = a_loop_ph + phase_step;
		if (a_loop_ph > phase_stop ) 
			{ /*****/
			/* ho finito la spazzolata */
			if ( auto_verbose ) 
				{
				printf ("automagico: ho finito la pazzolata\n");
				for (ind = 0 ; ind <= ph_max_index ; ind++) 
				printf ("ind = %d ampl_err = %d loop_phase = %d\n", ind, ampl_err[ind], loop_phase[ind]);
				}

		
                         if ( min_from_sat == 0 )
                                {
				oscill_is_on = 0;
				min_ampl_err = 4095;
       		                for (ind = 0 ; ind <= ph_max_index ; ind++)
					{
					if (ampl_err[ind] < 4090) oscill_is_on = 1;
					if (ampl_err[ind] < min_ampl_err ) 
						{
						min_ampl_err = ampl_err[ind];
						index_for_min1 = ind;
						}
					}
                                loop[sottop_selez] = loop_phase[index_for_min1] ;
                                if ( auto_verbose ) printf ("automagico: considerare min1 (massimo) %d \n", loop[sottop_selez]);
                                }
                         else
                                {
                                loop[sottop_selez] = loop_phase[index_for_min] ;
                                if ( auto_verbose ) printf ("automagico: considerare min (saturazione) %d \, loop[sottop_selez]n");
                                }




			if ( oscill_is_on == 0 )
				{
				/* non ha mai oscillato !! */
				if ( auto_verbose ) printf ("automagico:non ha mai oscillato !!\n");
				to_do = 4 ;
				}

			else
				{
				if ( auto_verbose ) printf ("automagico: ho analizzato %d punti\n", ph_max_index+1);
				if ( auto_verbose ) printf ("automagico: il max e' a %d gradi\n", loop_phase[index_for_min]);
				if ( auto_verbose ) printf ("automagico: il max e' a %d gradi\n", loop_phase[index_for_min1]);



				if (secondo_giro == 1 )
					{
					if ( auto_verbose ) printf ("\n\n\nora faccio il secondo giro !!\n\n\n");
					a_loop_ph = loop[sottop_selez] - 30 ;
					secondo_giro = 0;
					phase_step = 5;
					phase_stop = loop[sottop_selez] + 30 ;
                                	ph_index = 0;
					ph_max_index = 0;
					index_for_min = 0;
					min_from_sat = 0;
                                	if ( auto_verbose ) printf ("con start = %d stop = %d step = %d\n\n", a_loop_ph, phase_stop, phase_step);
					to_do = 2;
				
					}
				else
					{
					to_do = 3;
					if ( auto_verbose ) printf ("ho trovato il picco\n");
					/* devo porre il loop phase sul picco */

	                                if ( sottop_tipo[sottop_selez] == 0 )
       		                                 {
       		                                 /* BUNCHER */
       		                                 XmScrollBarSetValues (scrollBar16, loop[sottop_selez], 19, 1, 10, TRUE);
       		                                 if ( auto_verbose ) printf ("automagico: ho impostato ph = %d\n", loop[sottop_selez]);
       		                                 }

       		                         else
       		                                 {
       		                                 /* POSTCHOPPER */
       		                                 XmScrollBarSetValues (scrollBar7, loop[sottop_selez], 19, 1, 10, TRUE);
       		                                 if ( auto_verbose ) printf ("automagico: ho impostato ph = %d\n", loop[sottop_selez]);
       		                                 }



					}
				}
			} /*****/
		}

	if (ricerca_picco_onoff)
        oscill_interval = XtAppAddTimeOut(XtWidgetToApplicationContext(bulletinBoard), 200, oscillazione, NULL);

	loop_counter++;
	if ( auto_verbose ) printf ("automagico: esco con to_do = %d  \n", to_do);
	break;


	case 3:

	if ( auto_verbose ) printf ("automagico: seconda procedura terminata\n");
	ricerca_picco_cbk();

	break;


	case 4:

        if ( sottop_tipo[sottop_selez] == 0 )
                {
                /* BUNCHER */
		if ( power[sottop_selez] < 450 ) 
			{
			/* aumento il quiescent power */
			if ( auto_verbose ) printf ("automagico: aumento quiescent power di 4\n");
			power[sottop_selez] = power[sottop_selez] + 20 ;
                	XmScrollBarSetValues (scrollBar17, power[sottop_selez], 26, 1, 10, TRUE);
			}
		else
			{
			/* diminuisco attenuazione */
			if (attenuation[sottop_selez] < 32)
				{
				attenuation[sottop_selez] = attenuation[sottop_selez] + 1;
				if ( auto_verbose ) printf ("automagico: diminuisco attenuazione di 1\n");
				XmScrollBarGetValues (scrollBar18, &value, &slider_size, &increment, &page_increment);
                		XmScrollBarSetValues (scrollBar18, attenuation[sottop_selez],slider_size, increment, page_increment, TRUE);
				}
			else
				{
				/* la procedura e' finita */
				tutto_bene = 0;
				start_all_cbk();
				ricerca_picco_cbk();
				}
			}
                }

        else
                {
                /* POSTCHOPPER */
		if ( auto_verbose ) printf ("automagico: aumento quiescent power di 4\n");
		if ( power[sottop_selez] < 480 ) 
			{
			power[sottop_selez] = power[sottop_selez] + 20 ;
                	XmScrollBarSetValues (scrollBar6, power[sottop_selez], 26, 1, 10, TRUE);
			}
                else
                        {
                        /* la procedura e' finita */
                        tutto_bene = 0;
                        start_all_cbk();
                        ricerca_picco_cbk();
                        }
                }


	to_do = 2;
	amplitude_error = 4095 ;
	new_amplitude_error = 4095 ;
	phase_step=30;
	phase_start = 1;
	phase_stop = 360;
	ph_index = 0;
	a_loop_ph = phase_start;
	secondo_giro = 1;
	min_from_sat = 0;

        oscill_interval = XtAppAddTimeOut(XtWidgetToApplicationContext(bulletinBoard), 5, oscillazione, NULL);



	break;


	case 5:

        if ( auto_verbose ) printf ("automagico: stato = %d  loop = %d\n", to_do, loop_counter);
        if ( auto_verbose ) printf ("automagico: diminuisco quiescent power di uno\n");


        if ( sottop_tipo[sottop_selez] == 0 )
                {
                /* BUNCHER */
                if ( power[sottop_selez] > 100 )
                        {
                        /* diminuisco il quiescent power */
                        if ( auto_verbose ) printf ("automagico: diminuisco quiescent power di 1\n");
                        power[sottop_selez] = power[sottop_selez] - 1 ;
                        XmScrollBarSetValues (scrollBar17, power[sottop_selez], 26, 1, 10, TRUE);
                        to_do = 3;
                        }
                else
                        {
                        /* aumento attenuazione */
                        if (attenuation[sottop_selez] > 1)
                                {
                                attenuation[sottop_selez] = attenuation[sottop_selez] - 1;
                                if ( auto_verbose ) printf ("automagico: diminuisco attenuazione di 1\n");
                                XmScrollBarGetValues (scrollBar18, &value, &slider_size, &increment, &page_increment);
                                XmScrollBarSetValues (scrollBar18, attenuation[sottop_selez],slider_size, increment, page_increment, TRUE);
                        	to_do = 3;
                                }
                        else
                                {
                                /* la procedura e' finita */
                                tutto_bene = 0;
                                start_all_cbk();
                                ricerca_picco_cbk();
                                }

                        }
                }

        else
                {
                /* POSTCHOPPER */
                if ( auto_verbose ) printf ("automagico: diminuisco quiescent power di 1\n");
                if ( power[sottop_selez] > 1 ) 
			{
			power[sottop_selez] = power[sottop_selez] - 1 ;
                	XmScrollBarSetValues (scrollBar6, power[sottop_selez], 26, 1, 10, TRUE);
			}
		else
			{
                        /* la procedura e' finita */
                        tutto_bene = 0;
                        start_all_cbk();
                        ricerca_picco_cbk();
			}
                }











        strcpy(cmd,"!AMP?");

     /* write to server */
        if (  write(rf_fildes[sottop_selez],cmd,sizeof(cmd)) < 0 )
            {
            perror( "\nclient: error writing to s streamsocket\n" ) ;
            }
        if (net_verbose) printf (" scritto %s su   %d\n", cmd, rf_fildes[sottop_selez]);

     /* listen to server */
        if ( read( rf_fildes[sottop_selez], frame_to_r, nb_to_r ) < 0 )
            {
            perror( "\nclient: error reading from s streamsocket\n" ) ;
            }
        if (net_verbose) printf (" *letto %s da   %d\n", frame_to_r, rf_fildes[sottop_selez]);

        amplitude_error = atoi(frame_to_r);

        to_do= next_to_do;


        if ( amplitude_error < 20 ) /* l'errore di ampiezza e' in saturazione */
                                        /* campo troppo alto */
                {
                if ( auto_verbose ) printf ("**automagico: amplitude error = %d  OSCILLA !!!!\n", amplitude_error);
                to_do = 5;
                }

 
        if (ricerca_picco_onoff || auto_oscill_onoff)
        oscill_interval = XtAppAddTimeOut(XtWidgetToApplicationContext(bulletinBoard), 500, oscillazione, NULL);

	break;


	case 7:
	if ( auto_verbose ) printf ("tuning\n");

        strcpy(cmd,"!FREQ?");

         /* write to server */
         if (  write(rf_fildes[sottop_selez],cmd,sizeof(cmd)) < 0 )
                 perror( "\nclient: error writing to s streamsocket\n" ) ;
                 if (net_verbose) printf (" scritto %s su   %d\n", cmd, rf_fildes[sottop_selez]);

         /* listen to server */
         if ( read( rf_fildes[sottop_selez], frame_to_r, nb_to_r ) < 0 )
                 perror( "\nclient: error reading from s streamsocket\n" ) ;
         if (net_verbose) printf (" letto %s da   %d\n", frame_to_r, rf_fildes[sottop_selez]);

	 old_frequency_error = 2048 ;

         new_frequency_error = 2048 - atoi(frame_to_r);

	 frequency_error = new_frequency_error ;

        if ( auto_verbose ) printf ("automagico: err. di freq = %d\n", new_frequency_error);
        if ( auto_verbose ) printf ("slow = %d fast = %d fast_fast = %d\n", tu_step_slow[sottop_selez], tu_step_fast[sottop_selez], tu_step_fast_fast[sottop_selez]);


	if ( new_frequency_error > 2046 )
		{
		/* errore di frequenza saturo */
		to_do = 10 ;
		}
else

	to_do = 8;

	tu_direction = 1 ;
	ha_diminuito = 0 ;
	tu_fa_sl = 500;
	/*steps = tu_step_fast[sottop_selez];*/
	if ( 100 *  frequency_error / 2048.0 > 5.0 )
		steps = (int)(tu_step_fast_fast[sottop_selez]/2.0);
	else
		steps = (int)(tu_step_fast[sottop_selez]/2.0);

        if (auto_tuning_onoff)
        oscill_interval = XtAppAddTimeOut(XtWidgetToApplicationContext(bulletinBoard), 500, oscillazione, NULL);


	break;


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

        if ( auto_verbose ) printf ("automagico: old err. di freq = %d  %f KHz\n", old_frequency_error, old_frequency_error * 100.0 / 2048.0);
        if ( auto_verbose ) printf ("automagico:     err. di freq = %d  %f KHz\n", frequency_error, frequency_error * 100.0 / 2048.0);
        if ( auto_verbose ) printf ("automagico: new err. di freq = %d  %f KHz\n", new_frequency_error, new_frequency_error * 100.0 / 2048.0);



	if (new_frequency_error <= frequency_error)
		{
		/* l'errore di freq. non e' aumentato */
		if ( auto_verbose ) printf ("automagico: l'errore di freq. non e' aumentato\n");
		if (new_frequency_error * 100.0 / 2048.0 < end_fr_corr[sottop_selez])
			{
			/* ho raggiunto la soglia, termino il processo */
                	if ( auto_verbose ) printf ("automagico: ho raggiunto la soglia\n");
                	to_do = 9 ;
                	}
		else
			{
			/* non ho raggiunto la soglia, ripeto un passo eguale a prima */
                	if ( auto_verbose ) printf ("automagico: non ho raggiunto la soglia\n");
			to_do = 8;
			}
		}

	else
		{
		/* l'errore di frequenza e' aumentato */
		if ( auto_verbose ) printf ("automagico: l'errore di freq. e' aumentato\n");
		if ( steps >= 2)
			{
			/* posso ancora dimezzare il numero dei passi */
			steps = (int)(steps/2.0);
                	if ( auto_verbose ) printf ("automagico: dimezzo numero passi %d\n", steps);
			tu_direction = tu_direction * -1 ;
			to_do = 8;
			}
		else
			{
			/* non posso piu' dimezzare il numero dei passi */
                	if ( auto_verbose ) printf ("automagico: non posso piu' dimezzare il numero dei passi\n");
			to_do = 9 ;
			}
		}
	}


        if (auto_tuning_onoff)
        oscill_interval = XtAppAddTimeOut(XtWidgetToApplicationContext(bulletinBoard), 500, oscillazione, NULL);

	break;


	case 9:

        if ( auto_verbose ) printf ("automagico: pocedura tuning terminata\n");

	tuning_cbk();

	break;


	case 10:

        /* prova a fare un passo */
        sprintf(stringa,"!%1d, %d, %d",motor[sottop_selez], tu_direction * tu_step_fast_fast[sottop_selez], 400);


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

        	to_do = 10 ;
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

         	new_frequency_error = 2048 - atoi(frame_to_r);
        if ( auto_verbose ) printf ("automagico: new err. di freq = %d  %f KHz\n", new_frequency_error, new_frequency_error * 100.0 / 2048.0);

        	if ( new_frequency_error > 2046 )
                	{
                	/* errore di frequenza saturo */
                	to_do = 10 ;
                	}
		else
			to_do = 7;

		}



        if (auto_tuning_onoff)
        oscill_interval = XtAppAddTimeOut(XtWidgetToApplicationContext(bulletinBoard), 500, oscillazione, NULL);




	break;


	case 11:


        strcpy(cmd,"!AMP?");

     /* write to server */
        if (  write(rf_fildes[sottop_selez],cmd,sizeof(cmd)) < 0 )
            {
            perror( "\nclient: error writing to s streamsocket\n" ) ;
            }
        if (net_verbose) printf (" scritto %s su   %d\n", cmd, rf_fildes[sottop_selez]);

     /* listen to server */
        if ( read( rf_fildes[sottop_selez], frame_to_r, nb_to_r ) < 0 )
            {
            perror( "\nclient: error reading from s streamsocket\n" ) ;
            }
        if (net_verbose) printf (" *letto %s da   %d\n", frame_to_r, rf_fildes[sottop_selez]);

        amplitude_error = atoi(frame_to_r);

        to_do = 99;

        if ( amplitude_error <= 2300 && amplitude_error >= 1800 ) /* l'errore di ampiezza e' minore della soglia */
                {
                if ( auto_verbose ) printf ("*automagico: amplitude error = %\n", amplitude_error);
                to_do = 12;
                }

        if ( amplitude_error < 1800 && amplitude_error >= 500 ) /* campo troppo alto */
                {
                if ( auto_verbose ) printf ("automagico: amplitude error = %d diminuisco q.p.\n", amplitude_error);
        	if ( power[sottop_selez] > 0 ) power[sottop_selez] = power[sottop_selez] - 1 ;
                to_do = 11;
                }

        if ( amplitude_error < 500 ) /* campo troppo alto */
                {
                if ( auto_verbose ) printf ("automagico: amplitude error = %d diminuisco q.p.\n", amplitude_error);
                if ( power[sottop_selez] > 0 ) power[sottop_selez] = power[sottop_selez] - 4 ;
                to_do = 11;
                }


        if ( amplitude_error > 2300 && amplitude_error <= 3500) /* campo troppo basso */
                {
                if ( auto_verbose ) printf ("automagico: amplitude error = %d aumento q.p.\n", amplitude_error);
        	if ( power[sottop_selez] < 500 ) power[sottop_selez] = power[sottop_selez] + 1 ;
                to_do = 11;
                }

        if ( amplitude_error > 3500) /* campo troppo basso */
                {
                if ( auto_verbose ) printf ("automagico: amplitude error = %d aumento q.p.\n", amplitude_error);
                if ( power[sottop_selez] < 500 ) power[sottop_selez] = power[sottop_selez] + 4 ;
                to_do = 11;
                }


        if ( sottop_tipo[sottop_selez] == 0 )
        	{
        	/* BUNCHER */
        	XmScrollBarSetValues (scrollBar17, power[sottop_selez], 26, 1, 10, TRUE);
        	}

	else
        	{
        	/* POSTCHOPPER */
        	XmScrollBarSetValues (scrollBar6, power[sottop_selez], 26, 1, 10, TRUE);
        	}

        if (auto_zeroampl_onoff)
        oscill_interval = XtAppAddTimeOut(XtWidgetToApplicationContext(bulletinBoard), 300, oscillazione, NULL);

	break;



        case 12:

        if ( auto_verbose ) printf ("automagico: pocedura azzeramento err. amp. terminata\n");

        ampl_zero_cbk();

        break;



	}
}


