#define FALSE            0
#define MSGLEN 1024
#define BUF_SIZE 1024
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#include "ma_net_const.h"
#include "ma_net_ext_var.h"

close_net_channels()
{
char msg_tosend[MSGLEN], msg_torcv[MSGLEN] ;
int counter;


if (sbrodola)
printf ("sono in close_net_channels()\n");

sprintf(msg_tosend, "%c",0);
for (counter = 0 ; counter < 100 ;counter++)
	{
	if ( server[counter].socket != 999999 )
		{
		printf("chiudo il canale %d\n",counter);
		write( steerer_server[counter].fildes, msg_tosend, MSGLEN );
		close(server[counter].fildes);	/* ERRORE ??????  */
		}
	}

for (counter = 0 ; counter < 100 ;counter++)
	{
	if ( steerer_server[counter].socket != 999999 )
	    {
	    printf("chiudo il canale %d\n",counter);
	    write( steerer_server[counter].fildes, msg_tosend, MSGLEN );
	    close(steerer_server[counter].fildes);
	    }
	}	



}

open_steerer_channels()
{
int gen_chan , channel_addrlen , main_socket, gen_chan_addrlen;
int counter, cnt, cnt1, cnt2, trovato;
int not_error, chan_addrlen ;
char msg_tosend[MSGLEN], msg_torcv[MSGLEN] ;
struct sockaddr_in chan_addr ;
struct hostent * hp, * gethostbyname() ;
struct sockaddr_in gen_chan_addr, channel_addr;
FILE *dbfile;
int uscita;
char line[200];
char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ] ;
int number, magnet_server_counter, magnet_counter, alim_counter;
int read_retval;


printf ( "in open_steerer_channels()\n");


for (counter = 0 ; counter < 100 ;counter++)
	{
	if ( steerer_server[counter].socket != 999999 )
	    {

printf("\n\n\nsteerer_server[%d].socket = %d\n\n\n\n",counter,steerer_server[counter].socket);
	    printf ("\n");  
	    /* Create a socket to/from server ( c is client ) */
	
	    steerer_server[counter].fildes = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
	    if ( steerer_server[counter].fildes < 0 ) 
		    {
		    perror( "\nclient: error opening socket\n" ) ;
		    exit( 1 ) ;
		    }
    
	setsockopt(steerer_server[counter].fildes, SOL_SOCKET, SO_REUSEADDR, 0, 0);


    printf("ok socket\n");


        for ( cnt = 0 ; cnt < 8 ; cnt++ )
		{
		for ( cnt1 = 0 ; cnt1 < 4 ; cnt1++ )
			{
			if ( steerer_server[counter].alim[cnt][cnt1] != 999999 )
				{
				steerer_alim[steerer_server[counter].alim[cnt][cnt1]].fildes = 
				steerer_server[counter].fildes;

				steerer_alim[steerer_server[counter].alim[cnt][cnt1]].mode = 
				steerer_server[counter].modes[cnt];


				if (strncmp (steerer_server[counter].lines[cnt], "/dev/null", 9) == 0)
					steerer_alim[steerer_server[counter].alim[cnt][cnt1]].mode = 0 ;


				if (sbrodola)
				printf ("steerer_alim[%d].fildes = %d\n",
				steerer_server[counter].alim[cnt][cnt1], steerer_alim[steerer_server[counter].alim[cnt][cnt1]].fildes);

				}
			}
		}



    printf("ok iniz %d\n", counter);


	    /* connect socket using name specified by command line */
	    chan_addr.sin_family = AF_INET ;

	    printf ("counter = %d steerer_server[counter].socket = %d  gethostbyname di %s\n",counter,
	    steerer_server[counter].socket,steerer_server[counter].host);
	    hp = gethostbyname( steerer_server[counter].host ) ;
	    bcopy ( hp->h_addr, &( chan_addr.sin_addr.s_addr ), hp->h_length ) ;
	    chan_addr.sin_port = htons( steerer_server[counter].socket ) ;
	    chan_addrlen = sizeof( chan_addr ) ;

        printf("connect\n");

	    if ( connect( steerer_server[counter].fildes, &chan_addr, chan_addrlen ) < 0 ) 
		    {
		    close( steerer_server[counter].fildes ) ;
		    perror( "\nclient: error connecting socket\n" ) ;
		    exit( 1 ) ;
		    }
  	
	    printf( "client: host is %s, port is %d\n",
	    steerer_server[counter].host, ntohs( chan_addr.sin_port ) ) ;
	    }
	}

