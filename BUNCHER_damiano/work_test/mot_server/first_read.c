
#include "server.h"

/***********************************************************************

    SERVER DEPENDENT

    This is the function reading the first set of datas the client
    must send to the server: first of all there are 3 strings

    hostname socket_number (e.g. "console 5000" )

    then all the datas server dependent follow.
    The string "NONE" is recognized to mean 'no alarms must be managed'.
    The answer "DONE" is sent back to the client.
    if ( mode > 0 ) then normal mode, else stub mode

    DOC BEGIN

    All datas server-dependent must be sent by a separate second write.
    For this server they are verbose-option, tty name and cryostat numb.

    motor second message format, for verbose option and tty names
    cryo numbers
    ex: "-v /dev/tty17 /dev/tty17 NONE NONE 1 11 5 6"
    or: "-n /dev/tty17 /dev/tty17 NONE NONE 1 11 5 6"

    DOC END

***********************************************************************/

/* first message format, for alarm machine, socket info and mode
   mode = 0 means stub-mode, mode > 0 means normal mode
   ex: "console 4001 1" or "NONE 0 0" */
#define MSG1_FORMAT " %s %d %d "

/* motor second message format, for verbose option and tty names
   cryo numbers
   ex: "-v /dev/tty17 "
   or: "-n /dev/tty17 " */
#define MOT_MSG2F " -%c %s "

first_read( alarm_machine, alarm_socket, motcntr_mode )
int *alarm_socket, *motcntr_mode ;
char alarm_machine[ NCHARS ] ;
{
    extern int main_channel, verbose ;
    extern char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ] ;
    extern char myname[ NCHARS ] ;
    extern char tty_name[ NCHARS ] ;
    extern char alarm_headmsg[ NCHARS ] ;

    char dummy2[ NCHARS ], dummy3[ NCHARS ], dummy4[ NCHARS ] ;
    int cryo_name, int_dummy2, int_dummy3, int_dummy4 ;
    char c ;
    int not_error, read_retval, write_retval ;

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
                alarm_machine, alarm_socket, motcntr_mode ) ;
        if ( strcmp( alarm_machine, "NONE" ) == 0 ) {
            alarm_machine[0] = NULL ;
            *alarm_socket = 0 ;
            if ( verbose )
                printf( "\n%s: first_read no alarms\n", myname ) ;
            }
        else
            if ( verbose )
                printf( "\n%s: first_read alarms m. %s, s. %d\n",
                        myname, alarm_machine, *alarm_socket ) ;
        if ( verbose )
            if ( *motcntr_mode )
                printf( "\n%s: first_read normal mode\n", myname ) ;
            else
                printf( "\n%s: first_read stub mode\n", myname ) ;
        /* read from client */
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
        sscanf( client_msg, MOT_MSG2F, &c, tty_name ) ;
        if ( c == 'v' ) verbose = 1 ;
        else verbose = 0 ;
        sprintf( alarm_headmsg, "tty: %s", tty_name ) ;
    }
    /* end of local data reading */
}

