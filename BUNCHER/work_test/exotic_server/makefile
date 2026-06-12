
CC = cc
CFLAGS = -O3

SRCS = \
main_general.c\
check_alarm.c\
exec_command.c\
read_alladcs.c\
house_keeping.c\
first_read.c\
tty_init.c\
tty_reset.c\
on_alarm.c\
connect_alarm.c

OBJS = \
main_general.o\
check_alarm.o\
exec_command.o\
read_alladcs.o\
house_keeping.o\
first_read.o\
tty_init.o\
tty_reset.o\
on_alarm.o\
connect_alarm.o

SERVER = ma_server_general

INCLUDE = server.h  initmsgs.h server_const.h  server_vars.h

LIST = ma_server.list

all: $(SERVER)

$(SERVER): ${OBJS}
	cc -g ${OBJS} $(LIB) -o $@

main_general.o: server.h server_const.h server_vars.h
check_alarm.o: server.h server_const.h server_vars.h
exec_command.o: server.h server_const.h server_vars.h
	cc -DECR -O3 -c exec_command.c
house_keeping.o: server.h server_const.h server_vars.h
first_read.o: server.h initmsgs.h server_const.h server_vars.h
read_alladcs.o: server.h server_const.h server_vars.h
tty_init.o: server.h server_const.h server_vars.h
tty_reset.o: server.h server_const.h server_vars.h

clean:
	rm -f *.o *.BAK *JNL ma_server.c ma_server $(LIST) ma_server.cflow

