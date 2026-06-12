
#include "server.h"
#include "motor_server.h"

/***********************************************************************

    SERVER DEPENDENT
    This is the function performing local command execution
    if ( *motcntr_mode > 0 ) then normal mode, else stub mode

    DOC BEGIN

    RESET        'r'
    DOIT         '!'

    SERVER DEPENDENT commands are

    reset                 reset tty (send ON)
                          i.e. "reset"

    !motor steps speed    move motor [1-8] of steps at speed
                          i.e. "!4 500 100" means
                          move motor 4 of 500 steps at speed 100

    DOC END

***********************************************************************/

int alarm_flag = 1;

int on_alarm()
{
    extern char myname[ NCHARS ] ;
    extern char device_msg[ BUF_SIZE ] ;
    time_t local_time;

    printf( "\n%s timeout occured at^G %s\n",
    myname, ctime(&local_time));
    alarm_flag = 0 ;

}


do_read()
{
   int i ;
   char c ;
    extern char myname[ NCHARS ] ;

   extern char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ] ;
   extern FILE *rs232r, *rs232w ;
   extern int verbose;
    
char device_msg1[100];
   int not_echo_flag = TRUE ;
   alarm_flag = 1 ;
   do {
	if ( alarm_flag )
	    fgets( device_msg, BUF_SIZE, rs232r ) ; /* prompt & echo */
        if ( device_msg[0] == client_msg[1] ) /* echo of 1st char */
	    not_echo_flag = FALSE ;
        printf( "\n%s: stringa : %s\n", myname, device_msg ) ;
      } while ( not_echo_flag && alarm_flag ) ;
   if ( verbose )
        printf( "\n%s: prompt is %s\n", myname, device_msg ) ;
   if (alarm_flag){
       if ( alarm_flag )
           fgets( device_msg, BUF_SIZE, rs232r ) ; /* answer */
       if ( verbose )
           printf( "\n%s: answer is %s\n", myname, device_msg ) ;
       i = 0 ;
       do {
           if (alarm_flag)
               c = fgetc( rs232r ) ;
           i++ ;
          } while ( (c != '>') && alarm_flag ) ;
      }
   return (alarm_flag) ;
}

exec_command( motcntr_mode, flag_pointer )
int *motcntr_mode, *flag_pointer ;
{
    int on_alarm(), do_read() ;
    extern int fildes, verbose ;
    extern char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ] ;
    extern char myname[ NCHARS ] ;
    extern FILE *rs232r, *rs232w ;

    char command, c ;
    static int i = 0 ;
    int nsteps ;

    printf( "%s: (exec) command is %s \n", myname, client_msg ) ;

    sscanf( client_msg, "%c", &command ) ;
    strcpy( device_msg, POS_ANSWER ) ;
    switch ( command ) {
        case EOC :
        *flag_pointer = FALSE ;
        break;

        case RESET:

        *motcntr_mode = TRUE ;
        printf( "\n%s: trying a reset\n", myname ) ;
        close( fildes ) ;
        fclose( rs232r ) ;
        fclose( rs232w ) ;
        printf( "\n%s: trying an init\n", myname ) ;
        tty_init() ;
        printf( "\n%s: trying an init\n", myname ) ;
        tty_reset() ;

        if ( *motcntr_mode )
            strcpy( device_msg, " tty reset OK " ) ;
        else
            strcpy( device_msg, " tty reset failed " ) ;

        break ;

        case DOIT:
        /* motor server specific message execution: first char must
           be '!' */
        /* send the message */
        if ( *motcntr_mode ) {

            fprintf( rs232w, "%s\r", ( client_msg + 1 ) ) ;
            fflush( rs232w ) ;
	    signal( SIGALRM, on_alarm ) ;
	    alarm(25) ;
	    siginterrupt(SIGALRM, 1);

            if ( do_read() != 0 ){
		 printf( "MOTOR SERVER: last string was %s\n", device_msg ) ;
	        }
            else 
	        {
		 strcpy( device_msg, "Not Done" ) ;
		 printf( "MOTOR SERVER: READ ERROR^G^G^G\n" ) ;
		 *motcntr_mode = 0 ;
	        }

	    signal( SIGALRM, SIG_IGN ) ;
                
            }
        else {
                strcpy( device_msg, "Not Done 3" ) ;
            }
        break ;
        default:
        printf( "\n%s: unknown command\n", myname ) ;
        strcpy( device_msg, NEG_ANSWER ) ;
        break ;
    }
}

