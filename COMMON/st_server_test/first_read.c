
#include "server.h"
#include "server_const.h"
#include "server_ext_vars.h"


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


first_read( alarm_machine, alarm_socket, mode )
int *alarm_socket, *mode ;
char alarm_machine[ NCHARS ] ;
{

    char c ;
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

/*

	printf("primo messaggio ricevuto dal netm ::::::::::: \n %s \n",client_msg);
*/

	
        sscanf( client_msg, " %s %d %d %d %d %d %d %s %d %d %d %d %d %d %s %d %d %d %d %d %d %s %d %d %d %d %d %d %s %d %d %d %d %d %d %s %d %d %d %d %d %d %s %d %d %d %d %d %d %s %d %d %d %d %d %d",
		serial[0].name, &(serial[0].mode), &(serial[0].verbose),
		&(serial[0].alim[0]), &(serial[0].alim[1]),
		&(serial[0].alim[2]), &(serial[0].alim[3]),
		serial[1].name, &(serial[1].mode), &(serial[1].verbose),
		&(serial[1].alim[0]), &(serial[1].alim[1]),
		&(serial[1].alim[2]), &(serial[1].alim[3]),
		serial[2].name, &(serial[2].mode), &(serial[2].verbose),
		&(serial[2].alim[0]), &(serial[2].alim[1]),
		&(serial[2].alim[2]), &(serial[2].alim[3]),
		serial[3].name, &(serial[3].mode), &(serial[3].verbose),
		&(serial[3].alim[0]), &(serial[3].alim[1]),
		&(serial[3].alim[2]), &(serial[3].alim[3]),
		serial[4].name, &(serial[4].mode), &(serial[4].verbose),
		&(serial[4].alim[0]), &(serial[4].alim[1]),
		&(serial[4].alim[2]), &(serial[4].alim[3]),
		serial[5].name, &(serial[5].mode), &(serial[5].verbose),
		&(serial[5].alim[0]), &(serial[5].alim[1]),
		&(serial[5].alim[2]), &(serial[5].alim[3]),
		serial[6].name, &(serial[6].mode), &(serial[6].verbose),
		&(serial[6].alim[0]), &(serial[6].alim[1]),
		&(serial[6].alim[2]), &(serial[6].alim[3]),
		serial[7].name, &(serial[7].mode), &(serial[7].verbose),
		&(serial[7].alim[0]), &(serial[7].alim[1]),
		&(serial[7].alim[2]), &(serial[7].alim[3])) ;

	/*printf("%s %d %d %d %d %d %d *-* \n%s %d %d %d %d %d %d *-* \n%s %d %d %d %d %d %d *-* \n%s %d %d %d %d %d %d *-* \n%s %d %d %d %d %d %d *-* \n%s %d %d %d %d %d %d *-* \n%s %d %d %d %d %d %d *-* \n%s %d %d %d %d %d %d *-* \n\n",
		serial[0].name, serial[0].mode, serial[0].verbose,
		serial[0].alim[0], serial[0].alim[1],
		serial[0].alim[2], serial[0].alim[3],
		serial[1].name, serial[1].mode, serial[1].verbose,
		serial[1].alim[0], serial[1].alim[1],
		serial[1].alim[2], serial[1].alim[3],
		serial[2].name, serial[2].mode, serial[2].verbose,
		serial[2].alim[0], serial[2].alim[1],
		serial[2].alim[2], serial[2].alim[3],
		serial[3].name, serial[3].mode, serial[3].verbose,
		serial[3].alim[0], serial[3].alim[1],
		serial[3].alim[2], serial[3].alim[3],
		serial[4].name, serial[4].mode, serial[4].verbose,
		serial[4].alim[0], serial[4].alim[1],
		serial[4].alim[2], serial[4].alim[3],
		serial[5].name, serial[5].mode, serial[5].verbose,
		serial[5].alim[0], serial[5].alim[1],
		serial[5].alim[2], serial[5].alim[3],
		serial[6].name, serial[6].mode, serial[6].verbose,
		serial[6].alim[0], serial[6].alim[1],
		serial[6].alim[2], serial[6].alim[3],
		serial[7].name, serial[7].mode, serial[7].verbose,
		serial[7].alim[0], serial[7].alim[1],
		serial[7].alim[2], serial[7].alim[3]) ;*/

	



        strcpy( device_msg, POS_ANSWER ) ;
        write_retval = write( main_channel, device_msg, BUF_SIZE ) ;
        if ( write_retval < 0 ) {
            printf( "\n%s: error writing main socket\n", myname ) ;
            not_error = FALSE ;
        }
    }

}

