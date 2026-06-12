#include "server.h"
#include "server_const.h"


/***********************************************************************
    ma-device SERVER DEPENDENT reset function:
    to see if the device is alive
***********************************************************************/

tty_reset()
{
    extern char myname[ NCHARS ] ;
    printf( "\n%s: all right! tty on line\n", myname ) ;

}

