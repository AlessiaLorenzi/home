
#include "alpi.h"

/***********************************************************************

main_box_arr() 

          - arrange main menu widgets

***********************************************************************/

    main_box_arr() 
{
    extern Widget main_shell_window, main_box,
                  main_lab, main_warn_lab,
                  ma_but, exit_but ;
    extern Pixmap but_pixmap_on, but_pixmap_off ;

    Arg arg ;

    /* setup button pixmaps */

    XtSetArg( arg, XtNbackgroundPixmap, but_pixmap_off ) ;
    XtSetValues( ma_but, &arg, 1 ) ;
    XtSetValues( exit_but, &arg, 1 ) ;

    XtManageChild( main_lab ) ;

    XtSetArg( arg, XtNfromHoriz, main_lab ) ;
    XtSetValues( main_warn_lab, &arg, 1 ) ;
    XtManageChild( main_warn_lab ) ;

    /* arrange geometry layout */

    XtSetArg( arg, XtNfromVert, main_lab ) ;
    XtSetValues( ma_but, &arg, 1 ) ;
    XtSetValues( exit_but, &arg, 1 ) ;

    XtManageChild( ma_but ) ;

    XtSetArg( arg, XtNfromHoriz, ma_but ) ;
    XtSetValues( exit_but, &arg, 1 ) ;
    XtManageChild( exit_but ) ;

    XtManageChild( main_box ) ;

}
