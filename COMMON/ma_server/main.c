#include "server.h"
#include "server_const.h"
#include "server_vars.h"

FILE *rs232r, *rs232w ;
char tty_name[ NCHARS ] ;
int fildes ;
int main_socket;

/***********************************************************************

    This is a server-client process for a delta machine
    ( mc68000 vme architecture - risc 4.2bsd and system V environment ).
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
    if alarm are managed also cryostat_number, qwr_number must be sent
                               [0-23]           [0-100]

***********************************************************************/

/* global vars: used by several functions */

int main_channel, alarm_channel, alarm_enabled, alarm_addrlen,
    alarm_allowed, mode = 0, verbose = 0 ;
int giorno_corrente;
FILE *log_file;

    int supply_number;
    char *message;

char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ],
     alarm_msg[ BUF_SIZE ], alarm_headmsg[ NCHARS ], myname[ NCHARS ] ;

int address[100];
int alim_mode[100];

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
    
    int  alarm_socket,
        gen_chan, gen_chan_addrlen,
        read_retval, write_retval, signal_retval, not_end,
        channel_addrlen ;
    struct sockaddr_in gen_chan_addr, channel_addr ;
    struct hostent * hp, * gethostbyname() ;
    char alarm_machine[ NCHARS ] ;

    int connect_alarm() ;
    void on_alarm() ;

char file_daaprire[100];


time_t t;
struct tm *tempo;


t = time( (long*) 0);
tempo = localtime (&t);

strcpy( myname, argv[0] ) ;
main_socket = atoi( argv[1] ) ;


sprintf(file_daaprire, "./logs/server%d_%d.log", main_socket, tempo->tm_yday);
giorno_corrente = tempo->tm_yday;
log_file = fopen (file_daaprire, "a");




    /* check args */
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

    do { /* 1 */

        /* accept connection from client */
        main_channel = accept( gen_chan, &gen_chan_addr,
                               &gen_chan_addrlen ) ;
        printf( "\n%s: opening main channel\n", myname ) ;

        /* read the first bunch of datas from the client i.e.
           alarm server address and server-dependent stuff */
        alarm_allowed = FALSE ;
        alarm_machine[0] = NULL ;
        first_read( alarm_machine, &alarm_socket, &mode ) ;



        /* make server-dependent initial house-keeping */
        house_keeping( mode ) ;

        if ( alarm_machine[0] != NULL ) {

            /* alarm checking must be performed:
               connect alarm channel */
        //    alarm_allowed = connect_alarm( alarm_machine,
         //                                  alarm_socket ) ;

            /* disable alarm check (must be enabled by the client) */
            alarm_enabled = FALSE ;
        }

        not_end = TRUE ;

        /* start endless loop */
printf("endless loop %s\n", myname);
        do {
            /* read from client */
            read_retval = read( main_channel, client_msg, BUF_SIZE ) ;
            if ( read_retval <= 0 ) {
                if ( read_retval < 0 ) {
                    if ( errno != EINTR )
                        printf( "\n%s: error reading main socket\n", myname ) ;
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

//printf(" ********* exec command %s\n", myname);
//printf("\n\n\nGR -> %36s\n",  client_msg);


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
//printf("GR <- %36s\n\n",  device_msg);
                    /* unmask alarm signal, if necessary */
                    if ( alarm_enabled ) {
                        signal( SIGALRM, on_alarm ) ;
                        alarm( ALARM_PERIOD ) ;
                        }

                    }
            }
        } while ( not_end ) ;

        signal( SIGALRM, SIG_IGN ) ;

        printf( "\n%s: closing main channel\n", myname ) ;
        close( main_channel ) ;
        if ( alarm_machine[0] != NULL ) {
            printf( "\n%s: closing alarm channel\n", myname ) ;
            close( alarm_channel ) ;
            }

        /* ready to accept an other connection */

    } while( TRUE ) ;/* 1 */
}

