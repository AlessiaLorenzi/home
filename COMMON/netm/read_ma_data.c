#define FALSE            0
#define MSGLEN 1024
#define BUF_SIZE 1024
#include <sys/types.h>l
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#include "ma_net_const.h"
#include "ma_net_ext_var.h"

read_ma_data(file_name)
char *file_name;
{

int counter, trovato;
int not_error, channel[16], chan_addrlen ;
FILE *dbfile;
char line[200];
char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ] ;
int number, magnet_server_counter, magnet_counter, alim_counter;
int read_retval;


/*  inizializzazione variabili  */
/*         (provvisoria)        */

conta_bottoni = 0;



for ( number = 0 ; number < 100; number++ )
	{	
	magnet[number].magnet = 999999;
	server[number].socket = 999999;
	for ( alim_counter = 0; alim_counter<16; alim_counter++)
		server[number].alim[alim_counter] = 999999;
	}

dbfile = fopen ( file_name, "r" );
if ( dbfile == NULL )
	{
	printf ( "netm: error opening network data base file\n");
	fflush ( stdout );
	exit ( 2 );
	}

printf ("\n");
while ( fgets ( line, 100, dbfile) != NULL )
	{
	if ( strncmp ( line, "MAGNET", 6) == 0)
		{
		sscanf ( line, "MAGNET\t=%d", &number);
		magnet[number].magnet = number;
		if (sbrodola)
		{
		printf ( "numero = %d %d \n",number,magnet[number].magnet);
		printf ("magnet         block %d found  \n",number);
		}
		fscanf ( dbfile,"NAME\t=%s\n\
				ALIM1\t=%d\n\
				N_ALIM1\t=%s\n\
				A_ALIM1\t=%d\n\
				CU_ALIM1\t=%d\n\
				VO_ALIM1\t=%d\n\
				ALIM2\t=%d\n\
				N_ALIM2\t=%s\n\
				A_ALIM2\t=%d\n\
				CU_ALIM2\t=%d\n\
				VO_ALIM2\t=%d\n",
				
				magnet[number].name,

				&(magnet[number].alim1),
				magnet[number].n_alim1,
				&(magnet[number].a_alim1),
				&(magnet[number].cur_alim1),
				&(magnet[number].volt_alim1),

				&(magnet[number].alim2),
				magnet[number].n_alim2,
				&(magnet[number].a_alim2),
				&(magnet[number].cur_alim2),
				&(magnet[number].volt_alim2));


		}
/*printf ( 			"MAGNET\t=%d\n\
				NAME\t=%s\n\
				ALIM1\t=%d\n\
				N_ALIM1\t=%s\n\
				CU_ALIM1\t=%d\n\
				VO_ALIM1\t=%d\n\
				ALIM2\t=%d\n\
				N_ALIM2\t=%s\n\
				CU_ALIM2\t=%d\n\
				VO_ALIM2\t=%d\n",

				magnet[number].magnet,
				magnet[number].name,

				magnet[number].alim1,
				magnet[number].n_alim1,
				magnet[number].cur_alim1,
				magnet[number].volt_alim1,

				magnet[number].alim2,
				magnet[number].n_alim2,
				magnet[number].cur_alim2,
				magnet[number].volt_alim2);*/


	}
fclose ( dbfile );
magnet_server_counter = 0 ;





dbfile = fopen ( file_name, "r" );
if ( dbfile == NULL )
	{
	printf ( "netm: error opening network data base file\n");
	fflush ( stdout );
	exit ( 2 );
	}

