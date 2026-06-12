
#include "alpi.h"
#include "ma_const.h"
#include "ma_ext_vars.h"
#include "ma.bit"
#include "s8000.bit"
#include "bottsu"
#include "bottgiu"
#include <Shell.h>
#include "xpm.h"

#define MA_WIDTH      1022
#define MA_HEIGHT      516



/* callbacks */



void masave_act( masave_but, tag, masave_dat )
    Widget masave_but ;
    caddr_t tag ; /* not used */
    caddr_t masave_dat ; /* not used */
{

    extern Pixmap but_pixmap_on ;
    extern int creat_input_window() ;
    extern char input_output[2][64] ;

    Arg arg ;

    strcpy( input_output[0] , "write to file:" ) ;
    strcpy( input_output[1] , "MA_SETTINGS" ) ;

    if ( creat_input_window ( input_output, 9 ) ) {
        XtSetArg( arg, XtNbackgroundPixmap, but_pixmap_on ) ;
        XtSetValues( masave_but, &arg, 1 ) ;
        }
    else
        printf( "\nalpi: other input window open\n" ) ;

} ;



void maprint_act( maprint_but, tag, maprint_dat )
    Widget maprint_but ;
    caddr_t tag ;
    caddr_t maprint_dat ;
{

    extern Pixmap but_pixmap_on ;
    extern int creat_input_window() ;
    extern char input_output[2][64] ;

    Arg arg ;

    strcpy( input_output[0] , "print file:" ) ;
    strcpy( input_output[1] , "" ) ;

    if ( creat_input_window ( input_output, 11 ) ) {
        XtSetArg( arg, XtNbackgroundPixmap, but_pixmap_on ) ;
        XtSetValues( maprint_but, &arg, 1 ) ;
        }
    else
        printf( "\nalpi: other input window open\n" ) ;

} ;



void maread_act( maread_but, tag, maread_dat )
    Widget maread_but ;
    caddr_t tag ; /* not used */
    caddr_t maread_dat ; /* not used */
{

    extern Pixmap but_pixmap_on ;
    extern int creat_input_window() ;
    extern char input_output[2][64] ;

    Arg arg ;

    strcpy( input_output[0] , "read from file:" ) ;
    strcpy( input_output[1] , "MA_SETTINGS" ) ;

    if ( creat_input_window ( input_output, 10 ) ) {
        XtSetArg( arg, XtNbackgroundPixmap, but_pixmap_on ) ;
        XtSetValues( maread_but, &arg, 1 ) ;
        }
    else
        printf( "\nalpi: other input window open\n" ) ;

} ;



void maexit_act( maexit_but, tag, maexit_dat )
    Widget maexit_but ;
    caddr_t tag ; /* not used */
    caddr_t maexit_dat ; /* not used */
{
    extern Widget ma_but ;
    /* this widget is completely eq. to ma_but */
    ma_act( ma_but, NULL, NULL ) ;

} ;



/* STEFANIA: call back di maremall_but:
   porta tutti i p.s. in rem state */
void maremall_act( maremall_but, tag, maremall_dat )
    Widget maremall_but ;
    caddr_t tag ; /* not used */
    caddr_t maremall_dat ; /* not used */
{
extern int verbose ;
extern Widget main_warn_lab;
extern int pixel_white, pixel_black, pixel_red ;

extern int page_type ;
extern int mag_page_refresh() ;

int cnt ;

rect_onwidget( main_warn_lab, 0, 0, 180, 40, pixel_white ) ;
text_onwidget( main_warn_lab, 10, 27, " setting rem all p.s. ",22,
               pixel_red ) ;



for (cnt = 0 ; cnt<100 ; cnt++ )
    {
    if ( magnet[cnt].alim1 > 0 && magnet[cnt].alim1 < 100 )
        {
        /* STEFANIA il p.s. puo' anche essere LOCK,
           in tal caso va' messo UNLOCK */
        sprintf(to_server,"ma comm : ALIM = %2d !CMDSTATE",
                magnet[cnt].alim1);
        write(server_channel, to_server, BUF_SIZE ) ;
        read(server_channel, from_server, BUF_SIZE ) ;
        if ( strncmp(from_server,"STUB",4) == 0 )
             supply[magnet[cnt].alim1].on_off = STUB;
        /* printf("++++++++++++ stato alimentatore %d = %d\n",
           magnet[cnt].alim1,supply[magnet[cnt].alim1].on_off); */
        if ( strncmp(from_server,"REMOTE",6) != 0 )
            { /* non e' in remote, e' local oppure lock */
            if ( strncmp(from_server,"LOCK",4) == 0 ) 
                { /* unlock */
                sprintf(to_server,
                        "ma comm : ALIM = %2d !UNLOCK",
                        magnet[cnt].alim1);
                write(server_channel, to_server, BUF_SIZE ) ;
                read(server_channel, from_server, BUF_SIZE ) ;
                }
            /* ora dovrebbe essere local, comunque */
            sprintf(to_server,"ma comm : ALIM = %2d !REM",
                    magnet[cnt].alim1);
            write(server_channel, to_server, BUF_SIZE ) ;
            read(server_channel, from_server, BUF_SIZE ) ;
            /* rileggi lo stato */
            sprintf(to_server,"ma comm : ALIM = %2d !CMDSTATE",
                    magnet[cnt].alim1);
            write(server_channel, to_server, BUF_SIZE ) ;
            read(server_channel, from_server, BUF_SIZE ) ;
            if ( strncmp(from_server,"REMOTE",6) == 0 )
                supply[magnet[cnt].alim1].rem_loc = REMOTE;
            else
                if ( strncmp(from_server,"LOCAL",5) == 0 )
                    supply[magnet[cnt].alim1].rem_loc = LOCAL;
                else
                   supply[magnet[cnt].alim1].rem_loc = LOCK;
            }
        else 
           {
           supply[magnet[cnt].alim1].rem_loc = REMOTE;
           }
       }
   if ( magnet[cnt].alim2 > 0 && magnet[cnt].alim2 < 100 )
        {
        /* STEFANIA il p.s. puo' anche essere LOCK,
           in tal caso va' messo UNLOCK */
        sprintf(to_server,"ma comm : ALIM = %2d !CMDSTATE",
                magnet[cnt].alim2);
        write(server_channel, to_server, BUF_SIZE ) ;
        read(server_channel, from_server, BUF_SIZE ) ;
        if ( strncmp(from_server,"STUB",4) == 0 )
            supply[magnet[cnt].alim2].on_off = STUB;
        /* printf("++++++++++++ stato alimentatore %d = %d\n",
                  magnet[cnt].alim2,
                  supply[magnet[cnt].alim2].on_off); */
        if ( strncmp(from_server,"REMOTE",6) != 0 )
            { /* non e' in remote, e' local oppure lock */
            if ( strncmp(from_server,"LOCK",4) == 0 ) 
                { /* unlock */
                sprintf(to_server,
                        "ma comm : ALIM = %2d !UNLOCK",
                        magnet[cnt].alim2);
                write(server_channel, to_server, BUF_SIZE ) ;
                read(server_channel, from_server, BUF_SIZE ) ;
                }
            /* ora dovrebbe essere local, comunque */
            sprintf(to_server,"ma comm : ALIM = %2d !REM",
                    magnet[cnt].alim2);
            write(server_channel, to_server, BUF_SIZE ) ;
            read(server_channel, from_server, BUF_SIZE ) ;
            /* rileggi lo stato */
            sprintf(to_server,"ma comm : ALIM = %2d !CMDSTATE",
                    magnet[cnt].alim2);
            write(server_channel, to_server, BUF_SIZE ) ;
            read(server_channel, from_server, BUF_SIZE ) ;
            if ( strncmp(from_server,"REMOTE",6) == 0 )
                supply[magnet[cnt].alim2].rem_loc = REMOTE;
            else
                if ( strncmp(from_server,"LOCAL",5) == 0 )
                    supply[magnet[cnt].alim2].rem_loc = LOCAL;
                else
                    supply[magnet[cnt].alim2].rem_loc = LOCK;
            }
        else 
            {
            supply[magnet[cnt].alim2].rem_loc = REMOTE;
            }
        }
    }

rect_onwidget( main_warn_lab, 0, 0, 180, 40, pixel_white ) ;

if ( page_type != -1 )
    mag_page_refresh();

}


