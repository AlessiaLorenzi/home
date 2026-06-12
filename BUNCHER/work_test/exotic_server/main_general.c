#include "server.h"
#include "server_const.h"
#include "server_vars.h"
#include <time.h>
#include <math.h>
#include <stdlib.h>

FILE *rs232r, *rs232w ;
FILE *tesla_rs232r, *tesla_rs232w ;
FILE *file_soglie;
char tty_name[ NCHARS ] ;
char tesla_tty_name[ NCHARS ] ;
int fildes ;
int tesla_fildes ;
int proc_id ;
int tesla_type;
float soglia;
int teslarange;

/***********************************************************************

    This is a server-client process for DECstation 5000/200
    As a server it waits for a connection on a socket given
    as an argument; when the connection has been settled,
    the following informations must be read through the
    communication channel:

    - machine and socket for alarm messages, mode
      ( "NONE anything else" means 'no alarm' )
      ( mode=0 means 'stub mode' ; mode=1 means 'normal mode' )
    - other datas, server-dependent.

    Depending on the first bunch of informations read,
    alarm are managed or not.
    In both cases the server enter an infinite loop,
    waiting for commands and requests from client.
    When alarm are managed the server enable periodic checks
    triggered by alarm() system call.

    usage: server_name socket
           0   1

    example: ma_server 2001

    this server uses the RS232 tty driver, and the tty which is used
    must be off and set to 9600 baud (see /etc/inittab: 
        off:/etc/getty  scc3 9600 # used for 147
        off:/etc/getty tty11 9600 # used for 332XT )

    specific server data: verbose/not-verbose option, tty name
                          [-v/-n]                     /dev/tty[11-18]

***********************************************************************/

/* global vars: used by several functions */

int main_channel, alarm_channel, alarm_enabled, alarm_addrlen,
    alarm_allowed, mode = 0, verbose = 0 ;

    int supply_number;
    char *message;

char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ],
     alarm_msg[ BUF_SIZE ], alarm_headmsg[ NCHARS ], myname[ NCHARS ] ;

int address[100];
int alim_mode[100];
int tesla_ok;
float fondoscala;


int command_shame_id, answer_shame_id;
char *command_shame_pointer, *answer_shame_pointer;

int command_sem_id, answer_sem_id, access_sem_id, semop_retval;
int command_sem_val, answer_sem_val, access_sem_val;

struct sembuf semaphore_struct ;

char command_line[100];

main( argc, argv )
int argc ;
char **argv ;
{
    extern int main_channel, alarm_channel,
               alarm_enabled, alarm_allowed,
               mode, verbose ;
    extern char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ],
                alarm_msg[ BUF_SIZE ], myname[ NCHARS ] ;

    extern struct supply_data supply[100];
    
    int main_socket, alarm_socket,
        gen_chan, gen_chan_addrlen,
        read_retval, write_retval, signal_retval, not_end,
        channel_addrlen ;
    struct sockaddr_in gen_chan_addr, channel_addr ;
    struct hostent * hp, * gethostbyname() ;
    char alarm_machine[ NCHARS ] ;

    int connect_alarm() ;
    void on_alarm() ;


    int return_val;
    float tesla_value;
    float delta_field ;
    float tesla_set ;
    float kappa;
    int valore;
    int curr_read;
    int curr_write;
    int curr_delta;
    char tmp_msg[ BUF_SIZE ] ;
    static struct timeval timeout = {(long)1, (long)0};
    fd_set ready;

    struct timespec ts;

    int feed_onoff = 1;
    int field_ok;
    int curr_value;





    file_soglie = fopen ( argv[3], "r" );
    fscanf ( file_soglie, "%f", &soglia );
    fclose ( file_soglie ) ;

    printf("*******************************letta soglia %f dal file %s\n", soglia, argv[3]);
    

//    soglia = atof ( argv [3] ) ;

//    printf("******************************* soglia = %f\n", soglia);



/* creo memoria condivisa e semaforo */


