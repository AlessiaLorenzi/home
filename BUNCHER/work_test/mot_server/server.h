
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sgtty.h>

#define TRUE                  1
#define FALSE                 0
#define NCHARS               64
#define BUF_SIZE           1024
#define MIN_SOCKET         2000
#define MAX_SOCKET        12000
#define POS_ANSWER       "DONE"
#define NEG_ANSWER   "NOT_DONE"
#define REAL                  1
#define STUB                  0
#define NOT_EXISTING         -1

#define EOC          '0'
#define ALARMS_ON    '1'
#define ALARMS_OFF   '2'
#define RECORD_ON    '3'
#define RECORD_OFF   '4'

