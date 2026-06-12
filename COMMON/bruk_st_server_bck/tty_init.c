#include "server.h"
#include "server_const.h"
#include "server_ext_vars.h"
#include <sgtty.h>
#include <termios.h>






extern struct line serial[8];
extern struct alim_data alim[100];

/***********************************************************************
    tty SERVER DEPENDENT vars initializations
***********************************************************************/

tty_init()

{

    int  counter, cnt;
    int retrn;
    int ret_val;
struct termios tty_termios ;

    /* open line towards controller */

counter = 0;

	if ( serial[counter].mode == 1 )
		{
		/*if ( serial[counter].verbose ) */
			printf ("apertura della linea %s\n", serial[counter].name );

		serial[counter].fildes = open(serial[counter].name, O_RDWR ) ;
		if (serial[counter].fildes == -1)
			{
		if ( serial[counter].verbose ) 
			printf ("\007****************************   errore nel primo open %s\n", 
			sys_errlist[errno]);
			serial[counter].fildes = open(tty_name, O_RDWR ) ;
			if (serial[counter].fildes == -1)
				{
		if ( serial[counter].verbose ) 
				printf ("\007\007\007\007****   errore nel secondo open %s\n", 
				sys_errlist[errno]);
				fflush(stdout);
				exit (0);
				}
		if ( serial[counter].verbose ) 
				printf ("OK open 2 %s\n", serial[counter].name );
			}
		if ( serial[counter].verbose ) 
			printf ("OK open %s\n", serial[counter].name );








    ret_val = tcgetattr( serial[counter].fildes, &tty_termios ) ;
        if (ret_val == -1) {
                printf( "%s error (dev get attr)\n", myname ) ;
        return ;
                                        }

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

    ret_val = tcsetattr( serial[counter].fildes, TCSANOW, &tty_termios ) ;
        if (ret_val == -1) {
                printf( "%s error (dev set attr)\n", myname ) ;
        return ;
                                                        }








		serial[counter].rs232r = fdopen( serial[counter].fildes, "r" ) ;
		if ( serial[counter].rs232r == NULL ) 
			{
			printf( "\n%s: fdopen : error opening tty for reading\n", myname ) ;
			printf( "\n%s: error opening tty %s\n", myname, serial[counter].name ) ;
			printf( "\n%s: length = %d\n", myname, strlen(serial[counter].name));
			exit( 0 ) ;
			}
		if ( serial[counter].verbose ) 
		printf ("OK fdopen %s\n", serial[counter].name );

		serial[counter].rs232w = fdopen( serial[counter].fildes, "w" ) ;
		if ( serial[counter].rs232w == NULL ) 
			{
			printf( "\n%s: fdopen : error opening tty for writing\n", myname ) ;
			printf( "\n%s: error opening tty %s\n", myname, serial[counter].name ) ;
			printf( "\n%s: length = %d\n", myname, strlen(serial[counter].name));
			exit( 0 ) ;
			}
		if ( serial[counter].verbose ) 
		printf ("OK fdopen %s\n", serial[counter].name );
		}

	for (cnt = 0 ; cnt < 12 ; cnt++)
		{
		alim[serial[counter].alim[cnt]].rs232w = serial[counter].rs232w;
		alim[serial[counter].alim[cnt]].rs232r = serial[counter].rs232r;
		alim[serial[counter].alim[cnt]].fildes = serial[counter].fildes;
		}
}
 

