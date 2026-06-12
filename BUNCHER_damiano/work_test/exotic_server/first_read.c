
#include "server.h"
#include "initmsgs.h"
#include "server_const.h"


/***********************************************************************

    SERVER DEPENDENT

    This is the function reading the first set of datas the client
    must send to the server: first of all there are 3 strings

    hostname socket_number mode (e.g. "console 5000 1" )
 
    then all the datas server dependent follow.
    The string "NONE" is recognized to mean 'no alarm must be managed'.
    The answer "DONE" is sent back to the client.

    All datas server-dependent must be sent by a separate second write.
    For this server they are verbose option, tty name and ...

***********************************************************************/

extern struct supply_data supply[100];

first_read( alarm_machine, alarm_socket, mode )
int *alarm_socket, *mode ;
char alarm_machine[ NCHARS ] ;
{
    extern int main_channel, verbose ;
    extern char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ] ;
    extern char myname[ NCHARS ] ;
    extern char tty_name[ NCHARS ] ;
    extern char tesla_tty_name[ NCHARS ] ;
    extern char alarm_headmsg[ NCHARS ] ;
    extern int address[100];
    extern int alim_mode[100];

    char c ;
    char linea[ NCHARS ] ;
    int not_error, read_retval, write_retval ;
    int counter;
    verbose = TRUE ;
    not_error = TRUE ;

    /* read the first message from client */
    read_retval = read( main_channel, client_msg, BUF_SIZE ) ;
    if ( read_retval <= 0 ) {
        if ( read_retval < 0 )
            printf( "\n%s: error reading main socket\n", myname ) ;
        not_error = FALSE ;
        }
    if ( not_error ) {
        sscanf( client_msg, MSG1_FORMAT,
                alarm_machine, alarm_socket, mode ) ;
        if ( strcmp( alarm_machine, "NONE" ) == 0 ) {
            alarm_machine[0] = 0 ;
            *alarm_socket = 0 ;
            if ( verbose )
                printf( "\n%s: first_read no alarm\n", myname ) ;
            }
        else
            if ( verbose )
                printf( "\n%s: first_read alarm m. %s, s. %d\n",
                        myname, alarm_machine, *alarm_socket ) ;
        if ( verbose )
            if ( *mode )
                printf( "\n%s: first_read normal mode\n", myname ) ;
            else
                printf( "\n%s: first_read stub mode\n", myname ) ;
        strcpy( device_msg, POS_ANSWER ) ;
        write_retval = write( main_channel, device_msg, BUF_SIZE ) ;
        if ( write_retval < 0 ) {
            printf( "\n%s: error writing main socket\n", myname ) ;
            not_error = FALSE ;
        }
    }
    /* read the second message from client SERVER DEPENDENT */
    read_retval = read( main_channel, client_msg, BUF_SIZE ) ;
    if ( read_retval <= 0 ) {
        if ( read_retval < 0 )
            printf( "\n%s: error reading main socket\n", myname ) ;
        not_error = FALSE ;
        }
    if ( not_error ) {
        sscanf( client_msg, " -%c %s",
                            &c, tty_name ) ;

strncpy ( tesla_tty_name, tty_name + 12, 12 ) ;
tty_name[12] = 0 ;

	
        if ( c == 'v' )
	    verbose = TRUE ;
        else
	    verbose = FALSE ;
        printf ("mode = %c tty_name = %s tesla_tty_name = %s\n", c, tty_name, tesla_tty_name);
        sprintf( alarm_headmsg, "second message\n" ) ;

        strcpy( device_msg, POS_ANSWER ) ;
        write_retval = write( main_channel, device_msg, BUF_SIZE ) ;
        if ( write_retval < 0 ) {
            printf( "\n%s: error writing main socket\n", myname ) ;
            not_error = FALSE ;
        }
    }
    /* end of local data reading */
    /* read the third message from client SERVER DEPENDENT */
    read_retval = read( main_channel, client_msg, BUF_SIZE ) ;
    if ( read_retval <= 0 ) {
        if ( read_retval < 0 )
            printf( "\n%s: error reading main socket\n", myname ) ;
        not_error = FALSE ;
        }
    if ( not_error ) {
        sscanf( client_msg, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ",
                             &(supply[0].number), &(supply[0].address),
                             &(supply[1].number), &(supply[1].address),
                             &(supply[2].number), &(supply[2].address),
                             &(supply[3].number), &(supply[3].address),
                             &(supply[4].number), &(supply[4].address),
                             &(supply[5].number), &(supply[5].address),
                             &(supply[6].number), &(supply[6].address),
                             &(supply[7].number), &(supply[7].address),
                             &(supply[8].number), &(supply[8].address),
                             &(supply[9].number), &(supply[9].address),
                             &(supply[10].number), &(supply[10].address),
                             &(supply[11].number), &(supply[11].address),
                             &(supply[12].number), &(supply[12].address),
                             &(supply[13].number), &(supply[13].address),
                             &(supply[14].number), &(supply[14].address),
                             &(supply[15].number), &(supply[15].address)) ;
    }
    /* end of local data reading */

        strcpy( device_msg, POS_ANSWER ) ;
        write_retval = write( main_channel, device_msg, BUF_SIZE ) ;
        if ( write_retval < 0 ) {
            printf( "\n%s: error writing main socket\n", myname ) ;
            not_error = FALSE ;}

        for ( counter = 0 ; counter < 16 ; counter++ ) {
            if ( supply[counter].number >= 0 && supply[counter].number < 100)
                {
                address[supply[counter].number] = supply[counter].address;
                alim_mode[supply[counter].number] = *mode ;
                }
        }




}

