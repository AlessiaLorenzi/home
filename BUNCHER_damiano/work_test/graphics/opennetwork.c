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


#include "defs.h"
#include "w_defs.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

    FILE *soc_r[20];

#define TRUE    1


#define FALSE   0

    opennetwork(  )

{
    int counter=0;
    int counter2=0;
    int somma=0;
    int not_error, rval ;
    int s_chan_s, s_chan_addrlen ;
    struct sockaddr_in s_chan_addr ;
    char frame_to_s[1024], frame_to_r[1024] ;
    struct hostent * hp, * gethostbyname() ;
    int nb_to_s, nb_to_r ;


/* apertura canali controller rf */
for ( counter = 0 ; counter < max_sp ; counter++ )
	{
	if ( net_verbose ) printf (" \n host = %s porta = %d \n ",rf_host[counter],rf_socket[counter]);

	nb_to_s = sizeof( frame_to_s ) ;
	nb_to_r = sizeof( frame_to_r ) ;

	/* Create a socket to/from server ( c is client ) */
    
	s_chan_s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
	if ( s_chan_s < 0 ) 
		{
		perror( "\nclient: error opening to/from s streamsocket\n" ) ;
		exit( 0 ) ;
		}

	setsockopt(s_chan_s, SOL_SOCKET, SO_REUSEADDR, 0, 0);

	/* connect socket using name specified by command line */
	s_chan_addr.sin_family = AF_INET ;


	hp = gethostbyname(rf_host[counter]) ;
	if (hp == NULL)
		printf("\n errore in gethostbyname \n");


	bcopy ( hp->h_addr, &( s_chan_addr.sin_addr.s_addr ), hp->h_length ) ;

	s_chan_addr.sin_port = htons(rf_socket[counter]) ;

	s_chan_addrlen = sizeof( s_chan_addr ) ;

	if ( connect( s_chan_s, &s_chan_addr, s_chan_addrlen ) < 0 ) {
		close( s_chan_s ) ;

		printf("*************************************************************************\n");
		printf(" errore nella connessione con il socket %d\n", rf_socket[counter]);
		printf(" riguardante il controller rf di %s", etichetta[counter]);
		printf("*************************************************************************\n");
		exit( 0 ) ;
		}

		/* inserire qui' la lettura extra */


     /* listen to server */
        if ( read( s_chan_s, frame_to_r, nb_to_r ) < 0 )
            {
            perror( "\nclient: error reading from s streamsocket\n" ) ;
            }


	if (strncmp(frame_to_r, "Not Done", 8) == 0 ) rf_is_alive[counter] = 0;
	else rf_is_alive[counter] = 1;
	strcpy(primo_messaggio[counter], frame_to_r);


	if ( net_verbose ) printf( "\nclient: socket has port #%d and number %d stato = %d, answer from serever is :\"%s\"\n", ntohs( s_chan_addr.sin_port ),s_chan_s ,rf_is_alive[counter], frame_to_r) ;
	rf_fildes[counter] = s_chan_s ;


	soc_r[counter] = fdopen( s_chan_s, "r" ) ;
	if ( soc_r[counter] == NULL ) printf( "\n: error opening tty for reading\n" ) ;

	}


if ( net_verbose )
	{
	printf("\n");
	printf ("sommario delle connessioni controller rf: \n\n");
	for ( counter = 0 ; counter < max_sp ; counter++ )
		printf ("host : %s canale : %d descrittore %d dispositivo = %s", rf_host[counter], rf_socket[counter], rf_fildes[counter], etichetta[counter]);
	}



/* apertura canale cavita' passiva */





        if ( net_verbose ) printf (" \n host = %s porta = %d \n ",phase_host, phase_socket);

        nb_to_s = sizeof( frame_to_s ) ;
        nb_to_r = sizeof( frame_to_r ) ;

        /* Create a socket to/from server ( c is client ) */
   
        s_chan_s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
        if ( s_chan_s < 0 )
                {
                perror( "\nclient: error opening to/from s streamsocket\n" ) ;
                exit( 0 ) ;
                }

        setsockopt(s_chan_s, SOL_SOCKET, SO_REUSEADDR, 0, 0);

        /* connect socket using name specified by command line */
        s_chan_addr.sin_family = AF_INET ;


        hp = gethostbyname(phase_host) ;
        if (hp == NULL)
                printf("\n errore in gethostbyname \n");


        bcopy ( hp->h_addr, &( s_chan_addr.sin_addr.s_addr ), hp->h_length ) ;

        s_chan_addr.sin_port = htons(phase_socket) ;

        s_chan_addrlen = sizeof( s_chan_addr ) ;

        if ( connect( s_chan_s, &s_chan_addr, s_chan_addrlen ) < 0 ) {
                close( s_chan_s ) ;
                perror( "\nclient: error connecting to/from s streamsocket\n" ) ;
                exit( 0 ) ;
                }

                /* inserire qui' la lettura extra */


     /* listen to server */
        if ( read( s_chan_s, frame_to_r, nb_to_r ) < 0 )
            {
            perror( "\nclient: error reading from s streamsocket\n" ) ;
            }


        if (strncmp(frame_to_r, "Not Done", 8) == 0 ) phase_is_alive = 0;
        else phase_is_alive = 1;
        strcpy(phase_primo_messaggio, frame_to_r);


        if ( net_verbose ) printf( "\nclient: socket has port #%d and number %d stato = %d, answer from serever is :\"%s\"\
n", ntohs( s_chan_addr.sin_port ),s_chan_s ,phase_is_alive, frame_to_r) ;
        phase_fildes = s_chan_s ;
        


if ( net_verbose )
        {
        printf("\n");
        printf ("sommario delle connessioni cavita' passiva: \n\n");
        printf ("host : %s canale : %d descrittore %d dispositivo = %s", phase_host, phase_socket, phase_fildes, "cavita' passiva");
        }



















/* apertura canale selettore di fase */





        if ( net_verbose ) printf (" \n host = %s porta = %d \n ",le_host, le_socket);

        nb_to_s = sizeof( frame_to_s ) ;
        nb_to_r = sizeof( frame_to_r ) ;

        /* Create a socket to/from server ( c is client ) */
  
        s_chan_s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
        if ( s_chan_s < 0 )
                {
                perror( "\nclient: error opening to/from s streamsocket\n" ) ;
                exit( 0 ) ;
                }

        setsockopt(s_chan_s, SOL_SOCKET, SO_REUSEADDR, 0, 0);

        /* connect socket using name specified by command line */
        s_chan_addr.sin_family = AF_INET ;


        hp = gethostbyname(le_host) ;
        if (hp == NULL)
                printf("\n errore in gethostbyname \n");


        bcopy ( hp->h_addr, &( s_chan_addr.sin_addr.s_addr ), hp->h_length ) ;

        s_chan_addr.sin_port = htons(le_socket) ;

        s_chan_addrlen = sizeof( s_chan_addr ) ;

        if ( connect( s_chan_s, &s_chan_addr, s_chan_addrlen ) < 0 ) {
                close( s_chan_s ) ;
                perror( "\nclient: error connecting to/from s streamsocket\n" ) ;
                exit( 0 ) ;
                }

                /* inserire qui' la lettura extra */


     /* listen to server */
        if ( read( s_chan_s, frame_to_r, nb_to_r ) < 0 )
            {
            perror( "\nclient: error reading from s streamsocket\n" ) ;
            }


        if (strncmp(frame_to_r, "Not Done", 8) == 0 ) le_is_alive = 0;
        else le_is_alive = 1;
        strcpy(le_primo_messaggio, frame_to_r);


        if ( net_verbose ) printf( "\nclient: socket has port #%d and number %d stato = %d, answer from serever is :\"%s\"\
n", ntohs( s_chan_addr.sin_port ),s_chan_s ,le_is_alive, frame_to_r) ;
        le_fildes = s_chan_s ;



if ( net_verbose )
        {
        printf("\n");
        printf ("sommario delle connessioni selettore di fase: \n\n");
        printf ("host : %s canale : %d descrittore %d dispositivo = %s", le_host, le_socket, le_fildes, "selettore di fase");
        }



/* apertura canali controller di zanon */
for ( counter = 0 ; counter < max_sp ; counter++ )
	za_fildes[counter] = 0 ;

for ( counter = 0 ; counter < max_sp ; counter++ )
	{
	if ( za_fildes[counter] == 0 )
		{
		/* apro il canale */
		
		if ( net_verbose ) printf (" \ntuning :  host = %s porta = %d \n ",za_host[counter],za_socket[counter]);

		nb_to_s = sizeof( frame_to_s ) ;
		nb_to_r = sizeof( frame_to_r ) ;

        	s_chan_s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
        	if ( s_chan_s < 0 )
               	 	{
               	 	perror( "\nclient: error opening to/from s streamsocket\n" ) ;
               	 	exit( 0 ) ;
               	 	}

        	setsockopt(s_chan_s, SOL_SOCKET, SO_REUSEADDR, 0, 0);

        	/* connect socket using name specified by command line */
        	s_chan_addr.sin_family = AF_INET ;

        	hp = gethostbyname(za_host[counter]) ;
        	if (hp == NULL)
                	printf("\n errore in gethostbyname \n");

        	bcopy ( hp->h_addr, &( s_chan_addr.sin_addr.s_addr ), hp->h_length ) ;

        	s_chan_addr.sin_port = htons(za_socket[counter]) ;

        	s_chan_addrlen = sizeof( s_chan_addr ) ;

        	if ( connect( s_chan_s, &s_chan_addr, s_chan_addrlen ) < 0 ) {
                	close( s_chan_s ) ;
                	printf("*************************************************************************\n");
                	printf(" errore nella connessione con il socket %d\n", za_socket[counter]);
                	printf(" riguardante il controller tuning di %s", etichetta[counter]);
                	printf("*************************************************************************\n");
                	exit( 0 ) ;
                	}


		/* inserire qui' la lettura extra */


     /* listen to server */
        if ( read( s_chan_s, frame_to_r, nb_to_r ) < 0 )
            {
            perror( "\nclient: error reading from s streamsocket\n" ) ;
            }


	if (strncmp(frame_to_r, "Not Done", 8) == 0 ) tu_is_alive[counter] = 0;
	else tu_is_alive[counter] = 1;
	strcpy(tu_primo_messaggio[counter], frame_to_r);

        	if ( net_verbose ) printf( "\n++++tuning : client: socket has port #%d and number %d stato = %d, answer from server is %s\n", ntohs( s_chan_addr.sin_port ),s_chan_s,tu_is_alive[counter], frame_to_r ) ;
        	za_fildes[counter] = s_chan_s ;

		}
	else
		{
		if ( net_verbose ) printf (" \ntuning :  canale su host = %s porta = %d gia' aperto\n ",za_host[counter],za_socket[counter]);
		}

	for ( counter2 = 0 ; counter2 < max_sp ; counter2++ )
		{
		if ( strcmp ( za_host[counter], za_host[counter2] ) == 0 ) /* e' il medesimo host */
			{
			if ( za_socket[counter] == za_socket[counter2] ) /* e' il medesimo canale */
				{
				za_fildes[counter2] = za_fildes[counter] ;
				tu_is_alive[counter2] = tu_is_alive[counter] ;
				}
			}

/* creazione del canale di comunicazione per l'amplificatore */
/* vedo se e' uno di quelli gia' aperti per il tuning e ne ricopio i descrittori */

		if ( strcmp ( za_host[counter], za_ampli_host[counter2] ) == 0 ) /* e' il medesimo host */
			{
			if ( za_socket[counter] == za_ampli_socket[counter2] ) /* e' il medesimo canale */
				{
				za_ampli_fildes[counter2] = za_fildes[counter] ;
				ampli_is_alive[counter2] = tu_is_alive[counter] ;
        			strcpy(ampli_primo_messaggio[counter2], tu_primo_messaggio[counter]);
				}
			}
		}
	}



/* creazione del canale di comunicazione per l'amplificatore */
/* apro quei canali che non erano stati precedentemente aperti per il tuning */

for ( counter = 0 ; counter < max_sp ; counter++ )
        {
        if ( za_ampli_fildes[counter] == 0 )
                {
                /* apro il canale */

                if ( net_verbose ) printf (" \nampli : host = %s porta = %d \n ",za_ampli_host[counter],za_ampli_socket[counter]);

                nb_to_s = sizeof( frame_to_s ) ;
                nb_to_r = sizeof( frame_to_r ) ;

                s_chan_s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
                if ( s_chan_s < 0 )
                        {
                        perror( "\nclient: error opening to/from s streamsocket\n" ) ;
                        exit( 0 ) ;
                        }

                setsockopt(s_chan_s, SOL_SOCKET, SO_REUSEADDR, 0, 0);

                /* connect socket using name specified by command line */
                s_chan_addr.sin_family = AF_INET ;

                hp = gethostbyname(za_ampli_host[counter]) ;
                if (hp == NULL)
                        printf("\n errore in gethostbyname \n");

                bcopy ( hp->h_addr, &( s_chan_addr.sin_addr.s_addr ), hp->h_length ) ;

                s_chan_addr.sin_port = htons(za_ampli_socket[counter]) ;

                s_chan_addrlen = sizeof( s_chan_addr ) ;

                if ( connect( s_chan_s, &s_chan_addr, s_chan_addrlen ) < 0 ) {
                        close( s_chan_s ) ;

			printf("*************************************************************************\n");
			printf(" errore nella connessione con il socket %d\n", rf_socket[counter]);
			printf(" riguardante il controller rf di %s", etichetta[counter]);
			printf("*************************************************************************\n");
                        exit( 0 ) ;
                        }


                /* inserire qui' la lettura extra */


     /* listen to server */
        if ( read( s_chan_s, frame_to_r, nb_to_r ) < 0 )
            {
            perror( "\nclient: error reading from s streamsocket\n" ) ;
            }


        if (strncmp(frame_to_r, "Not Done", 8) == 0 ) ampli_is_alive[counter] = 0;
        else ampli_is_alive[counter] = 1;
        strcpy(ampli_primo_messaggio[counter], frame_to_r);



                if ( net_verbose ) printf( "\nampli : client: socket has port #%d and number %d \n", ntohs( s_chan_addr.sin_port ),s_chan_s ) ;
                za_ampli_fildes[counter] = s_chan_s ;

                }
        else
                {
                if ( net_verbose ) printf (" \nampli :  canale su host = %s porta = %d gia' aperto\n ",za_ampli_host[counter],za_ampli_socket[counter]);
                }

/* vedo se coincide con uno precedentemente aperto */
        for ( counter2 = 0 ; counter2 < max_sp ; counter2++ )
                {
                if ( strcmp ( za_ampli_host[counter], za_ampli_host[counter2] ) == 0 ) /* e' il medesimo host */
                        {
                        if ( za_ampli_socket[counter] == za_ampli_socket[counter2] ) /* e' il medesimo canale */
				{
                                za_ampli_fildes[counter2] = za_ampli_fildes[counter] ;
				ampli_is_alive[counter2] = ampli_is_alive[counter] ;
        			strcpy(ampli_primo_messaggio[counter2], ampli_primo_messaggio[counter]);
				}
                        }
                }
        }


if ( net_verbose )
	{
	printf("\n");
	printf ("sommario delle connessioni cestelli di zanon : \n");

	printf("\n");
	printf("tuning");
	printf("\n");

	for ( counter = 0 ; counter < max_sp ; counter++ )
		printf ("host : %s canale : %d descrittore %d stato = %d dispositivo = %s", za_host[counter], za_socket[counter], za_fildes[counter], tu_is_alive[counter], etichetta[counter]);

	printf("\n");
	printf("amplificatori");
	printf("\n");

	for ( counter = 0 ; counter < max_sp ; counter++ )
		printf ("host : %s canale : %d descrittore %d dispositivo = %s", za_ampli_host[counter], za_ampli_socket[counter], za_ampli_fildes[counter], etichetta[counter]);
	}


/* creazione del canale di comunicazione per il chopper */

chopper_fildes = 0;

	for ( counter2 = 0 ; counter2 < max_sp ; counter2++ )
		{
		/* guardo se e' un canale precedentemene aperto per il tuning */
		if ( strcmp ( chopper_host, za_host[counter2] ) == 0 ) /* e' il medesimo host */
			{
			if ( chopper_socket == za_socket[counter2] ) /* e' il medesimo canale */
				{
				chopper_fildes = za_fildes[counter2] ;
				chopper_is_alive = tu_is_alive[counter2] ;
				}
			}

		/* guardo se e' un canale precedentemene aperto per l'ampli */
		if ( strcmp ( chopper_host, za_ampli_host[counter2] ) == 0 ) /* e' il medesimo host */
			{
			if ( chopper_socket == za_ampli_socket[counter2] ) /* e' il medesimo canale */
				{
				chopper_fildes = za_ampli_fildes[counter2] ;
				chopper_is_alive = ampli_is_alive[counter2] ;
				}
			}
		}

	/* altrimenti ne apro uno di nuovo !!!!!!!! */
        if ( chopper_fildes == 0 )
                {
                /* apro il canale */

                if ( net_verbose ) printf (" \nchopper :  host = %s porta = %d \n ",chopper_host,chopper_socket);

                nb_to_s = sizeof( frame_to_s ) ;
                nb_to_r = sizeof( frame_to_r ) ;

                s_chan_s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
                if ( s_chan_s < 0 )
                        {
                        perror( "\nclient: error opening to/from s streamsocket\n" ) ;
                        exit( 0 ) ;
                        }

                setsockopt(s_chan_s, SOL_SOCKET, SO_REUSEADDR, 0, 0);

                /* connect socket using name specified by command line */
                s_chan_addr.sin_family = AF_INET ;

                hp = gethostbyname(chopper_host) ;
                if (hp == NULL)
                        printf("\n errore in gethostbyname \n");

                bcopy ( hp->h_addr, &( s_chan_addr.sin_addr.s_addr ), hp->h_length ) ;

                s_chan_addr.sin_port = htons(chopper_socket) ;

                s_chan_addrlen = sizeof( s_chan_addr ) ;

                if ( connect( s_chan_s, &s_chan_addr, s_chan_addrlen ) < 0 ) {
                        close( s_chan_s ) ;
                        perror( "\nclient: error connecting to/from s streamsocket\n" ) ;
                        exit( 0 ) ;
                        }

                /* inserire qui' la lettura extra */


     /* listen to server */
        if ( read( s_chan_s, frame_to_r, nb_to_r ) < 0 )
            {
            perror( "\nclient: error reading from s streamsocket\n" ) ;
            }

	if (strncmp(frame_to_r, "Not Done", 8) == 0 ) chopper_is_alive = 0;
	else chopper_is_alive = 1;



                if ( net_verbose ) printf( "\nchopper : client: socket has port #%d and number %d \n", ntohs( s_chan_addr.sin_port ),s_chan_s ) ;
                chopper_fildes = s_chan_s ;

                }
        else
                {
                if ( net_verbose ) printf (" \nchopper :  canale %d su host = %s porta = %d gia' aperto\n ", chopper_fildes ,chopper_host , chopper_socket);
                }




/* creazione del canale di comunicazione per il tuner cavita' passiva */

phase_tuning_fildes = 0;

        for ( counter2 = 0 ; counter2 < max_sp ; counter2++ )
                {
                /* guardo se e' un canale precedentemene aperto per il tuning */
                if ( strcmp ( phase_tuning_host, za_host[counter2] ) == 0 ) /* e' il medesimo host */
                        {
                        if ( phase_tuning_socket == za_socket[counter2] ) /* e' il medesimo canale */
                                {
                                phase_tuning_fildes = za_fildes[counter2] ;
                                phase_tuning_is_alive = tu_is_alive[counter2] ;
                                }
                        }

                /* guardo se e' un canale precedentemene aperto per l'ampli */
                if ( strcmp ( phase_tuning_host, za_ampli_host[counter2] ) == 0 ) /* e' il medesimo host */
                        {
                        if ( phase_tuning_socket == za_ampli_socket[counter2] ) /* e' il medesimo canale */
                                {
                                phase_tuning_fildes = za_ampli_fildes[counter2] ;
                                phase_tuning_is_alive = ampli_is_alive[counter2] ;
                                }
                        }
                }


if ( net_verbose ) printf (" \ntuning cavita' passiva :  canale %d su host = %s porta = %d gia' aperto\n ", phase_tuning_fildes ,phase_tuning_host , phase_tuning_socket);
}
