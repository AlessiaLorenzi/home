#include "server.h"
#include "server_const.h"


/***********************************************************************

    SERVER DEPENDENT
    This is the function performing local housekeeping.
    if ( mode ) then normal mode, else stub mode

***********************************************************************/

house_keeping( mode )
int mode ;
{
    extern int main_channel ;
    extern char device_msg[ BUF_SIZE ] ;
    extern char myname[ NCHARS ] ;
    int write_retval ;

    /* tty W/R */
    if ( mode ) {
        tty_init() ;
        tty_reset() ;
    }
    else
        strcpy( device_msg, " stub reset done " ) ;
}

