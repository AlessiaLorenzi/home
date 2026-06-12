
#include "alpi.h"
#include "alpi_vars.h"

#include "ma_const.h"
#include "alpi.bit"
#include "but.bit"
#include "barbg10x40.bit"


#include "ma_vars.h"

#include <Shell.h>

#define MAIN_WIDTH  514
#define MAIN_HEIGHT 144

#define WHITE         "white"
#define BLACK         "black"
#define GREY          "grey"
#define CORAL         "darkorange"
#define WHEAT         "cornsilk"
#define ORANGE        "orange"
#define GOLDENROD     "goldenrod"
#define RED           "red"
#define GREEN         "limegreen"
#define BLUE          "skyblue"
#define YELLOW        "yellow"
#define LIGHTSTEELBLUE3        "LightSteelBlue3"

/***********************************************************************

alpi main:

          - open the communication channel to a multiplexer,
          - make up al things of general interest: colors, pixmaps ...
          - create main menu and wait for events

***********************************************************************/

int main( argc, argv )
    unsigned int argc ;
    char **argv ;
{
    extern Display *display ;
    extern Window window ;
    extern int def_screen ;
    extern int verbose ;

    extern Widget main_window, main_shell_widget ;

    extern int pixel_white, pixel_black,
               pixel_grey, pixel_coral,
               pixel_wheat, pixel_orange,
               pixel_goldenrod, pixel_red,
               pixel_green, pixel_blue, pixel_yellow,
		pixel_lightsteelblue3;

    extern Pixmap but_pixmap_off, but_pixmap_on,
                  barbg10x40_pix ;

    Pixmap alpi_pixmap ;

    Colormap colormap ;
    XColor exact_color, screen_color ;
    Status status ;

    Arg args[9] ;

    /* start communication */

    if ( argc < 3 ) {
        printf( "\n usage: alpi server_host server_socket\n" ) ;
        exit(1) ;
        }
    init_com( argv[1], argv[2] ) ;

    if ( argc == 4 && strcmp( argv[3], "verbose" ) == 0 )
        verbose = TRUE ;
    else
        verbose = FALSE ;

    /* initialize and set a shell widget */

    main_window = XtInitialize( "main_window", "main_window",
                                 NULL, 0, &argc, argv ) ;

    /* initialize global graphic vars */

    display    = XtDisplay( main_window ) ;
    window     = XDefaultRootWindow( display ) ;
    def_screen = XDefaultScreen( display ) ;

    /* initialize internal colors */

    colormap   = XDefaultColormap( display, def_screen ) ;

    status = XAllocNamedColor( display, colormap, WHITE,
                               &screen_color, &exact_color ) ;
    pixel_white = status ? screen_color.pixel :
                  WhitePixel( display, def_screen ) ;

    status = XAllocNamedColor( display, colormap, BLACK,
                               &screen_color, &exact_color ) ;
    pixel_black = status ? screen_color.pixel :
                  BlackPixel( display, def_screen ) ;

    status = XAllocNamedColor( display, colormap, GREY,
                               &screen_color, &exact_color ) ;
    pixel_grey  = status ? screen_color.pixel :
                  WhitePixel( display, def_screen ) ;

    status = XAllocNamedColor( display, colormap, CORAL,
                               &screen_color, &exact_color ) ;
    pixel_coral = status ? screen_color.pixel :
                  WhitePixel( display, def_screen ) ;

    status = XAllocNamedColor( display, colormap, WHEAT,
                               &screen_color, &exact_color ) ;
    pixel_wheat = status ? screen_color.pixel :
                  WhitePixel( display, def_screen ) ;

    status = XAllocNamedColor( display, colormap, ORANGE,
                               &screen_color, &exact_color ) ;
    pixel_orange = status ? screen_color.pixel :
                  WhitePixel( display, def_screen ) ;

    status = XAllocNamedColor( display, colormap, GOLDENROD,
                               &screen_color, &exact_color ) ;
    pixel_goldenrod = status ? screen_color.pixel :
                  WhitePixel( display, def_screen ) ;

    status = XAllocNamedColor( display, colormap, RED,
                               &screen_color, &exact_color ) ;
    pixel_red = status ? screen_color.pixel :
                  WhitePixel( display, def_screen ) ;

    status = XAllocNamedColor( display, colormap, GREEN,
                               &screen_color, &exact_color ) ;
    pixel_green = status ? screen_color.pixel :
                  WhitePixel( display, def_screen ) ;

    status = XAllocNamedColor( display, colormap, BLUE,
                               &screen_color, &exact_color ) ;
    pixel_blue  = status ? screen_color.pixel :
                  WhitePixel( display, def_screen ) ;

    status = XAllocNamedColor( display, colormap, YELLOW,
                               &screen_color, &exact_color ) ;
    pixel_yellow  = status ? screen_color.pixel :
                  WhitePixel( display, def_screen ) ;

    status = XAllocNamedColor( display, colormap, LIGHTSTEELBLUE3,
                               &screen_color, &exact_color ) ;
    pixel_lightsteelblue3  = status ? screen_color.pixel :
                  WhitePixel( display, def_screen ) ;

    status = XAllocNamedColor( display, colormap, "saddlebrown",
                               &screen_color, &exact_color ) ;
    pixel_brown  = status ? screen_color.pixel :
                  WhitePixel( display, def_screen ) ;

    /* initialize internal pixmaps */

    but_pixmap_off   = XCreatePixmapFromBitmapData( display, window ,
                     but_bits, but_width, but_height,
                     ( unsigned long ) pixel_black,
                     ( unsigned long ) pixel_wheat,
                     DefaultDepth( display,
                     def_screen ) ) ;

    but_pixmap_on    = XCreatePixmapFromBitmapData( display, window ,
                     but_bits, but_width, but_height,
                     ( unsigned long ) pixel_black,
                     ( unsigned long ) pixel_coral,
                     DefaultDepth( display,
                     def_screen ) ) ;

    barbg10x40_pix  = XCreatePixmapFromBitmapData( display, window ,
                     barbg10x40_bits,
                     barbg10x40_width, barbg10x40_height,
                     ( unsigned long ) pixel_black,
                     ( unsigned long ) pixel_white,
                     DefaultDepth( display,
                     def_screen ) ) ;

    alpi_pixmap     = XCreatePixmapFromBitmapData( display, window ,
                     alpi_bits, alpi_width, alpi_height,
                     ( unsigned long ) pixel_black,
                     ( unsigned long ) pixel_white,
                     DefaultDepth( display,
                     def_screen ) ) ;

    /* initialize and set an application shell widget */

    /* not resizable page */
    XtSetArg( args[0], XtNminWidth,  MAIN_WIDTH ) ;
    XtSetArg( args[1], XtNminHeight, MAIN_HEIGHT ) ;
    XtSetArg( args[2], XtNmaxWidth,  MAIN_WIDTH ) ;
    XtSetArg( args[3], XtNmaxHeight, MAIN_HEIGHT ) ;
    XtSetArg( args[4], XtNiconName, "alpi" ) ;
    XtSetArg( args[5], XtNallowShellResize, TRUE ) ;
    XtSetArg( args[6], XtNwidth,  MAIN_WIDTH ) ;
    XtSetArg( args[7], XtNheight, MAIN_HEIGHT ) ;
    XtSetArg( args[8], XtNiconPixmap, alpi_pixmap ) ;
    
    main_shell_window = XtCreateApplicationShell( "main_shell_window",
                        topLevelShellWidgetClass, args, 9 ) ;

    /* resizable page
    XtSetArg( args[0], XtNiconName, "alpi" ) ;
    XtSetArg( args[1], XtNallowShellResize, TRUE ) ;
    XtSetArg( args[2], XtNiconPixmap, alpi_pixmap ) ;
    
    main_shell_window = XtCreateApplicationShell( "main_shell_window",
                        topLevelShellWidgetClass, args, 3 ) ; */

    /* set and initialize the main menu */

    main_box_ini() ;

    /* now everything was prepared, do it! */

    XtManageChild( main_box ) ;

    XtRealizeWidget( main_shell_window ) ;

    /* wait for events */

    XtMainLoop() ;

}