printf ("\n");
while ( fgets ( line, 100, dbfile) != NULL )
	{
	if ( strncmp ( line, "BUTTON", 6) == 0)
		{
		conta_bottoni++;
		sscanf ( line, "BUTTON\t=%d", &number);
		button[number].button = number;
		if (sbrodola)
		{
		printf ( "numero = %d %d \n",number,magnet[number].magnet);
		printf ("button         block %d found  \n",number);
		}
		fscanf ( dbfile,"TYPE\t=%d\n\
				OBJ1\t=%d\n\
				OBJ2\t=%d\n\
				OBJ3\t=%d\n\
				OBJ4\t=%d\n",
				
				&(button[number].type),
				&(button[number].obj1),
				&(button[number].obj2),
				&(button[number].obj3),
				&(button[number].obj4));


		}
		/*	printf("\t\t\t\tBUTTON\t=%d\n\
				TYPE\t=%d\n\
				OBJ1\t=%d\n\
				OBJ2\t=%d\n\
				OBJ3\t=%d\n\
				OBJ4\t=%d\n",
				
				button[number].button,
				button[number].type,
				button[number].obj1,
				button[number].obj2,
				button[number].obj3,
				button[number].obj4);*/

	}




printf ("netm: trovati %d bottoni \n", conta_bottoni);




dbfile = fopen ( file_name, "r" );
if ( dbfile == NULL )
	{
	printf ( "netm: error opening network data base file\n");
	fflush ( stdout );
	exit ( 2 );
	}


printf ("\n");
while ( fgets ( line, 100, dbfile) != NULL )
	{
	if ( strncmp ( line, "SERVER", 6) == 0)
		{
		sscanf ( line, "SERVER\t=%d", &number);
		if (sbrodola)
		{
		printf ( "numero = %d \n", number );
		printf ("magnet  server block %d found  \n",number);
		}
		magnet_server_counter++;
		fscanf ( dbfile,"SOCKET\t=%d\n\
				HOST\t=%s\n\
				LINE\t=%s\n\
				TYPE\t=%d\n\
				MODE\t=%d\n\
				VERBOSE\t=%d\n\
				AHOST\t=%s\n\
				ASOCKET\t=%d\n\
				ALIM\t=%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d \n",
				

				&(server[number].socket),
				server[number].host,
				server[number].line,
				&(server[number].type),
				&(server[number].mode),
				&(server[number].verbose),
				server[number].ahost,
				&(server[number].asocket),
				&(server[number].alim[0]),
				&(server[number].alim[1]),
				&(server[number].alim[2]),
				&(server[number].alim[3]),
				&(server[number].alim[4]),
				&(server[number].alim[5]),
				&(server[number].alim[6]),
				&(server[number].alim[7]),
				&(server[number].alim[8]),
				&(server[number].alim[9]),
				&(server[number].alim[10]),
				&(server[number].alim[11]),
				&(server[number].alim[12]),
				&(server[number].alim[13]),
				&(server[number].alim[14]),
				&(server[number].alim[15]));
		if (sbrodola)
		printf ( "SOCKET\t=%d\n\
				HOST\t=%s\n\
				LINE\t=%s\n\
				TYPE\t=%d\n\
				MODE\t=%d\n\
				VERBOSE\t=%d\n\
				AHOST\t=%s\n\
				ASOCKET\t=%d\n\
				ALIM\t=%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d \n",
				

				server[number].socket,
				server[number].host,
				server[number].line,
				server[number].type,
				server[number].mode,
				server[number].verbose,
				server[number].ahost,
				server[number].asocket,
				server[number].alim[0],
				server[number].alim[1],
				server[number].alim[2],
				server[number].alim[3],
				server[number].alim[4],
				server[number].alim[5],
				server[number].alim[6],
				server[number].alim[7],
				server[number].alim[8],
				server[number].alim[9],
				server[number].alim[10],
				server[number].alim[11],
				server[number].alim[12],
				server[number].alim[13],
				server[number].alim[14],
				server[number].alim[15]);


		}

	}

fclose ( dbfile );




dbfile = fopen ( file_name, "r" );
if ( dbfile == NULL )
	{
	printf ( "netm: error opening network data base file\n");
	fflush ( stdout );
	exit ( 2 );
	}