/*printf ("open_steerer_channels() : inizio loop trasmissione dati \n");*/
for (counter = 0 ; counter < 100 ;counter++)
	{
	if ( steerer_server[counter].socket != 999999 )
	    {



	    sprintf(msg_tosend, "%s %d %d %d %d %d %d\n\
				%s %d %d %d %d %d %d\n\
				%s %d %d %d %d %d %d\n\
				%s %d %d %d %d %d %d\n\
				%s %d %d %d %d %d %d\n\
				%s %d %d %d %d %d %d\n\
				%s %d %d %d %d %d %d\n\
				%s %d %d %d %d %d %d\n",

				steerer_server[counter].lines[0],
				steerer_server[counter].modes[0],
				steerer_server[counter].verbose[0],
				steerer_server[counter].alim[0][0],
				steerer_server[counter].alim[0][1],
				steerer_server[counter].alim[0][2],
				steerer_server[counter].alim[0][3],

				steerer_server[counter].lines[1],
				steerer_server[counter].modes[1],
				steerer_server[counter].verbose[1],
				steerer_server[counter].alim[1][0],
				steerer_server[counter].alim[1][1],
				steerer_server[counter].alim[1][2],
				steerer_server[counter].alim[1][3],

				steerer_server[counter].lines[2],
				steerer_server[counter].modes[2],
				steerer_server[counter].verbose[2],
				steerer_server[counter].alim[2][0],
				steerer_server[counter].alim[2][1],
				steerer_server[counter].alim[2][2],
				steerer_server[counter].alim[2][3],

				steerer_server[counter].lines[3],
				steerer_server[counter].modes[3],
				steerer_server[counter].verbose[3],
				steerer_server[counter].alim[3][0],
				steerer_server[counter].alim[3][1],
				steerer_server[counter].alim[3][2],
				steerer_server[counter].alim[3][3],

				steerer_server[counter].lines[4],
				steerer_server[counter].modes[4],
				steerer_server[counter].verbose[4],
				steerer_server[counter].alim[4][0],
				steerer_server[counter].alim[4][1],
				steerer_server[counter].alim[4][2],
				steerer_server[counter].alim[4][3],

				steerer_server[counter].lines[5],
				steerer_server[counter].modes[5],
				steerer_server[counter].verbose[5],
				steerer_server[counter].alim[5][0],
				steerer_server[counter].alim[5][1],
				steerer_server[counter].alim[5][2],
				steerer_server[counter].alim[5][3],

				steerer_server[counter].lines[6],
				steerer_server[counter].modes[6],
				steerer_server[counter].verbose[6],
				steerer_server[counter].alim[6][0],
				steerer_server[counter].alim[6][1],
				steerer_server[counter].alim[6][2],
				steerer_server[counter].alim[6][3],

				steerer_server[counter].lines[7],
				steerer_server[counter].modes[7],
				steerer_server[counter].verbose[7],
				steerer_server[counter].alim[7][0],
				steerer_server[counter].alim[7][1],
				steerer_server[counter].alim[7][2],
				steerer_server[counter].alim[7][3]);
    
    
	   /* write to server */
	   if ( write( steerer_server[counter].fildes, msg_tosend, MSGLEN ) < 0 ) 
		    {
		    perror( "\nclient: error writing socket\n" ) ;
		    not_error = FALSE ;
		    }
    
	    /* listen to server */
	    if ( read( steerer_server[counter].fildes, msg_torcv, MSGLEN ) < 0 ) 
		    {
		    perror( "\nclient: error reading socket\n" ) ;
		    not_error = FALSE ;
		    }
	 /*   if ( strncmp( "DONE", msg_torcv, 4) == 0 )
		    printf ("open_steerer_channels() : trasmissione primo gruppo dati del server %d ok \n",counter);*/
    
	    }
	}

}




open_net_channels()
{
int gen_chan , channel_addrlen , main_socket, gen_chan_addrlen;
int counter, cnt, trovato;
int not_error, chan_addrlen ;
char msg_tosend[MSGLEN], msg_torcv[MSGLEN] ;
struct sockaddr_in chan_addr ;
struct hostent * hp, * gethostbyname() ;
struct sockaddr_in gen_chan_addr, channel_addr;
FILE *dbfile;
int uscita;
char line[200];
char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ] ;
int number, magnet_server_counter, magnet_counter, alim_counter;
int read_retval;


sbrodola = 1 ;

/*printf ( "in open_net_channels()\n");*/

