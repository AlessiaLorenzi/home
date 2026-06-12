#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#define TRUE                  1
#define FALSE                 0
#define NCHARS               64
#define BUF_SIZE           1024
#define MIN_SOCKET         2000
#define MAX_SOCKET        12000
#define POS_ANSWER       "DONE"
#define NEG_ANSWER   "NOT DONE"
#define ALARM_PERIOD          3 /* in sec. */
#define EOC          '0'
#define ALARMS_ON    '1'
#define ALARMS_OFF   '2'

/***********************************************************************
    SERVER DEPENDENT const and vars
***********************************************************************/

#define ALLADCS      'a'
#define RESET        'r'
#define DOIT         '!'
#define STAT         '#'

#define TTY_TIMEOUT 5
