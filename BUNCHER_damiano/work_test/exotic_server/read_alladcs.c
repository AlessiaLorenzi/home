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
    extern int command_shame_id, answer_shame_id;
    extern char *command_shame_pointer, *answer_shame_pointer;
    extern int command_sem_id, answer_sem_id, access_sem_id, semop_retval;
    extern int command_sem_val, answer_sem_val, access_sem_val;
    extern struct sembuf semaphore_struct ;


    /***
    fprintf( rs232w, "ADR %d\r",address[supply_number]);
    fflush(rs232w);
    ***/
    if ( verbose ) 
        printf ("in read_alladcs() ADR %d\n",address[supply_number]);


    /* ask adcs values */
    device_msg[0] = 0;




     semop_retval = semop( access_sem_id, &semaphore_struct, 1 ) ;
     if( semop_retval == -1 )
          printf( "*3* error in semaphore operation\n" ) ;
     else
          {
          access_sem_val = semctl( access_sem_id, 0, GETVAL, NULL ) ;
    
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

	}

        /* turn on the green light */
        access_sem_val = semctl( access_sem_id,  0, SETVAL, 1 ) ;


     
    if ( verbose ) printf( "\n%s: answer to client is %s\n", myname,
                           device_msg ) ;
}