for (counter = 0 ; counter < 100 ;counter++)
	{
	if ( server[counter].socket != 999999 )
	    {
if (sbrodola)
/*printf("\n\n\nserver[%d].socket = %d\n\n\n\n",counter,server[counter].socket);*/
	    /*printf ("\n");  */
	    /* Create a socket to/from server ( c is client ) */
	
	    server[counter].fildes = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
	    if ( server[counter].fildes < 0 ) 
		    {
		    perror( "\nclient: error opening socket\n" ) ;
		    exit( 1 ) ;
		    }
    
    	setsockopt(server[counter].fildes, SOL_SOCKET, SO_REUSEADDR, 0, 0);


        for ( cnt = 0 ; cnt < 16 ; cnt++ )
		{
		if ( server[counter].alim[cnt] != 999999 )
			{
			alim[server[counter].alim[cnt]].fildes = server[counter].fildes;
			alim[server[counter].alim[cnt]].mode = server[counter].mode;


			/*printf ("*3* alim[%d].mode = %d \n", server[counter].alim[cnt], alim[server[counter].alim[cnt]].mode);*/
			}
		}




    
	    /* connect socket using name specified by command line */
	    chan_addr.sin_family = AF_INET ;
	    if (sbrodola)
	    /*printf ("counter = %d server[counter].socket = %d  gethostbyname di %s\n",counter,
	    server[counter].socket,server[counter].host);*/
	    hp = gethostbyname( server[counter].host ) ;
	    bcopy ( hp->h_addr, &( chan_addr.sin_addr.s_addr ), hp->h_length ) ;
	    chan_addr.sin_port = htons( server[counter].socket ) ;
	    chan_addrlen = sizeof( chan_addr ) ;
    
	    if ( connect( server[counter].fildes, &chan_addr, chan_addrlen ) < 0 ) 
		    {
		    close( server[counter].fildes ) ;
		    perror( "\nclient: error connecting socket\n" ) ;
		    exit( 1 ) ;
		    }
    	    /*if (sbrodola)	
	    printf( "client: host is %s, port is %d\n",
	    server[counter].host, ntohs( chan_addr.sin_port ) ) ;*/
	    }
	}

printf ("open_net_channels() : inizio loop trasmissione dati \n");
for (counter = 0 ; counter < 100 ;counter++)
	{
	if ( server[counter].socket != 999999 )
	    {
	    if (sbrodola)
	    printf("*1* %s %d %d \n", server[counter].ahost,
	    server[counter].asocket, server[counter].mode);

	    sprintf(msg_tosend, " %s %d %d ", server[counter].ahost,
	    server[counter].asocket, server[counter].mode);
    
    
	   /* write to server */
	   if ( write( server[counter].fildes, msg_tosend, MSGLEN ) < 0 ) 
		    {
		    perror( "\nclient: error writing socket\n" ) ;
		    not_error = FALSE ;
		    }
    
	    /* listen to server */
	    if ( read( server[counter].fildes, msg_torcv, MSGLEN ) < 0 ) 
		    {
		    perror( "\nclient: error reading socket\n" ) ;
		    not_error = FALSE ;
		    }
	    if ( strncmp( "DONE", msg_torcv, 4) == 0 )
		    printf ("*2* trasmissione primo gruppo dati del server %d ok \n",counter);
    
		    
	    if ( server[counter].verbose == 1)
		    sprintf(msg_tosend," -%c %s",'v',server[counter].line);
	    if ( server[counter].verbose == 0)
		    sprintf(msg_tosend," -%c %s",'n',server[counter].line);
    
	   /* write to server */
	   if ( write( server[counter].fildes, msg_tosend, MSGLEN ) < 0 ) 
		    {
		    perror( "\nclient: error writing socket\n" ) ;
		    not_error = FALSE ;
		    }
    
	    /* listen to server */
	    if ( read( server[counter].fildes, msg_torcv, MSGLEN ) < 0 ) 
		    {
		    perror( "\nclient: error reading socket\n" ) ;
		    not_error = FALSE ;
		    }
	    if ( strncmp( "DONE", msg_torcv, 4) == 0 )
		    printf ("open_net_channels() : trasmissione secondo gruppo dati del server %d ok \n",counter);
    
    
	    sprintf ( msg_tosend,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ",
	    server[counter].alim[0], server[counter].alim_addr[0], 
	    server[counter].alim[1], server[counter].alim_addr[1], 
	    server[counter].alim[2], server[counter].alim_addr[2], 
	    server[counter].alim[3], server[counter].alim_addr[3], 
	    server[counter].alim[4], server[counter].alim_addr[4], 
	    server[counter].alim[5], server[counter].alim_addr[5], 
	    server[counter].alim[6], server[counter].alim_addr[6], 
	    server[counter].alim[7], server[counter].alim_addr[7], 
	    server[counter].alim[8], server[counter].alim_addr[8], 
	    server[counter].alim[9], server[counter].alim_addr[9], 
	    server[counter].alim[10], server[counter].alim_addr[10], 
	    server[counter].alim[11], server[counter].alim_addr[11], 
	    server[counter].alim[12], server[counter].alim_addr[12], 
	    server[counter].alim[13], server[counter].alim_addr[13], 
	    server[counter].alim[14], server[counter].alim_addr[14], 
	    server[counter].alim[15], server[counter].alim_addr[15]);
    
	  
    
     /* write to server */
	   if ( write( server[counter].fildes, msg_tosend, MSGLEN ) < 0 ) 
		    {
		    perror( "\nclient: error writing socket\n" ) ;
		    not_error = FALSE ;
		    }
    
    
     
	    /* listen to server */
	    if ( read( server[counter].fildes, msg_torcv, MSGLEN ) < 0 ) 
		    {
		    perror( "\nclient: error reading socket\n" ) ;
		    not_error = FALSE ;
		    }
	    if ( strncmp( "DONE", msg_torcv, 4) == 0 )
		    printf ("open_net_channels() : trasmissione terzo gruppo dati del server %d ok \n",counter);
    
    
	    }

	}
	write(main_channel ,"channels opened" ,BUF_SIZE );

}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/




