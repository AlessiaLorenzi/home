
struct sockaddr_in alarm_addr ;
struct alim_data alim[100];
struct line serial[8];

int verbose, alarm_enabled, alarm_allowed ;
char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ];
char myname[ NCHARS ], alarm_headmsg[ NCHARS ] ;
int address[100];
int supply_number;
int main_channel, alarm_channel;
char tty_name[ NCHARS ] ;




int timeout_flag ;
