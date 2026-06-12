#
#	This is a makefile for the rf-controller server
#       of the network package for :
#       " Supervisione e Controllo di un Acceleratore Lineare Per Ioni "
#       

CC = cc
CFLAGS = -g

SRCS = \
main.c\
exec_command.c\
house_keeping.c\
first_read.c\
tty_init.c\
tty_reset.c

OBJS = \
main.o\
exec_command.o\
house_keeping.o\
first_read.o\
tty_init.o\
tty_reset.o


SERVER = st_server

INCLUDE = server.h  server_const.h  server_vars.h

LIST = st_server.list

all: $(SERVER)

$(SERVER): ${OBJS}
	cc -g ${OBJS} $(LIB) -o $@

main.o: server.h server_const.h server_vars.h
check_alarm.o: server.h server_const.h server_vars.h
exec_command.o: server.h server_const.h server_vars.h
house_keeping.o: server.h server_const.h server_vars.h
first_read.o: server.h server_const.h server_vars.h
read_alladcs.o: server.h server_const.h server_vars.h
tty_init.o: server.h server_const.h server_vars.h
tty_reset.o: server.h server_const.h server_vars.h

#

clean:
	rm -f *.o *.BAK *JNL ma_server.c st_server $(LIST) ma_server.cflow

list:
	pr -n -f -l66 ${INCLUDE} ${SRCS} > $(LIST) ;

edit:
	vi makefile ${INCLUDE} ${SRCS}

lint:
	lint -YBSD -x ${SRCS}

source:
	cat ${INCLUDE} > ma_server.c ;\
	awk -f source.awk ${SRCS} >> ma_server.c ;

stat:
	make list ;\
	ls -l $(LIST) ; wc $(LIST)

cflow:
	make source ; date > ma_server.cflow ;
	echo "CFLOW" >> ma_server.cflow ;
	cflow ma_server.c >> ma_server.cflow ;
	echo "CFLOW -R" >> ma_server.cflow ;
	cflow -r ma_server.c >> ma_server.cflow ;
