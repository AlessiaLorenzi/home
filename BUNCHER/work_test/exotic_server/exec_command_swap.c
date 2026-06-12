
#include "server.h"
#include "server_const.h"

static int timeout_flag = 0 ;

/***********************************************************************

    SERVER DEPENDENT PS
    This is the function performing local command execution
    if ( *mode ) then normal mode, else stub mode

***********************************************************************/

exec_command( mode, flag_pointer )
int *mode, *flag_pointer ;
{
    extern int fildes ;
    extern int verbose, alarm_enabled, alarm_allowed ;
    extern char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ] ;
    extern char myname[ NCHARS ], alarm_headmsg[ NCHARS ] ;
    extern FILE *rs232r, *rs232w ;
    extern struct supply_data supply[100];
    extern int address[100];
    extern int alim_mode[100];
    extern int supply_number;
    extern char *message;
    int return_val;

    char tmp_msg[ BUF_SIZE ] ;

    static int out_cur = 0, tesla = 0, out_volt = 0,
               int15pos = 0, int15neg = 0, int5pos = 0, 
    delta_t = 0, vce = 0, opt_iout = 0, aux_iout = 0;

    /* for stub mode */
    static int a = 0, p = 2048, kf = 1024, f = 25 ;

    static struct timeval timeout = {(long)3, (long)0};
    fd_set ready;

    int status, loop_power, locked ;
    char command, answer[ BUF_SIZE ] ;

    extern int timeout_flag ;
    extern void on_timeout() ;

    int rem_loc = REMOTE, retry;

    if ( verbose )
        printf( "%s: (exec) command is %s \n", myname, client_msg ) ;

    /* system call che serve per la gestione del TIMEOUT */
    FD_ZERO(&ready);

    message = client_msg + 10;

    sscanf ( client_msg, "ALIM = %d", &supply_number);

    sscanf( message, "%c", &command ) ;

    sprintf(device_msg, "STUB alim = %d in exec_command",supply_number);

    switch ( command ) {

        case EOC :
        *flag_pointer = FALSE ;
        break;

        case ALARMS_ON :
        if ( alarm_allowed )
            alarm_enabled = TRUE ;
        else
            strcpy( device_msg, NEG_ANSWER ) ;
        break;

        case ALARMS_OFF :
        alarm_enabled = FALSE ;
        break;

        case ALLADCS:
        device_msg[0] = 0 ;
        if ( alim_mode[supply_number])
            read_alladcs() ;
        else {
            strcpy( device_msg, "AD 0 = 999 " ) ;
            strcat( device_msg, "AD 2 = 999 " ) ;
            strcat( device_msg, "AD 3 = 999 " ) ;
            strcat( device_msg, "AD 4 = 999 " ) ;
            strcat( device_msg, "AD 5 = 999 " ) ;
            strcat( device_msg, "AD 6 = 999 " ) ;
            strcat( device_msg, "AD 7 = 999 " ) ;
            }
        break ;

        case RESET:
        if ( *mode ) {
            printf( "\n%s: tentativo di reset\n", myname ) ;
	    fclose( rs232r ) ;
	    fclose( rs232w ) ;
            printf( "\n%s: tentativo di init\n", myname ) ;
            tty_init() ;
            alim_mode[supply_number] = 1 ;
	    }
        else
            strcpy( device_msg, " stub device reset " ) ;
        break ;

        case DOIT:
        /* ma server specific message execution: first char must
           be '!' */

#ifndef ECR
        if ( alim_mode[supply_number] ) { /* if real */
            if ((( strncmp ( message , "!WA", 3 ) == 0 ) || 
                 ( strncmp ( message , "!N", 2 ) == 0 )  ||
                 ( strncmp ( message , "!RS", 3 ) == 0 ) ||
                 ( strncmp ( message , "!F", 2 ) == 0 ))) {
                fprintf( rs232w, "ADR %d\r",address[supply_number]);
                fprintf( rs232w, "CMDSTATE\r") ; 
                /* fflush( rs232w ) ;  ??? */
                fgets( device_msg, BUF_SIZE, rs232r ) ; /* answer */
                fgetc( rs232r ) ;
                if ( strncmp ( device_msg , "LOCAL", 5 ) == 0 )
                    rem_loc = LOCAL ;
                if ( strncmp ( device_msg , "REMOTE", 6 ) == 0 )
                    rem_loc = REMOTE ;
                }
            if ( ( ( strncmp ( message , "!WA", 3 ) != 0 ) && 
                   ( strncmp ( message , "!N", 2 ) != 0 )  &&
                   ( strncmp ( message , "!RS", 3 ) != 0 ) &&
                   ( strncmp ( message , "!F", 2 ) != 0 ) ) ||
                 ( ( ( strncmp ( message , "!WA", 3 ) == 0 ) || 
                     ( strncmp ( message , "!N", 2 ) == 0 )  ||
                     ( strncmp ( message , "!RS", 3 ) == 0 ) ||
                     ( strncmp ( message , "!F", 2 ) == 0 ) ) &&
                     ( rem_loc == REMOTE) ) ) {

                    retry = 0;
                    fprintf( rs232w, "ADR %d\r",address[supply_number]);
                    fprintf( rs232w, "%s\r", ( message + 1 ) ) ;
                    /* fflush( rs232w ) ;  ??? */

                    if ( ( strncmp ( message , "!AD", 3 ) == 0 )       ||
                         ( strncmp ( message , "!RA", 3 ) == 0 )       ||
                         ( strncmp ( message , "!WA", 3 ) == 0 )       ||
                         ( strncmp ( message , "!CMDSTATE", 9 ) == 0 ) ||
                         ( strncmp ( message , "!S1", 3 ) == 0 ) ) {
                        FD_SET(fildes, &ready);
                        if ( return_val = select( getdtablesize(),
                                                  &ready,
                                                  (fd_set *)NULL,
                                                  (fd_set *)NULL,
                                                  &timeout) > 0 ) {
                            fgets( device_msg, BUF_SIZE, rs232r ) ;
                            fgetc( rs232r ) ; 
                            }
                        FD_CLR(fildes, &ready);
                        if ( return_val == 0 )
                            on_timeout();
                        if ( verbose )
                            printf( "\n%s: answer is %s", myname, device_msg ) ;
                        }
                    }
            else 
                sprintf ( device_msg, "MA SERVER DONE" ) ;
            }
#else
        /* codice ECR */
        if ( alim_mode[supply_number] ) { /* if real */
            /* if NOT( WA OR N OR RS OR F ) OR
                     ( WA OR N OR RS OR F )AND( REMOTE ) */
            if ( ( ( strncmp ( message , "!WA", 3 ) != 0 ) && 
                   ( strncmp ( message , "!N", 2 ) != 0 )  &&
                   ( strncmp ( message , "!RS", 3 ) != 0 ) &&
                   ( strncmp ( message , "!F", 2 ) != 0 ) ) ||
                 ( ( ( strncmp ( message , "!WA", 3 ) == 0 ) || 
                     ( strncmp ( message , "!N", 2 ) == 0 )  ||
                     ( strncmp ( message , "!RS", 3 ) == 0 ) ||
                     ( strncmp ( message , "!F", 2 ) == 0 ) ) &&
                     ( rem_loc == REMOTE) ) ) {

                    if ( verbose )
                        printf( "\n%s: message is >%s<\n", myname, message ) ;
                    fprintf( rs232w, "%s\r", ( message + 1 ) ) ;
                    /*** tcdrain( fildes ) ; ***/
                    fflush( rs232w ) ;
 
                    if ( ( strncmp ( message , "!AD", 3 ) == 0 )       ||
                         ( strncmp ( message , "!RA", 3 ) == 0 )       ||
                         ( strncmp ( message , "!WA", 3 ) == 0 )       ||
                         ( strncmp ( message , "!CMDSTATE", 9 ) == 0 ) ||
                         ( strncmp ( message , "!S1", 3 ) == 0 ) ) {

                        if ( verbose )
                            printf( "\n%s: FDSET\n", myname ) ;
                        FD_SET(fildes, &ready);
			timeout.tv_sec = 5 ;
			timeout.tv_usec = 0 ;
                        if ( return_val = select( getdtablesize(),
                                                  &ready,
                                                  (fd_set *)NULL,
                                                  (fd_set *)NULL,
                                                  &timeout) > 0 ) {
                            fgets( tmp_msg, BUF_SIZE, rs232r ) ;
                            /* fgetc( rs232r ) ; */
                            }

                        FD_CLR(fildes, &ready);
                        if ( verbose )
                            printf( "\n%s: FDCLR\n", myname ) ;

                        if (return_val == 0)
                            on_timeout();
                        else
                            strcpy( device_msg, tmp_msg+1 ) ;
                        if ( verbose )
                            printf( "\n%s: answer is %s\n", myname, tmp_msg+1 );
                        }
                    }
            else 
                sprintf ( device_msg, "MA SERVER DONE" ) ;
            }

#endif

        else {
            /* DO IT IN STUB MODE  */
            if ( strncmp ( message , "!AD 0", 5 ) == 0 ) {
                out_cur++;
                if ( out_cur > 999 ) out_cur = 0 ;
                sprintf ( device_msg, "%3d", out_cur ) ;
                }

            if ( strncmp ( message , "!AD 1", 5 ) == 0 ) {
                sprintf ( device_msg, "%3d", tesla ) ;
                tesla++;
                if ( tesla > 999 ) tesla = 0 ;
                }

            if ( strncmp ( message , "!AD 2", 5 ) == 0 ) {
                sprintf ( device_msg, "%3d", out_volt ) ;
                out_volt++;
                if ( out_volt > 999 ) out_volt = 0 ;
                }

            if ( strncmp ( message , "!AD 3", 5 ) == 0 ) {
                sprintf ( device_msg, "%3d", int15pos ) ;
                int15pos++;
                if ( int15pos > 999 ) int15pos = 0 ;
                }

            if ( strncmp ( message , "!AD 4", 5 ) == 0 ) {
                sprintf ( device_msg, "%3d", int15neg ) ;
                int15neg++;
                if ( int15neg > 999 ) int15neg = 0 ;
                }

            if ( strncmp ( message , "!AD 5", 5 ) == 0 ) {
                sprintf ( device_msg, "%3d", int5pos ) ;
                int5pos++;
                if ( int5pos > 999 ) int5pos = 0 ;
                }

            if ( strncmp ( message , "!AD 6", 5 ) == 0 ) {
                sprintf ( device_msg, "+%2d", delta_t ) ;
                delta_t++;
                if ( delta_t > 99 ) delta_t = 0 ;
                }

            if ( strncmp ( message , "!AD 7", 5 ) == 0 ) {
                sprintf ( device_msg, "%3d", vce ) ;
                vce++;
                if ( vce > 999 ) vce = 0 ;
                }

            if ( strncmp ( message , "!AD 8", 5 ) == 0 ) {
                sprintf ( device_msg, "%5d", opt_iout ) ;
                opt_iout++;
                if ( opt_iout > 99999 ) opt_iout = 0 ;
                }

            if ( strncmp ( message , "!AD 9", 5 ) == 0 ) {
                sprintf ( device_msg, "%3d", aux_iout ) ;
                aux_iout++;
                if ( aux_iout > 999 ) aux_iout = 0 ;
                }

            if ( strncmp ( message , "!ADR", 4 ) == 0 )
                sprintf ( device_msg, "%d", 255 ) ;

            if ( strncmp ( message , "!ASW", 4 ) == 0 )
                sprintf ( device_msg, "%s", "R" ) ;

            if ( strncmp ( message , "!BAUD", 5 ) == 0 )
                sprintf ( device_msg, "%d", 9600 ) ;

            if ( strncmp ( message , "!CMD", 4 ) == 0 )
                sprintf ( device_msg, "%s", "REM" ) ;

            if ( strncmp ( message , "!CMDSTATE", 9 ) == 0 )
                sprintf ( device_msg, "%s", "STUB" ) ;

            if ( strncmp ( message , "!ERRC", 5 ) == 0 )
                sprintf ( device_msg, "MA SERVER DONE" ) ;

            if ( strncmp ( message , "!ERRT", 5 ) == 0 )
                sprintf ( device_msg, "MA SERVER DONE" ) ;

            if ( strncmp ( message , "!F", 2 ) == 0 )
                sprintf ( device_msg, "MA SERVER DONE" ) ;

            if ( strncmp ( message , "!LALL", 5 ) == 0 )
                sprintf ( device_msg, "MA SERVER DONE" ) ;

            if ( strncmp ( message , "!LOC", 4 ) == 0 )
                sprintf ( device_msg, "MA SERVER DONE" ) ;

            if ( strncmp ( message , "!LOCK", 5 ) == 0 )
                sprintf ( device_msg, "MA SERVER DONE" ) ;

            if ( strncmp ( message , "!N", 2 ) == 0 )
                sprintf ( device_msg, "MA SERVER DONE" ) ;

            if ( strncmp ( message , "!NASW", 5 ) == 0 )
                sprintf ( device_msg, "MA SERVER DONE" ) ;

            if ( strncmp ( message , "!NERR", 5 ) == 0 )
                sprintf ( device_msg, "MA SERVER DONE" ) ;

            if ( strncmp ( message , "!PO", 3 ) == 0 )
                sprintf ( device_msg, "+" ) ;

            if ( strncmp ( message , "!PO +", 5 ) == 0 )
                sprintf ( device_msg, "MA SERVER DONE" ) ;

            if ( strncmp ( message , "!PO -", 5 ) == 0 )
                sprintf ( device_msg, "MA SERVER DONE" ) ;

            if ( strncmp ( message , "!RA", 3 ) == 0 )
                sprintf ( device_msg, "%6d", 999999 ) ;

            if ( strncmp ( message , "!REM", 4 ) == 0 )
                sprintf ( device_msg, "MA SERVER DONE" ) ;

            if ( strncmp ( message , "!RS", 3 ) == 0 )
                sprintf ( device_msg, "MA SERVER DONE" ) ;

            if ( strncmp ( message , "!S1", 3 ) == 0 )
                sprintf ( device_msg, "STUB" ) ;

            if ( strncmp ( message , "!UNLOCK", 7) == 0 )
                sprintf ( device_msg, "MA SERVER DONE" ) ;

            if ( strncmp ( message , "!WA", 3 ) == 0 )
                sprintf ( device_msg, "STUB" ) ;

            if ( verbose )
                printf( "\n%s: answer is %s", myname, device_msg ) ;
        }
        break ;
        default:
        printf( "\n%s: unknown command\n", myname ) ;
        strcpy( device_msg, NEG_ANSWER ) ;
        break ;
    }
}

/***********************************************************************

   SERVER DEPENDENT
   This is the function performing timeout actions when /dev/tty does
   not answer.

***********************************************************************/

void on_timeout()
{
    extern int verbose ;
    extern char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ] ;
    extern char myname[ NCHARS ] ;
    extern char *message;
    extern int timeout_flag, mode ;
    extern int supply_number;
    extern int alim_mode[100];
    FILE *timeout_file;

    fflush( stdout ) ;
    printf( "\n%s: TIMEOUT\n\n", myname ) ;
    fflush( stdout ) ;
    system( "date" ) ;
    fflush( stdout ) ;
    printf( "\n%s: no answer from device\n", myname ) ;
    fflush( stdout ) ;
    timeout_flag = 1 ;
    alim_mode[supply_number] = 0 ;

system("date >> timeout_file");
timeout_file = fopen("timeout_file","a");
fprintf(timeout_file, "alimentatore = %d\n\n", supply_number);
fclose(timeout_file);
}