ma_send_data()
{

int gen_chan , channel_addrlen , main_socket, gen_chan_addrlen;
int counter, trovato;
int not_error, chan_addrlen ;
char msg_tosend[MSGLEN], msg_torcv[MSGLEN] ;
struct sockaddr_in chan_addr ;
struct hostent * hp, * gethostbyname() ;
struct sockaddr_in gen_chan_addr, channel_addr;
FILE *dbfile;
int uscita;
char line[200];
char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ] ;
int number, magnet_server_counter, magnet_counter, alim_counter;
int read_retval;


	
   /* 1 */
	printf ("1 messaggio ricevuto\n");

	for (counter=1 ; counter < 42 ; counter++)
		{
		sprintf( device_msg, "%d %d %d %d %d %d",
		button[counter].button,
		button[counter].type,
		button[counter].obj1,
		button[counter].obj2,
		button[counter].obj3,
		button[counter].obj4);
		printf ("messaggio da trasmettere : %s\n",device_msg);
       if ( write(main_channel ,device_msg ,BUF_SIZE ) < 0 ) {
            perror( "\nclient: error writing socket\n" ) ;
            not_error = FALSE ;
		}
printf("trasmesso messaggio n. %d\n",counter);
    read_retval = read( main_channel, client_msg, BUF_SIZE ) ;
printf("ricevuto  messaggio n. %d  %s\n",counter,client_msg);
   
	 if ( read_retval <= 0 ) 
		{
       	 	if ( read_retval < 0 )
        	    perror( "error reading main socket" ) ;
      	  	not_error = FALSE ;
		}

if (strncmp(client_msg, "record ricevuto",15)==0)
	{
	printf("ho trasmesso il record n. %d \n",counter);
	}
		}

	printf ("2 messaggio ricevuto\n");
	for (counter=1 ; counter < 100 ; counter++)
		{

		sprintf(device_msg,"%d %s %d %s %d %d %d %d %s %d %d %d",

				magnet[counter].magnet,
				magnet[counter].name,

				magnet[counter].alim1,
				magnet[counter].n_alim1,
				magnet[counter].a_alim1,
				magnet[counter].cur_alim1,
				magnet[counter].volt_alim1,

				magnet[counter].alim2,
				magnet[counter].n_alim2,
				magnet[counter].a_alim2,
				magnet[counter].cur_alim2,
				magnet[counter].volt_alim2);

       if ( write(main_channel ,device_msg ,BUF_SIZE ) < 0 ) 
		{
		perror( "\nclient: error writing socket\n" ) ;
		not_error = FALSE ;
		}
	printf("trasmesso messaggio n. %d\n",counter);
	read_retval = read( main_channel, client_msg, BUF_SIZE ) ;
	printf("ricevuto  messaggio n. %d  %s\n",counter,client_msg);
    if ( read_retval <= 0 ) 
	{
        if ( read_retval < 0 )
            perror( "error reading main socket" ) ;
        not_error = FALSE ;
	}

if (strncmp(client_msg, "record ricevuto",15)==0)
	{
	printf("ho trasmesso il record n. %d \n",counter);
	}
		}







	} /* 1 */
