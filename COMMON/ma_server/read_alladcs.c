#include "server.h"
#include "server_const.h"


/***********************************************************************
    server specific read_alladcs function: read all adcs 
***********************************************************************/

read_alladcs()
{
    extern int supply_number;
    extern int verbose ;
    extern FILE *rs232r, *rs232w ;
    extern char myname[ NCHARS ], device_msg[BUF_SIZE] ;
    extern int address[100];
    extern int alim_mode[100];
    char req_str[BUF_SIZE], ans_str[BUF_SIZE], adc_str[BUF_SIZE] ;
    int i ;


    fprintf( rs232w, "ADR %d\r",address[supply_number]);
    fflush(rs232w);
    if ( verbose ) 
        printf ("in read_alladcs() ADR %d\n",address[supply_number]);


    /* ask adcs values */
    device_msg[0] = NULL;


	i=0;
        sprintf( req_str, "AD %1d\r", i ) ;
        /* send the message */
        fprintf( rs232w, req_str ) ;
        fflush( rs232w ) ; 
        /* get the answer */
        fgets( ans_str, BUF_SIZE, rs232r ) ; /* this is the answer */
	fgetc ( rs232r ) ;
        sprintf( adc_str, "AD %1d = ", i ) ;
        strcat( device_msg, adc_str ) ;
        strcat( device_msg, ans_str ) ;

	i=2;
        sprintf( req_str, "AD %1d\r", i ) ;
        /* send the message */
        fprintf( rs232w, req_str ) ;
        fflush( rs232w ) ; 
        /* get the answer */
        fgets( ans_str, BUF_SIZE, rs232r ) ; /* this is the answer */
	fgetc ( rs232r ) ;
        sprintf( adc_str, "AD %1d = ", i ) ;
        strcat( device_msg, adc_str ) ;
        strcat( device_msg, ans_str ) ;

	i=3;
        sprintf( req_str, "AD %1d\r", i ) ;
        /* send the message */
        fprintf( rs232w, req_str ) ;
        fflush( rs232w ) ; 
        /* get the answer */
        fgets( ans_str, BUF_SIZE, rs232r ) ; /* this is the answer */
	fgetc ( rs232r ) ;
        sprintf( adc_str, "AD %1d = ", i ) ;
        strcat( device_msg, adc_str ) ;
        strcat( device_msg, ans_str ) ;

	i=4;
        sprintf( req_str, "AD %1d\r", i ) ;
        /* send the message */
        fprintf( rs232w, req_str ) ;
        fflush( rs232w ) ; 
        /* get the answer */
        fgets( ans_str, BUF_SIZE, rs232r ) ; /* this is the answer */
	fgetc ( rs232r ) ;
        sprintf( adc_str, "AD %1d = ", i ) ;
        strcat( device_msg, adc_str ) ;
        strcat( device_msg, ans_str ) ;

	i=5;
        sprintf( req_str, "AD %1d\r", i ) ;
        /* send the message */
        fprintf( rs232w, req_str ) ;
        fflush( rs232w ) ; 
        /* get the answer */
        fgets( ans_str, BUF_SIZE, rs232r ) ; /* this is the answer */
	fgetc ( rs232r ) ;
        sprintf( adc_str, "AD %1d = ", i ) ;
        strcat( device_msg, adc_str ) ;
        strcat( device_msg, ans_str ) ;

	i=6;
        sprintf( req_str, "AD %1d\r", i ) ;
        /* send the message */
        fprintf( rs232w, req_str ) ;
        fflush( rs232w ) ; 
        /* get the answer */
        fgets( ans_str, BUF_SIZE, rs232r ) ; /* this is the answer */
	fgetc ( rs232r ) ;
        sprintf( adc_str, "AD %1d = ", i ) ;
        strcat( device_msg, adc_str ) ;
        strcat( device_msg, ans_str ) ;

	i=7;
        sprintf( req_str, "AD %1d\r", i ) ;
        /* send the message */
        fprintf( rs232w, req_str ) ;
        fflush( rs232w ) ; 
        /* get the answer */
        fgets( ans_str, BUF_SIZE, rs232r ) ; /* this is the answer */
	fgetc ( rs232r ) ;
        sprintf( adc_str, "AD %1d = ", i ) ;
        strcat( device_msg, adc_str ) ;
        strcat( device_msg, ans_str ) ;




    if ( verbose ) printf( "\n%s: answer to client is %s\n", myname,
                           device_msg ) ;
}

