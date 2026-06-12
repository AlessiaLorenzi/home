
#include "server.h"

/***********************************************************************

    SERVER DEPENDENT
    This is the function performing local housekeeping.
    if ( mode > 0 ) then normal mode, else stub mode

    DOC BEGIN

    "/bin/stty 9600 line 0 raw < " on tty
    open and reset each tty [ ON ]

    DOC END

***********************************************************************/

house_keeping( motcntr_mode )
int motcntr_mode ;
{
    extern int main_channel ;
    extern char device_msg[ BUF_SIZE ] ;
    extern char myname[ NCHARS ] ;
    int write_retval ;

    /* tty W/R */
    if ( motcntr_mode ) {
printf("-----------hk1-------------\n");

        tty_init() ;
printf("-----------hk2-------------\n");
        tty_reset() ;
printf("-----------hk3-------------\n");
    }
    else
        strcpy( device_msg, "Not Done (housekeeping stub reset) " ) ;
    write_retval = write( main_channel, device_msg, BUF_SIZE ) ;

    if ( write_retval < 0 )
        printf( "\n%s: error writing main socket\n", myname ) ;

}