printf ("\n");
while ( fgets ( line, 100, dbfile) != NULL )
	{
	if ( strncmp ( line, "STEERER_SERVER", 14) == 0)
		{
		sscanf ( line, "STEERER_SERVER\t=%d", &number);
		/*printf ( "numero = %d \n", number );*/
		printf ("steerer server block %d found  \n",number);
		fscanf ( dbfile,"SOCKET\t=%d\n\
				HOST\t=%s\n\
				LMVA\t=%s %d %d %d %d %d %d\n\
				LMVA\t=%s %d %d %d %d %d %d\n\
				LMVA\t=%s %d %d %d %d %d %d\n\
				LMVA\t=%s %d %d %d %d %d %d\n\
				LMVA\t=%s %d %d %d %d %d %d\n\
				LMVA\t=%s %d %d %d %d %d %d\n\
				LMVA\t=%s %d %d %d %d %d %d\n\
				LMVA\t=%s %d %d %d %d %d %d\n",


				&(steerer_server[number].socket),
				steerer_server[number].host,

				steerer_server[number].lines[0],
				&(steerer_server[number].modes[0]),
				&(steerer_server[number].verbose[0]),
				&(steerer_server[number].alim[0][0]),
				&(steerer_server[number].alim[0][1]),
				&(steerer_server[number].alim[0][2]),
				&(steerer_server[number].alim[0][3]),

				steerer_server[number].lines[1],
				&(steerer_server[number].modes[1]),
				&(steerer_server[number].verbose[1]),
				&(steerer_server[number].alim[1][0]),
				&(steerer_server[number].alim[1][1]),
				&(steerer_server[number].alim[1][2]),
				&(steerer_server[number].alim[1][3]),

				steerer_server[number].lines[2],
				&(steerer_server[number].modes[2]),
				&(steerer_server[number].verbose[2]),
				&(steerer_server[number].alim[2][0]),
				&(steerer_server[number].alim[2][1]),
				&(steerer_server[number].alim[2][2]),
				&(steerer_server[number].alim[2][3]),

				steerer_server[number].lines[3],
				&(steerer_server[number].modes[3]),
				&(steerer_server[number].verbose[3]),
				&(steerer_server[number].alim[3][0]),
				&(steerer_server[number].alim[3][1]),
				&(steerer_server[number].alim[3][2]),
				&(steerer_server[number].alim[3][3]),

				steerer_server[number].lines[4],
				&(steerer_server[number].modes[4]),
				&(steerer_server[number].verbose[4]),
				&(steerer_server[number].alim[4][0]),
				&(steerer_server[number].alim[4][1]),
				&(steerer_server[number].alim[4][2]),
				&(steerer_server[number].alim[4][3]),

				steerer_server[number].lines[5],
				&(steerer_server[number].modes[5]),
				&(steerer_server[number].verbose[5]),
				&(steerer_server[number].alim[5][0]),
				&(steerer_server[number].alim[5][1]),
				&(steerer_server[number].alim[5][2]),
				&(steerer_server[number].alim[5][3]),

				steerer_server[number].lines[6],
				&(steerer_server[number].modes[6]),
				&(steerer_server[number].verbose[6]),
				&(steerer_server[number].alim[6][0]),
				&(steerer_server[number].alim[6][1]),
				&(steerer_server[number].alim[6][2]),
				&(steerer_server[number].alim[6][3]),

				steerer_server[number].lines[7],
				&(steerer_server[number].modes[7]),
				&(steerer_server[number].verbose[7]),
				&(steerer_server[number].alim[7][0]),
				&(steerer_server[number].alim[7][1]),
				&(steerer_server[number].alim[7][2]),
				&(steerer_server[number].alim[7][3]));

/*		printf ( "SOCKET\t=%d\n\
				HOST\t=%s\n\
				LMVA\t=%s %d %d %d %d %d %d\n\
				LMVA\t=%s %d %d %d %d %d %d\n\
				LMVA\t=%s %d %d %d %d %d %d\n\
				LMVA\t=%s %d %d %d %d %d %d\n\
				LMVA\t=%s %d %d %d %d %d %d\n\
				LMVA\t=%s %d %d %d %d %d %d\n\
				LMVA\t=%s %d %d %d %d %d %d\n\
				LMVA\t=%s %d %d %d %d %d %d\n",


				steerer_server[number].socket,
				steerer_server[number].host,

				steerer_server[number].lines[0],
				steerer_server[number].modes[0],
				steerer_server[number].verbose[0],
				steerer_server[number].alim[0][0],
				steerer_server[number].alim[0][1],
				steerer_server[number].alim[0][2],
				steerer_server[number].alim[0][3],

				steerer_server[number].lines[1],
				steerer_server[number].modes[1],
				steerer_server[number].verbose[1],
				steerer_server[number].alim[1][0],
				steerer_server[number].alim[1][1],
				steerer_server[number].alim[1][2],
				steerer_server[number].alim[1][3],

				steerer_server[number].lines[2],
				steerer_server[number].modes[2],
				steerer_server[number].verbose[2],
				steerer_server[number].alim[2][0],
				steerer_server[number].alim[2][1],
				steerer_server[number].alim[2][2],
				steerer_server[number].alim[2][3],

				steerer_server[number].lines[3],
				steerer_server[number].modes[3],
				steerer_server[number].verbose[3],
				steerer_server[number].alim[3][0],
				steerer_server[number].alim[3][1],
				steerer_server[number].alim[3][2],
				steerer_server[number].alim[3][3],

				steerer_server[number].lines[4],
				steerer_server[number].modes[4],
				steerer_server[number].verbose[4],
				steerer_server[number].alim[4][0],
				steerer_server[number].alim[4][1],
				steerer_server[number].alim[4][2],
				steerer_server[number].alim[4][3],

				steerer_server[number].lines[5],
				steerer_server[number].modes[5],
				steerer_server[number].verbose[5],
				steerer_server[number].alim[5][0],
				steerer_server[number].alim[5][1],
				steerer_server[number].alim[5][2],
				steerer_server[number].alim[5][3],

				steerer_server[number].lines[6],
				steerer_server[number].modes[6],
				steerer_server[number].verbose[6],
				steerer_server[number].alim[6][0],
				steerer_server[number].alim[6][1],
				steerer_server[number].alim[6][2],
				steerer_server[number].alim[6][3],

				steerer_server[number].lines[7],
				steerer_server[number].modes[7],
				steerer_server[number].verbose[7],
				steerer_server[number].alim[7][0],
				steerer_server[number].alim[7][1],
				steerer_server[number].alim[7][2],
				steerer_server[number].alim[7][3]);
*/

		}

	}