/* STEFANIA: call back di maresetall_but: reset di tutti i p.s. */
void maresetall_act( maresetall_but, tag, maresetall_dat )
    Widget maresetall_but ;
    caddr_t tag ; /* not used */
    caddr_t maresetall_dat ; /* not used */
{
extern int verbose ;
extern Widget main_warn_lab;
extern int pixel_white, pixel_black, pixel_red ;

int cnt ;

rect_onwidget( main_warn_lab, 0, 0, 180, 40, pixel_white ) ;
text_onwidget( main_warn_lab, 10, 27, "  resetting all p.s.  ",22,
               pixel_red ) ;

for (cnt = 0 ; cnt<100 ; cnt++ )
    {
    if ( magnet[cnt].alim1 > 0 && magnet[cnt].alim1 < 100 )
        {
	if  ( supply[magnet[cnt].alim1].on_off == ON )
		{
           	 sprintf ( to_server ,"ma comm : ALIM = %2d !F",
                      magnet[cnt].alim1);
            	write( server_channel, to_server, BUF_SIZE ) ;
            	read( server_channel, from_server, BUF_SIZE ) ;
            	supply[magnet[cnt].alim1].on_off = OFF;
		}
        sprintf ( to_server ,"ma comm : ALIM = %2d !RS",
                  magnet[cnt].alim1);
        write( server_channel, to_server, BUF_SIZE ) ;
        read( server_channel, from_server, BUF_SIZE ) ;
        }
    if ( magnet[cnt].alim2 > 0 && magnet[cnt].alim2 < 100 )
        {
	if  ( supply[magnet[cnt].alim2].on_off == ON )
		{
           	 sprintf ( to_server ,"ma comm : ALIM = %2d !F",
                      magnet[cnt].alim2);
            	write( server_channel, to_server, BUF_SIZE ) ;
            	read( server_channel, from_server, BUF_SIZE ) ;
            	supply[magnet[cnt].alim2].on_off = OFF;
		}
        sprintf ( to_server ,"ma comm : ALIM = %2d !RS",
                  magnet[cnt].alim2);
        write( server_channel, to_server, BUF_SIZE ) ;
        read( server_channel, from_server, BUF_SIZE ) ;
        }
    }

rect_onwidget( main_warn_lab, 0, 0, 180, 40, pixel_white ) ;

if ( page_type != -1 )
    mag_page_refresh();
}



void mawarning_act(mawarning_but, tag, mawarning_dat )
    Widget mawarning_but ;
    caddr_t tag ; /* not used */
    caddr_t mawarning_dat ; /* not used */
{
extern int verbose ;
extern Widget main_warn_lab;
extern int pixel_white, pixel_black, pixel_red, pixel_brown ;

extern int page_type ;
extern int mag_page_refresh() ;
Arg arg[8];


XtSetArg( arg[0], XtNbackground, pixel_brown );
XtSetArg( arg[1], XtNlabel, " " );
XtSetValues( mawarning_lab, &arg, 2 ) ;
XtSetValues( mawarning_but, &arg, 2 ) ;

}
/* STEFANIA: call back di mazecurall_but:
   azzera la corrente di tutti i p.s. */
