
#include "server.h"

/***********************************************************************
    tty SERVER DEPENDENT vars initializations
***********************************************************************/
int  res_open_flag = 1;



int res_on_open()
{
    extern char myname[ NCHARS ] ;
    extern char tty_name[ NCHARS ] ;
    extern *motcntr_mode;
    extern char device_msg[ BUF_SIZE] ;
    time_t local_time;

    printf( "\n%s open timeout occured at^G %s\n",
            myname, ctime(&local_time));
    res_open_flag = 0 ;
    motcntr_mode = 0 ;
    sprintf(device_msg, "Not Done : open timeout on line %s", tty_name);

}



tty_init()

{
    extern char myname[ NCHARS ] ;
    extern char tty_name[ NCHARS ] ;
    extern FILE *rs232r, *rs232w ;
    extern char device_msg[ BUF_SIZE] ;

    struct sgttyb pp;
    extern int fildes, errno;
    extern char *sys_errlist[] ;

    int ret_val ;
    int ldisc = NTTYDISC ;

    /* open line towards controller */

    pp.sg_ispeed = B9600 ;
    pp.sg_ospeed = B9600 ;
    pp.sg_flags = RAW | TANDEM ;


    res_open_flag = 1;

    /* open line towards controller */


        signal( SIGALRM, res_on_open ) ;
        alarm(20) ;
        siginterrupt(SIGALRM, 1);

    fildes = open(tty_name, O_RDWR ) ;
    if (fildes == -1) {
        printf( "%s error (dev open) %s\n", myname, sys_errlist[errno] ) ;
        res_open_flag = 0 ;
        motcntr_mode = 0 ;
        sprintf(device_msg, "Not Done : %s on line %s", sys_errlist[errno], tty_name );
        }

    if ( motcntr_mode )
       {
    ret_val = ioctl( fildes, TIOCSETD, &ldisc ) ;
    if ( ret_val < 0 )
        printf( "\n%s: error on ioctl (ldisc) \n", myname ) ;
	}

    /* set della velocita' della porta */
    if ( motcntr_mode )
        {
    ret_val = ioctl( fildes, TIOCSETN, &pp ) ;
    if ( ret_val < 0 )
        printf( "\n%s: error on ioctl (raw) \n", myname ) ;
	}

    if ( motcntr_mode )
        {
    rs232r = fdopen( fildes, "r" ) ;
    if ( rs232r == NULL ) 
		{
        printf( "\n%s: error opening tty %s for reading\n", myname, tty_name ) ;
        exit( 0 ) ;
        }
    rs232w = fdopen( fildes, "w" ) ;
    if ( rs232w == NULL ) 
{
        	printf( "\n%s: error opening tty %s for writing\n", myname, tty_name ) ;
        	exit( 0 ) ;
        	}
	
	}

        signal( SIGALRM, SIG_IGN ) ;

}

