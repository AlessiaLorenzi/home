#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//Dimensione buffer rete. Il buffer seriale è di 1 char
#define GENLEN 1024

//Timeout in us*100
#define TIMEOUT 10000

//Routine gestione errori con chiusura controllata
//msg è il parametro da passare per la gestione dell'errore caso per caso
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct line		{
			char name[100];
			int mode;
			int verbose;
			int alim[4];
			int schan;
			} ;


struct alim_data	{
			int address;
			int mode;
			int schan;
			int linea;
                        int retry;
			} ;
//retry determina il fattore di recupero dello steerer se trovato offline
//alim 1.2.3.4 sono le linee di ciscuno degli 8 scatolotti collegati
struct alim_data alim[100];
struct line serial[8];

int main(int argc, char** argv)
{
    //termios genera una struttura con i paramentri del terminale
    struct termios sio;
    //sio -> seriale, sio_bkp -> backup temporaneo seriale

    //Contatore caratteri di terminazione
    bool crctr;
    //Counter: i è per le linee; j per alim/buffer; t per il tempo;
    int i=0, j=0, t=0;
    //buffer, il buffer comune principale; 
    //main_socket, la stringa col socket sul quale si trova il NetM...
    char buffer[GENLEN], buffer_recovery[GENLEN], main_socket[5];
    //...passata come argomento all'avvio
    strcpy(main_socket, argv[1] );

    fd_set rdset;

    unsigned char c='0'; //buffer di 1 carattere x tty

    //Pulizia preventiva celle struttura
    for (i=0;i<100;i++)
    {
        alim[i].linea = 999999;
    }

    printf("Steerers Server\nVersion 2.1.1\nBy M. Roetta\n\n");
    printf("Socket: %s\n\n",main_socket);
    //--------------------------------------------------------------------------
    //Variabili TCP/IP dedicate
    int gen_chan, gen_addrlen, net_chan;
    struct sockaddr_in gen_addr, chan_addr ;
    struct hostent * hp, * gethostbyname() ;
    socklen_t chan_addrlen;

    //Avvio canale TCP/IP personalizzato
    gen_chan = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
    if (gen_chan == -1)
        handle_error("socket");
    gen_addr.sin_family = AF_INET ;
    gen_addr.sin_addr.s_addr = INADDR_ANY ;
    gen_addr.sin_port = htons( atoi( main_socket ) ) ;
    gen_addrlen = sizeof( gen_addr ) ;
    chan_addrlen = sizeof( sockaddr_in ) ;

    //Assegnazione canale al socket
    if (bind( gen_chan,(struct sockaddr *) &gen_addr, sizeof(struct sockaddr_in) )==-1)
        handle_error("bind");

    //il numeretto è la quantità di connessioni pendenti sul socket tollerata
    if (listen ( gen_chan, 5 )==-1)
        handle_error("listen");

    //ATTESA CONNESSIONE DA NETM------------------------------------------------
    //NetM si connette ora
    net_chan = accept( gen_chan,(struct sockaddr *) &gen_addr, &chan_addrlen );
    if (net_chan == -1)
        handle_error("socket");

    //Pulisco struttura dati linee e alimentatori
    for ( i = 0 ; i < 8 ; i++ )
	{
	strcpy( serial[i].name, "NULL" );
	serial[i].mode = 999999;
	serial[i].verbose = 999999;
	for ( j = 0 ; j < 4 ; j++ )
		serial[j].alim[j] = 999999;
	}

    //Leggo primo pacchetto da rete
    if (read(net_chan, buffer, GENLEN)==-1)
        handle_error("read");

    //Mattone da interpretare inviato dal NetM al server
    sscanf( buffer, " %s %d %d %d %d %d %d %s %d %d %d %d %d %d %s %d %d %d %d %d %d %s %d %d %d %d %d %d %s %d %d %d %d %d %d %s %d %d %d %d %d %d %s %d %d %d %d %d %d %s %d %d %d %d %d %d",
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

    //Confermo scrittura
    printf("Primo pacchetto OK.\n");
    //Pulisco buffer e invio ACK al NetM
    for (i=0;i<GENLEN;i++)
        buffer[i]='\0';
    strcpy(buffer,"DONE");
    if (write(net_chan, buffer, GENLEN)==-1)
        handle_error("write");

    //Inizializzo strutture dati
    for ( i = 0 ; i < 8 ; i++ )
	{
	//Se come nome ha ricevuto dev null la rendo fantasma
        //La var mode è già stata impostata prima, questo è un controllo speciale
        if ( strncmp (  serial[i].name, "/dev/null", 9 ) == 0 )
            serial[i].mode = 0;

        //Ogni alimentatore definito viene trattato
	for ( j = 0 ; j < 4 ; j++ )
            {
            if ( serial[i].alim[j] != 999999 )
            	{
                    alim[serial[i].alim[j]].address = j + 1;
                    alim[serial[i].alim[j]].mode = serial[i].mode;
                    alim[serial[i].alim[j]].linea = i;
		}
            }
	}

    //Serial time!!-------------------------------------------------------------
    //Pulisco struttura
    memset(&sio,0,sizeof(sio));

    //configurazione linea seriale globale
    //8n1
    sio.c_iflag=0;
    sio.c_oflag=0;
    sio.c_cflag=CS8|CREAD|CLOCAL;
    sio.c_lflag=0;
    sio.c_cc[VMIN]=1;
    sio.c_cc[VTIME]=5;

    cfsetospeed(&sio,B9600);
    cfsetispeed(&sio,B9600);

    //Apertura canali e impostazione riferimenti per ciascun alim
    for (i=0;i<8;i++)
        if (serial[i].mode == 1)
        {
            serial[i].schan=open(serial[i].name, O_RDWR | O_NDELAY);
            tcsetattr(serial[i].schan,TCSANOW,&sio);
            for (j = 0 ; j < 4 ; j++)
            {
                alim[ serial[i].alim[j] ].schan = serial[i].schan;
            }
        }

    printf("Apertura porte...OK.\n");
    //Ricerca degli alimentatori------------------------------------------------
    for (i=0;i<8;i++)
    {
        //Pulisco flag
        crctr = true;
        if (serial[i].mode == 1)
        {
            printf("Ricerca QPSU@%s\n",serial[i].name);
            //Pulisco il buffer
            memset(&buffer,0,sizeof(buffer));
            //Copio comando reset in buffer
            strcpy(buffer, "1\r");
            //Scrivo sul canale seriale il comando
            write(serial[i].schan,&buffer,10);

            //Timeout di 10 secondi, la risposta interrompe il ciclo
            for (t=0;t<TIMEOUT && crctr;t++)
            {
                //Lettura canale alla ricerca di un '>'
                if (read(serial[i].schan,&c,1)>0)
                    {
                        if (c=='>')
                        crctr = false;
                    }
                usleep(100);
            }
            //Se risponde '>' lo segno come 'presente'
            if (!crctr)
            {
                printf("QPSU trovato.\n");
                serial[i].mode = 1;

                //Imposto la condizione di recupero:
                //Se l'alimentatore risponde almeno una volta
                //e poi viene disconnesso lo segno come recuperabile.
                //NB, solo la prima linea dello scatolotto viene controllata
                //questo riduce il ritardo da 4 secondi a 0,5 secondi!
                alim[ serial[i].alim[0] ].retry = 1;
            }
            else
            {
                printf("QPSU non trovato.\n");
                serial[i].mode=0;
                for (j = 0 ; j < 4 ; j++)
                    {
                        alim[ serial[i].alim[j] ].mode = 0;
                    }
            }
        }
    }
    //--------------------------------------------------------------------------
    //Variabili ciclo pseudo infinito
    int supply_n, supply_box; //supply_n 1..100 - supply_box 1..4
    char *message, command;
    float current;
    bool online = true, error=false, sconosciuto=false;
    //Ciclo pseudo infinito-----------------------------------------------------
    do
    {
        //Pulisco buffer comune
        for (i=0;i<GENLEN;i++)
            buffer[i]='\0';

        //Leggo dati da rete
        if (read(net_chan, buffer, GENLEN)==-1)
            break;

        printf("Message by Netwk:%s \n",buffer);

        //Se non è un comando previsto metto sconosciuto
        if (strncmp(buffer,"ALIM =",6)==0)
        {
            //Conversione e comprensione messaggio solo se previsto
            sscanf(buffer,"ALIM = %d", &supply_n);
            message = buffer + 10;
            sscanf(message,"%c", &command);
        }
        else
        {
            command = 'u';
        }
        //Se è un comando di setting leggo la corrente associata
        if (command == '!')
        {
            message = message +3;
            sscanf(message,"%f", &current);
            printf("alim: %d | cmd:%c | I:%f\n", supply_n,command,current);
        }
        else
        {
            printf("alim: %d | cmd:%c | I:--\n", supply_n,command);
        }

        //converto supply_n da 1..100 a 1..4
        supply_box = alim[supply_n].address;
        printf("alim selected:%d\n",supply_box);

        //Pulisco il buffer comune
        for (i=0;i<GENLEN;i++)
        buffer[i]='\0';

        //Interpreto il comando---------------------------------------------
        switch (command)
        {
                case '!':
                    sprintf(buffer,"%1d %5.2f\r",supply_box,current);
                    break;

                case '#':
                    sprintf(buffer,"%1d\r",supply_box);
                    break;

                case 's':
                    //Imposto il server per spegnersi
                    online=false;
                    //Imposto sconosciuto per saltare l'esecuzione scrittura su seriale
                    sconosciuto=true;
                    break;

                default:
                    sconosciuto=true;
                    break;
        }
        //Pre-esecuzione--------------------------------------------------------
        //necessita di buffer_recovery!!
        //Routine fantasma di recupero (!!SPERIMENTALE-ATTENZIONE!!)   
        if (alim[supply_n].mode == 0 && alim[supply_n].retry == 1)
        {
            printf("Recovering...\n");
            crctr = true;
            c='0';  //Reset bufferino seriale
            //Elimino frammenti pendenti (molto probabili)
            tcflush(alim[supply_n].schan, TCIFLUSH );
            //Pulizia buffer recupero
            memset(&buffer_recovery,0,sizeof(buffer_recovery));
            //Copio comando check in buffer_recovery
            strcpy(buffer_recovery, "1\r");
            //Scrivo sul canale seriale il comando
            write(alim[supply_n].schan,&buffer_recovery,10);

            //Timeout di mezzo secondo, la risposta interrompe il ciclo
            for (t=0;t<5000 && crctr;t++)
            {
                //Lettura canale alla ricerca di un '>'
                if (read(alim[supply_n].schan,&c,1)>0)
                    {
                        if (c=='>')
                        crctr = false;
                    }
                usleep(100);
            }
            //Riabilito??
            if (!crctr)
            {
                //Riabilito tutti e 4 gli Alim della linea ricollegata
                for (i=0;i<100;i++)
                {
                    if (alim[i].linea == alim[supply_n].linea)
                        alim[i].mode = 1;
                    printf("%d Restored Succesfully!\n",i);
                }
            }           
        }
        
        //Esecuzione------------------------------------------------------------
        if (alim[supply_n].mode == 1 && !sconosciuto)
        {
            //Elimino eventuali dati presenti nella linea a causa di una improvvisa
            //perdita di tensione e conseguente reset
            tcflush(alim[supply_n].schan, TCIFLUSH );

            //Scrivo i dati via seriale
            write(alim[supply_n].schan, buffer, strlen(buffer));

            //Pulisco il buffer nuovamente per evitare frammenti indesiderati
            for (i=0;i<GENLEN;i++)
                buffer[i]='\0';

            j=0;    //posizione su buffer locale
            c='0';  //bufferino seriale

            //Ciclo principale struttura !!ATTENZIONE!!
            //100000*100us = 10 sec = timeout MASSIMO per la risposta
            for (i=0;i<=TIMEOUT && c!='>';i++)
            {
                if (read( alim[supply_n].schan,&c,1 )>0)
                {
                    buffer[j]=c;
                    j++;
                }
                usleep(100);
                if (i==TIMEOUT)
                {
                    printf("QPSU Failure.\n");
                    error=true;
                }
            }
            //Termino la stringa ricevuta
            buffer[j]='0';

            //Gestione errore disconnessione QPSU
            if (!error)
            {
                printf("Message by Serial:%s\n",buffer);
                if (write(net_chan, buffer, GENLEN)==-1)
                    handle_error("write");
            }
            else
            {
                //Il timeout genera un BAN permanente sul PSU
                alim[supply_n].mode = 0;

                //Disabilito alimentatori fratelli sulla linea padre
                for (i=0;i<100;i++)
                {
                    if (alim[i].linea == alim[supply_n].linea)
                        alim[i].mode = 0;
                }

                for (i=0;i<GENLEN;i++)
                    buffer[i]='\0';
                strcpy(buffer,"STUB");
                printf("Message by Serial:%s\n",buffer);
                if (write(net_chan, buffer, GENLEN)==-1)
                    handle_error("write");
            }
        }
        else
        {
            //Svuoto buffer comune
            for (i=0;i<GENLEN;i++)
                    buffer[i]='\0';
            
            //Condizione 'sconosciuto'
            if (command == 'u')
            {
                strcpy(buffer,"NOT DONE");
                if (write(net_chan, buffer, GENLEN)==-1)
                    handle_error("write");
            }

            //Condizione 'stubbed'
            if (alim[supply_n].mode == 0)
            {
                strcpy(buffer,"STUB");
                if (write(net_chan, buffer, GENLEN)==-1)
                    handle_error("write");
            }
        }
        //Reimposto flag
        error=false;
        sconosciuto=false;
    } while (online);

    //Chiudo il canale di rete verso il NetM
    close(net_chan);
    printf("Server scollegato.\n");
    
    //Chiudo i canali seriali allocati/aperti
    for (i=0;i<8;i++)
        if (serial[i].mode == 1)
            close(serial[i].schan);

    //TERMINE PROGRAMMA---------------------------------------------------------
    printf("Processo terminato!\n");
}