void mazecurall_act( mazecurall_but, tag, mazecurall_dat )
    Widget mazecurall_but ;
    caddr_t tag ; /* not used */
    caddr_t mazecurall_dat ; /* not used */
{
extern int verbose ;
extern Widget main_warn_lab;
extern int pixel_white, pixel_black, pixel_red ;

extern int page_type ;
extern int mag_page_refresh() ;

int cnt, power_supply_number ;

rect_onwidget( main_warn_lab, 0, 0, 180, 40, pixel_white ) ;
text_onwidget( main_warn_lab, 10, 27, " zeroing all currents ",22,
               pixel_red ) ;

for (cnt = 0 ; cnt<100 ; cnt++ )
    {
    if ( magnet[cnt].alim1 > 0 && magnet[cnt].alim1 < 100 )
        {

        power_supply_number = magnet[cnt].alim1 ;
        supply[power_supply_number].wa_current = 0 ;
        supply[power_supply_number].current = 0.0 ;
        ma_current_cmd( power_supply_number, 0 ) ;

        }
    if ( magnet[cnt].alim2 > 0 && magnet[cnt].alim2 < 100 )
        {

        power_supply_number = magnet[cnt].alim2 ;
        supply[power_supply_number].wa_current = 0 ;
        supply[power_supply_number].current = 0.0 ;
        ma_current_cmd( power_supply_number, 0 ) ;

        }
    }

rect_onwidget( main_warn_lab, 0, 0, 180, 40, pixel_white ) ;

if ( page_type != -1 )
    mag_page_refresh();

}

/* STEFANIA2 (il ritorno) : cambiamenti su button_act per inserire i
   tipi 7 e 8 ;
   la versione originale della callback era troppo bizantina e l'ho semplificata
   con questa logica (la logica precedentemente implementatata):
   - se nessuna pagina era gia' aperta => aprila ("apri solo").
   - se una pagina era gia' aperta ed era proprio quella
        del bottone pigiato => chiudila ("chiudi solo").
   - se una pagina era gia' aperta ed non era quella
        del bottone pigiato => chiudi quella corrente
        e apri quella nuova ("chiudi e apri").
   (tutto cio' quasi indipendentemente dal tipo di pagina, visto che
    le cose da fare sono quasi sempre le stesse) */

/* callback associata a ciascun bottone del sinottico */
int button_act ( widget, tag, button_dat )
Widget widget;
caddr_t tag ; /* numero bot. che attiva la cbk, quando viene eseguita */
caddr_t button_dat ; /* non usato */
{
extern int verbose ;
extern Widget ma_but;
Arg args[9] ;

numero_bottone = (int)tag;

if ( verbose )
    printf ("page_type = %d numero_bottone = %d\
    button[numero_bottone].type = %d bottone_premuto = %d \n",
    page_type, numero_bottone, button[numero_bottone].type,
    bottone_premuto);

/* tipo -1: nulla
         1: 1 alimentatore (es. dipolo ID1)
         2: 2 alimentatori con steerer
         3: 3 alimentatori
         4: 2 alimentatori senza steerer
       5,6: non usate */

if  ( page_type == -1 )

    /* se nessuna pag. era aperta precedentemente => "apri solo" */
    {

    /* disabilita l'uscita dal sinottico dei magneti */
    XtSetArg( args[0], XtNsensitive,  FALSE ) ;
    XtSetValues( ma_but, args, 1 ) ;
    XtSetValues( maexit_but, args, 1 ) ;

    switch ( button[numero_bottone].type ) /* apri la pagina */
        {
        case 1:
        crea_pag_1();
        XtRealizeWidget(ma_comm_shell);
        page_type = 1;
        bottone_premuto = numero_bottone ;
        refresh_onoff_act(1) ;
        mag_page_refresh();
        break ;

        case 2:
        crea_pag_2();
        XtRealizeWidget(ma_comm_shell);
        page_type = 2;
        bottone_premuto = numero_bottone ;
        refresh_onoff_act(1) ;
        mag_page_refresh();
	printf("ma_page.c: chiamo ma2q3q_refresh 1")	;
        ma2q3q_refresh() ;
        break ;

        case 4:
        crea_pag_4();
        XtRealizeWidget(ma_comm_shell);
        page_type = 4;
        bottone_premuto = numero_bottone ;
        refresh_onoff_act(1) ;
        mag_page_refresh();
	printf("ma_page.c: chiamo ma2q3q_refresh 2")	;
        ma2q3q_refresh() ;
        break ;

        case 3:
        crea_pag_3();
        XtRealizeWidget(ma_comm_shell);
        bottone_premuto = numero_bottone ;
        page_type = button[numero_bottone].type ; /* 3, 5 o 6 */
        refresh_onoff_act(1) ;
        mag_page_refresh();
        break ;

        case 7:
        crea_pag_7(); /* STEFANIA2 (il ritorno)
        XtRealizeWidget e' interno */
        page_type = 7;
        bottone_premuto = numero_bottone ;
        break ;

        case 8:
        crea_pag_8(); /* STEFANIA2 (il ritorno) XtRealizeWidget e' interno */
        page_type = 8;
        bottone_premuto = numero_bottone ;
        break ;

        default:
        printf( "tipo non previsto!\n" ) ;
        page_type = 7 ;
        bottone_premuto = numero_bottone ;
        break ;

        } /* end sw. sul tipo di pagina da aprire */

    } /* end caso "apri solo" */

else 

    /* c'era gia' una pagina aperta (di tipo 1,2,3,4,5,6 ...  )
       qui ci sono due possibilita':
       - pulsante ri-premuto "chiudi solo"
       - altro pulsante premuto "chiudi ed apri" */

    {

    /* intanto spegni il refresh e chiudi la finestra gia' aperta */
    if ( page_type != 7 && page_type != 8 )
        refresh_onoff_act(0) ;
    /* chiudi la finestra */
    XtDestroyWidget ( ma_comm_shell );

    /* azioni speciali, che dipendono dal tipo di pag. gia' aperta */
    switch ( page_type )
        {

        case 1: 
        case 3:
        case 4:
        case 7:
        case 8:
        /* nessuna azione speciale */
        break;

        case 2: 
        /* chiudere anche pagina steerer (se aperta) */
        if ( pagina_steerers == 1 )
            {
            pagina_steerers = 0 ;
	    st_refresh_onoff_act(0);
            XtDestroyWidget (steerer_shell);
            }
        break;

        } /* end sw. azioni speciali */

    if ( bottone_premuto == numero_bottone ) /* caso "chiudi solo" */

        /* memorizza "nessuna finestra aperta" e ripristina */
        {
        page_type = -1;
        XtSetArg( args[0], XtNsensitive,  TRUE ) ;
        XtSetValues( ma_but, args, 1 ) ;
        XtSetValues( maexit_but, args, 1 ) ;
        } /* end if (pulsante ri-premuto), "chiudi solo" */

    else  /* bottone_premuto != numero_bottone, "chiudi ed apri" */

        {

        /* la vecchia era gia' stata chiusa, apri la finestra nuova */

        switch ( button[numero_bottone].type ) /* crea la pagina */
            {
            case 1:
            crea_pag_1();
            XtRealizeWidget(ma_comm_shell);
            page_type = 1;
            bottone_premuto = numero_bottone ;
            refresh_onoff_act(1) ;
            mag_page_refresh();
            break ;

            case 2:
            crea_pag_2();
            XtRealizeWidget(ma_comm_shell);
            page_type = 2;
            bottone_premuto = numero_bottone ;
            refresh_onoff_act(1) ;
            mag_page_refresh();
	printf("ma_page.c: chiamo ma2q3q_refresh 3")	;
            ma2q3q_refresh() ;
            break ;

            case 4:
            crea_pag_4();
            XtRealizeWidget(ma_comm_shell);
            page_type = 4;
            bottone_premuto = numero_bottone ;
            refresh_onoff_act(1) ;
            mag_page_refresh();
	printf("ma_page.c: chiamo ma2q3q_refresh 4")	;
            ma2q3q_refresh() ;
            break ;

            case 3:
            crea_pag_3();
            XtRealizeWidget(ma_comm_shell);
            bottone_premuto = numero_bottone ;
            page_type = button[numero_bottone].type ; /* 3, 5 o 6 */
            refresh_onoff_act(1) ;
            mag_page_refresh();
            break ;

            case 7:
            crea_pag_7(); /* STEFANIA2 (il ritorno) : vedi sopra */
            bottone_premuto = numero_bottone ;
            page_type = 7;
            break ;

            case 8:
            crea_pag_8(); /* STEFANIA2 (il ritorno) : vedi sopra */
            bottone_premuto = numero_bottone ;
            page_type = 8;
            break ;

            default:
            printf( "tipo non previsto!\n" ) ;
            bottone_premuto = numero_bottone ;
            break ;

            } /* end sw. sul tipo di pagina da aprire */

        } /* end-else caso "chiudi e apri" */

    } /* end-else => casi "chiudi solo" oppure "chiudi e apri" */
        
}

