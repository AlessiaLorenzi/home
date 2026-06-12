
extern struct sockaddr_in alarm_addr ;
extern struct alim_data alim[100];
extern struct line serial[8];

extern int verbose, alarm_enabled, alarm_allowed ;
extern char client_msg[ BUF_SIZE ], device_msg[ BUF_SIZE ];
extern char myname[ NCHARS ], alarm_headmsg[ NCHARS ] ;
extern int address[100];
extern int supply_number;

extern int timeout_flag ;
extern int main_channel, alarm_channel;
extern char tty_name[ NCHARS ] ;

extern int timeout_flag ;

extern int errno;    
//extern char *sys_errlist[ ];