/* shared memory setup, mode 666 */
command_shame_id = shmget( IPC_PRIVATE, SHAREDMEM_SIZE,
                     ( IPC_CREAT | IPC_EXCL | 0666 ) ) ;
printf( "\nsharedmem_id is : %d\n", command_shame_id ) ;
command_shame_pointer = shmat( command_shame_id, 0, 0 ) ;
if ( (int)command_shame_pointer == -1 )
    printf( "error in shared mem attach\n" ) ;
else
    printf( "shared mem attached to : %d\n", command_shame_pointer ) ;
strcpy( command_shame_pointer, "" ) ; /* clear shared memory */


/* shared memory setup, mode 666 */
answer_shame_id = shmget( IPC_PRIVATE, SHAREDMEM_SIZE,
                     ( IPC_CREAT | IPC_EXCL | 0666 ) ) ;
printf( "\nsharedmem_id is : %d\n", answer_shame_id ) ;
answer_shame_pointer = shmat( answer_shame_id, 0, 0 ) ;
if ( (int)answer_shame_pointer == -1 )
    printf( "error in shared mem attach\n" ) ;
else
    printf( "shared mem attached to : %d\n", answer_shame_pointer ) ;
strcpy( answer_shame_pointer, "" ) ; /* clear shared memory */

			    

/* semaphore setup, mode 666 */
command_sem_id = semget( IPC_PRIVATE, 1,
                     ( IPC_CREAT | IPC_EXCL | 0666 ) ) ;
printf( "\nsemaphore_id is %d\n", command_sem_id ) ;
/* set semaphore, i.e. green light */
command_sem_val = semctl( command_sem_id,  0, SETVAL, 1 ) ;



/* semaphore setup, mode 666 */
answer_sem_id = semget( IPC_PRIVATE, 1,
                     ( IPC_CREAT | IPC_EXCL | 0666 ) ) ;
printf( "\nsemaphore_id is %d\n", answer_sem_id ) ;
/* set semaphore, i.e. green light */
answer_sem_val = semctl( answer_sem_id,  0, SETVAL, 1 ) ;

 

/* semaphore setup, mode 666 */
access_sem_id = semget( IPC_PRIVATE, 1,
                     ( IPC_CREAT | IPC_EXCL | 0666 ) ) ;
printf( "\nsemaphore_id is %d\n", access_sem_id ) ;
/* set semaphore, i.e. green light */
access_sem_val = semctl( access_sem_id,  0, SETVAL, 1 ) ;


