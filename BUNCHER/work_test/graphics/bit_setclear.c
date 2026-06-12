/*
 * Motif required Headers
 */
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#if (XmVersion >= 1002)
#include <Xm/RepType.h>
#endif
#include <Xm/MwmUtil.h>


#include "defs.h"
#include "w_defs.h"
#define EOL '\n'


bit_set(controller, porta, bit_position)
int controller, porta, bit_position;
{ 
char mess[100], mess1[100];
char stringa[1024], stringa1[64], stringa2[64];
char frame_to_s[1024], frame_to_r[1024] ;
int nb_to_s, nb_to_r ;
nb_to_s = sizeof( stringa ) ;
nb_to_r = sizeof( frame_to_r ) ;
strcpy(stringa,"!bset ");

if ( net_verbose ) printf("bit_set : descrittore = %d porta = %d  bit = %d \n", controller, porta, bit_position);

sprintf(stringa1,"%2d", porta);
strcat(stringa, stringa1);
strcat(stringa, ",");
sprintf(stringa2,"%1d", bit_position);
strcat(stringa, stringa2);


if ( net_verbose ) printf("\ndescrittore= %d\n",controller);
if ( net_verbose ) printf("\nora scrivo al server \n");
if ( net_verbose ) printf("\nla stringa e* : %s \n",stringa);
if ( net_verbose ) printf("\nla stringa e* lunga %d caratteri \n",strlen(stringa));


           /* write to server */
           if (  write(controller,stringa,nb_to_s) < 0 ) {
                perror( "\nclient: error writing to s streamsocket\n" ) ;
                }

if ( net_verbose ) printf("\nora ascolto il server \n");

            /* listen to server */
            if ( read( controller, frame_to_r, nb_to_r ) < 0 ) {
                perror( "\nclient: error reading from s streamsocket\n" ) ;
                }

strcpy(answer_str, frame_to_r);
if ( net_verbose ) printf("\nora esco da bit_set \n");

}


bit_clear(controller, porta, bit_position)
int controller, porta, bit_position;
{
char mess[100], mess1[100];
char stringa[1024], stringa1[64], stringa2[64];
char frame_to_s[1024], frame_to_r[1024] ;
int nb_to_s, nb_to_r ;
nb_to_s = sizeof( stringa ) ;
nb_to_r = sizeof( frame_to_r ) ;
strcpy(stringa,"!bclr ");

if ( net_verbose ) printf("bit_clear : descrittore = %d porta = %d  bit = %d \n", controller, porta, bit_position);

sprintf(stringa1,"%2d", porta);
strcat(stringa, stringa1);
strcat(stringa, ",");
sprintf(stringa2,"%1d", bit_position);
strcat(stringa, stringa2);

           /* write to server */
           if (  write(controller,stringa,nb_to_s) < 0 ) {
                perror( "\nclient: error writing to s streamsocket\n" ) ;
                }

            /* listen to server */
            if ( read( controller, frame_to_r, nb_to_r ) < 0 ) {
                perror( "\nclient: error reading from s streamsocket\n" ) ;
               }


strcpy(answer_str, frame_to_r);
if ( net_verbose ) printf("\nora esco da bit_clear \n");
}




scriviporta(controller, porta, valore)
int controller, porta, valore;
{ 
char mess[100], mess1[100];
/*char stringa[64], stringa1[64], stringa2[64];*/
char stringa[1024], stringa1[64], stringa2[64];
char frame_to_s[1024], frame_to_r[1024] ;
int nb_to_s, nb_to_r, valore_letto, port_value ;
nb_to_s = sizeof( stringa ) ;
nb_to_r = sizeof( frame_to_r ) ;
strcpy(stringa,"!wr ");
/*if ( VERBOSE ) printf("porta = %d  valore = %d \n",valore);*/
sprintf(stringa1,"%2d", porta);
strcat(stringa, stringa1);
strcat(stringa, ",");
sprintf(stringa2,"%1d", valore);
strcat(stringa, stringa2);


           /* write to server */
           if (  write(controller, stringa, nb_to_s) < 0 ) {
                perror( "\nclient: error writing to s streamsocket\n" ) ;
                }

            /* listen to server */
            if ( read( controller, frame_to_r, nb_to_r ) < 0 ) {
                perror( "\nclient: error reading from s streamsocket\n" ) ;
                }



strcpy(answer_str, frame_to_r);

#ifdef PIPPO

    if (strncmp("Stub",answer_string,4) == 0)
	{
	if ( VERBOSE ) printf ("bit clear 2 chiamo errore\n");
	if ( controller == 0 )
		errore("Il cestello buncher non risponde", 10);
	if ( controller == 1 )
		errore("Il cestello buttafuori non risponde", 10);
	return(2);
	}

    if (strncmp("Stub",answer_string,4) != 0)
	{
	sscanf(frame_to_r, "Port = %d Data = %d",&port_value, &valore_letto);
	if ( valore_letto != valore )
		{
		if ( VERBOSE ) printf ("bit clear 1 chiamo errore\n");
		strcpy(mess, "errore nel ");
		if ( controller == 0 )
			strcat(mess, "cestello buncher    porta ");
		if ( controller == 1 )
			strcat(mess, "cestello buttafuori porta ");
		sprintf(mess1, "%1d", porta);
		strcat(mess, mess1);
		errore(mess, 10);
		return(1);
		}

	else
		{
		return(0);
		}
	}



#endif


}




int leggiporta(controller, porta)
int controller, porta;
{
int valore,port_value;
/*char stringa[64], stringa1[64], stringa2[64];*/
char stringa[1024], stringa1[64], stringa2[64];
char frame_to_s[1024], frame_to_r[1024] ;
int nb_to_s, nb_to_r ;
nb_to_s = sizeof( stringa ) ;
nb_to_r = sizeof( frame_to_r ) ;
strcpy(stringa,"!rd ");
/*("porta = %d  \n",porta);*/
sprintf(stringa1,"%2d", porta);
strcat(stringa, stringa1);


           /* write to server */
           if (  write(controller,stringa,nb_to_s) < 0 ) {
                perror( "\nclient: error writing to s streamsocket\n" ) ;
                }

            /* listen to server */
            if ( read( controller, frame_to_r, nb_to_r ) < 0 ) {
                perror( "\nclient: error reading from s streamsocket\n" ) ;
                }

strcpy(answer_str, frame_to_r);
/*if ( net_verbose ) printf("\nla risposta di leggiporte e* %s\n", frame_to_r);*/

sscanf(frame_to_r, "Port = %d Data = %d",&port_value, &valore);

if ( net_verbose ) printf("valore in leggiporta = %d\n",valore);

return(valore);
}

#ifdef PIPPONE

leggibit(controller, porta,bit_position)
int controller, porta, bit_position;
{
int valore;
int maschera=1;
char frame_to_s[1024], frame_to_r[1024] ;
int nb_to_s, nb_to_r ;
nb_to_s = sizeof( frame_to_s ) ;
nb_to_r = sizeof( frame_to_r ) ;
valore=leggiporta(porta);
maschera=maschera << bit_position;
if ( VERBOSE ) printf("bit position = %d maschera = %d \n",bit_position,maschera);
valore = valore & maschera;
if (valore !=0)
valore= 1;
return(valore);
}
#endif
