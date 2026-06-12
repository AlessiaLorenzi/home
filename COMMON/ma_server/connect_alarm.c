#include "server.h"
#include "server_const.h"


/***********************************************************************

    This is the function connecting alarm socket to the remote server

***********************************************************************/

int connect_alarm( alarm_machine, alarm_socket )
int alarm_socket ;
char alarm_machine[ NCHARS ] ;
{
    extern int alarm_channel, alarm_addrlen ;
    extern char myname[ NCHARS ] ;
    extern struct sockaddr_in alarm_addr ;

    struct hostent * hp, * gethostbyname() ;

    /* Create a socket to/from alarm server ( server is client ) */
    alarm_channel = socket( AF_INET, SOCK_DGRAM, 0 ) ;
    if ( alarm_channel < 0 ) {
        printf( "\n%s: opening alarm socket\n", myname ) ;
        return ( FALSE ) ;
        }

    /* connect socket using name and socket-number given
       as parameters */
    alarm_addr.sin_family = AF_INET ;
    hp = gethostbyname( alarm_machine ) ;
    bcopy ( hp->h_addr, &( alarm_addr.sin_addr.s_addr ),
            hp->h_length ) ;
    alarm_addr.sin_port = htons( alarm_socket ) ;
    alarm_addrlen = sizeof( alarm_addr ) ;

    printf( "\n%s: alarm socket is %d\n", myname,
                ntohs( alarm_addr.sin_port ) ) ;

    return( TRUE ) ;

}

