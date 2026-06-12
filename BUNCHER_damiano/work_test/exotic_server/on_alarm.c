#include "server.h"
#include "server_const.h"


/***********************************************************************

    This is the function performing periodic checks, if necessary
    ( i.e. if checks are enabled )

***********************************************************************/

void on_alarm()
{
    extern int mode ;

    check_alarm( mode ) ;

    signal( SIGALRM, on_alarm ) ;
    alarm( ALARM_PERIOD ) ;
}

