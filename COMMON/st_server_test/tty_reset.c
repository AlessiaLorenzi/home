#include "server.h"
#include "server_const.h"
#include "server_ext_vars.h"
#include <sys/time.h>

/***********************************************************************
    ma-device SERVER DEPENDENT reset function:
    to see if the device is alive
***********************************************************************/

tty_reset()
{

    extern int verbose ;
    extern void loop_timeout() ;

    static struct timeval timeout = { (long) 10, (long)0 };

    time_t local_time;

    int i, line_counter, return_val ;
    char c ;
    fd_set ready;

    char s[BUF_SIZE] ;

    fflush( stdout ) ;
 /*printf( "\n%s: tty reset activated\n", myname ) ;
 printf ("\nRESET begin\n" );*/

#ifdef ORA_FACCIO_RESET  
printf("======================= eseguo il reset\n");
    for ( line_counter = 0 ; line_counter < 8 ; line_counter++ )
        {
        if ( serial[line_counter].mode == 1 )
                {

                fprintf( serial[line_counter].rs232w, "002\r" ) ;
                fflush( serial[line_counter].rs232w ) ; 
                sleep(1) ;

                FD_SET( serial[line_counter].fildes, &ready);
                if ( return_val = select ( getdtablesize(), &ready,
                              (fd_set *)NULL, (fd_set *)NULL, &timeout )> 0 )
                    { /* read 3 strings ending with '>' */

                    i = 0 ;
                    do {
                        c = fgetc( serial[line_counter].rs232r ) ;
                        device_msg[i] = c ;
                        i++ ;
                        } while ( c != '>' ) ;
                    device_msg[i] = 0 ;

                        printf( "%s: 1st answer is ^^^ %s $$$\n",
                                    myname, device_msg ) ;

                    i = 0 ;
                    do {
                        c = fgetc( serial[line_counter].rs232r ) ;
                        device_msg[i] = c ;
                        i++ ;
                        } while ( c != '>' ) ;
                    device_msg[i] = 0 ;

                        printf( "%s: 2nd answer is ^^^ %s $$$\n",
                                myname, device_msg ) ;

                    i = 0 ;
                    do {
                        c = fgetc( serial[line_counter].rs232r ) ;
                        device_msg[i] = c ;
                        i++ ;
                        } while ( c != '>' ) ;
                    device_msg[i] = 0 ;

                        printf( "%s: 3rd answer is ^^^ %s $$$\n",
                            myname, device_msg ) ;

                    }

                FD_CLR( serial[line_counter].fildes, &ready);
                if ( return_val == 0 ) 
			{
			local_time = time((time_t *) 0 );
			printf( "\n%s timeout on reset at %s\n",
			myname, ctime(&local_time));
			alim[serial[line_counter].alim[0]].mode = 0 ;
			alim[serial[line_counter].alim[1]].mode = 0 ;
			alim[serial[line_counter].alim[2]].mode = 0 ;
			alim[serial[line_counter].alim[3]].mode = 0 ;
			}


                }
        }


#endif

/*
printf ("RESET end\n" );*/ 
    fflush( stdout ) ;
}

