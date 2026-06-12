#
#	This is a makefile for the motor server
#       of the network package for :
#       " Supervisione e Controllo di un Acceleratore Lineare Per Ioni "
#       

CC = cc
CFLAGS = -O2

SRCS = \
main.c\
check_alarm.c\
exec_command.c\
house_keeping.c\
first_read.c\
tty_init.c\
tty_reset.c

OBJS = \
main.o\
check_alarm.o\
exec_command.o\
house_keeping.o\
first_read.o\
tty_init.o\
tty_reset.o

SERVER = rf_server

INCLUDE = server.h motor_server.h

all: $(SERVER)

$(SERVER): ${OBJS}
	cc ${OBJS} -o $@

main.o: server.h motor_server.h
check_alarm.o: server.h
exec_command.o: server.h motor_server.h
	cc -O2 -c exec_command.c
house_keeping.o: server.h
first_read.o: server.h
tty_init.o: server.h
tty_reset.o: server.h
	cc -O2 -c tty_reset.c

#

clean:
	rm -f *.o mot_server.c mot_server.doc mot_server

edit:
	vi makefile ${INCLUDE} ${SRCS}

source:
	cat ${INCLUDE} > motor_server.c ;\
	awk -f source.awk ${SRCS} >> motor_server.c ;

doc:
	awk -f doc.awk\
	main.c\
	exec_command.c\
	first_read.c\
	house_keeping.c\
	check_alarm.c\
	> motor_server.doc

