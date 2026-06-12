#
#	This is a makefile for alpi
#       of the graphic package for :
#       " Supervisione e Controllo di un Acceleratore Lineare Per Ioni "
#       

CC = cc
 CFLAGS =\
 -g -I/usr/include/mit -I/usr/include/X11 -I/usr/include/X11/Xaw
#CFLAGS = \
#-Olimit 1024 -O2 -I/usr/include -I/usr/include/X11 -I/usr/include/X11/Xaw

LDFLAGS =  -lpthread -lXaw -lXt -lXmu -lXext -lX11 -lm -L/usr/X11R6/lib

SRCS = \
main.c\
init_com.c\
main_box_ini.c\
main_box_arr.c\
text_onwidget.c\
warn_window.c\
rect_onwidget.c\
condis_window.c\
dialog_window.c\
st_act.c\
st_callbacks.c\
knob_unit_thread.c\
box_test.c

LSRCS = \
callbacks.c\
timeout.c\
fill_box.c\
mafill_2q3q.c\
alarm.c

OBJS = \
main.o\
init_com.o\
main_box_ini.o\
main_box_arr.o\
text_onwidget.o\
warn_window.o\
rect_onwidget.o\
condis_window.o\
dialog_window.o\
st_act.o\
st_callbacks.o\
ma_page.o\
callbacks.o\
box_test.o\
knob_unit_thread.o

LOBJS = timeout.o fill_box.o mafill_2q3q.o alarm.o 

MALIB = mapage_lib.a

LIBS = $(MALIB)

LIST = alpi.list

all: alpi ${LIBS}

$(MALIB): $(LOBJS)
	ar ur $(MALIB) $(LOBJS) ; ranlib $(MALIB)

alpi: ${OBJS} ${LIBS}
	cc  -g ${OBJS} ${LIBS} $(LDFLAGS) -o $@ ;

main.o: alpi.bit but.bit barbg10x40.bit alpi.h alpi_vars.h
init_com.o: alpi.h
main_box_ini.o: alpi.h
main_box_arr.o: alpi.h
condis_window.o: condis.bit
dialog_window.o: dialog.bit
warn_window.o: warn.bit
text_onwidget.o:
rect_onwidget.o:

ma_page.o: alpi.h ma.bit ma_vars.h
callbacks.o: ma_vars.h alpi.h
timeout.o:  ma_vars.h alpi.h
#

clean:
	rm -f core make.out *.o *.BAK *JNL alpi $(LIST) ${LIBS}

