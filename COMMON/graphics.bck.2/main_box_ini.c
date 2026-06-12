
#include "alpi.h"

/***********************************************************************

main_box_ini() 

          - callbacks of active widgets
          - make up the main menu box and its widgets
          - arrange it

***********************************************************************/

/* callbacks */

/* Magnets machine window open-close button callback */

void ma_act( ma_but, tag, ma_dat )
    Widget ma_but ;
    caddr_t tag ; /* not used */
    caddr_t ma_dat ; /* not used */
{
    extern Widget exit_but ;
    extern Pixmap but_pixmap_off, but_pixmap_on ;
    static status = OFF ;
    Arg arg ;
    extern XtIntervalId  alarminterval_id;

    if ( status == OFF ) {
        XtSetArg( arg, XtNbackgroundPixmap, but_pixmap_on ) ;
        XtSetValues( ma_but, &arg, 1 ) ;
        XtSetArg( arg, XtNsensitive, FALSE ) ;
        XtSetValues( exit_but, &arg, 1 ) ;
        open_mapage() ;
        status = ON ;
        }
    else {
        /* switch off display */

        if ( alarminterval_id != NULL )
            XtRemoveTimeOut(alarminterval_id);
        XtSetArg( arg, XtNbackgroundPixmap, but_pixmap_off ) ;
        XtSetValues( ma_but, &arg, 1 ) ;
        status = OFF ;
        XtSetArg( arg, XtNsensitive, TRUE ) ;
        XtSetValues( exit_but, &arg, 1 ) ;
        close_mapage() ;
        }
} ;

/* exit (with confirm) button callback */

void exit_act( exit_but, tag, exit_dat )
    Widget exit_but ;
    caddr_t tag ; /* not used */
    caddr_t exit_dat ; /* not used */
{
    extern Widget ma_but ;
    extern Pixmap but_pixmap_off, but_pixmap_on ;

    static status = OFF ;
    Arg arg ;

    if ( status == OFF ) {
        XtSetArg( arg, XtNsensitive, FALSE ) ;
        XtSetValues( ma_but, &arg, 1 ) ;
        XtSetValues( exit_but, &arg, 1 ) ;
        XtSetArg( arg, XtNbackgroundPixmap, but_pixmap_on ) ;
        XtSetValues( exit_but, &arg, 1 ) ;
        status = ON ;
        create_confirm_window( 0 ) ;
        }
    else {
        XtSetArg( arg, XtNbackgroundPixmap, but_pixmap_off ) ;
        XtSetValues( exit_but, &arg, 1 ) ;
        status = OFF ;
        XtSetArg( arg, XtNsensitive, TRUE ) ;
        XtSetValues( ma_but, &arg, 1 ) ;
        XtSetValues( exit_but, &arg, 1 ) ;
        }
} ;

/* end callbacks */

/* init main menu box and widgets */

    main_box_ini() 
{

    extern Widget main_shell_window, main_box,
                  main_lab, main_warn_lab,
                  ma_but, exit_but ;

    /* set and initialize the main box label and button widgets */

    main_box = XtCreateWidget( "main_box", formWidgetClass,
                               main_shell_window, NULL, 0 ) ;

    main_lab = XtCreateWidget( "main_lab", labelWidgetClass,
                               main_box, NULL, 0 ) ;

    main_warn_lab = XtCreateWidget( "main_warn_lab", labelWidgetClass,
                                    main_box, NULL, 0 ) ;

    ma_but = XtCreateWidget( "ma_but", commandWidgetClass,
                             main_box, NULL, 0 ) ;
    XtAddCallback( ma_but, XtNcallback, ma_act, NULL ) ;

    exit_but = XtCreateWidget( "exit_but", commandWidgetClass,
                               main_box, NULL, 0 ) ;
    XtAddCallback( exit_but, XtNcallback, exit_act, NULL ) ;

    /* arrange all */

    main_box_arr() ;

}
