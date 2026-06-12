#define FALSE            0
#define MSGLEN 1024
#define BUF_SIZE 1024
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>

#include "ma_net_const.h"
#include "ma_net_var.h"


main( argc, argv )

int argc ;
char **argv ;

{
int gen_chan , channel_addrlen , main_socket, gen_chan_addrlen;
int counter, trovato;
int not_error, channel[16], chan_addrlen ;
char msg_tosend[MSGLEN], msg_torcv[MSGLEN] ;
struct sockaddr_in chan_addr ;
struct hostent * hp, * gethostbyname() ;
struct sockaddr_in gen_chan_addr, channel_addr;
FILE *dbfile;
int uscita;
char line[200];
char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ] ;
int number, magnet_server_counter, magnet_counter, alim_counter;
int read_retval, alimentatore;
char *command;





char file_daaprire[100];
int giorno_corrente;

float ci, cl, vl;
char s1[20], s2[20] ,s3[20];

FILE *log_file;

time_t t;
struct tm *tempo;


main_socket = atoi( argv[2] ) ;
t = time( (long*) 0);
tempo = localtime (&t);

sprintf(file_daaprire, "./netm_log/GIORNO_%d", tempo->tm_yday);
giorno_corrente = tempo->tm_yday;
log_file = fopen (file_daaprire, "a");

if (log_file != NULL)
{
fprintf(log_file, "START \n");
fflush(log_file);
}

else
printf("errore nella apertura file di log\n");



for ( number = 0 ; number < 100; number++ )
	{	
	magnet[number].magnet = 999999;
	server[number].socket = 999999;
	steerer_server[number].socket = 999999;
	for ( alim_counter = 0; alim_counter<16; alim_counter++)
		server[number].alim[alim_counter] = 999999;
	}


printf ("argc = %d\n", argc);

if ( argc < 2 )
	{
	printf ( "netm: network data base file missing\n");
	fflush ( stdout );
	exit ( 1 );
	}



read_ma_data(argv[1]);



    /* Create a socket towards the client ( as a server ) */
    gen_chan = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
    if ( gen_chan < 0 ) {
        perror( "error opening socket" ) ;
        exit( 0 ) ;
        }

    /* give a name to socket using wildcards */
    gen_chan_addr.sin_family = AF_INET ;
    gen_chan_addr.sin_addr.s_addr = INADDR_ANY ;
    gen_chan_addr.sin_port = htons( main_socket ) ;
    gen_chan_addrlen = sizeof( gen_chan_addr ) ;
    channel_addrlen = sizeof( channel_addr ) ;
    if ( bind( gen_chan, &gen_chan_addr, gen_chan_addrlen ) < 0 ) {
        perror( "error binding socket" ) ;
        exit( 0 ) ;
        }

    /* Find out assigned port number and print it out */
    if ( getsockname( gen_chan, &gen_chan_addr, &gen_chan_addrlen )
         < 0 ) {
        perror( "error getting socket name" ) ;
        exit( 0 ) ;
        }
