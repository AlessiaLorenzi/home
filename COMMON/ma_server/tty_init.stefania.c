
#include "server.h"
#include <termios.h>
#include <unistd.h>

/***********************************************************************
    tty SERVER DEPENDENT vars initializations
***********************************************************************/

tty_init()

{
    extern char myname[ NCHARS ] ;
    extern char tty_name[ NCHARS ] ;
    extern FILE *rs232r, *rs232w ;

    /* linux */
    struct termios tty_termios ;
    //extern int ps_mode, ga_mode ;
    int ps_mode ;
    extern int fildes, errno;

    int ret_val ;

    /* linux : linux tty setup is DANFISYK PS
    B9600 ; B9600 ;
    CS8 ; i.e. 1 stop bit and 8 bit
    RAW ; i.e. char read */

    if ( ps_mode ) {

        fildes = open(tty_name, O_RDWR ) ;
        if (!(fildes > 0)) {
            printf( "%s error (dev open)\n", myname ) ;
	    ps_mode = 0 ;
            return ;
            }
        else {
            ret_val = tcgetattr( fildes, &tty_termios ) ;
            if ( ret_val < 0 ) {
                printf( "%s error (dev get attr)\n", myname ) ;
	        ps_mode = 0 ;
	        return ;
	        }
            }

        /* set del CS8 */
        cfmakeraw( &tty_termios ) ;

        /* set della velocita' della porta */
        ret_val = cfsetospeed( &tty_termios, B9600 ) ;
        if (ret_val == -1) {
            printf( "%s error (dev setting out speed)\n", myname ) ;
            return ;
            }
        ret_val = cfsetispeed( &tty_termios, B9600 ) ;
        if (ret_val == -1) {
            printf( "%s error (dev setting in speed)\n", myname ) ;
            return ;
            }

        ret_val = tcsetattr( fildes, TCSANOW, &tty_termios ) ;
        if (ret_val == -1) {
            printf( "%s error (dev set attr)\n", myname ) ;
            return ;
            }

        rs232r = fdopen( fildes, "r" ) ;
        if ( rs232r == NULL ) {
            printf( "\n%s: error opening tty %s for reading\n",
                    myname, tty_name ) ;
	    ps_mode = 0 ;
            return ;
            }
        else
            setvbuf( rs232r, (char *)NULL, _IONBF, 0 );

        rs232w = fdopen( fildes, "w" ) ;
        if ( rs232w == NULL ) {
            printf( "\n%s: error opening tty %s for writing\n",
                    myname, tty_name ) ;
	    ps_mode = 0 ;
            return ;
            }
        else
            setvbuf( rs232w, (char *)NULL, _IONBF, 0 );
        }

    /* linux : linux tty setup is Gauss-meter
    B9600 ; B9600 ;
    CS8 ; i.e. 1 stop bit and 8 bit
    RAW ; i.e. char read */

}

