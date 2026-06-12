/* STEERERS */
#include "server.h"
#include "server_const.h"
#include "server_vars.h"

char tty_name[ NCHARS ] ;

int main_channel, alarm_channel, alarm_enabled, alarm_addrlen,
    alarm_allowed, mode = 0, verbose = 0 ;

    int supply_number;
    char *message;

main( argc, argv )
int argc ;
char **argv ;
{
    extern int alarm_enabled, alarm_allowed,
               mode, verbose ;




    int main_socket, alarm_socket,
        gen_chan, gen_chan_addrlen,
        read_retval, write_retval, signal_retval, not_end,
        channel_addrlen ;
    struct sockaddr_in gen_chan_addr, channel_addr ;
    struct hostent * hp, * gethostbyname() ;
    char alarm_machine[ NCHARS ] ;

int alim_counter, line_counter;


/*
    int connect_alarm() ;
    void on_alarm() ;
*/

    /* check args */
    strcpy( myname, argv[0] ) ;
    main_socket = atoi( argv[1] ) ;
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

	setsockopt(gen_chan, SOL_SOCKET, SO_REUSEADDR, 0, 0);


    /* give a name to socket using wildcards */
    gen_chan_addr.sin_family = AF_INET ;
    gen_chan_addr.sin_addr.s_addr = INADDR_ANY ;
    gen_chan_addr.sin_port = htons( main_socket ) ;
    gen_chan_addrlen = sizeof( gen_chan_addr ) ;
    channel_addrlen = sizeof( channel_addr ) ;
    if ( bind( gen_chan,(const struct sockaddr *)&gen_chan_addr, gen_chan_addrlen ) < 0 ) {
        printf( "\n%s: error binding socket\n", myname ) ;
        exit( 0 ) ;
        }

    /* Find out assigned port number and print it out */
    if ( getsockname( gen_chan, &gen_chan_addr, &gen_chan_addrlen )
         < 0 ) {
        printf( "\n%s: error getting socket name\n", myname ) ;
        exit( 0 ) ;
        }
    //printf( "\n%s: main socket is %d\n", myname,
     //       ntohs( gen_chan_addr.sin_port ) ) ;

    /* Start accepting connections */
    if ( listen ( gen_chan, 5 ) < 0 ) {
        printf( "\n%s: error listening socket\n", myname ) ;
        exit( 0 ) ;
        }

    do { /* 1 */

        /* accept connection from client */
        main_channel = accept( gen_chan, &gen_chan_addr,
                               &gen_chan_addrlen ) ;
        /*printf( "\n%s: opening main channel\n", myname ) ;*/

        /* read the first bunch of datas from the client i.e.
           alarm server address and server-dependent stuff */
        alarm_allowed = FALSE ;
        alarm_machine[0] = '\0' ;
        //alarm_machine[0] = NULL ;


for ( line_counter = 0 ; line_counter < 8 ; line_counter++ )
	{
	strcpy( serial[line_counter].name, "NULL" );
	serial[line_counter].mode = 999999;
	serial[line_counter].verbose = 999999;
	for ( alim_counter = 0 ; alim_counter < 4 ; alim_counter++ )
		serial[line_counter].alim[alim_counter] = 999999;
	}



        first_read( alarm_machine, &alarm_socket, &mode ) ;



/* inizializza strutture */


for ( line_counter = 0 ; line_counter < 8 ; line_counter++ )
	{
	if ( strncmp (  serial[line_counter].name, "/dev/null", 9 ) == 0 )
		serial[line_counter].mode = 0;

	for ( alim_counter = 0 ; alim_counter < 12 ; alim_counter++ )
		{
		if ( serial[line_counter].alim[alim_counter] != 999999 )
			{
			alim[serial[line_counter].alim[alim_counter]].address = alim_counter + 1;
			alim[serial[line_counter].alim[alim_counter]].mode = serial[line_counter].mode;
			alim[serial[line_counter].alim[alim_counter]].verbose = serial[line_counter].verbose;
			if (alim[serial[line_counter].alim[alim_counter]].verbose)
			printf (" ++ ++ MAIN  :  alim = %d, address = %d , verbose = %d \n", serial[line_counter].alim[alim_counter], alim[serial[line_counter].alim[alim_counter]].address,alim[serial[line_counter].alim[alim_counter]].verbose );
			}
		}
	}



        /* make server-dependent initial house-keeping */

        house_keeping( mode ) ;

        not_end = TRUE ;


        /* start endless loop */
        do {
            /* read from client */
            read_retval = read( main_channel, client_msg, BUF_SIZE ) ;
            if ( read_retval <= 0 ) {
                if ( read_retval < 0 ) {
                    if ( errno != EINTR )
                        printf( "\n%s: error reading main socket\n", myname ) ;
                    client_msg[0] = '\0' ;
                    }
                else
                   not_end = FALSE ;
                }
            if ( not_end ) {
                if ( client_msg[0] != '\0' ) {

                    /* cmd has been read, then exec */
//printf("chiamo exec_command------------\n");
                    exec_command( &mode, &not_end ) ;
//printf("exec_command ended------------\n");

                    /* answer to client */
                    do {
                        write_retval = write( main_channel,device_msg, BUF_SIZE ) ;
                        if ( write_retval <= 0 )
                            printf( "\n%s: error writing main socket\n",myname ) ;
                        } while ( write_retval <= 0 ) ;

                    }
            }
        } while ( not_end ) ;
printf("exec_command ended------------\n");

        signal( SIGALRM, SIG_IGN ) ;

        /*printf( "\n%s: closing main channel\n", myname ) ;*/
        close( main_channel ) ;
        if ( alarm_machine[0] != '\0' ) {
            printf( "\n%s: closing alarm channel\n", myname ) ;
            close( alarm_channel ) ;
            }

        /* ready to accept an other connection */

    } while( TRUE ) ;/* 1 */
}

