CC = cc

CFLAGS = -g 

LDFLAGS = -lm -g

SRCS = net.c

OBJS = net.o

LSRCS = read_ma_data.c net_calls.c 

LOBJS = read_ma_data.o net_calls.o

NETLIB = net_lib.a

LIBS = $(NETLIB)



netm : ${OBJS} ${LIBS}
	cc ${OBJS} ${LIBS} $(LDFLAGS) -o $@ ;


$(NETLIB): $(LOBJS)
	ar ur $(NETLIB) $(LOBJS) ; ranlib $(NETLIB)



clean:
	rm -f core make.out *.o *.BAK *JNL netm ${LIBS}