fclose ( dbfile );




dbfile = fopen ( file_name, "r" );
if ( dbfile == NULL )
	{
	printf ( "netm: error opening network data base file\n");
	fflush ( stdout );
	exit ( 2 );
	}


printf ("\n");
while ( fgets ( line, 100, dbfile) != NULL )
	{
	if ( strncmp ( line, "_STEERER", 8) == 0)
		{
		sscanf ( line, "_STEERER\t=%d", &number);
		steerer[number].number = number;
		/*printf ( "numero = %d \n", number );*/
/*		printf ("steerer block %d found  \n",number);*/
		fscanf ( dbfile,"NAME\t=%s\n\
				ALIM1\t=%d\n\
				ALIM2\t=%d\n",
				steerer[number].name,
				&(steerer[number].alim1),
				&(steerer[number].alim2));
	


/*
printf ( 			"STEERER\t=%d\n\
				NAME\t=%s\n\
				ALIM1\t=%d\n\
				ALIM2\t=%d\n",
				steerer[number].number,
				steerer[number].name,
				steerer[number].alim1,
				steerer[number].alim2);
*/


		}	
	}
fclose ( dbfile );




printf ("\nci sono %d server per i magneti\n", magnet_server_counter);

magnet_server_counter = 0;
magnet_counter = 0;