/* end callbacks */

/***********************************************************************

open_mapage()

          - open ma top page
          - make up command box widgets
          - make up cryostats box widgets 
          - start updating ma top page

***********************************************************************/


leggi_steerer(al)
int al;
{
int nl;
float ci, cl, vl;
char s1[20], s2[20] ,s3[20];
extern         agg_etich();

        sprintf ( to_server ,"st comm : ALIM = %2d #", al);
        if ( verbose )
                    printf ("ALPI send: %s\n", to_server);
        write( server_channel, to_server, BUF_SIZE ) ;
        read( server_channel, from_server, BUF_SIZE ) ;

        if ( strncmp ( from_server, "STUB", 4 ) != 0 )
                {
                sscanf (from_server+2, "%s %f %s %f %s %f",
                                        s1, &ci, s2, &cl, s3, &vl);

                /*printf ("alim. n. %d, valore impostato = %f, valore letto = %f, valore vero = %f, v = %f \n",al,  steerer_supply[al].current, ci, cl, vl);*/
                steerer_supply[al].current = cl;

                }

        if (strncmp(from_server, "STUB", 4) == 0)
                steerer_supply[al].mode = 0 ;

}



void leggi_stati()
{
extern int verbose ;
extern Widget main_warn_lab, maalarm_lab ;
extern int pixel_white, pixel_black,
pixel_grey, pixel_coral, pixel_red ;
Arg args[10];
char string[100];

int cnt, corrente_letta;

rect_onwidget( main_warn_lab, 0, 0, 180, 40, pixel_white ) ;
text_onwidget( main_warn_lab, 10, 27, "reading data from MPSs",22,
               pixel_red ) ;


printf("\n\n\n\n\n\n");
for (cnt = 0 ; cnt<100 ; cnt++ ) /*questo e' un loop sul numero dei magneti*/
        {
        if ( magnet[cnt].alim1 > 0 && magnet[cnt].alim1 < 100 )
            {
            strcpy(string, "reading ");
            strcat(string, magnet[cnt].n_alim1);
            strcat(string, "      ");

            rect_onwidget( main_warn_lab, 0, 0, 270, 40, pixel_white ) ;
            text_onwidget( main_warn_lab, 20, 27, string, 15, pixel_red ) ;


            sprintf(to_server,"ma comm : ALIM = %2d !S1", magnet[cnt].alim1);
            write(server_channel, to_server, BUF_SIZE ) ;
            read(server_channel, from_server, BUF_SIZE ) ;
            rect_onwidget( main_warn_lab, 0, 0, 270, 40, pixel_white ) ;


//            strcpy(ma_suppl_status[magnet[cnt].alim1],from_server);
            if ( from_server[0] == '.' )
                supply[magnet[cnt].alim1].on_off = ON;
            if ( from_server[0] == '!' )
                supply[magnet[cnt].alim1].on_off = OFF;
            if ( from_server[0] == 'S' )
                {
                supply[magnet[cnt].alim1].on_off = STUB;
                if ( strncmp (from_server, "STUB alim", 9) == 0 )
                    printf ("[01;31ml'alimentatore di %s non risponde[0m\n",magnet[cnt].n_alim1);
                else
                    printf ("[01;35ml'alimentatore di %s e' stato disattivato[0m\n",magnet[cnt].n_alim1);
                }

            sprintf(to_server,"ma comm : ALIM = %2d !RA", magnet[cnt].alim1);
            write(server_channel, to_server, BUF_SIZE ) ;
            read(server_channel, from_server, BUF_SIZE ) ;
            sscanf(from_server,"%d",&corrente_letta);
            if ( corrente_letta > 999999 )
                corrente_letta = 999999 ;
            supply[magnet[cnt].alim1].current = 
                ((float)corrente_letta/1000000.0)*
                supply[magnet[cnt].alim1].maxcurrent;
                /* STEFANIA: inizializz. di ra_current e wa_current */
            supply[magnet[cnt].alim1].ra_current = corrente_letta ;
            supply[magnet[cnt].alim1].wa_current = corrente_letta ;

            if ( verbose )
                printf("alim = %d current = %d\n",magnet[cnt].alim1, corrente_letta);

            /* STEFANIA il p.s. puo' anche essere LOCK,
               in tal caso va' messo UNLOCK */
            sprintf(to_server,"ma comm : ALIM = %2d !CMDSTATE",
                    magnet[cnt].alim1);
            write(server_channel, to_server, BUF_SIZE ) ;
            read(server_channel, from_server, BUF_SIZE ) ;
            if ( strncmp(from_server,"LOCK",4) == 0 )
                {
                sprintf(to_server,"ma comm : ALIM = %2d !UNLOCK",
                        magnet[cnt].alim1);
                write(server_channel, to_server, BUF_SIZE ) ;
                read(server_channel, from_server, BUF_SIZE ) ;
                supply[magnet[cnt].alim1].rem_loc = LOCAL;
                }
            else 
                {
                if ( strncmp(from_server,"LOCAL",5) == 0 ) 
                    supply[magnet[cnt].alim1].rem_loc = LOCAL;
                if ( strncmp(from_server,"REMOTE",6) == 0 ) 
                    supply[magnet[cnt].alim1].rem_loc = REMOTE;
                }
            }
        if ( magnet[cnt].alim2 > 0 && magnet[cnt].alim2 < 100 )
            {
            strcpy(string, "reading ");
            strcat(string, magnet[cnt].n_alim2);
            strcat(string, "      ");
            rect_onwidget( main_warn_lab, 0, 0, 270, 40, pixel_white ) ;
            text_onwidget( main_warn_lab, 20, 27, string, 15, pixel_red ) ;
            sprintf(to_server,"ma comm : ALIM = %2d !S1", magnet[cnt].alim2);
            write(server_channel, to_server, BUF_SIZE ) ;
            read(server_channel, from_server, BUF_SIZE ) ;
            rect_onwidget( main_warn_lab, 0, 0, 270, 40, pixel_white ) ;
//            strcpy(ma_suppl_status[magnet[cnt].alim2],from_server);
            if ( from_server[0] == '.' )
                supply[magnet[cnt].alim2].on_off = ON;
            if ( from_server[0] == '!' )
                supply[magnet[cnt].alim2].on_off = OFF;
            if ( from_server[0] == 'S' )
                {
                supply[magnet[cnt].alim2].on_off = STUB;
                if ( strncmp (from_server, "STUB alim", 9) == 0 )
                    printf ("[01;31ml'alimentatore di %s non risponde[0m\n",magnet[cnt].n_alim2);
                else
                    printf ("[01;35ml'alimentatore di %s e' stato disattivato[0m\n",magnet[cnt].n_alim2);
                }
                sprintf(to_server,"ma comm : ALIM = %2d !RA",
                        magnet[cnt].alim2);
                write(server_channel, to_server, BUF_SIZE ) ;
                read(server_channel, from_server, BUF_SIZE ) ;
                sscanf(from_server,"%d",&corrente_letta);
                if ( corrente_letta > 999999 )
                    corrente_letta = 999999 ;
                supply[magnet[cnt].alim2].current = 
                ((float)corrente_letta/1000000.0)*
                supply[magnet[cnt].alim2].maxcurrent;
                /* STEFANIA: inizializz. di ra_current e wa_current */
                supply[magnet[cnt].alim2].ra_current = corrente_letta ;
                supply[magnet[cnt].alim2].wa_current = corrente_letta ;

if ( verbose ) printf("alim = %d current = %d\n",magnet[cnt].alim2, corrente_letta);

                /* STEFANIA il p.s. puo' anche essere LOCK,
                   in tal caso va' messo UNLOCK */
                sprintf(to_server,"ma comm : ALIM = %2d !CMDSTATE",
                        magnet[cnt].alim2);
                write(server_channel, to_server, BUF_SIZE ) ;
                read(server_channel, from_server, BUF_SIZE ) ;
                if ( strncmp(from_server,"LOCK",4) == 0 )
                    {
                    sprintf(to_server,"ma comm : ALIM = %2d !UNLOCK",
                            magnet[cnt].alim2);
                    write(server_channel, to_server, BUF_SIZE ) ;
                    read(server_channel, from_server, BUF_SIZE ) ;
                    supply[magnet[cnt].alim2].rem_loc = LOCAL;
                    }
                else 
                    {
                    if ( strncmp(from_server,"LOCAL",5) == 0 ) 
                        supply[magnet[cnt].alim2].rem_loc = LOCAL;
                    if ( strncmp(from_server,"REMOTE",6) == 0 ) 
                        supply[magnet[cnt].alim2].rem_loc = REMOTE;
                    }
                }
        }


//rect_onwidget( main_warn_lab, 0, 0, 180, 40, pixel_white ) ;
//text_onwidget( main_warn_lab, 10, 27, "reading data from steerers",22, pixel_red ) ;
for ( cnt = 1; cnt < numero_bottoni + 1 ; cnt++ ) /* SOSTITUITO */
    {
        if ( button[cnt].type == 2 )
            {
            if ( button[cnt].obj1 != 9999 &&
                 button[cnt].obj1 != 999999 )
                {
                strcpy(string, "reading ");
                strcat(string, steerer[button[cnt].obj1].name);
                strcat(string, "      ");

                rect_onwidget( main_warn_lab, 0, 0, 270, 40, pixel_white ) ;
                text_onwidget( main_warn_lab, 20, 27, string,
                               18, pixel_red ) ;

                leggi_steerer(steerer[button[cnt].obj1].alim1);
                leggi_steerer(steerer[button[cnt].obj1].alim2);
                rect_onwidget( main_warn_lab, 0, 0, 270, 40, pixel_white ) ;
                }
    
            if ( button[cnt].obj3 != 9999 &&
                 button[cnt].obj3 != 999999 )
                { 
                strcpy(string, "reading ");
               strcat(string, steerer[button[cnt].obj3].name);
                strcat(string, "      ");

                rect_onwidget( main_warn_lab, 0, 0, 270, 40, pixel_white ) ;
                text_onwidget( main_warn_lab, 20, 27, string, 18, pixel_red ) ;

                leggi_steerer(steerer[button[cnt].obj3].alim1);
                leggi_steerer(steerer[button[cnt].obj3].alim2);
                rect_onwidget( main_warn_lab, 0, 0, 270, 40, pixel_white ) ;
                }


            }

    } /* end for */
} /* end leggi_stati */



