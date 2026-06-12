#include "server.h"
#include "server_const.h"
#include "server_ext_vars.h"

/***********************************************************************

    SERVER DEPENDENT
    This is the function performing local housekeeping.
    if ( mode ) then normal mode, else stub mode

***********************************************************************/

house_keeping()

{
    int write_retval ;

    /* tty W/R */
        tty_init() ;
        tty_reset() ;
}

