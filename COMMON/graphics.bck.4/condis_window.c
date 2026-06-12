/**********************************************************************

usage: 

creat_confirm_window( action_type ) must be called by
the appropriate button callback and an appropriate action
must be added in the confirm_act callback

!suggested values for .Xdefaults file
confirm*font                              :courier_bold12
confirm*borderColor                       :gold
confirm_box.background                    :white
confirm_box.foreground                    :black
confirm*confirm_but.background            :wheat
confirm*dismiss_but.background            :wheat

**********************************************************************/

#include "condis.bit"

#include <stdio.h> 
#include <Intrinsic.h>
#include <StringDefs.h>
#include <Command.h>
#include <Box.h>
#include <Shell.h>

#define SHELLWIDTH 275
#define SHELLHEIGHT 65
#define BORDERWIDTH 1
#define DEFDIST 10

#define UNSET -1
#define NO     0
#define YES    1
#define BUF_SIZE 1024
Widget confirm_shell ;

int condis_answer = UNSET ;

/* callbacks */

void yes_act( w, action_type, call_data )
     Widget w ;
     int action_type ;
     caddr_t call_data ;   /* unused */
{
    extern Widget confirm_shell,
                  exit_but ;
    extern condis_answer ;
    extern int server_channel;
    condis_answer = YES ;

    switch( action_type ) {

        case 0:
write( server_channel,"ma local: exit" , BUF_SIZE ) ;
	
	close (server_channel);
            exit(0) ;
            break ;

        default:

            printf( " action type: %d\n", action_type ) ;
            break ;
    }

    XtDestroyWidget( confirm_shell ) ;
}

void no_act( w, action_type, call_data )
     Widget w ;
     int action_type ;
     caddr_t call_data ;   /* unused */
{
    extern Widget confirm_shell,
                  exit_but ;
    extern condis_answer ;

    Arg arg ;

    condis_answer = NO ;

    switch( action_type ) {

        case 0:

            exit_act( exit_but, NULL, NULL ) ;
            break ;

        default:

            printf( " action type: %d\n", action_type ) ;
            break ;
    }

    XtDestroyWidget( confirm_shell ) ;
}

/* end callbacks */

void create_confirm_window ( action_type )
     int action_type;
{
    extern Display *display ;
    extern Window window ;
    extern Pixmap but_pixmap_off ;
    extern int pixel_black, pixel_white ;

    extern Widget confirm_shell ;
    Widget confirm_box, confirm_but, dismiss_but ;

    extern int condis_answer ;

    Arg args[10] ;

    int width, height ;
    Pixmap pixmap ;
    int def_screen ;

    def_screen = XDefaultScreen( display ) ;
    pixmap     = XCreatePixmapFromBitmapData( display, window ,
                 condis_bits, condis_width, condis_height,
                 ( unsigned long ) pixel_black,
                 ( unsigned long ) pixel_white,
                 DefaultDepth( display,
                 def_screen ) ) ;

    /* now build the window */

    XtSetArg( args[0], XtNminWidth,  SHELLWIDTH ) ;
    XtSetArg( args[1], XtNminHeight, SHELLHEIGHT ) ;
    XtSetArg( args[2], XtNmaxWidth,  SHELLWIDTH ) ;
    XtSetArg( args[3], XtNmaxHeight, SHELLHEIGHT ) ;
    XtSetArg( args[4], XtNwidth,  SHELLWIDTH ) ;
    XtSetArg( args[5], XtNheight, SHELLHEIGHT ) ;
    XtSetArg( args[6], XtNallowShellResize, TRUE ) ;
    XtSetArg( args[7], XtNiconPixmap, pixmap ) ;

    switch( action_type ) {

        case 0:
            XtSetArg( args[8], XtNiconName, "exit" ) ;
            break ;

        default:
            XtSetArg( args[7], XtNiconName, "unknown confirm" ) ;
            break ;
    }

    confirm_shell  = XtCreateApplicationShell( "confirm_shell",
                    applicationShellWidgetClass, args, 9 ) ;

    width  = XDisplayWidth ( XtDisplay( confirm_shell ),
                 XDefaultScreen( XtDisplay( confirm_shell ) ) ) ;
    height = XDisplayHeight( XtDisplay( confirm_shell ),
                 XDefaultScreen( XtDisplay( confirm_shell ) ) ) ;

    XtSetArg( args[0], XtNx, ( width - SHELLWIDTH ) / 2 ) ;
    XtSetArg( args[1], XtNy, ( height - SHELLHEIGHT ) / 2 ) ;
    XtSetValues( confirm_shell, args, 2 ) ;

    XtSetArg( args[0], XtNhSpace, DEFDIST ) ;
    XtSetArg( args[1], XtNvSpace, DEFDIST ) ;
    XtSetArg( args[2], XtNborderWidth, BORDERWIDTH ) ;
    confirm_box    = XtCreateManagedWidget( "confirm_box",
             boxWidgetClass, confirm_shell, args, 3 ) ;

    XtSetArg( args[0], XtNbackgroundPixmap, but_pixmap_off ) ;
    confirm_but = XtCreateManagedWidget( "confirm_but",
              commandWidgetClass, confirm_box, args, 1 ) ;
    XtAddCallback( confirm_but, XtNcallback, yes_act, action_type ) ;

    XtSetArg( args[0], XtNbackgroundPixmap, but_pixmap_off ) ;
    dismiss_but = XtCreateManagedWidget( "dismiss_but",
           commandWidgetClass, confirm_box, args, 1 ) ;
    XtAddCallback( dismiss_but, XtNcallback, no_act, action_type ) ;

    XtSetArg( args[0], XtNwidth,  120 ) ;
    XtSetArg( args[1], XtNheight, 40 ) ;
    XtSetArg( args[2], XtNborderWidth, BORDERWIDTH ) ;
    XtSetArg( args[3], XtNresize, FALSE ) ;
    XtSetArg( args[4], XtNlabel, "confirm" ) ;
    XtSetValues( confirm_but, args, 5 ) ;
    XtSetArg( args[4], XtNlabel, "cancel exit" ) ;
    XtSetValues( dismiss_but, args, 5 ) ;

    XtRealizeWidget( confirm_shell ) ;
    XSync( display, FALSE ) ;
    /* sleep(1) ; */
//    XGrabPointer( display, XtWindow( confirm_box ), TRUE,
//                  ~NoEventMask, GrabModeSync, GrabModeSync,
//                  XtWindow( confirm_box ), None, CurrentTime ) ;
    XAllowEvents( display, AsyncBoth, CurrentTime ) ;
    XSync( display, FALSE ) ;
}
