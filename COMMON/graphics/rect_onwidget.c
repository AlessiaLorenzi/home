
#include <Intrinsic.h>

/***********************************************************************

rect_onwidget( w, x, y, width, height, color )
Widget w ;
int x, y, width, height, color ;

          - draw a rectangle on a widget

the following global vars are assumed to be inialized:
Display *display ;

***********************************************************************/

void rect_onwidget( w, x, y, width, height, color )
    Widget w ;
    int x, y, width, height, color ;
{
    extern Display *display ;
    Window window ;
    GContext gc ;
    XGCValues xgcv ;

    XFlush( display ) ;
    window = XtWindow( w ) ;

    gc = (GContext)XtGetGC( w, 0, &xgcv);

    xgcv.function = GXcopy ;
    xgcv.foreground = color ;
    xgcv.plane_mask = 65535 ;
    xgcv.fill_style = FillSolid ;
    xgcv.graphics_exposures = True ;

    XChangeGC( display, gc,
               GCForeground | GCFunction | GCPlaneMask | GCFillStyle |
               GCGraphicsExposures, &xgcv);

    XFillRectangle( display, window, gc, x, y, width, height ) ;
    XFlush( display ) ;
   
}