void open_mapage()
{
    extern Display *display ;
    extern Window window ;
    extern int def_screen ;

    extern Widget main_warn_lab, ma_shell_window,
                  mashell_box, maprint_but,
                  macom_box, ma_lab, maexit_but, masave_but, maread_but,
                  button_box, wid_array[100], maprint_but;
    extern Pixmap but_pixmap_on, but_pixmap_off, ma_pixmap, s8000_pixmap ;
    extern int pixel_white, pixel_black,
               pixel_grey, pixel_coral, pixel_red ;

    extern int verbose ;
    extern int server_channel ;
    extern char to_server[BUF_SIZE], from_server[BUF_SIZE] ;
    extern maalarm_act();

    Arg arg, args[20] ;
    int counter;
    extern Pixmap alarmon_pixmap, alarmoff_pixmap ;
    char button_label[100][100];
    Pixmap *shapemask_return;
    XpmAttributes attributes;
 

    numero_bottone = -1;
    page_type = -1;
    bottone_premuto = -1;



    ma_pixmap = XCreatePixmapFromBitmapData( display, window ,
                ma_bits, ma_width, ma_height,
                ( unsigned long ) pixel_black,
                ( unsigned long ) pixel_white,
                DefaultDepth( display, def_screen ) ) ;

    s8000_pixmap = XCreatePixmapFromBitmapData( display, window ,
                s8000_bits, s8000_width, s8000_height,
                ( unsigned long ) pixel_black,
                ( unsigned long ) pixel_white,
                DefaultDepth( display, def_screen ) ) ;

    /* initialize and set a shell widget for the ma page */

    /* not resizable page
    XtSetArg( args[0], XtNminWidth,  RF_WIDTH ) ;
    XtSetArg( args[1], XtNminHeight, RF_HEIGHT ) ;
    XtSetArg( args[2], XtNmaxWidth,  RF_WIDTH ) ;
    XtSetArg( args[3], XtNmaxHeight, RF_HEIGHT ) ;
    XtSetArg( args[4], XtNiconName, "magnets" ) ;
    XtSetArg( args[5], XtNallowShellResize, TRUE ) ;
    XtSetArg( args[6], XtNwidth,  RF_WIDTH ) ;
    XtSetArg( args[7], XtNheight, RF_HEIGHT ) ;
    XtSetArg( args[8], XtNiconPixmap, ma_pixmap ) ;

    ma_shell_window = XtCreateApplicationShell( "ma_shell_window",
                      applicationShellWidgetClass, args, 9 ) ;
    */

    /* resizable page */
    XtSetArg( args[0], XtNiconName, "magnets" ) ;
    XtSetArg( args[1], XtNallowShellResize, TRUE ) ;
    XtSetArg( args[2], XtNiconPixmap, ma_pixmap ) ;


    ma_shell_window = XtCreateApplicationShell( "ma_shell_window",
                      applicationShellWidgetClass, args, 3 ) ;
    
    mashell_box = XtCreateWidget( "mashell_box", boxWidgetClass,
                                  ma_shell_window, NULL, 0 ) ;

    /* set initialize the ma command box widget */

    macom_box = XtCreateWidget( "macom_box", formWidgetClass,
                                mashell_box, NULL, 0 ) ;



    XtSetArg( args[0], XtNy, 760 ) ;
    XtSetValues( ma_shell_window, args, 1 ) ;

    /* STEFANIA : nuovi widget della ma_page  =>
       maalarm_lab (indica l'ultimo mag. controllato)
       maremall_but (mette in rem tutti i p.s.)
       maresetall_but (reset di tutti p.s.)
       mazecurall_but (azzera tutte le correnti) */

    ma_lab = XtCreateWidget( "ma_lab", labelWidgetClass,
                             macom_box, NULL, 0 ) ;

    maalarm_lab = XtCreateWidget( "maalarm_lab", labelWidgetClass,
                             macom_box, NULL, 0 ) ;

    maexit_but = XtCreateWidget( "maexit_but", commandWidgetClass,
                                 macom_box, NULL, 0 ) ;
    XtAddCallback( maexit_but, XtNcallback, maexit_act, NULL ) ;

    masave_but = XtCreateWidget( "masave_but", commandWidgetClass,
                                 macom_box, NULL, 0 ) ;
    XtAddCallback( masave_but, XtNcallback, masave_act, NULL ) ;

    maprint_but = XtCreateWidget( "maprint_but", commandWidgetClass,
                                 macom_box, NULL, 0 ) ;
    XtAddCallback( maprint_but, XtNcallback, maprint_act, NULL ) ;

    maread_but = XtCreateWidget( "maread_but", commandWidgetClass,
                                 macom_box, NULL, 0 ) ;
    XtAddCallback( maread_but, XtNcallback, maread_act, NULL ) ;

    maalarm_but = XtCreateWidget( "maalarm_but", commandWidgetClass,
                                 macom_box, NULL, 0 ) ;
    XtAddCallback( maalarm_but, XtNcallback, maalarm_act, NULL ) ;

    maremall_but = XtCreateWidget( "maremall_but", commandWidgetClass,
                                 macom_box, NULL, 0 ) ;
    XtAddCallback( maremall_but, XtNcallback, maremall_act, NULL ) ;

    maresetall_but = XtCreateWidget( "maresetall_but",
                                     commandWidgetClass,
                                     macom_box, NULL, 0 ) ;
    XtAddCallback( maresetall_but, XtNcallback, maresetall_act, NULL ) ;

    mazecurall_but = XtCreateWidget( "mazecurall_but",
                                     commandWidgetClass,
                                     macom_box, NULL, 0 ) ;
    XtAddCallback( mazecurall_but, XtNcallback, mazecurall_act, NULL ) ;

    mawarning_lab = XtCreateWidget( "mawarning_lab", labelWidgetClass,
                             macom_box, NULL, 0 ) ;

    mawarning_but = XtCreateWidget( "mawarning_but",
                                     commandWidgetClass,
                                     macom_box, NULL, 0 ) ;
    XtAddCallback( mawarning_but, XtNcallback, mawarning_act, NULL ) ;

    /* arrange ma page command widgets */

    XtSetArg( arg, XtNbackgroundPixmap, but_pixmap_off ) ;
    XtSetValues( maexit_but, &arg, 1 ) ;
    XtSetValues( masave_but, &arg, 1 ) ;
    XtSetValues( maread_but, &arg, 1 ) ;
    XtSetValues( maalarm_but, &arg, 1 ) ;
    XtSetValues( maremall_but, &arg, 1 ) ;
    XtSetValues( maresetall_but, &arg, 1 ) ;
    XtSetValues( mazecurall_but, &arg, 1 ) ;
    XtSetValues( maprint_but, &arg, 1 ) ;

    XtManageChild( ma_lab ) ;

    /* STEFANIA: tutti i pulsanti sono sotto le due label */
    XtSetArg( arg, XtNfromVert, ma_lab ) ;
    XtSetValues( maexit_but, &arg, 1 ) ;
    XtSetValues( masave_but, &arg, 1 ) ;
    XtSetValues( maread_but, &arg, 1 ) ;
    XtSetValues( maalarm_but, &arg, 1 ) ;
    XtSetValues( maremall_but, &arg, 1 ) ;
    XtSetValues( maresetall_but, &arg, 1 ) ;
    XtSetValues( mazecurall_but, &arg, 1 ) ;

    XtSetArg( arg, XtNfromHoriz, ma_lab ) ;
    XtSetValues( maalarm_lab, &arg, 1 ) ;
    XtManageChild( maalarm_lab ) ;

    XtSetArg( arg, XtNfromHoriz, maalarm_lab ) ;
    XtSetValues( maprint_but, &arg, 1 ) ;
    XtManageChild( maprint_but ) ;

    XtManageChild( masave_but ) ;

    XtSetArg( arg, XtNfromHoriz, masave_but ) ;
    XtSetValues( maread_but, &arg, 1 ) ;
    XtManageChild( maread_but ) ;

    XtSetArg( arg, XtNfromHoriz, maread_but ) ;
    XtSetValues( maremall_but, &arg, 1 ) ;
    XtManageChild( maremall_but ) ;

    XtSetArg( arg, XtNfromHoriz, maremall_but ) ;
    XtSetValues( mazecurall_but, &arg, 1 ) ;
    XtManageChild( mazecurall_but ) ;

    XtSetArg( arg, XtNfromHoriz, mazecurall_but ) ;
    XtSetValues( maresetall_but, &arg, 1 ) ;
    XtManageChild( maresetall_but ) ;

    XtSetArg( arg, XtNfromHoriz, maresetall_but ) ;
    XtSetValues( maalarm_but, &arg, 1 ) ;
    XtManageChild( maalarm_but ) ;

    XtSetArg( arg, XtNfromHoriz, maalarm_but ) ;
    XtSetValues( maexit_but, &arg, 1 ) ;
    XtManageChild( maexit_but ) ;

    XtSetArg( arg, XtNfromHoriz, maprint_but ) ;
    XtSetValues( mawarning_lab, &arg, 1 ) ;
    XtManageChild( mawarning_lab ) ;

    XtSetArg( arg, XtNfromHoriz, mawarning_lab ) ;
    XtSetValues( mawarning_but, &arg, 1 ) ;
    XtManageChild( mawarning_but ) ;

    XtManageChild( macom_box ) ;

    /* open communication channels */

    strcpy( to_server, "ma local: open channels" ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;

    if ( verbose ) printf( "\nalpi:%s\n", from_server ) ;

    /* filling data structures */

    strcpy( to_server, "ma local: send data" ) ;
    write( server_channel, to_server, BUF_SIZE ) ;

    for (counter=0 ; counter < 100 ; counter++)
        {
        button[counter].button = 9999 ;
        button[counter].type = 9999 ;
        button[counter].obj1 = 9999 ;
        button[counter].obj2 = 9999 ;
        button[counter].obj3 = 9999 ;
        button[counter].obj4 = 9999 ;
        }


    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf(from_server,"%d", &numero_bottoni);
    write( server_channel, "record ricevuto", BUF_SIZE ) ;

    if ( verbose )
        printf("magnetik: numero bottoni %d\n", numero_bottoni);

    for (counter=1 ; counter < numero_bottoni+1 ; counter++)
        {
        read( server_channel, from_server, BUF_SIZE ) ;
        sscanf(from_server,"%d %d %d %d %d %d",
        &(button[counter].button),
        &(button[counter].type),
        &(button[counter].obj1),
        &(button[counter].obj2),
        &(button[counter].obj3),
        &(button[counter].obj4));
        write( server_channel, "record ricevuto", BUF_SIZE ) ;
        }

    for (counter=1 ; counter < 100 ; counter++)
        {
        strcpy( magnet[counter].n_alim1, "NULL");
        strcpy( magnet[counter].n_alim2, "NULL");
        read( server_channel, from_server, BUF_SIZE ) ;
        sscanf( from_server,
                "%d %s %d %s %d %d %d %d %s %d %d %d",

                &(magnet[counter].magnet),
                magnet[counter].name,

                &(magnet[counter].alim1),
                magnet[counter].n_alim1,
                &(magnet[counter].a_alim1),
                &(magnet[counter].cur_alim1),
                &(magnet[counter].volt_alim1),

                &(magnet[counter].alim2),
                magnet[counter].n_alim2,
                &(magnet[counter].a_alim2),
                &(magnet[counter].cur_alim2),
                &(magnet[counter].volt_alim2));


        write( server_channel, "record ricevuto", BUF_SIZE ) ;

        }



    for (counter=0 ; counter < 100 ; counter++)
        {
        read( server_channel, from_server, BUF_SIZE ) ;
        write( server_channel, "record ricevuto", BUF_SIZE ) ;
        sscanf(from_server, "%d", &(supply[counter].mode));
        /*printf("  ++  ++  %d %d\n",counter, supply[counter].mode);*/
        }



    strcpy( to_server, "st local: send data" ) ;
    write( server_channel, to_server, BUF_SIZE ) ;



    for (counter=1 ; counter < 100 ; counter++)
        {
        steerer[counter].steerer = 0 ;
        }



    for (counter=1 ; counter < 100 ; counter++)
        {
        read( server_channel, from_server, BUF_SIZE ) ;
        sscanf(from_server,"%s %d %d %d",
        steerer[counter].name,
        &(steerer[counter].steerer),
        &(steerer[counter].alim1),
        &(steerer[counter].alim2));
                

        write( server_channel, "record ricevuto", BUF_SIZE ) ;


        /*printf("   ^^^^^^^^^^^^^^^   %s %d %d %d\n",
                  steerer[counter].name,
                  steerer[counter].steerer,
                  steerer[counter].alim1,
                  steerer[counter].alim2);*/
 

        }



    for (counter=0 ; counter < 100 ; counter++)
        {
        read( server_channel, from_server, BUF_SIZE ) ;
        write( server_channel, "record ricevuto", BUF_SIZE ) ;
        sscanf(from_server, "%d", &(steerer_supply[counter].mode));
        /* printf("  -- --  %d %d\n",
                  counter, steerer_supply[counter].mode); */
        }



    for ( counter = 0 ; counter < 100 ; counter++)
        {
        if ( magnet[counter].alim1 < 100 && magnet[counter].alim1 >= 0)
            {
            supply[magnet[counter].alim1].maxcurrent =
            magnet[counter].cur_alim1;
            supply[magnet[counter].alim1].maxvoltage =
            magnet[counter].volt_alim1;
            if ( verbose )
                printf ( "alim = %d maxcurr = %d maxvolt = %d\n",
                magnet[counter].alim1,
                supply[magnet[counter].alim1].maxcurrent, 
                supply[magnet[counter].alim1].maxvoltage);
            }

        if ( magnet[counter].alim2 < 100 && magnet[counter].alim2 >= 0)
                {
                supply[magnet[counter].alim2].maxcurrent =
                    magnet[counter].cur_alim2;
                supply[magnet[counter].alim2].maxvoltage =
                    magnet[counter].volt_alim2;
                if ( verbose )
                    printf ( "alim = %d maxcurr = %d maxvolt = %d\n",
                    magnet[counter].alim2,
                    supply[magnet[counter].alim2].maxcurrent, 
                    supply[magnet[counter].alim2].maxvoltage);
                }
        }


    if ( verbose )
        printf( "\nalpi  **** :%s\n", from_server ) ;
    rect_onwidget( main_warn_lab, 0, 0, 180, 40, pixel_white ) ;
    text_onwidget( main_warn_lab, 20, 27, "ma configuration ...",
                   20, pixel_red ) ;



    button_box = XtCreateWidget( "button_box", boxWidgetClass,
                                  mashell_box, NULL, 0 ) ;

    XtSetArg( args[0], XtNwidth,  1280 ) ;
    XtSetArg( args[1], XtNheight, 600 ) ;
    XtSetArg( args[2], XtNresize, FALSE);
    XtSetValues(button_box , args, 3 ) ;


    for (counter=0 ; counter < 100 ; counter++)
        {
        if ( button[counter].button != 9999 &&
             button[counter].button != 999999 )
            {
            sprintf ( button_label[counter],"button_%d",
                      counter);

            wid_array[counter] =
            XtCreateWidget(button_label[counter],
            commandWidgetClass, button_box, NULL, 0);
            XtSetArg( args[0], XtNlabel,
                      magnet[counter].name ) ;
            XtSetValues(wid_array[counter] , args, 1 ) ;

            XtAddCallback ( wid_array[counter],
                            XtNcallback, button_act,
                            counter);
            XtManageChild ( wid_array[counter] );
            }
        }


    XtManageChild( button_box ) ;



    /* now everything was prepared, do it! */

    XtManageChild( mashell_box ) ;

    XtRealizeWidget( ma_shell_window ) ;


    for (counter=0 ; counter < 100 ; counter++)
            {
            supply[counter].on_off = OFF;
            supply[counter].rem_loc = REMOTE;
            }

    leggi_stati() ;

    maalarm_act() ;

}

/***********************************************************************

close_mapage()

          - close ma top page

***********************************************************************/

void close_mapage()
{
    extern Widget ma_shell_window ;

    extern int server_channel ;
    extern char to_server[BUF_SIZE], from_server[BUF_SIZE] ;

    Arg arg ;

    /* close communication channels */


    if (ma_alarms_onoff == ON)
        maalarm_act() ;
    write( server_channel,"ma local: close channels" , BUF_SIZE ) ;
    if ( verbose )
        printf ("chiusura canale \n");
    XtDestroyWidget( ma_shell_window ) ;

}

