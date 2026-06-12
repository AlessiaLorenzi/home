
#include "server.h"
#include "server_const.h"
#include "server_vars.h"

#include <sys/types.h>
#include <sys/time.h>


/***********************************************************************

    SERVER DEPENDENT
    This is the function performing local command execution
    if ( *mode ) then normal mode, else stub mode

***********************************************************************/

void loop_timeout(alimentatore)
int alimentatore;
{
    extern char myname[ NCHARS ] ;
    time_t local_time;

    local_time = time((time_t *) 0 );
    printf( "\n%s timeout occured at %s\n",
            myname, ctime(&local_time));

    alim[alimentatore].mode = 0 ;
    strcpy( device_msg, "STUB" ) ;
}


exec_command( mode, flag_pointer )
int *mode, *flag_pointer ;
{

    int i ;
    char c ;

    extern int verbose, supply_number;
    extern char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ];
    extern char myname[ NCHARS ] ;
    extern struct supply_data supply[100];
    extern char *message;

    float valore ;

    char to_dev[ BUF_SIZE ] ;
    char command, answer[ BUF_SIZE ] ;

    char return_string[100];

    int return_val ;

    fd_set ready;

    static struct timeval timeout = { (long)3, (long)0 }; /* 3 seconds */

    FD_ZERO(&ready);
    message = client_msg + 10;
    sscanf ( client_msg, "ALIM = %d", &supply_number);
    sscanf( message, "%c", &command ) ;
   /* if ( verbose )
        printf( "\n%s: command received from netm is %s\n",
                myname, message ) ;*/
    strcpy( device_msg, NEG_ANSWER ) ;

    switch ( command ) {

        case EOC :

        *flag_pointer = FALSE ;
        break;


	case STAT:
	
        if (alim[supply_number].mode == 1 )
        {
        message = message + 3 ;
        sprintf ( to_dev, "%1d\r", alim[supply_number].address) ;

       /* if ( verbose ) printf( "\n%s: command to device is %s\n", myname, to_dev ) ;*/

        fprintf (alim[supply_number].rs232w ,"PRT = %s", to_dev);
        fflush (alim[supply_number].rs232w);

        fprintf (alim[supply_number].rs232w ,"CUR/");
        fflush (alim[supply_number].rs232w);

        FD_SET(alim[supply_number].fildes, &ready);
        if ( return_val = select ( getdtablesize(), &ready,
                      (fd_set *)NULL, (fd_set *)NULL, &timeout )> 0 )
            {
	    fgets(return_string, 100, alim[supply_number].rs232r);

            FD_CLR(alim[supply_number].fildes, &ready);

            /*if ( verbose )
                printf( "\n%s: first echo + answer is ^^^%s$$$\n",
                        myname, return_string ) ;*/
            } ;

        if ( return_val == 0 )
            loop_timeout(supply_number);
        else
            strcpy( device_msg, return_string ) ;

        }
        else
        {
       /* if ( verbose )
            printf ("DOIT STUB MODE alim. = %d posizione = %d falso \n",
                     supply_number, alim[supply_number].address);*/
        strcpy( device_msg, "STUB" ) ;

        }

        break ;

	






        case DOIT:

        /* ma server specific message execution: first char must
           be '!' */

        if (alim[supply_number].mode == 1 )
        {
        
        message = message + 3 ;
        sscanf( message, "%f", &valore ) ;
        sprintf ( to_dev, "%1d %5.2f\r", alim[supply_number].address, valore ) ;
        /*if ( verbose )
            printf( "\n%s: command to device is %s\n",
                    myname, to_dev ) ;*/
        fprintf (alim[supply_number].rs232w ,"%s", to_dev);
        fflush (alim[supply_number].rs232w);

        FD_SET(alim[supply_number].fildes, &ready);
        if ( return_val = select ( getdtablesize(), &ready,
                      (fd_set *)NULL, (fd_set *)NULL, &timeout )> 0 )
            {
            i = 0 ;
            do {
                c = fgetc( alim[supply_number].rs232r ) ;
                return_string[i] = c ;
                i++ ;
                } while ( c != '>' ) ;
            FD_CLR(alim[supply_number].fildes, &ready);
            return_string[i] = 0 ;

           /* if ( verbose )
                printf( "\n%s: first echo + answer is ^^^%s$$$\n",
                        myname, return_string ) ;*/
            } ;

        if ( return_val == 0 )
            loop_timeout(supply_number);
        else
            strcpy( device_msg, return_string ) ;






        sprintf ( to_dev, "%1d\r", alim[supply_number].address ) ;
        /*if ( verbose )
            printf( "\n%s: command to device is %s\n",
                    myname, to_dev ) ;*/
        fprintf (alim[supply_number].rs232w ,"%s", to_dev);
        fflush (alim[supply_number].rs232w);

        FD_SET(alim[supply_number].fildes, &ready);
        if ( return_val = select ( getdtablesize(), &ready,
                      (fd_set *)NULL, (fd_set *)NULL, &timeout )> 0 )
            {
            i = 0 ;
            do {
                c = fgetc( alim[supply_number].rs232r ) ;
                return_string[i] = c ;
                i++ ;
                } while ( c != '>' ) ;
            FD_CLR(alim[supply_number].fildes, &ready);
            return_string[i] = 0 ;

           /* if ( verbose )
                printf( "\n%s: second echo + answer is ^^^%s$$$\n",
                        myname, return_string ) ;*/
            } ;

        if ( return_val == 0 )
            loop_timeout(supply_number);
        else
            strcpy( device_msg, return_string ) ;









        }
        else
        {
        if ( verbose )
            printf ("DOIT STUB MODE alim. = %d posizione = %d falso \n",
                     supply_number, alim[supply_number].address);
        strcpy( device_msg, "STUB" ) ;

        }

        break ;
        default:
        printf( "\n%s: unknown command\n", myname ) ;
        strcpy( device_msg, "NOT DONE" ) ;
        break ;

    }

}