/*    printf( "\n%s: main socket is %d\n", myname,
            ntohs( gen_chan_addr.sin_port ) ) ;*/


    /* Start accepting connections */
    if ( listen ( gen_chan, 5 ) < 0 ) {
        perror( "error listening socket" ) ;
        exit( 0 ) ;
        }





    do {	/*inizio del main loop */
//printf("inizio loop \n");
        /* accept connection from client */
        main_channel = accept( gen_chan, &gen_chan_addr,
                               &gen_chan_addrlen ) ;
uscita = 1;
do {
/*printf ("top loop \n");*/
    read_retval = read( main_channel, client_msg, BUF_SIZE ) ;
    if ( read_retval <= 0 ) {
        if ( read_retval < 0 )
            perror( "error reading main socket" ) ;
        not_error = FALSE ;
        }
/*
printf ("messaggio ricevuto dalla grafica  ==>> %s\n",client_msg);
*/


if ( strncmp ( client_msg, "ma local: open channels", 23) == 0 )

	{

//	printf ("ora chiamo open_net_channels \n");

	open_net_channels();

//	printf ("ora chiamo open_steerer_channels \n");

	open_steerer_channels();

	}



if ( strncmp ( client_msg, "ma local: close channels", 24) == 0 )
close_net_channels();


if ( strncmp ( client_msg, "ma local: exit", 14) == 0 )
	{
	uscita = 0;
	}


if ( strncmp ( client_msg, "st comm : ", 10) == 0 )
	{
	command = (client_msg+10);
	/*printf ("steerers : comando = %s\n",command);*/
	if ( strncmp ( "ALIM = ", command, 7) == 0)
		{
		/*printf ("decodificata seconda parte\n");*/
		sscanf ( command, "ALIM = %d", &alimentatore);

			
		/*printf ("scrivo sul canale\n");*/
		if ( write(steerer_alim[alimentatore].fildes ,command ,BUF_SIZE ) < 0 ) 
		perror( "\nclient: 10  error writing socket\n" ) ;
		/*printf ("leggo la risposta\n");*/
		if ( read(steerer_alim[alimentatore].fildes ,msg_torcv ,BUF_SIZE ) < 0 ) 
		perror( "\nclient: error reading socket\n" ) ;
		/*printf("steerers : il server risponde ==>> %s  *alim* = %d \n",msg_torcv, alimentatore);	*/


t = time( (long*) 0);
tempo = localtime (&t);
if ( giorno_corrente != tempo->tm_yday )
        {
        fflush(log_file);
        fclose(log_file);
        sprintf(file_daaprire, "./netm_log/GIORNO_%d", tempo->tm_yday);
        giorno_corrente = tempo->tm_yday;
        log_file = fopen (file_daaprire, "a");
        }
if ( strncmp ( msg_torcv, "STUB", 4 ) != 0 )
        {
        sscanf (msg_torcv+2, "%s %f %s %f %s %f", s1, &ci, s2, &cl, s3, &vl);
        if (log_file != NULL)
                fprintf(log_file,"%d %d %s %5.2f  %5.2f %5.2f \n",
	tempo->tm_yday , tempo->tm_hour*3600+tempo->tm_min*60+tempo->tm_sec, 
	command, ci, cl, vl);

        else
                printf("errore nella apertura file di log\n");

        fflush (log_file);
        }



		/*printf(" rispondo alla grafica \n");*/

		if ( write(main_channel ,msg_torcv ,BUF_SIZE ) < 0 ) 
			{
			perror( "\n 11 client: error writing socket\n" ) ;
			not_error = FALSE ;
			}



		}
	
	
	}




if ( strncmp ( client_msg, "ma comm : ", 10) == 0 )
	{
	/*printf ("decodificata prima parte  == > %s\n",client_msg);*/
	command = (client_msg+10);
	/*printf ("comando = %s\n",command);*/
	if ( strncmp ( "ALIM = ", command, 7) == 0)
		{
		/*printf ("decodificata seconda parte\n");*/
		sscanf ( command, "ALIM = %d", &alimentatore);
		/*printf ( "alimentatore = %d\n", alimentatore);*/
/*		printf ("scrivo sul canale %d il comando %s\n", 
		alim[alimentatore].fildes, command);			*/
		/*printf ("scrivo sul canale\n");*/
		if ( write(alim[alimentatore].fildes ,command ,BUF_SIZE ) < 0 ) 
		{
//		printf(" alimentatore = %d   fildes = %d\n", alimentatore, alim[alimentatore].fildes);
		perror( "\n 12 client: error writing socket\n" ) ;
		}
		/*printf ("leggo la risposta\n");*/
		if ( read(alim[alimentatore].fildes ,msg_torcv ,BUF_SIZE ) < 0 ) 
		perror( "\nclient: error reading socket\n" ) ;
/*		printf(" il server risponde ==>> %s\n",msg_torcv);	*/
		/*printf(" rispondo alla grafica \n");*/
		if ( write(main_channel ,msg_torcv ,BUF_SIZE ) < 0 ) 
			{
			perror( "\n 13 client: error writing socket\n" ) ;
			not_error = FALSE ;
			}

		}
	
	
	}

if ( strncmp ( client_msg, "st local: send data", 19) == 0 )

	{
        for (counter=1 ; counter < 100 ; counter++)
                {

                sprintf( device_msg, "%s %d %d %d",
		steerer[counter].name,
		steerer[counter].number,
		steerer[counter].alim1,
		steerer[counter].alim2);


		if ( write(main_channel ,device_msg ,BUF_SIZE ) < 0 ) 
			{
			perror( "\n 14 client: error writing socket\n" ) ;
			not_error = FALSE ;
                	}

		read_retval = read( main_channel, client_msg, BUF_SIZE ) ;


		if ( read_retval <= 0 )
                	{
                	if ( read_retval < 0 )
                   		 perror( "error reading main socket" ) ;
                	not_error = FALSE ;
                	}

		}


        for (counter=0 ; counter < 100 ; counter++)
                {
		sprintf(device_msg,"%d", steerer_alim[counter].mode); 
				
		if ( write(main_channel ,device_msg ,BUF_SIZE ) < 0 ) 
			{
			perror( "\nclient: error writing socket\n" ) ;
			not_error = FALSE ;
			}
		/*printf("trasmesso messaggio n. %d\n",counter);*/
		read_retval = read( main_channel, client_msg, BUF_SIZE ) ;
		/*printf("ricevuto  messaggio n. %d  %s\n",counter,client_msg);*/
    		if ( read_retval <= 0 ) 
			{
			if ( read_retval < 0 )
			perror( "error reading main socket" ) ;
			not_error = FALSE ;
			}

		if (strncmp(client_msg, "record ricevuto",15)==0)
			{
			/*printf("ho trasmesso il record n. %d \n",counter);*/
			}
		}

	}

if ( strncmp ( client_msg, "ma local: send data", 19) == 0 )
	{ /* 1 */
        /*printf ("messaggio ricevuto\n");*/

       sprintf( device_msg, "%d", conta_bottoni);
       if ( write(main_channel ,device_msg ,BUF_SIZE ) < 0 ) {
            perror( "\n 15 client: error writing socket\n" ) ;
            not_error = FALSE ;
                }
    read_retval = read( main_channel, client_msg, BUF_SIZE ) ;

         if ( read_retval <= 0 )
                {
                if ( read_retval < 0 )
                    perror( "error reading main socket" ) ;
                not_error = FALSE ;
                }

        for (counter=1 ; counter < conta_bottoni+1 ; counter++)/* SOSTITUITO */
                {
                sprintf( device_msg, "%d %d %d %d %d %d",
                button[counter].button,
                button[counter].type,
                button[counter].obj1,
                button[counter].obj2,
                button[counter].obj3,
                button[counter].obj4);
              printf ("button = %d messaggio da trasmettere : %s\n",counter, device_msg);
       if ( write(main_channel ,device_msg ,BUF_SIZE ) < 0 ) {
            perror( "\n 15 client: error writing socket\n" ) ;
            not_error = FALSE ;
                }
/*printf("trasmesso messaggio n. %d\n",counter);*/
    read_retval = read( main_channel, client_msg, BUF_SIZE ) ;
/*printf("ricevuto  messaggio n. %d  %s\n",counter,client_msg);*/

         if ( read_retval <= 0 )
                {
                if ( read_retval < 0 )
                    perror( "error reading main socket" ) ;
                not_error = FALSE ;
                }

if (strncmp(client_msg, "record ricevuto",15)==0)
        {
/*      printf("ho trasmesso il record n. %d \n",counter);*/
        }
                }




         for (counter=1 ; counter < 100 ; counter++)
                {

                sprintf(device_msg,"%d %s %d %s %d %d %d %d %s %d %d %d",

                                magnet[counter].magnet,
                                magnet[counter].name,

                                magnet[counter].alim1,
                                magnet[counter].n_alim1,
                                magnet[counter].a_alim1,
                                magnet[counter].cur_alim1,
                                magnet[counter].volt_alim1,

                                magnet[counter].alim2,
                                magnet[counter].n_alim2,
                                magnet[counter].a_alim2,
                                magnet[counter].cur_alim2,
                                magnet[counter].volt_alim2);

       if ( write(main_channel ,device_msg ,BUF_SIZE ) < 0 ) {
            perror( "\n 16 client: error writing socket\n" ) ;
            not_error = FALSE ;
                }
/*printf("trasmesso messaggio n. %d\n",counter);*/
    read_retval = read( main_channel, client_msg, BUF_SIZE ) ;
/*printf("ricevuto  messaggio n. %d  %s\n",counter,client_msg);*/
    if ( read_retval <= 0 ) {
        if ( read_retval < 0 )
            perror( "error reading main socket" ) ;
        not_error = FALSE ;
        }


if (strncmp(client_msg, "record ricevuto",15)==0)
        {
        /*printf("ho trasmesso il record n. %d \n",counter);*/
        }
                }





	/*printf ("3 messaggio ricevuto\n");*/



	for (counter=0 ; counter < 100 ; counter++)
		{

		sprintf(device_msg,"%d", alim[counter].mode); 
		/*printf("  ** ** %d\n", alim[counter].mode); */
				
       if ( write(main_channel ,device_msg ,BUF_SIZE ) < 0 ) 
		{
		perror( "\nclient: error writing socket\n" ) ;
		not_error = FALSE ;
		}
	/*printf("trasmesso messaggio n. %d\n",counter);*/
	read_retval = read( main_channel, client_msg, BUF_SIZE ) ;
	/*printf("ricevuto  messaggio n. %d  %s\n",counter,client_msg);*/
    if ( read_retval <= 0 ) 
	{
        if ( read_retval < 0 )
            perror( "error reading main socket" ) ;
        not_error = FALSE ;
	}

if (strncmp(client_msg, "record ricevuto",15)==0)
	{
	/*printf("ho trasmesso il record n. %d \n",counter);*/
	}
		}





}


	} while (uscita) ;/*end of client loop */

	/*printf ("sono uscito dal client loop \n");*/

	} while (1) ; /*end of main loop */

}

