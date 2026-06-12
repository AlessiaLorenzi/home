#include "server.h"
#include "server_const.h"

#define STAT "stat?\r"
#define MASK_B0 0x01
#define MASK_B7 0x80

/***********************************************************************

    SERVER DEPENDENT
    This is the function performing local alarm check
    if ( mode ) then normal mode, else stub mode

***********************************************************************/

check_alarm( mode )
int mode ;
{
    extern int verbose, alarm_channel, alarm_addrlen ;
    extern char device_msg[ BUF_SIZE ], alarm_msg[ BUF_SIZE ] ;
    extern char myname[ NCHARS ] ;
    extern char alarm_headmsg[ NCHARS ] ;
    extern FILE *rs232r, *rs232w ;
    extern struct sockaddr_in alarm_addr ;

    int loop_power, locked, status, sendto_retval ;

    /* ma server specific alarm check */
    if ( mode ) {
        fprintf( rs232w, "%s", "S1\r" ) ;
        /* fflush( rs232w ) ; ??? */
        /* get the answer */

        fgets( device_msg, BUF_SIZE, rs232r ) ; /* this is the answer */
        if ( verbose ) printf( "\n%s: answer is %s\n", myname,
                               device_msg ) ;

        sprintf( alarm_msg, "%s: %s", alarm_headmsg, device_msg ) ;
        sendto_retval = sendto( alarm_channel, alarm_msg,
                                BUF_SIZE,
                                0, &alarm_addr, alarm_addrlen ) ;
        if ( sendto_retval < 0 )
            printf( "\n%s: error writing alarm socket\n", myname ) ;
        }
    else {
        sprintf( alarm_msg, "%s: unknown", alarm_headmsg ) ;
        sendto_retval = sendto( alarm_channel, alarm_msg, BUF_SIZE,
                                0, &alarm_addr, alarm_addrlen ) ;
        if ( sendto_retval < 0 )
            printf( "\n%s: error writing alarm socket\n", myname ) ;
        }

}

