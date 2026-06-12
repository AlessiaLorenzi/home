
#include <Intrinsic.h>

/* each char is 7x14 */
/*#define FONTNAME "terminal_bold_narrow18"*/
#define FONTNAME "-adobe-times-medium-i-normal--24-240-75-75-p-125-iso8859-1"

/***********************************************************************

text_onwidget( w, x, y, s, slength, color )
Widget w ;
int x, y, slengtho, color ;
char *s ;

          - draw a rectangle on a widget

the following global vars are assumed to be inialized:
Display *display ;

***********************************************************************/


/* draw a text on a widget */

void text_onwidget( w, x, y, s, slength, color )
    Widget w ;
    int x, y, slength, color ;
    char *s ;
{
    extern Display *display ;
    Window window ;
    GContext gc ;
    XGCValues xgcv ;
    XFontStruct *fontstruct ;

    XFlush( display ) ;

    if ( ( fontstruct = XLoadQueryFont( display, FONTNAME ) ) ==
         NULL ) {
        perror("cannot opent font\n") ;
        return ;
    }

    window = XtWindow( w ) ;

    gc = (GContext)XtGetGC( w, 0, &xgcv);

    XSetFont( display, gc, fontstruct->fid ) ;
    XSetForeground( display, gc, color ) ;
    XDrawString( display, window, gc, x, y, s, slength ) ;
    XFlush( display ) ;
}

