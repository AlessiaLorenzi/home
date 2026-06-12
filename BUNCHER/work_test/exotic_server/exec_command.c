#include <unistd.h>
#include "server.h"
#include "server_const.h"

static int timeout_flag = 0 ;
extern int tesla_ok;

/***********************************************************************

    SERVER DEPENDENT PS
    This is the function performing local command execution
    if ( *mode ) then normal mode, else stub mode
    commands:
    ! DOIT
    @ RUN special local proc (ramp)
    r RESET reset tty
    a ALLADCS

***********************************************************************/

exec_command( mode, flag_pointer )
int *mode, *flag_pointer ;
{
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
    extern char *message;
    int return_val;
    float tesla_value;

    char tmp_msg[ BUF_SIZE ] ;

    static int out_cur = 0, tesla = 0, out_volt = 0,
               int15pos = 0, int15neg = 0, int5pos = 0, 
    delta_t = 0, vce = 0, opt_iout = 0, aux_iout = 0;

    static struct timeval timeout = {(long)2, (long)0};
    static struct timeval timeout1 = {(long)0, (long)10000};
    fd_set ready;

    char command ;

    extern int timeout_flag ;
    extern void on_timeout() ;
    extern void on_tesla_timeout() ;

    int rem_loc = REMOTE ;

    /* vars for ramps */
    static int ini_cur, fin_cur, sta_cur, period, dcur, dt, nsteps ;
    static int proc_id =-1, father_status= 1, child_status ;
    int istep, PSDcurrent, delay, file_id ;
    



    extern int command_shame_id, answer_shame_id;
    extern char *command_shame_pointer, *answer_shame_pointer;

    extern int command_sem_id, answer_sem_id, access_sem_id, semop_retval;
    extern int command_sem_val, answer_sem_val, access_sem_val;

    extern struct sembuf semaphore_struct ;

    char risp[100];


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
        if ( alim_mode[supply_number] && father_status )
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
        if ( *mode && father_status  ) {
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

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
	
        /* codice ECR */
    if ( alim_mode[supply_number] && father_status  ) 
      { /* start real mode */



	semop_retval = semop( access_sem_id, &semaphore_struct, 1 ) ;
        if( semop_retval == -1 )
            printf( "*3* error in semaphore operation\n" ) ;
        else
            {
            access_sem_val = semctl( access_sem_id, 0, GETVAL, NULL ) ;




	if (  strncmp ( message , "!REG", 4 ) == 0 || strncmp ( message , "!WA", 3 ) == 0 )
	// comandi che non devono andare all'alimentatore
	{

                        if  ( strncmp ( message , "!REG", 4 ) == 0 )  
				{
				semop_retval = semop( command_sem_id, &semaphore_struct, 1 ) ;

        			if( semop_retval == -1 )
            				printf( "*1* error in semaphore operation\n" ) ;
        			else
           				{
           				command_sem_val = semctl( command_sem_id, 0, GETVAL, NULL ) ;
	   				sprintf ( command_shame_pointer, "REG\000");
           				}

           			command_sem_val = semctl( command_sem_id,  0, SETVAL, 1 ) ;

				}


             if  ( strncmp ( message , "!WA", 3 ) == 0 )  
		{
		semop_retval = semop( command_sem_id, &semaphore_struct, 1 ) ;

		if( semop_retval == -1 )
       			printf( "*1* error in semaphore operation\n" ) ;
       		else
       			{
       			command_sem_val = semctl( command_sem_id, 0, GETVAL, NULL ) ;
			sprintf ( command_shame_pointer, "SET %s", message+4);
       			}

		command_sem_val = semctl( command_sem_id,  0, SETVAL, 1 ) ;

	    	strcpy( device_msg, message+4);
		}

	}


	    else
            /* if real and not ramping */

            /* if NOT( WA OR N OR RS OR F ) OR
                     ( WA OR N OR RS OR F )AND( REMOTE ) */
            if ((  ( strncmp ( message , "!N", 2 ) != 0 ) && 
                   ( strncmp ( message , "!RS", 3 ) != 0 ) &&
                   ( strncmp ( message , "!F", 2 ) != 0 ) ) ||
                 ( ( ( strncmp ( message , "!N", 2 ) == 0 ) || 
                     ( strncmp ( message , "!RS", 3 ) == 0 ) ||
                     ( strncmp ( message , "!F", 2 ) == 0 ) ) &&
                     ( rem_loc == REMOTE) ) )   {

                    if ( verbose )
                        printf( "%s: message is >%s<\n\n", myname, message ) ;
                    fprintf( rs232w, "%s\r", ( message + 1 ) ) ;
                    /*** tcdrain( fildes ) ; ***/
                    fflush( rs232w ) ;
 
                    if ( ( strncmp ( message , "!AD", 3 ) == 0 )       ||
                         ( strncmp ( message , "!RA", 3 ) == 0 )       ||
                       //  ( strncmp ( message , "!WA", 3 ) == 0 )       ||
                         ( strncmp ( message , "!CMDSTATE", 9 ) == 0 ) ||
                         ( strncmp ( message , "!S1", 3 ) == 0 ) ) {

                        if ( verbose )
                            printf( "\n%s: FDSET\n", myname ) ;
                        FD_SET(fildes, &ready);
                        timeout.tv_sec = 2 ;
                        timeout.tv_usec = 0 ;
                        if ( return_val = select( getdtablesize(),
                                                  &ready,
                                                  (fd_set *)NULL,
                                                  (fd_set *)NULL,
                                                  &timeout) > 0 ) {
                            fgets( tmp_msg, BUF_SIZE, rs232r ) ;
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





timeout1.tv_sec = 0 ;
timeout1.tv_usec = 10000 ;

FD_SET(fildes, &ready);
return_val=select(getdtablesize(), &ready, (fd_set *)NULL, (fd_set *)NULL, &timeout1);
FD_CLR(fildes, &ready);

                        while ( return_val >0 )
				{
			        fgets( risp, BUF_SIZE, rs232r );
				FD_SET(fildes, &ready);
				return_val=select(getdtablesize(), &ready, (fd_set *)NULL, (fd_set *)NULL, &timeout1);
				FD_CLR(fildes, &ready);
				}







			
			// correzione per S1
			
			if ( strncmp ( message , "!S1", 3 ) == 0 )
				{
				semop_retval = semop( answer_sem_id, &semaphore_struct, 1 ) ;

        			if( semop_retval == -1 )
            				printf( "*1* error in semaphore operation\n" ) ;
        			else
           				{
           				answer_sem_val = semctl( answer_sem_id, 0, GETVAL, NULL ) ;
					if ( strncmp ( answer_shame_pointer, "A", 1 ) == 0 )
						{
						device_msg[4] = '!' ;
						device_msg[5] = '!' ;
						}
					if ( strncmp ( answer_shame_pointer, "B", 1 ) == 0 )
						{
						device_msg[4] = '.' ;
						device_msg[5] = '.' ;
						}
					if ( strncmp ( answer_shame_pointer, "C", 1 ) == 0 )
						{
						device_msg[4] = '.' ;
						device_msg[5] = '!' ;
						}
					if ( strncmp ( answer_shame_pointer, "D", 1 ) == 0 )
						{
						device_msg[23] = '!' ;
						}
					else
						device_msg[23] = '.' ;


					printf("------------------mem = %c, device_msg[4] = %c, device_msg[5] = %c\n",answer_shame_pointer, device_msg[4], device_msg[5]);
					
           				}

           			answer_sem_val = semctl( answer_sem_id,  0, SETVAL, 1 ) ;
				}
			
                        } /*  end if msg with anser */



 
                    if ( strncmp ( message , "!AD 0", 5 ) == 0 )  {

			semop_retval = semop( answer_sem_id, &semaphore_struct, 1 ) ;

        		if( semop_retval == -1 )
            			printf( "*1* error in semaphore operation\n" ) ;
        		else
           			{
           			answer_sem_val = semctl( answer_sem_id, 0, GETVAL, NULL ) ;
	   			strcpy(device_msg, answer_shame_pointer);
           			}

           		answer_sem_val = semctl( answer_sem_id,  0, SETVAL, 1 ) ;
	 
		    } /*  end if msg with anser */


                        if  ( strncmp ( message , "!WA", 3 ) == 0 )  
				{
				semop_retval = semop( command_sem_id, &semaphore_struct, 1 ) ;

        			if( semop_retval == -1 )
            				printf( "*1* error in semaphore operation\n" ) ;
        			else
           				{
           				command_sem_val = semctl( command_sem_id, 0, GETVAL, NULL ) ;
	   				sprintf ( command_shame_pointer, "SET %s", message+4);
           				}

           			command_sem_val = semctl( command_sem_id,  0, SETVAL, 1 ) ;

			    	strcpy( device_msg, message+4);
				}



                    }
            else 
                sprintf ( device_msg, "MA SERVER DONE" ) ;
            }
	    /* turn on the green light */
	    access_sem_val = semctl( access_sem_id,  0, SETVAL, 1 ) ;

      } /* end real mode */


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

void on_tesla_timeout()
{
    extern int verbose ;
    extern char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ] ;
    extern char myname[ NCHARS ] ;
    extern char *message;
    extern int timeout_flag, mode ;

    fflush( stdout ) ;
    printf( "\n%s: TESLA TIMEOUT\n\n", myname ) ;
    fflush( stdout ) ;
    tesla_ok = 0 ;
}


