/**********************************************************************

usage: 

open_warn_window( message ) may be called everywhere

newmsg_warn_window( message ) may be called everywhere

close_warn_window() may be called everywhere

!suggested values for .Xdefaults file

alpi*warn_box.background                :goldenrod
alpi*warn_lab.background                :white
alpi*warn_lab.foreground                :red

**********************************************************************/

#include "warn.bit"

#include <stdio.h> 
#include <Intrinsic.h>
#include <StringDefs.h>
#include <Command.h>
#include <Box.h>
#include <Shell.h>

#define MSGLEN 25
#define SHELLWIDTH 300
#define SHELLHEIGHT 160
#define BORDERWIDTH 1
#define DEFDIST 10

#define UNSET -1
#define NO     0
#define YES    1

Widget warn_shell = NULL ;
Widget warn_lab ;

void open_warn_window ( message )
     char message[MSGLEN] ;
{
    extern Display *display ;
    extern Window window ;
    extern int def_screen ;
    extern int pixel_white, pixel_black ;

    extern Widget warn_shell, warn_lab ;
    Widget warn_box ;

    Arg args[9] ;

    int width, height ;
    Pixmap pixmap ;

    /* if not already opened now build the window */

    if ( warn_shell == NULL ) {

        pixmap     = XCreatePixmapFromBitmapData( display, window ,
                     warn_bits, warn_width, warn_height,
                     ( unsigned long ) pixel_black,
                     ( unsigned long ) pixel_white,
                     DefaultDepth( display,
                     def_screen ) ) ;

        XtSetArg( args[0], XtNminWidth,  SHELLWIDTH ) ;
        XtSetArg( args[1], XtNminHeight, SHELLHEIGHT ) ;
        XtSetArg( args[2], XtNmaxWidth,  SHELLWIDTH ) ;
        XtSetArg( args[3], XtNmaxHeight, SHELLHEIGHT ) ;
        XtSetArg( args[4], XtNwidth,  SHELLWIDTH ) ;
        XtSetArg( args[5], XtNheight, SHELLHEIGHT ) ;
        XtSetArg( args[6], XtNallowShellResize, TRUE ) ;
        XtSetArg( args[7], XtNiconPixmap, pixmap ) ;
        XtSetArg( args[8], XtNiconName, "warn" ) ;

        warn_shell  = XtCreateApplicationShell( "warn_shell",
                      applicationShellWidgetClass, args, 9 ) ;

        width  = XDisplayWidth ( XtDisplay( warn_shell ),
                     XDefaultScreen( XtDisplay( warn_shell ) ) ) ;
        height = XDisplayHeight( XtDisplay( warn_shell ),
                     XDefaultScreen( XtDisplay( warn_shell ) ) ) ;

        XtSetArg( args[0], XtNx, ( width - SHELLWIDTH ) / 2 ) ;
        XtSetArg( args[1], XtNy, ( height - SHELLHEIGHT ) / 2 ) ;
        XtSetValues( warn_shell, args, 2 ) ;

        XtSetArg( args[0], XtNhSpace, DEFDIST ) ;
        XtSetArg( args[1], XtNvSpace, DEFDIST ) ;
        XtSetArg( args[2], XtNborderWidth, BORDERWIDTH ) ;
        warn_box    = XtCreateWidget( "warn_box",
                 boxWidgetClass, warn_shell, args, 3 ) ;

        XtSetArg( args[0], XtNwidth,  278 ) ;
        XtSetArg( args[1], XtNheight, 138 ) ;
        XtSetArg( args[2], XtNborderWidth, BORDERWIDTH ) ;
        XtSetArg( args[3], XtNresize, FALSE ) ;
        XtSetArg( args[4], XtNlabel, "" ) ;
        warn_lab = XtCreateWidget( "warn_lab",
                  labelWidgetClass, warn_box, args, 5 ) ;

        XtManageChild( warn_box ) ;

        XtManageChild( warn_lab ) ;

        XtRealizeWidget( warn_shell ) ;

        }
}

void close_warn_window ()
{
    extern Widget warn_shell ;

    if ( warn_shell != NULL  ) {
        XtDestroyWidget( warn_shell ) ;
        warn_shell = NULL ;
        }
}

void newmsg_warn_window ( message )

     char message[MSGLEN] ;

{
    extern Display *display ;
    extern int pixel_white, pixel_red ;
    extern Widget warn_lab ;
    int length ;

    length = strlen( message ) ;
    if ( warn_shell != NULL  ) {

        rect_onwidget( warn_lab, 0, 0, 278, 138, pixel_white ) ;
        text_onwidget( warn_lab, ( 278 - 12 * length ) / 2, 60,
                       message, length, pixel_red ) ;
        XFlush( display ) ;

        }
}

