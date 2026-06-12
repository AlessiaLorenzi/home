
#include "server.h"
#include "motor_server.h"

FILE *rs232r, *rs232w ;
char tty_name[ NCHARS ] ;
int motcntr_mode = FALSE;

int timeout_flag = FALSE ;

int fildes = -1 ;

/* global vars: used by several functions */

int main_channel, alarm_channel, alarm_enabled, alarm_addrlen,
    alarm_allowed, verbose = 0 ;

char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ],
     alarm_msg[ BUF_SIZE ], alarm_headmsg[ NCHARS ], myname[ NCHARS ] ;

struct sockaddr_in alarm_addr ;

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
 
        /* start endless loop */
        do {

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

            if ( not_end ) {
                if ( client_msg[0] != NULL ) {

                    /* cmd has been read, then exec */

                    /* mask alarm signal, if necessary */
                    if ( alarm_enabled )
                        signal( SIGALRM, SIG_IGN ) ;

                    exec_command( &motcntr_mode, &not_end ) ;

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
        } while ( not_end ) ;

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

