#include "server.h"
#include "server_const.h"
#include <unistd.h>
#include <string.h>


extern int tesla_ok;
extern int fildes ;
extern int tesla_fildes ;
extern int verbose, alarm_enabled, alarm_allowed ;
extern char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ] ;
extern char myname[ NCHARS ], alarm_headmsg[ NCHARS ] ;
extern FILE *rs232r, *rs232w ;
extern FILE *tesla_rs232r, *tesla_rs232w ;
extern struct supply_data supply[100];
extern int address[100];
extern int alim_mode[100];
extern int supply_number;
extern int tesla_type;
extern char *message;
int return_val;
float tesla_value;

char tmp_msg[ BUF_SIZE ] ;
fd_set ready;



/***********************************************************************
    ma-device SERVER DEPENDENT reset function:
    to see if the device is alive
***********************************************************************/

tty_reset()
{
    extern char myname[ NCHARS ] ;
    printf( "\n%s: all right! tty on line\n", myname ) ;

}


tesla_reset()
{
	int car;
	int count;
    static struct timeval timeout = {(long)2, (long)0};
    extern char myname[ NCHARS ] ;
    printf( "\n%s: tesla : all right! tty on line\n", myname ) ;



            FD_ZERO(&ready);
            fprintf( tesla_rs232w, ""  ) ;
            fflush( tesla_rs232w ) ;
	    sleep(1);


            FD_SET(tesla_fildes, &ready);
            timeout.tv_sec = 5 ;
            timeout.tv_usec = 0 ;
            if ( return_val = select( getdtablesize(),
                              &ready,
                              (fd_set *)NULL,
                              (fd_set *)NULL,
                              &timeout) > 0 ) {
	    count=0;
	    do 
		{
		car = fgetc(tesla_rs232r);
		tmp_msg[count] = car;
		count++;
		} while ( car != 13);
            }

	    FD_CLR(tesla_fildes, &ready);
            tesla_ok = 0;
            if (return_val == 0)
                {
                tesla_ok = 0;
                on_tesla_timeout();
                }
        else
                {
                tesla_ok = 1 ;
		printf(" in tty_reset, answer from tesla = %s\n", tmp_msg);
		if ( strstr ( tmp_msg, "DTM133") != NULL ) tesla_type = 1;
		if ( strstr ( tmp_msg, "DTMS") != NULL ) tesla_type = 2;
                }

}