/* structure used to clear the semaphore (red light) */
semaphore_struct.sem_num = 0 ;
semaphore_struct.sem_op = -1 ;
semaphore_struct.sem_flg = 0 ;

    

    /* check args */
    strcpy( myname, argv[0] ) ;
    main_socket = atoi( argv[1] ) ;
    fondoscala = (float)atoi( argv[2] ) ;
    printf("*******************************fondoscala = %f\n", fondoscala);
    if ( main_socket < MIN_SOCKET || main_socket > MAX_SOCKET ) {
         printf( "\n%s: socket number out of range, bye\n", myname ) ;
         exit( 0 ) ;
    }
    strcat( myname, argv[1] ) ;

    /* Create a socket towards the client ( as a server ) */
    gen_chan = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
    if ( gen_chan < 0 ) {
        printf( "\n%s: error opening socket\n", myname ) ;
        exit( 0 ) ;
        }

    /* give a name to socket using wildcards */
    gen_chan_addr.sin_family = AF_INET ;
    gen_chan_addr.sin_addr.s_addr = INADDR_ANY ;
    gen_chan_addr.sin_port = htons( main_socket ) ;
    gen_chan_addrlen = sizeof( gen_chan_addr ) ;
    channel_addrlen = sizeof( channel_addr ) ;
    if ( bind( gen_chan, &gen_chan_addr, gen_chan_addrlen ) < 0 ) {
        printf( "\n%s: error binding socket\n", myname ) ;
        exit( 0 ) ;
        }

    /* Find out assigned port number and print it out */
    if ( getsockname( gen_chan, &gen_chan_addr, &gen_chan_addrlen )
         < 0 ) {
        printf( "\n%s: error getting socket name\n", myname ) ;
        exit( 0 ) ;
        }
    printf( "\n%s: main socket is %d\n", myname,
            ntohs( gen_chan_addr.sin_port ) ) ;

    /* Start accepting connections */
    if ( listen ( gen_chan, 5 ) < 0 ) {
        printf( "\n%s: error listening socket\n", myname ) ;
        exit( 0 ) ;
        }


    do { /* 1 */ /* proceso padre */

        /* accept connection from client */
        main_channel = accept( gen_chan, &gen_chan_addr,
                               &gen_chan_addrlen ) ;
        printf( "\n%s: opening main channel\n", myname ) ;

        /* read the first bunch of datas from the client i.e.
           alarm server address and server-dependent stuff */
        alarm_allowed = FALSE ;
        alarm_machine[0] = 0 ;
        first_read( alarm_machine, &alarm_socket, &mode ) ;

        /* make server-dependent initial house-keeping */
        house_keeping( mode ) ;


        not_end = TRUE ;


proc_id = fork();

   if ( proc_id )  

        /* start endless loop */
        do {
            /* read from client */
            read_retval = read( main_channel, client_msg, BUF_SIZE ) ;
            if ( read_retval <= 0 ) {
                if ( read_retval < 0 ) {
                    if ( errno != EINTR )
                        printf( "\n%s: error reading main socket\n", myname ) ;
                    client_msg[0] = 0 ;
                    }
                else
                   not_end = FALSE ;
                }
            if ( not_end ) {
                if ( client_msg[0] != 0 ) {

                    /* cmd has been read, then exec */
                    if ( verbose )
                        printf("%s: (main) msg is %s\n", myname, client_msg ) ;

                    /* mask alarm signal, if necessary */
                    if ( alarm_enabled )
                        signal( SIGALRM, SIG_IGN ) ;

                    if ( mode )
                        exec_command( &mode, &not_end ) ;
                    else
                        sprintf(device_msg, "STUB");

                    /* answer to client */
                    do {
                        write_retval = write( main_channel,
                                              device_msg, BUF_SIZE ) ;
                        if ( write_retval <= 0 )
                            printf( "\n%s: error writing main socket\n",
                                    myname ) ;
                        } while ( write_retval <= 0 ) ;
                    /* unmask alarm signal, if necessary */
                    if ( alarm_enabled ) {
                        signal( SIGALRM, on_alarm ) ;
                        alarm( ALARM_PERIOD ) ;
                        }

                    }
            }
        } while ( not_end ) ;



   else
   	{
	/* processo figlio */
	sleep (10);

do
	{
	//printf ("io sono %d figlio di %d\n", getpid(), getppid() );


////////////////////////////////////////////////////////////////////
//                      LEGGO CAMPO		                  //
////////////////////////////////////////////////////////////////////
		
	FD_ZERO(&ready);
	fprintf( tesla_rs232w, "F\n"  ) ;
	fflush( tesla_rs232w ) ;

//	if ( verbose )
//    	printf( "\n%s: TESLA FDSET\n", myname ) ;
	FD_SET(tesla_fildes, &ready);
	timeout.tv_sec = 1 ;
	timeout.tv_usec = 0 ;
	if ( return_val = select( getdtablesize(),
                          &ready,
                          (fd_set *)NULL,
                          (fd_set *)NULL,
                          &timeout) > 0 ) {
    	fgets( tmp_msg, BUF_SIZE, tesla_rs232r ) ;
    	}

	FD_CLR(tesla_fildes, &ready);
//	if ( verbose )
//    	printf( "\n%s: TESLA FDCLR\n", myname ) ;
	
	tesla_ok = 0;
	if (return_val == 0)
		{
		tesla_ok = 0;
    		on_tesla_timeout();
		}
	else
    		{
		tesla_ok = 1 ;
    		sscanf(tmp_msg, "F %fT", &tesla_value);
    		}



	if (tesla_type == 1 ) printf ("#########################################il teslametro e' un 133\n");
	if (tesla_type == 2 )
	{
	//printf ("#########################################il teslametro e' un 150\n");
	// devo vedere qual'e' il fondo scala impostato
        FD_ZERO(&ready);
        fprintf( tesla_rs232w, "IR\n"  ) ;
        fflush( tesla_rs232w ) ;

        FD_SET(tesla_fildes, &ready);
        timeout.tv_sec = 1 ;
        timeout.tv_usec = 0 ;
        if ( return_val = select( getdtablesize(),
                          &ready,
                          (fd_set *)NULL,
                          (fd_set *)NULL,
                          &timeout) > 0 ) {
        fgets( tmp_msg, BUF_SIZE, tesla_rs232r ) ;
        }

        FD_CLR(tesla_fildes, &ready);
        tesla_ok = 0;
        if (return_val == 0)
                {
                tesla_ok = 0;
                on_tesla_timeout();
                }
        else
                {
                tesla_ok = 1 ;
                sscanf(tmp_msg, "IR %d", &teslarange);
		if ( teslarange == 0 ) printf ("answer = %s, range = 0.3\n", tmp_msg);
		if ( teslarange == 1 ) printf ("answer = %s, range = 0.6 \n", tmp_msg);
		if ( teslarange == 2 ) printf ("answer = %s, range = 1.2\n", tmp_msg);
		if ( teslarange == 3 ) printf ("answer = %s, range = 3.0\n", tmp_msg);


		if ( fabsf ( tesla_value ) <= 0.3  && teslarange > 0 )
			{
			printf ("FONDOSCALA TROPPO ALTO, DEVE ESSERE 0\n");
			}
		if ( fabsf ( tesla_value ) > 0.3 && fabsf ( tesla_value ) <= 0.6 && teslarange != 1 )
			{
			//fondoscala da cambiare
			if ( teslarange == 0 )
				{
				// fondoscala troppo basso
				printf ("FONDOSCALA TROPPO BASSO, DEVE ESSERE 1\n");
				}
			if ( teslarange > 1)
				{
				// fondoscala troppo alto
				printf ("FONDOSCALA TROPPO ALTO, DEVE ESSERE 1\n");
				}
			}
		if ( fabsf ( tesla_value ) > 0.6 && fabsf ( tesla_value ) <= 1.2 && teslarange != 2 )
			{
			//fondoscala da cambiare
			if ( teslarange <= 2) 
				{
				// fondoscala troppo basso
				printf ("FONDOSCALA TROPPO BASSO, DEVE ESSERE 2\n");
				}
			if ( teslarange > 2)
				{
				// fondoscala troppo alto
				printf ("FONDOSCALA TROPPO ALTO, DEVE ESSERE 2\n");
				}
			}
		if ( fabsf ( tesla_value ) > 1.2 && teslarange != 3 )
			{
			printf ("FONDOSCALA TROPPO BASSO, DEVE ESSERE 3\n");
			}
                }
	}
////////////////////////////////////////////////////////////////////
//          LEGGO CORRENTE REALE DALL'ALIMENTATORE                //
////////////////////////////////////////////////////////////////////
        	semop_retval = semop( access_sem_id, &semaphore_struct, 1 ) ;
        	if( semop_retval == -1 )
            		printf( "*1* error in semaphore operation\n" ) ;
        	else 
			{
       			access_sem_val = semctl( access_sem_id, 0, GETVAL, NULL ) ;
			fprintf( rs232w, "AD 8\r" ) ;
			fflush( rs232w ) ;
                        FD_SET(fildes, &ready);
                        timeout.tv_sec = 2 ;
                        timeout.tv_usec = 0 ;
                        if ( return_val = select( getdtablesize(),
	                     &ready, (fd_set *)NULL, (fd_set *)NULL, &timeout) > 0 ) 
				{
      				fgets( tmp_msg, BUF_SIZE, rs232r ) ;
                        	}
                        FD_CLR(fildes, &ready);
			}

		if (return_val == 0)
    			on_timeout();
		else
    			{
    			sscanf(tmp_msg, "%d", &curr_value);
    			}

        	/* turn on the green light */ 
        	access_sem_val = semctl( access_sem_id,  0, SETVAL, 1 ) ;
		
////////////////////////////////////////////////////////////////////
//                    	 LEGGO COMANDO                            //
////////////////////////////////////////////////////////////////////

        semop_retval = semop( command_sem_id, &semaphore_struct, 1 ) ;
        if( semop_retval == -1 )
            printf( "*2* error in semaphore operation\n" ) ;
        else 
		{
       		command_sem_val = semctl( command_sem_id, 0, GETVAL, NULL ) ;
    		strcpy ( command_line, command_shame_pointer ) ;
		}	
    		strcpy ( command_shame_pointer, "" ) ;
        /* turn on the green light */ 
        command_sem_val = semctl( command_sem_id,  0, SETVAL, 1 ) ;
	

	if ( strncmp ( command_line, "SET", 3 ) == 0 )
		{
		sscanf ( command_line, "SET %d", &valore );
		tesla_set = ( (float)valore * fondoscala ) / 1000000000.0 ;
		//printf ("***********************devo impostare un campo di %f tesla\n", tesla_set);
		if ( fabs ( tesla_set - tesla_value ) > soglia * 100.0 || feed_onoff == 0 )
//		if ( fabs ( tesla_set - tesla_value ) > 0.05 || feed_onoff == 0 )
			{
        		semop_retval = semop( access_sem_id, &semaphore_struct, 1 ) ;
        		if( semop_retval == -1 )
            			printf( "*1* error in semaphore operation\n" ) ;
        		else 
				{
                    		fprintf( rs232w, "WA %06d\r", valore ) ;
                    		fflush( rs232w ) ;
		    		printf("+++++++++++++++++++++++++++++++++++++ ora scrivo sull'alimentatore %06d\n", valore);	
				}
        		/* turn on the green light */ 
        		access_sem_val = semctl( access_sem_id,  0, SETVAL, 1 ) ;

			ts.tv_sec = 0 ;
			//ts.tv_nsec = 200000000;
			ts.tv_nsec = 000000000;
			nanosleep ( &ts, NULL ) ;

			}
			
		}


	if ( strncmp ( command_line, "REG", 3 ) == 0 )
		{
		if ( feed_onoff == 0 ) feed_onoff = 1 ;
		else feed_onoff = 0 ;
		printf("------------------------------------------- feed = %d\n", feed_onoff);
		}


	
////////////////////////////////////////////////////////////////////
//                     	 SCRIVO MEMORIA CONDIVISA                 //
////////////////////////////////////////////////////////////////////

        semop_retval = semop( answer_sem_id, &semaphore_struct, 1 ) ;
        if( semop_retval == -1 )
            printf( "*1* error in semaphore operation\n" ) ;
        else 
		{
       		answer_sem_val = semctl( answer_sem_id, 0, GETVAL, NULL ) ;
		if ( feed_onoff )
				{
				if ( field_ok )
					sprintf(answer_shame_pointer, "B%f %d\000", 1000.0 * tesla_value, curr_value);
				else
					sprintf(answer_shame_pointer, "C%f  %d\000", 1000.0 * tesla_value, curr_value);
				}
		else
				{
				sprintf(answer_shame_pointer, "A%f  %d\000", 1000.0 * tesla_value, curr_value);
				}
		if ( tesla_ok == 0 ) 
			sprintf(answer_shame_pointer, "D\000");
		}	
        /* turn on the green light */ 
        answer_sem_val = semctl( answer_sem_id,  0, SETVAL, 1 ) ;

    	//printf( "\n%s: tesla_answer is %s\n", myname, tmp_msg );
////////////////////////////////////////////////////////////////////
//                        IMPOSTO VALORE CORRENTE                 //
////////////////////////////////////////////////////////////////////


	//delta_field = 100.0 * ( tesla_set - fabsf(tesla_value) ) ;
	delta_field = 100.0 * ( tesla_set - fabsf(tesla_value) ) / (fondoscala / 1000.0) ;
	printf("valore letto = %f valore impostato = %f variazione percentuale = %f\n", 
	tesla_value, tesla_set, delta_field);
	field_ok = 0;
	if ( fabsf(delta_field) > soglia )
//	if ( fabsf(delta_field) > 0.008 )
		{
		field_ok = 0;
		// campo fuori dai valori prefissati
        	semop_retval = semop( access_sem_id, &semaphore_struct, 1 ) ;
        	if( semop_retval == -1 )
            		printf( "*1* error in semaphore operation\n" ) ;
        	else 
			{
       			access_sem_val = semctl( access_sem_id, 0, GETVAL, NULL ) ;

			// leggo stato dell'alimentatore
                        fprintf( rs232w, "S1\r" ) ;
                        fflush( rs232w ) ;
                        FD_SET(fildes, &ready);
                        timeout.tv_sec = 2 ;
                        timeout.tv_usec = 0 ;
                        if ( return_val = select( getdtablesize(),
                        &ready, (fd_set *)NULL, (fd_set *)NULL, &timeout) > 0 )
                                {
                                fgets( tmp_msg, BUF_SIZE, rs232r ) ;
                                }


			if ( tmp_msg[23] == '.' && tesla_ok && feed_onoff  ) 
			{
			//printf("        READY\n");
			//printf("        READY\n");
			//printf("        READY\n");
			//printf("        READY\n");
			// leggo corrente dall'alimentatore
			fprintf( rs232w, "RA\r" ) ;
			fflush( rs232w ) ;
                        FD_SET(fildes, &ready);
                        timeout.tv_sec = 2 ;
                        timeout.tv_usec = 0 ;
                        if ( return_val = select( getdtablesize(),
	                     &ready, (fd_set *)NULL, (fd_set *)NULL, &timeout) > 0 ) 
				{
      				fgets( tmp_msg, BUF_SIZE, rs232r ) ;
                        	}
                        FD_CLR(fildes, &ready);
			sscanf(tmp_msg, "%d", &curr_read);
			//printf("letto dall'alimentatore : %s %d **\n", tmp_msg, curr_read);

				if ( delta_field > 0.5 ) kappa = 1.0;
				else

				kappa = 0.1 ;

			curr_delta = (int)(999999.0 * kappa * delta_field / 100.0 );
			//if ( delta_field > 0.0 )
		//		{
		//		curr_delta = 10000;
		//		}
		//	else
		//		{
		//		curr_delta = -10000;
		//		}
			printf("corrente letta dall'alimentatore : %d variazione : %d\n", 
				curr_read, curr_delta);


		    // scrivo nuovo valore di corrente 
                    fprintf( rs232w, "WA %06d\r", curr_read + curr_delta ) ;
                    fflush( rs232w ) ;
		    printf("ora scrivo sull'alimentatore %06d\n", curr_read + curr_delta);	
			}	

			}
        	/* turn on the green light */ 
        	access_sem_val = semctl( access_sem_id,  0, SETVAL, 1 ) ;
		}
	else
		{
		// campo entro i valori prefissati
		field_ok = 1;
		}
    	//printf( "\n%s: tesla_answer is %s\n", myname, tmp_msg );

	ts.tv_sec = 1 ;
	ts.tv_nsec = 00000000;
	nanosleep ( &ts, NULL ) ;


	} while ( TRUE );	
   	}



        printf( "\n%s: closing main channel\n", myname ) ;
        close( main_channel ) ;

        if ( alarm_machine[0] != 0 ) {
            printf( "\n%s: closing alarm channel\n", myname ) ;
            close( alarm_channel ) ;
            }

        /* ready to accept an other connection */

    } while( TRUE ) ;/* 1 */

   
}

