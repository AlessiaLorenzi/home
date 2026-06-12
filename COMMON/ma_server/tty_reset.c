#include "server.h"
#include "server_const.h"


/***********************************************************************
    ma-device SERVER DEPENDENT reset function:
    to see if the device is alive
***********************************************************************/

tty_reset()
{
    extern char tty_name[ NCHARS ] ;
    extern char myname[ NCHARS ], device_msg[BUF_SIZE] ;
    extern FILE *rs232r, *rs232w ;
    int i ;
    char c ;

    char s[BUF_SIZE] ;
    printf( "\n%s: tty reset activated\n", myname ) ;
    printf( "\n%s: all right! tty on line\n", myname ) ;
}

