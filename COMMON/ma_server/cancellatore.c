#include "server.h"
#include "server_const.h"
#include "server_vars.h"

FILE *rs232r, *rs232w ;
char tty_name[ NCHARS ] ;
int fildes ;
int main_socket;


/* global vars: used by several functions */

int main_channel, alarm_channel, alarm_enabled, alarm_addrlen,
    alarm_allowed, mode = 0, verbose = 0 ;
int giorno_corrente;
FILE *log_file;

    int supply_number;
    char *message;

char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ],
     alarm_msg[ BUF_SIZE ], alarm_headmsg[ NCHARS ], myname[ NCHARS ] ;

int address[100];
int alim_mode[100];

main( argc, argv )
int argc ;
char **argv ;
{
    extern int main_channel, alarm_channel,
               alarm_enabled, alarm_allowed,
               mode, verbose ;
    extern char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ],
                alarm_msg[ BUF_SIZE ], myname[ NCHARS ] ;

    extern struct supply_data supply[100];
    
    int  alarm_socket,
        gen_chan, gen_chan_addrlen,
        read_retval, write_retval, signal_retval, not_end,
        channel_addrlen ;
    struct sockaddr_in gen_chan_addr, channel_addr ;
    struct hostent * hp, * gethostbyname() ;
    char alarm_machine[ NCHARS ] ;

    int connect_alarm() ;
    void on_alarm() ;

char file_daaprire[100];


time_t t;
struct tm *tempo;


t = time( (long*) 0);
tempo = localtime (&t);


sprintf(file_daaprire, "rm /usr/users/magnets/logs/server????_%d.log", tempo->tm_yday - 20 );
/*printf("%s\n", file_daaprire);*/
system(file_daaprire);

}

