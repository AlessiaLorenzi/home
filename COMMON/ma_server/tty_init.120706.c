#include <termios.h>
#include <unistd.h>

#include "server.h"
#include "server_const.h"

extern int fildes ;
extern int tesla_fildes ;

/***********************************************************************
    tty SERVER DEPENDENT vars initializations
***********************************************************************/

tty_init()

{
    extern char tty_name[ NCHARS ] ;
    extern char tesla_tty_name[ NCHARS ] ;
    extern char myname[ NCHARS ] ;
    extern FILE *rs232r, *rs232w ;
    extern FILE *tesla_rs232r, *tesla_rs232w ;

    int ret_val;
    /* linux */
    struct termios tty_termios ;
    extern int errno;


    /* open line towards controller */
    fildes = open(tty_name, O_RDWR ) ;
    if (fildes == -1)
        {
        printf ("\007*** errore nel primo open\n" ) ; 
        fildes = open(tty_name, O_RDWR ) ;
        if (fildes == -1)
                {
                printf ("*** open error \n" ) ;
                fflush(stdout);
                exit (0);
                }
        }

    ret_val = tcgetattr( fildes, &tty_termios ) ;
    if (ret_val == -1) {
        printf( "%s error (dev get attr)\n", myname ) ;
        return ;
        }

    /* set del CS8 */
    /*** cfmakeraw( &tty_termios ) ; ***/

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


     cfmakeraw(  &tty_termios ) ;
        if (ret_val == -1) {
                printf( "%s error (dev set attr)\n", myname ) ;
        return ;
        }

    ret_val = tcsetattr( fildes, TCSANOW, &tty_termios ) ;
    if (ret_val == -1) {
        printf( "%s error (dev set attr)\n", myname ) ;
        return ;
        }


    rs232r = fdopen( fildes, "r" ) ;
    if ( rs232r == NULL ) {
        printf( "\n%s: error opening tty for reading\n", myname ) ;
        exit( 0 ) ;
        }
    rs232w = fdopen( fildes, "w" ) ;
    if ( rs232w == NULL ) {
        printf( "\n%s: error opening tty for writing\n", myname ) ;
        exit( 0 ) ;
        }
    sleep(1) ;
}
 
