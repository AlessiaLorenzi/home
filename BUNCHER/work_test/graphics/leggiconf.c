/*
 * Motif required Headers
 */
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#if (XmVersion >= 1002)
#include <Xm/RepType.h>
#endif
#include <Xm/MwmUtil.h>


#include <sys/types.h>l
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#include "defs.h"
#include "w_defs.h"



leggiconf()
{


FILE *dbfile;
FILE *device_file;
FILE *calibra_file;
FILE *ph_table_pt;
char line[200];
char file_name[200];
int index;
int sub_index;
int counter, ordinata;
float ascissa;



strcpy(file_name, "configurazione/");
strcat (file_name, "conf");


dbfile = fopen ( file_name, "r" );
if ( dbfile == NULL )
        {
        printf ( "leggiconf : error opening configuration file\n");
        fflush ( stdout );
        exit ( 2 );
        }
else
	if (open_verbose) printf ("\n   ----  configurazione in %s\n", file_name);

index = 0;
sub_index = 0;
max_sp = 0;
max_pages=0;

while ( fgets ( line, 100, dbfile) != NULL )
        {
	sscanf(line, "%s %s %d", nome_dispositivo[index], tipo_dispositivo[index], &(w_index[index]));
	if (open_verbose)  printf ( "\nnome = %s tipo = %s\n", nome_dispositivo[index], tipo_dispositivo[index]);

	strcpy(file_name, "configurazione/");
	strcat (file_name, nome_dispositivo[index]);

        if (strcmp(tipo_dispositivo[index], "postchopper") == 0 ) 
		{
		device_file = fopen ( file_name, "r" );
		if ( device_file == NULL )
		        {
		        printf ( "leggiconf : error opening configuration file\n");
		        fflush ( stdout );
		        exit ( 2 );
		        }
		else
			{
			if (open_verbose) printf ("\n   ----  ho aperto  %s, sub_index = %d\n", file_name, sub_index);
			sub_page_index[index] = sub_index;
			padre[sub_index] = index;

			/* assegnazione del nome del file di config */
			strcpy(nome_file_config[sub_index], nome_dispositivo[index]);

			/* lettura del file di configurazione */

                        /* riga relativa al nome del dispositivo */
			fgets(etichetta[sub_index], 100, device_file);

			/* nome del file di calibrazione */
			fgets ( line, 100, device_file);
			sscanf(line, "%s", nome_calibr[sub_index]);

			/* costanti per il calcolo del campo rf */
			fgets ( line, 100, device_file);
			sscanf(line, "%f %f %f %f", &(kappa[sub_index]), &(min_field[sub_index]), &(max_field[sub_index]), &(atten_field[sub_index]) );

			/* host su cui gira il server rf e numero porta */
			fgets ( line, 100, device_file);
			sscanf(line, "%s %d", rf_host[sub_index], &(rf_socket[sub_index]) );

			/* host, numero porta, numero motore tuning */
			fgets ( line, 100, device_file);
			sscanf(line, "%s %d %d", za_host[sub_index], &(za_socket[sub_index]), &(motor[sub_index]) );

			/* numero porta parallela, numero bit amplificatore */
			fgets ( line, 100, device_file);
                        sscanf(line, "%s %d %d %d", za_ampli_host[sub_index], &(za_ampli_socket[sub_index]), &(za_par_io_port[sub_index]), &(za_ampl_bit[sub_index]) );
			
			/* numero passi slow tuning numero passi fast tuning */
			fgets ( line, 100, device_file);
			sscanf(line, "%d %d %d %f", &(tu_step_slow[sub_index]) , &(tu_step_fast[sub_index]), &(tu_step_fast_fast[sub_index]), &(end_fr_corr[sub_index]));
			
			/* soglia intervento zero amplitude error e variaz. quiesc. pow. */
			fgets ( line, 100, device_file);
			sscanf(line, "%d %d", &(zero_a_soglia[sub_index]), &(zero_a_step[sub_index]));
			
			/* soglia intervento zero frequency error e num. passi inviati al motore */
			fgets ( line, 100, device_file);
			sscanf(line, "%d %d", &(zero_f_soglia[sub_index]), &(zero_f_step[sub_index]));
			
			/* porta fase postchopper */
			fgets ( line, 100, device_file);
			sscanf(line, "%d", &(postchopper_ph_port[index]));

			/* variabili cursori paginette piccole */
                        fgets ( line, 100, device_file);
                        sscanf(line, "%d %f", &(amp_err_min[sub_index]), &(max_var_field[sub_index]));
	
			fclose ( device_file ) ;
			if (open_verbose) printf ("\n   ----  ho chiuso  %s\n", file_name);
			sottop_tipo[sub_index] = 1;
			sub_index++;
			}
		}

        if (strcmp(tipo_dispositivo[index], "buncher") == 0 ) 
		{
		strcpy(file_name, "configurazione/");
		strcat (file_name, nome_dispositivo[index]);
		strcat (file_name, "_a" );
		device_file = fopen ( file_name, "r" );
		if ( device_file == NULL )
		        {
		        printf ( "leggiconf : error opening configuration file\n");
		        fflush ( stdout );
		        exit ( 2 );
		        }
		else
			{
			if (open_verbose) printf ("\n   ----  ho aperto  %s, sub_index = %d\n", file_name, sub_index);
			sub_page_index[index] = sub_index;
			padre[sub_index] = index;

			/* assegnazione del nome del file di config */

			strcpy(nome_file_config[sub_index], nome_dispositivo[index]);
			strcat (nome_file_config[sub_index], "_a" );


			/* lettura del file di configurazione */

                        /* riga relativa al nome del dispositivo */
			fgets(etichetta[sub_index], 100, device_file);

			/* nome del file di calibrazione */
			fgets ( line, 100, device_file);
			sscanf(line, "%s", nome_calibr[sub_index]);

			/* costanti per il calcolo del campo rf */
			fgets ( line, 100, device_file);
			sscanf(line, "%f %f %f %f", &(kappa[sub_index]), &(min_field[sub_index]), &(max_field[sub_index]), &(atten_field[sub_index]) );

			/* host su cui gira il server rf e numero porta */
			fgets ( line, 100, device_file);
			sscanf(line, "%s %d", rf_host[sub_index], &(rf_socket[sub_index]) );

			/* host, numero porta, numero motore tuning */
			fgets ( line, 100, device_file);
			sscanf(line, "%s %d %d", za_host[sub_index], &(za_socket[sub_index]), &(motor[sub_index]) );

			/* numero porta parallela, numero bit amplificatore */
			fgets ( line, 100, device_file);
                        sscanf(line, "%s %d %d %d", za_ampli_host[sub_index], &(za_ampli_socket[sub_index]), &(za_par_io_port[sub_index]), &(za_ampl_bit[sub_index]) );
			
			/* numero passi slow tuning numero passi fast tuning */
			fgets ( line, 100, device_file);
			sscanf(line, "%d %d %d %f", &(tu_step_slow[sub_index]) , &(tu_step_fast[sub_index]), &(tu_step_fast_fast[sub_index]), &(end_fr_corr[sub_index]));
			
			/* soglia intervento zero amplitude error e variaz. quiesc. pow. */
			fgets ( line, 100, device_file);
			sscanf(line, "%d %d", &(zero_a_soglia[sub_index]), &(zero_a_step[sub_index]));
			
			/* soglia intervento zero frequency error e num. passi inviati al motore */
			fgets ( line, 100, device_file);
			sscanf(line, "%d %d", &(zero_f_soglia[sub_index]), &(zero_f_step[sub_index]));

                        /* variabili cursori paginette piccole */
                        fgets ( line, 100, device_file);
                        sscanf(line, "%d %f", &(amp_err_min[sub_index]), &(max_var_field[sub_index]));




			fclose ( device_file ) ;
			if (open_verbose) printf ("\n   ----  ho chiuso  %s\n", file_name);
			sottop_tipo[sub_index] = 0;
			is_the_second[sub_index] = 0;
			sub_index++;
			}

		strcpy(file_name, "configurazione/");
		strcat (file_name, nome_dispositivo[index]);
		strcat (file_name, "_b" );
		device_file = fopen ( file_name, "r" );
		if ( device_file == NULL )
		        {
		        printf ( "leggiconf : error opening configuration file\n");
		        fflush ( stdout );
		        exit ( 2 );
		        }
		else
			{
			if (open_verbose) printf ("\n   ----  ho aperto  %s, sub_index = %d\n", file_name, sub_index);
			padre[sub_index] = index;

			/* lettura del file di configurazione */

			/* assegnazione del nome del file di config */

			strcpy(nome_file_config[sub_index], nome_dispositivo[index]);
			strcat (nome_file_config[sub_index], "_b" );

                        /* riga relativa al nome del dispositivo */
			fgets(etichetta[sub_index], 100, device_file);
			
			/* nome del file di calibrazione */
			fgets ( line, 100, device_file);
			sscanf(line, "%s", nome_calibr[sub_index]);

			/* costanti per il calcolo del campo rf */
			fgets ( line, 100, device_file);
			sscanf(line, "%f %f %f %f", &(kappa[sub_index]), &(min_field[sub_index]), &(max_field[sub_index]), &(atten_field[sub_index]) );

			/* host su cui gira il server rf e numero porta */
			fgets ( line, 100, device_file);
			sscanf(line, "%s %d", rf_host[sub_index], &(rf_socket[sub_index]) );

			/* host, numero porta, numero motore tuning */
			fgets ( line, 100, device_file);
			sscanf(line, "%s %d %d", za_host[sub_index], &(za_socket[sub_index]), &(motor[sub_index]) );

			/* numero porta parallela, numero bit amplificatore */
			fgets ( line, 100, device_file);
                        sscanf(line, "%s %d %d %d", za_ampli_host[sub_index], &(za_ampli_socket[sub_index]), &(za_par_io_port[sub_index]), &(za_ampl_bit[sub_index]) );
			
			/* numero passi slow tuning numero passi fast tuning */
			fgets ( line, 100, device_file);
			sscanf(line, "%d %d %d %f", &(tu_step_slow[sub_index]) , &(tu_step_fast[sub_index]), &(tu_step_fast_fast[sub_index]), &(end_fr_corr[sub_index]));
			
			/* soglia intervento zero amplitude error e variaz. quiesc. pow. */
			fgets ( line, 100, device_file);
			sscanf(line, "%d %d", &(zero_a_soglia[sub_index]), &(zero_a_step[sub_index]));
			
			/* soglia intervento zero frequency error e num. passi inviati al motore */
			fgets ( line, 100, device_file);
			sscanf(line, "%d %d", &(zero_f_soglia[sub_index]), &(zero_f_step[sub_index]));

                        /* variabili cursori paginette piccole */
                        fgets ( line, 100, device_file);
                        sscanf(line, "%d %f", &(amp_err_min[sub_index]), &(max_var_field[sub_index]));



			fclose ( device_file ) ;
			if (open_verbose) printf ("\n   ----  ho chiuso  %s\n", file_name);
			sottop_tipo[sub_index] = 0;
			is_the_second[sub_index] = 1;
			sub_index++;
			}


                strcpy(file_name, "configurazione/");
                strcat (file_name, nome_dispositivo[index]);
                strcat (file_name, "_t" );
                device_file = fopen ( file_name, "r" );
                if ( device_file == NULL )
                        {
                        printf ( "leggiconf : error opening configuration file\n");
                        fflush ( stdout );
                        exit ( 2 );
                        }
                else
                        {
                        if (open_verbose) printf ("\n   ----  ho aperto  %s, sub_index = %d\n", file_name, sub_index);

                        /* lettura del file di configurazione */

                        /* host su cui gira il server dei tubi e numero socket */
                        fgets ( line, 100, device_file);
                        sscanf(line, "%s %d", tubi_host[index], &(tubi_socket[index]) );

                        /* numero porta parallela, numero bit tubi */
                        fgets ( line, 100, device_file);
                        sscanf(line, "%d %d",  &(tubi_par_io_port[index]), &(tubi_bit[index]) );

			/* numero porta fase buncher */
                        fgets ( line, 100, device_file);
                        sscanf(line, "%d",  &(buncher_ph_port[index]) );


                        fclose ( device_file ) ;
                        if (open_verbose) printf ("\n   ----  ho chiuso  %s\n", file_name);
                        }

		}

	index++;
	max_pages++;
	}

max_sp =  sub_index;

if (debug_verbose) printf ("--------------------- ho trovato %d sottopagine e %d pagine\n", max_sp, max_pages);


if (debug_verbose) for (index = 0 ; index < 20 ; index++)
			{
			printf("\n");
			printf("%2d %12s  ++   %12s ", index, nome_dispositivo[index], tipo_dispositivo[index]);
			printf("con sottopagine ");
        		if (strcmp(tipo_dispositivo[index], "buncher") == 0 ) 
				printf("%d %d \n", sub_page_index[index], sub_page_index[index] +1);
        		if (strcmp(tipo_dispositivo[index], "postchopper") == 0 ) 
				printf("%d \n", sub_page_index[index]);
			}



fclose(dbfile);

if (open_verbose) printf ("\n   ----  ho chiuso %s\n", file_name);


if (debug_verbose) for (index = 0 ; index < max_sp ; index++)
			{
			printf("\n%d	%s", index, etichetta[index]);
			printf("	%s\n", nome_calibr[index]);
			printf("	%s %d\n", rf_host[index], rf_socket[index]);
			printf("	%s %d %d\n", za_host[index], za_socket[index], motor[index]);
			printf("	%s %d %d %d\n", za_ampli_host[index], za_ampli_socket[index], za_par_io_port[index], za_ampl_bit[index]);
			}


		/* cavita' passiva */

                strcpy(file_name, "configurazione/");
                strcat (file_name, "passiva" );
                device_file = fopen ( file_name, "r" );
                if ( device_file == NULL )
                        {
                        printf ( "leggiconf : error opening configuration file\n");
                        fflush ( stdout );
                        exit ( 2 );
                        }
                else
                        {
                        if (open_verbose) printf ("\n   ----  ho aperto  %s, sub_index = %d\n", file_name, sub_index);

                        /* lettura del file di configurazione */

                        /* host su cui gira il server del controller rf della cavita' passiva e numero socket */
                        fgets ( line, 100, device_file);
                        sscanf(line, "%s %d", phase_host, &(phase_socket) );

                        /* host su cui gira il server del tuning della cavita' passiva e numero socket */
                        fgets ( line, 100, device_file);
                        sscanf(line, "%s %d", phase_tuning_host, &(phase_tuning_socket) );

                        /* porta ritardo generale */
                        fgets ( line, 100, device_file);
                        sscanf(line, "%d", &(general_phase_port) );

			/* numero motore tuning */
			fgets ( line, 100, device_file);
			sscanf(line, "%d", &(phase_motor) );

			/* host su cui gira il server del selettore di fase e numero socket */
                        fgets ( line, 100, device_file);
                        sscanf(line, "%s %d", le_host, &(le_socket) );



                        fclose ( device_file ) ;
                        if (open_verbose) printf ("\n   ----  ho chiuso  %s\n", file_name);
                        }





for (index = 0 ; index < max_sp ; index++)
	{
	calibra_file = fopen(nome_calibr[index], "r");
	if(calibra_file == NULL )
		{
		perror("non sono riuscito ad aprire il file di calibrazione \n");
		exit(0);
		}
	for(counter=0;counter<153;counter++)
		{
		fscanf(calibra_file,"%f %d \n", &ascissa, &ordinata);
		ascisse[counter][index]=ascissa;
		ordinate[counter][index]=ordinata;
		}
	}

if (calibr_verbose) for (index = 0 ; index < max_sp ; index++)
	{
	printf ("\n\n");
        for(counter=0;counter<153;counter++)
                {
                printf("%f %d \n", ascisse[counter][index], ordinate[counter][index]);
		}
	}






strcpy(file_name, "configurazione/");
strcat (file_name, "chopper");




	device_file = fopen ( file_name, "r" );
	if ( device_file == NULL )
	        {
	        printf ( "leggiconf : error opening configuration file\n");
	        fflush ( stdout );
	        exit ( 2 );
	        }
	else
		{
		if (open_verbose) printf ("\n   ----  ho aperto  %s per il chopper !!!!!!!!!!!!!!!!!\007\n", file_name);
		sub_page_index[index] = sub_index;

		/* lettura del file di configurazione chopper */

		/* host su cui gira il server chopper e numero socket */
		fgets ( line, 100, device_file);
		sscanf(line, "%s %d", chopper_host, &chopper_socket );

		/* numero porta parallela, numero bit comando on/off */
		fgets ( line, 100, device_file);
                sscanf(line, "%d %d", &chopper_par_io_port, &chopper_onoff_bit);

		/* numero bit comando int/ext */
		fgets ( line, 100, device_file);
                sscanf(line, "%d", &chopper_intext_bit);

		/* numero porta comando multiplate delay */
		fgets ( line, 100, device_file);
                sscanf(line, "%d", &chopper_m_d_port);


		/* numero porta comando width */
		fgets ( line, 100, device_file);
                sscanf(line, "%d", &chopper_p_w_port);
			

		/* numero porta comando duty cycle */
		fgets ( line, 100, device_file);
                sscanf(line, "%d", &chopper_d_c_port);

		/* numero porta fast pulsing slow pulsing */
		fgets ( line, 100, device_file);
                sscanf(line, "%d", &chopper_fp_sp_port);
 

                /* numero bit comando fast pulsing slow pulsing */
                fgets ( line, 100, device_file);
                sscanf(line, "%d", &chopper_spfp_bit);

		/* numero porta fase chopper */
		fgets ( line, 100, device_file);
                sscanf(line, "%d", &chopper_phase_port);

		fclose ( device_file ) ;
		}



ph_table_pt = fopen("ph_table","r");
if(ph_table_pt == NULL )
{
perror("non sono riuscito ad aprire il file di calibrazione della fase \n");
exit(0);
}


for(counter=0;counter<4096;counter++)
{
fscanf(ph_table_pt,"%f \n", &(table[counter]));
}



}
