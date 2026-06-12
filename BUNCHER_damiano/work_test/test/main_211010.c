
#include "server.h"
#include "motor_server.h"

FILE *rs232r, *rs232w ;
char tty_name[ NCHARS ] ;
int motcntr_mode = FALSE;

int timeout_flag = FALSE ;

int fildes = -1 ;
int proc_id;
char comando[100];
char risposta[100];





/* global vars: used by several functions */

int main_channel, alarm_channel, alarm_enabled, alarm_addrlen,
    alarm_allowed, verbose = 0 ;

char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ],
     alarm_msg[ BUF_SIZE ], alarm_headmsg[ NCHARS ], myname[ NCHARS ] ;

struct sockaddr_in alarm_addr ;

int command_shame_id, answer_shame_id;
char *command_shame_pointer, *answer_shame_pointer;

int command_sem_id, answer_sem_id, access_sem_id, semop_retval;
int command_sem_val, answer_sem_val, access_sem_val;

struct sembuf semaphore_struct ;

main( argc, argv )
int argc ;
char **argv ;
{
    extern int main_channel, alarm_channel,
               alarm_enabled, alarm_allowed,
               verbose ;
    extern int motcntr_mode ;
    extern char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ],
                alarm_msg[ BUF_SIZE ], myname[ NCHARS ] ;

   int main_socket, alarm_socket,
        gen_chan, gen_chan_addrlen,
        read_retval, write_retval, signal_retval, not_end,
        channel_addrlen ;
    struct sockaddr_in gen_chan_addr, channel_addr ;
    struct hostent * hp, * gethostbyname() ;
    char alarm_machine[ NCHARS ] ;








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
    if ( main_socket < MIN_SOCKET || main_socket > MAX_SOCKET ) {
         printf( "\n%s: socket number out of range\n", myname ) ;
         exit( 0 ) ;
    }
    strcat( myname, argv[1] ) ;

    /* Create a socket towards the client ( as a server ) */
    gen_chan = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
    if ( gen_chan < 0 ) {
        printf( "\n%s: error opening socket\n", myname ) ;
        exit( 0 ) ;
        }


