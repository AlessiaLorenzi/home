
#include "alpi.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>

/***********************************************************************

init_com( host, socketnumber )
char *host, *socketnumber ;

          - open the communication channel to a network server

***********************************************************************/

init_com( host, socketnumber )
char *host, *socketnumber ;
{
    extern int server_channel ;
    extern char to_server[BUF_SIZE], from_server[BUF_SIZE] ;
    int server_chan_addrlen ;
    struct sockaddr_in server_chan_addr ;
    struct hostent * hp, * gethostbyname() ;

    /*printf( "\nalpi: host is %s socket has port number %s\n",
            host, socketnumber ) ;*/

    /* Create a socket to/from server ( as a client ) */
    
    server_channel = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
    if ( server_channel < 0 ) {
        perror( "\nclient: error opening to/from s streamsocket\n" ) ;
        exit( 0 ) ;
        }

	setsockopt(server_channel, SOL_SOCKET, SO_REUSEADDR, 0, 0);

    /* connect socket using name specified by command line */

    server_chan_addr.sin_family = AF_INET ;
    hp = gethostbyname( host ) ;
    bcopy ( hp->h_addr, &( server_chan_addr.sin_addr.s_addr ),
            hp->h_length ) ;
    server_chan_addr.sin_port = htons( atoi( socketnumber ) ) ;
    server_chan_addrlen = sizeof( server_chan_addr ) ;

    if ( connect( server_channel,
                  &server_chan_addr,
                  server_chan_addrlen ) < 0 ) {
        close( server_channel ) ;
        perror( "\nclient: error connecting streamsocket\n" ) ;
        exit( 0 ) ;
        }
    /*printf( "\nalpi: socket has port number %d\n",
            ntohs( server_chan_addr.sin_port ) ) ;*/

}