for ( number = 0 ; number < 100; number++ )
	{
	if ( magnet[number].magnet != 999999 )
		magnet_counter++;
	if ( server[number].socket != 999999 )
		magnet_server_counter++;
	}

printf ("\nci sono %d magneti\n", magnet_counter);
printf ("\nci sono %d server per i magneti\n", magnet_server_counter);




/*riempimento struttura*/


/*for (counter = 0 ; counter < 16 ;counter++)
printf ("server[0].alim[%d] = %d\n",counter, server[0].alim[counter]);

for (counter = 0 ; counter < 100 ;counter++)
printf ("magnet[%d].a_alim1 = %d magnet[%d].a_alim2 = %d\n",counter, magnet[counter].a_alim1,counter, magnet[counter].a_alim2);*/


for (counter = 0 ; counter < 100 ;counter++) /*loop sui server */
    {
    if ( server[counter].socket != 999999)
        {
        for ( alim_counter = 0; alim_counter <16; alim_counter++)
            {
            if ( server[counter].alim[alim_counter] != 999999 )
		{
		trovato = 0;
		for ( magnet_counter = 0; magnet_counter <100; magnet_counter++)
		    {
		    if ( magnet[magnet_counter].magnet != 999999 )
			{
			if ( server[counter].alim[alim_counter] == magnet[magnet_counter].alim1 )
			    {
			    server[counter].alim_addr[alim_counter] = magnet[magnet_counter].a_alim1;
			    if (sbrodola)
			    printf("l'alimentatore n. %d e' attaccato al server %d ,ed ha indirizzo %d\n",
			    server[counter].alim[alim_counter], counter,
			    server[counter].alim_addr[alim_counter] );
			    trovato++;
			    }
    
			if ( server[counter].alim[alim_counter] == magnet[magnet_counter].alim2 )
			    {
			    server[counter].alim_addr[alim_counter] = magnet[magnet_counter].a_alim2;
			    if (sbrodola)
			    printf("l'alimentatore n. %d e' attaccato al server %d ed ha indirizzo %d\n",
			    server[counter].alim[alim_counter], counter,
			    server[counter].alim_addr[alim_counter] );
			    trovato++;
			    }
    
			}
		    }
		 if ( trovato < 1 && sbrodola) 
		 printf ("non ho trovato nessun magnete per l'alimentatore %d\n",
		 server[counter].alim[alim_counter]);
		 if ( trovato > 1 && sbrodola) 
		 printf ("ho trovato %d magneti per l'alimentatore %d\n",
		 trovato, server[counter].alim[alim_counter]);
		}	      
            }
        }
    }


for ( magnet_counter = 0; magnet_counter <100; magnet_counter++)
    {
    if ( magnet[magnet_counter].magnet != 999999 )
        {
       

        trovato = 0;
        for (counter = 0 ; counter < 100 ;counter++) /*loop sui server */      
            {
            for ( alim_counter = 0; alim_counter <16; alim_counter++)
                {
                if ( server[counter].alim[alim_counter] == magnet[magnet_counter].alim1 )
                    trovato++;
                }
            }
        if ( trovato < 1 ) 
            printf ("non ho trovato nessun server per l'alimentatore %d\n",
            magnet[magnet_counter].alim1);
        if ( trovato > 1 ) 
            printf ("ho trovato %d server per l'alimentatore %d\n",
            trovato, magnet[magnet_counter].alim2);


        trovato = 0;
        for (counter = 0 ; counter < 100 ;counter++) /*loop sui server */      
            {
            for ( alim_counter = 0; alim_counter <16; alim_counter++)
                {
                if ( server[counter].alim[alim_counter] == magnet[magnet_counter].alim2 )
                    trovato++;
                }
            }
        if ( trovato < 1 ) 
            printf ("non ho trovato nessun server per l'alimentatore %d\n",
            magnet[magnet_counter].alim2);
        if ( trovato > 1 ) 
            printf ("ho trovato %d server per l'alimentatore %d\n",
            trovato, magnet[magnet_counter].alim2);
        }
    }
  

}