//if (setsockopt(gen_chan, SOL_SOCKET, SO_REUSEADDR, 0, 0) < 0 )
//	{
//        printf( "\n%s: error setsockopt\n", myname ) ;
//        }



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

    do {

        /* accept connection from client */
        main_channel = accept( gen_chan, &gen_chan_addr,
                               &gen_chan_addrlen ) ;
        printf( "\n%s: opening main channel\n", myname ) ;

        /* read the first bunch of datas from the client i.e.
           alarm server address and server-dependent stuff */
        alarm_allowed = FALSE ;
        alarm_machine[0] = NULL ;

/* modifiche by andrea */

	alarm_socket = 0;

	strcpy(tty_name, argv[2]);

	if ( strcmp ("v", argv[3]) == 0 )
		verbose = 1;

	if ( strcmp ("t", argv[4]) == 0 )
		motcntr_mode = TRUE ;	
	

        /* make server-dependent initial house-keeping */
        house_keeping( motcntr_mode ) ;


        not_end = TRUE ;

	////////////////////////////////////////////////////////////////////
	//                     	 SCRIVO MEMORIA CONDIVISA                 //
	////////////////////////////////////////////////////////////////////

      	semop_retval = semop( command_sem_id, &semaphore_struct, 1 ) ;
      	if( semop_retval == -1 )
         	printf( "*1* error in semaphore operation\n" ) ;
      	else 
		{
       		//  ??  serve ??  answer_sem_val = semctl( answer_sem_id, 0, GETVAL, NULL ) ;
		strcpy(command_shame_pointer, "");
		}

   			/* turn on the green light */ 
      	command_sem_val = semctl( command_sem_id,  0, SETVAL, 1 ) ;

	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////
	//                     	 SCRIVO MEMORIA CONDIVISA                 //
	////////////////////////////////////////////////////////////////////

      	semop_retval = semop( answer_sem_id, &semaphore_struct, 1 ) ;
      	if( semop_retval == -1 )
        	printf( "*1* error in semaphore operation\n" ) ;
      	else 
		{
       		//  ??  serve ??  answer_sem_val = semctl( answer_sem_id, 0, GETVAL, NULL ) ;
		strcpy(answer_shame_pointer, "busy");
		}

   	/* turn on the green light */ 
      	answer_sem_val = semctl( answer_sem_id,  0, SETVAL, 1 ) ;

	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////


   proc_id = fork();

   if ( proc_id )  

 
        /* start endless loop */
        do {   ///////////////////////////////////////////////////////////////// PADRE

            /* read from client */
            read_retval = read( main_channel, client_msg, BUF_SIZE ) ;
            if ( read_retval <= 0 ) {
                if ( read_retval < 0 ) {
                    if ( errno != EINTR )
                        printf( "\n%s: error reading main socket\n",
                                myname ) ;
                    client_msg[0] = NULL ;
                    }
                else
                   not_end = FALSE ;
                }

printf("\n\n\n\nP FROM GRAPHICS == >>%s\n", client_msg);


            if ( not_end ) {
                if ( client_msg[0] != NULL ) {

                    /* cmd has been read, then exec */

                    /* mask alarm signal, if necessary */
                    if ( alarm_enabled )
                        signal( SIGALRM, SIG_IGN ) ;



		if ( strncmp ( client_msg+1, "bclr", 4 ) == 1 || strncmp ( client_msg+1, "bset", 4 ) == 1 
		|| strncmp ( client_msg+1, "wr", 2 ) == 1 || strncmp ( client_msg+1, "rd", 2 ) == 1 )

			{
			printf ( "\n\nP come una volta \n\n%s %s \n", client_msg, myname);
                    	exec_command( &motcntr_mode, &not_end ) ;
			}
		else
			{
			////////////////////////////////////////////////////////////////////
			//                     	 LEGGO MEMORIA CONDIVISA                  //
			////////////////////////////////////////////////////////////////////

        		semop_retval = semop( answer_sem_id, &semaphore_struct, 1 ) ;
        		if( semop_retval == -1 )
            			printf( "*1* error in semaphore operation\n" ) ;
        		else 
				{
				strcpy(risposta, answer_shame_pointer);
				}

     			/* turn on the green light */ 
        		answer_sem_val = semctl( answer_sem_id,  0, SETVAL, 1 ) ;

			////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////

			printf("P risposta dal figlio : %s\n", risposta);

			if ( strcmp(risposta, "busy" ) != 0 )
				{
				
				// il comado precedente e' stato eseguito ed il figlio e' pronto
				
				printf ( "P scrivo mem. cond. \n");
				////////////////////////////////////////////////////////////////////
				//                     	 SCRIVO MEMORIA CONDIVISA                 //
				////////////////////////////////////////////////////////////////////

        			semop_retval = semop( command_sem_id, &semaphore_struct, 1 ) ;
        			if( semop_retval == -1 )
            				printf( "*1* error in semaphore operation\n" ) ;
        			else 
				{
       					//  ??  serve ??  answer_sem_val = semctl( answer_sem_id, 0, GETVAL, NULL ) ;
					sprintf(command_shame_pointer, "%s\000", client_msg);
					}

     				/* turn on the green light */ 
        			command_sem_val = semctl( command_sem_id,  0, SETVAL, 1 ) ;

				////////////////////////////////////////////////////////////////////
				////////////////////////////////////////////////////////////////////
				////////////////////////////////////////////////////////////////////
				}
			else
				{
				printf("P figlio non pronto\n");
				// comando in corso di esecuzione (busy)
				// oppure uscito con errore (error)
				}


			}


printf("\n\n\n\nP TO GRAPHICS == >>%s\n", device_msg);

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
                        printf( "\n%s: alarm not yet ready\n",
                                myname ) ;
                        }

                    }
            }
        } while ( not_end ) ; ///////////////////////////////////




	else
		{////////////////////////////////////FIGLIO
		printf("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF FIGLIO start\n" );
		sleep(10);
		printf("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF FIGLIO ready\n" );

		////////////////////////////////////////////////////////////////////
		//                     	 SCRIVO MEMORIA CONDIVISA                 //
		////////////////////////////////////////////////////////////////////

        	semop_retval = semop( answer_sem_id, &semaphore_struct, 1 ) ;
        	if( semop_retval == -1 )
            		printf( "*1* error in semaphore operation\n" ) ;
        			else 
			{
       			//  ??  serve ??  answer_sem_val = semctl( answer_sem_id, 0, GETVAL, NULL ) ;
			strcpy(answer_shame_pointer, "done");
			}

   			/* turn on the green light */ 
      			answer_sem_val = semctl( answer_sem_id,  0, SETVAL, 1 ) ;

		////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////

		do
			{
			////////////////////////////////////////////////////////////////////
			//                     	 LEGGO MEMORIA CONDIVISA                  //
			////////////////////////////////////////////////////////////////////

        		semop_retval = semop( command_sem_id, &semaphore_struct, 1 ) ;
        		if( semop_retval == -1 )
            			printf( "*1* error in semaphore operation\n" ) ;
        		else 
				{
				strcpy(comando, command_shame_pointer);
				}

     			/* turn on the green light */ 
        		command_sem_val = semctl( command_sem_id,  0, SETVAL, 1 ) ;

			////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////
			
			if ( comando[0] != 0)
				{
				strcpy(command_shame_pointer, "" ); // mettere gestione semaforo
			
				//////////////////////////////////////////////////////////////////////////////
				//                     	 SCRIVO MEMORIA CONDIVISA                 //
				////////////////////////////////////////////////////////////////////

        			semop_retval = semop( answer_sem_id, &semaphore_struct, 1 ) ;
        			if( semop_retval == -1 )
            				printf( "*1* error in semaphore operation\n" ) ;
        			else 
					{
       					//  ??  serve ??  answer_sem_val = semctl( answer_sem_id, 0, GETVAL, NULL ) ;
					strcpy(answer_shame_pointer, "busy");
					}

     				/* turn on the green light */ 
        			answer_sem_val = semctl( answer_sem_id,  0, SETVAL, 1 ) ;

				////////////////////////////////////////////////////////////////////
				////////////////////////////////////////////////////////////////////
				////////////////////////////////////////////////////////////////////
				strcpy(client_msg, comando);
				printf("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF command is %s \n", client_msg);

                    		exec_command( &motcntr_mode, &not_end ) ;

				//sleep(10); // simulo ritardo
				printf("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF comando eseguito\n");
			
				////////////////////////////////////////////////////////////////////
				//                     	 SCRIVO MEMORIA CONDIVISA                 //
				////////////////////////////////////////////////////////////////////

        			semop_retval = semop( answer_sem_id, &semaphore_struct, 1 ) ;
        			if( semop_retval == -1 )
            				printf( "*1* error in semaphore operation\n" ) ;
        			else 
					{
       					//  ??  serve ??  answer_sem_val = semctl( answer_sem_id, 0, GETVAL, NULL ) ;
					strcpy(answer_shame_pointer, "done");
					}

     				/* turn on the green light */ 
        			answer_sem_val = semctl( answer_sem_id,  0, SETVAL, 1 ) ;

				////////////////////////////////////////////////////////////////////
				////////////////////////////////////////////////////////////////////
				///////////////////////////////////////////////////////////////////////////
				}

			
			
		
			usleep(100000); // microsecondi
			} while (TRUE);
		}


        /* the following statements will be cleared in future */
        alarm(0) ;
        signal( SIGALRM, SIG_IGN ) ;

        if ( motcntr_mode ) {
            /* close tty */
            printf( "\n%s: closing tty\n", myname ) ;
            fclose( rs232w ) ;
            fclose( rs232r ) ;
            }

        printf( "\n%s: closing main channel\n", myname ) ;
        close( main_channel ) ;

        /* ready to accept an other connection */

    } while( TRUE ) ;
}

