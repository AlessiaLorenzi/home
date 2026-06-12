
/* alpi global vars + top page widgets */

/* graphic global vars */

Display *display ;
Window   window ;
int def_screen ;
int verbose = FALSE ;

/* network global vars */

int server_channel ;
char to_server[BUF_SIZE], from_server[BUF_SIZE] ;

/* top page widgets */

Widget main_window, 
       main_shell_window, main_box,
       main_lab, main_warn_lab,
       ma_but, exit_but , button_box, wid_array[100];

/* colors and pixmaps */

int    pixel_white, pixel_black, pixel_grey, pixel_coral,
       pixel_wheat, pixel_orange, pixel_goldenrod, pixel_red,
       pixel_green, pixel_blue, pixel_yellow,
	pixel_lightsteelblue3, pixel_brown;

Pixmap but_pixmap_off, but_pixmap_on,
       barbg10x40_pix, ma_pixmap, s8000_pixmap ;

