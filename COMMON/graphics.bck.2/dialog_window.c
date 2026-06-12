/**********************************************************************

usage: input_output[0][INOUT_LENGTH] is the label string,
       input_output[1][INOUT_LENGTH] is string to be read
it must be declared as follows:

extern char input_output[2][INOUT_LENGTH] ;

creat_input_window( input_output, action_type ) must be called by
the appropriate button callback and an appropriate action
must be added in the d_confirm_act callback

**********************************************************************/

#define INOUT_LENGTH 64

#include "alpi.h"
#include "dialog.bit"
#include <Dialog.h>
#include <Shell.h>

/* the two strings must be initialized before calling 
   creat_input_window */
char input_output[INOUT_LENGTH][INOUT_LENGTH] ;

Widget dialog_shell = NULL , dialog;

/* callbacks */




void d_confirm_act( w, action_type, call_data )
     Widget w ;
     int action_type ;
     caddr_t call_data ;   /* unused */
{
    extern void d_dismiss_act() ;
    extern int pixel_coral, pixel_wheat ;
    extern void rfread_data() ;
    extern int page_type ;
    extern int mag_page_refresh() ;
    char string[INOUT_LENGTH] ;

    Arg arg ;

    XtSetArg( arg, XtNbackground, pixel_coral ) ;
    XtSetValues( dialog , &arg, 1 ) ;


    sscanf( XawDialogGetValueString( dialog ), " %s", string ) ;
    switch( action_type ) {

        /* MA */

        case 9:
            printf("saving ma data in %s\n", string ) ;
            masave_data( string ) ; 
        break ;

        case 10:
            printf("reading ma data in %s\n", string ) ;
            maread_data( string ) ; 
            if ( page_type != -1 )
                mag_page_refresh();
        break ;

        case 11:
            printf("printing ma data\n") ;
            maprint_data( string ) ;
            printf("ok printed ma data\n") ;
        break ;

        default:
            printf( " action type: %d strig: %s\n",
                    action_type, string ) ;
            break ;
    }

printf("* * 9 * * \n");

    d_dismiss_act( w, action_type, NULL ) ;

printf("* * 10 * * \n");
}




void d_dismiss_act( w, action_type, call_data )
     Widget w ;
     int action_type ;
     caddr_t call_data ;   /* unused */
{
    extern Pixmap but_pixmap_off ;
    extern Widget dialog_shell ;
    extern Widget masave_but, maread_but, maprint_but ;

    Arg arg ;

printf("* * 11 * * \n");

    XtSetArg( arg, XtNbackgroundPixmap, but_pixmap_off ) ;
    switch( action_type ) {

        /* RF */
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        break ;

        /* MA */
        case 9:

            XtSetValues( masave_but, &arg, 1 ) ;
            break ;

        case 10:

            XtSetValues( maread_but, &arg, 1 ) ;
            break ;

        case 11:

            XtSetValues( maprint_but, &arg, 1 ) ;
            break ;

        default:

            break ;

    }
    if ( dialog_shell != NULL ) {
        XtDestroyWidget( dialog_shell ) ;
        dialog_shell = NULL ;
        }

printf("* * 12 * * \n");

}

/* end callbacks */

int creat_input_window ( input_output, action_type )
    char input_output[INOUT_LENGTH][INOUT_LENGTH] ;
    int action_type ;
{
    extern Display *display ;
    extern Window window ;
    extern int pixel_white, pixel_black ;

    Widget dialog_box, dialog_confirm, dialog_dismiss ;

    Arg args[5] ;

    Pixmap pixmap ;

    pixmap     = XCreatePixmapFromBitmapData( display, window ,
                     dialog_bits, dialog_width, dialog_height,
                     ( unsigned long ) pixel_black,
                     ( unsigned long ) pixel_white,
                     DefaultDepth( display,
                     XDefaultScreen( display ) ) ) ;

    if ( dialog_shell == NULL ) {
        
        XtSetArg( args[0], XtNiconName, "input" ) ;
        XtSetArg( args[1], XtNallowShellResize, TRUE ) ;
        XtSetArg( args[2], XtNiconPixmap, pixmap ) ;
        XtSetArg( args[3], XtNx, 500 ) ;
        XtSetArg( args[4], XtNy, 400 ) ;
    
        dialog_shell  = XtCreateApplicationShell( "dialog_shell",
                        applicationShellWidgetClass, args, 5 ) ;
    
        dialog_box    = XtCreateManagedWidget( "dialog_box",
                       boxWidgetClass, dialog_shell, NULL, 0 ) ;

        XtSetArg( args[0], XtNlabel, input_output[0] ) ;
        XtSetArg( args[1], XtNvalue, input_output[1] ) ;
        dialog = XtCreateManagedWidget( "dialog",
                 dialogWidgetClass, dialog_box, args, 2 ) ;

/*
printf("input_output[0] = %s\ninput_output[1] = %s \n", input_output[0], input_output[1]);


        XtSetArg( args[1], XtNlabel, "stringa 1" ) ;
        XtSetArg( args[1], XtNx, 900 ) ;
  



dialog = XtCreateManagedWidget( "dialog",
                 dialogWidgetClass, dialog_box, args, 2 ) ;*/


    
        dialog_confirm = XtCreateManagedWidget( "dialog_confirm",
                  commandWidgetClass, dialog, NULL, 0 ) ;
        XtAddCallback( dialog_confirm, XtNcallback,
                       d_confirm_act, action_type ) ;
    
        dialog_dismiss = XtCreateManagedWidget( "dialog_dismiss",
               commandWidgetClass, dialog, NULL, 0 ) ;
        XtAddCallback( dialog_dismiss, XtNcallback,
                       d_dismiss_act, action_type ) ;

        XtRealizeWidget( dialog_shell ) ;

        return( TRUE ) ;

        }
    else
        return( FALSE ) ;

}

