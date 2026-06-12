
#include "server.h"
#include "motor_server.h"

/***********************************************************************
    motor SERVER DEPENDENT reset function: to see if the device is alive
***********************************************************************/
int res_counter, res_alarm_flag = 1;

int res_on_alarm()
{
    extern char myname[ NCHARS ] ;
    extern char device_msg[ BUF_SIZE] ;
    extern char tty_name[ NCHARS ] ;
    time_t local_time;

    printf( "\n%s reset timeout occured at^G %s\n",
            myname, ctime(&local_time));
    res_alarm_flag = 0 ;
    sprintf(device_msg, "Not Done : reset timeout on line %s", tty_name);

}

res_do_read()
{
    int i ;
    char c ;
    extern int verbose;
    extern char device_msg[ BUF_SIZE ];
    extern FILE *rs232r, *rs232w;
    char stringa[100];
    int not_echo_flag = TRUE ;
    extern char myname[ NCHARS ] ;
    
    res_alarm_flag = 1 ;
    do {
	if ( res_alarm_flag ) 
            fgets( device_msg, BUF_SIZE, rs232r ) ; /* prompt & echo */
        if ( strncmp( device_msg, "addr?", 5 ) == 0 )
             not_echo_flag = FALSE ;
       } while ( not_echo_flag && res_alarm_flag ) ;
    if ( verbose )
         printf( "\n%s: prompt is %s\n", myname, device_msg ) ;
    if (res_alarm_flag){
        fgets( device_msg, BUF_SIZE, rs232r ) ; /* answer */
    if ( verbose )
         printf( "\n%s: answer is %s\n", myname, device_msg ) ;
    i = 0;
    do {
         c = fgetc( rs232r ) ;
         i++ ;
       } while ( ( c != '>' ) && res_alarm_flag ) ;
    fgets( stringa, BUF_SIZE, rs232r ) ; /* cr pendente */
    }
    return (res_alarm_flag) ;

}

tty_reset()
{
    extern char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ] ;
    extern char myname[ NCHARS ] ;
    extern FILE *rs232r, *rs232w ;

    extern int fildes,  verbose ;
    extern *motcntr_mode;
    char command, c ;
    int i, return_val;

    /* strcpy(device_msg, "Not Done"); */
printf("-----------ttr1-------------\n");

    if ( motcntr_mode ) {
printf("-----------ttr1a-------------\n");
        fprintf( rs232w, "addr?\r" ) ;
printf("-----------ttr1b-------------\n");
        fflush( rs232w ) ;
printf("-----------ttr2-------------\n");
        signal( SIGALRM, res_on_alarm ) ;
        alarm(10) ;
	siginterrupt(SIGALRM, 1);
        if ( res_do_read() != 0 ){
             printf( "RF SERVER: last string was %s\n", device_msg ) ;
	   } 
        else
	   {
             printf( "RF SERVER: READ ERROR^G^G^G\n" ) ;
	     motcntr_mode = FALSE ; /* set stub mode */
           }      
printf("-----------ttr3-------------\n");
        signal( SIGALRM, SIG_IGN ) ;
         
        }
printf("-----------ttr4-------------\n");

    printf( "\n%s: all right! tty on line\n", myname ) ;

}

