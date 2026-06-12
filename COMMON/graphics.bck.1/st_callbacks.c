#include "alpi.h"
#include "ma_const.h"
#include "ma_ext_vars.h"
#include "ma.bit"
#include "bottsu"
#include "bottgiu"
#include <Shell.h>
#include <Grip.h>

#define SCROLLTRANS "\
<BtnDown>: StartScroll(Forward) MoveThumb() NotifyScroll(Proportional)\n\
<BtnMotion>: MoveThumb() NotifyScroll(Proportional)\n\
<BtnUp>: MoveThumb() NotifyThumb() EndScroll()"

#define GRIP_TRANS "\
<EnterWindow>: GripAction(enter)\n\
<LeaveWindow>: GripAction(leave)\n\
<BtnDown>:     GripAction(press)\n\
<BtnUp>:       GripAction(release)\n\
<Key>Right:    GripAction(right)\n\
<Key>Left:     GripAction(left)\n\
<Key>Up:       GripAction(up)\n\
<Key>Down:     GripAction(down)"

#define CURRENT_LIMIT 3.5

extern int pixel_orange, pixel_grey ;
Arg args[9];

char string[100];
unsigned long valuemask;
XSetWindowAttributes attributes ;


XWindowAttributes attributi_finestra;
GContext gc;
XGCValues xgcv;
XGCValues val_gc;



st_refresh_onoff_act(status)
int status;
{
extern int pixel_orange, pixel_grey ;
extern st_timeout_proc();

Arg arg ;

if ( status == 0)
    {
    st_refresh_onoff = 0;
    if ( st_interval_id != NULL )
        XtRemoveTimeOut(st_interval_id);
    st_interval_id = NULL ;
    }
if ( status == 1)
    {
    st_refresh_onoff = 1;
    st_interval_id = XtAddTimeOut ( 250, st_timeout_proc, NULL);
    }
}



/* STEFANIA2 (il ritorno) : modificato per pag. di tipo 7 e 8 */
st_page_act()
{
extern crea_steerer_page(), st_refresh_a(), st_refresh_b();

if ( pagina_steerers == 0 )
    /* la pagina e' chiusa */
    {
    crea_steerer_page( 0 );
    st_refresh_a();
    st_refresh_b();
    pagina_steerers = 1 ;
    XtSetArg( args[0], XtNbackground, pixel_orange ) ;
    XtSetValues(st_page_but , args, 1 ) ;
st_refresh_onoff_act(1);
    }

else
    /* la pagina e' aperta */
    {
    st_refresh_onoff_act(0);
    XtDestroyWidget (steerer_shell);
    pagina_steerers = 0 ;
    XtSetArg( args[0], XtNbackground, pixel_grey ) ;
    XtSetValues(st_page_but , args, 1 ) ;
    }

}



st_fill_box(
numero_steerer,
steerer_box,
st_name_lab,
st_onoff_but,
st_display_lab,
st_incdec_1, st_incdec_2,
st_real_1, st_real_2,
st_grip_1, st_grip_2,
st_curdwn_fast_but_1,st_curdwn_fast_but_2, 
st_curdwn_but_1,st_curdwn_but_2, 
st_cur_ruler_1,st_cur_ruler_2,
st_curup_but_1, st_curup_but_2,
st_curup_fast_but_1, st_curup_fast_but_2)
        

int numero_steerer;
Widget steerer_box,
*st_name_lab,
*st_onoff_but,
*st_display_lab,
*st_incdec_1, *st_incdec_2,
*st_real_1, *st_real_2,
*st_grip_1, *st_grip_2,
*st_curdwn_fast_but_1, *st_curdwn_fast_but_2,
*st_curdwn_but_1, *st_curdwn_but_2,
*st_cur_ruler_1, *st_cur_ruler_2,
*st_curup_but_1, *st_curup_but_2,
*st_curup_fast_but_1, *st_curup_fast_but_2;

{

        extern st_curdwn_fast_act(), st_curdwn_act(),
               st_jump_act(), st_scroll_act(),
               st_curup_act(), 
               st_curup_fast_act(), st_onoff_act(),
               st_remloc_act(), st_reset_act(),
               st_grip_act() ;



*st_display_lab = XtCreateWidget ( "st_display_lab",
               labelWidgetClass, steerer_box, NULL, 0 ) ;

XtManageChild(*st_display_lab);



*st_name_lab = XtCreateWidget ( "st_name_lab",
               labelWidgetClass, steerer_box, NULL, 0 ) ;

XtSetArg( args[0], XtNlabel, steerer[numero_steerer].name ) ;
XtSetArg(args[1], XtNfromHoriz, *st_display_lab);
XtSetValues(*st_name_lab , args, 2 ) ;


XtManageChild(*st_name_lab);



*st_incdec_1 = XtCreateWidget ( "st_incdec_1",
                    labelWidgetClass, steerer_box, NULL, 0 ) ;
XtSetArg(args[0], XtNfromVert, *st_name_lab);
XtSetArg(args[1], XtNfromHoriz, *st_display_lab);
XtSetValues(*st_incdec_1 , args, 2 ) ;
XtManageChild(*st_incdec_1);


*st_real_1 = XtCreateWidget ( "st_real_1",
                    labelWidgetClass, steerer_box, NULL, 0 ) ;
XtSetArg(args[0], XtNfromVert, *st_name_lab);
XtSetArg(args[1], XtNfromHoriz, *st_incdec_1);
XtSetValues(*st_real_1 , args, 2 ) ;
XtManageChild(*st_real_1);



*st_grip_1 = XtCreateWidget ( "st_grip_1",
                    gripWidgetClass, steerer_box, NULL, 0 ) ;
XtSetArg(args[0], XtNfromVert, *st_incdec_1);
XtSetArg(args[1], XtNfromHoriz, *st_display_lab);
XtSetValues(*st_grip_1 , args, 2 ) ;
XtManageChild(*st_grip_1);



XtOverrideTranslations( *st_grip_1, XtParseTranslationTable(GRIP_TRANS));
XtAddCallback(  *st_grip_1, XtNcallback, st_grip_act, steerer[numero_steerer].alim1 ) ;




/***************          BOTTONE        ************************/
*st_curdwn_fast_but_1 = XtCreateWidget( "st_curdwn_fast_but_1",
                                      commandWidgetClass,
                                      steerer_box,NULL, 0);

XtAddCallback(*st_curdwn_fast_but_1 , XtNcallback,
              st_curdwn_fast_act, steerer[numero_steerer].alim1 ) ;

XtSetArg(args[0], XtNfromVert, *st_incdec_1);
XtSetArg(args[1], XtNfromHoriz, *st_grip_1);
XtSetValues(*st_curdwn_fast_but_1, args, 2);
XtManageChild(*st_curdwn_fast_but_1);



/***************          BOTTONE        ************************/
*st_curdwn_but_1 = XtCreateWidget("st_curdwn_but_1", commandWidgetClass,
                 steerer_box,NULL, 0);

XtAddCallback( *st_curdwn_but_1 , XtNcallback,
               st_curdwn_act, steerer[numero_steerer].alim1 ) ;

XtSetArg(args[0], XtNfromVert, *st_incdec_1);
XtSetArg(args[1], XtNfromHoriz, *st_curdwn_fast_but_1);
XtSetArg(args[2], XtNbitmap,"bottsu");
XtSetValues(*st_curdwn_but_1, args, 2);
XtManageChild(*st_curdwn_but_1);


/*****************          BARRA           ************************/

XtSetArg(args[0], XtNlength,500);
XtSetArg(args[1], XtNthickness,30);
XtSetArg(args[2], XtNorientation,XtorientHorizontal);

*st_cur_ruler_1 = XtCreateWidget ( "st_cur_ruler_1", scrollbarWidgetClass,
                                 steerer_box, args, 3);

XtOverrideTranslations( *st_cur_ruler_1,
                        XtParseTranslationTable(SCROLLTRANS));

steerer_supply[steerer[numero_steerer].alim1].x_oppure_y = 1 ;



XtAddCallback( *st_cur_ruler_1 , XtNjumpProc,
               st_jump_act, steerer[numero_steerer].alim1  ) ;


XtAddCallback( *st_cur_ruler_1, XtNscrollProc,
               st_scroll_act,steerer[numero_steerer].alim1 ) ;

XtSetArg(args[0], XtNfromHoriz, *st_curdwn_but_1);
XtSetArg(args[1], XtNfromVert, *st_incdec_1);
XtSetValues(*st_cur_ruler_1, args,2);
XtManageChild(*st_cur_ruler_1);



/*****************          BOTTONE        ************************/
*st_curup_but_1 = XtCreateWidget("st_curup_but_1",
                commandWidgetClass, steerer_box,NULL,0);

XtAddCallback( *st_curup_but_1 , XtNcallback,
               st_curup_act, steerer[numero_steerer].alim1 ) ;

XtSetArg(args[0], XtNfromVert, *st_incdec_1);
XtSetArg(args[1], XtNfromHoriz, *st_cur_ruler_1);
/*XtSetArg(args[2], XtNbitmap,bottgiu);*/
XtSetValues(*st_curup_but_1, args, 2);
XtManageChild(*st_curup_but_1);



/*****************          BOTTONE        ************************/
*st_curup_fast_but_1 = XtCreateWidget("st_curup_fast_but_1",
                     commandWidgetClass, steerer_box,NULL,0);

XtAddCallback(*st_curup_fast_but_1 , XtNcallback,
              st_curup_fast_act, steerer[numero_steerer].alim1 ) ;

XtSetArg(args[0], XtNfromVert, *st_incdec_1);
XtSetArg(args[1], XtNfromHoriz, *st_curup_but_1);
XtSetValues(*st_curup_fast_but_1, args, 2);
XtManageChild(*st_curup_fast_but_1);



*st_incdec_2 = XtCreateWidget ( "st_incdec_2",
                    labelWidgetClass, steerer_box, NULL, 0 ) ;
XtSetArg(args[0], XtNfromVert, *st_grip_1);
XtSetArg(args[1], XtNfromHoriz, *st_display_lab);
XtSetValues(*st_incdec_2 , args, 2 ) ;
XtManageChild(*st_incdec_2);


*st_real_2 = XtCreateWidget ( "st_real_2",
                    labelWidgetClass, steerer_box, NULL, 0 ) ;
XtSetArg(args[0], XtNfromVert, *st_grip_1);
XtSetArg(args[1], XtNfromHoriz, *st_incdec_2);
XtSetValues(*st_real_2 , args, 2 ) ;
XtManageChild(*st_real_2);



*st_grip_2 = XtCreateWidget ( "st_grip_2",
                    gripWidgetClass, steerer_box, NULL, 0 ) ;
XtSetArg(args[0], XtNfromVert, *st_incdec_2);
XtSetArg(args[1], XtNfromHoriz, *st_display_lab);
XtSetValues(*st_grip_2 , args, 2 ) ;
XtManageChild(*st_grip_2);



XtOverrideTranslations( *st_grip_2, XtParseTranslationTable(GRIP_TRANS));
XtAddCallback(  *st_grip_2, XtNcallback, st_grip_act, steerer[numero_steerer].alim2 ) ;


/***************          BOTTONE        ************************/
*st_curdwn_fast_but_2 = XtCreateWidget( "st_curdwn_fast_but_2",
                                      commandWidgetClass,
                                      steerer_box,NULL, 0);

XtAddCallback(*st_curdwn_fast_but_2 , XtNcallback,
              st_curdwn_fast_act, steerer[numero_steerer].alim2 ) ;

XtSetArg(args[0], XtNfromVert, *st_incdec_2);
XtSetArg(args[1], XtNfromHoriz, *st_grip_2);
XtSetValues(*st_curdwn_fast_but_2, args, 2);
XtManageChild(*st_curdwn_fast_but_2);


/***************          BOTTONE        ************************/
*st_curdwn_but_2 = XtCreateWidget("st_curdwn_but_2", commandWidgetClass,
                 steerer_box,NULL, 0);

XtAddCallback( *st_curdwn_but_2 , XtNcallback,
               st_curdwn_act, steerer[numero_steerer].alim2 ) ;

XtSetArg(args[0], XtNfromVert, *st_incdec_2);
XtSetArg(args[1], XtNfromHoriz, *st_curdwn_fast_but_2);
XtSetArg(args[2], XtNbitmap,"bottsu");
XtSetValues(*st_curdwn_but_2, args, 2);
XtManageChild(*st_curdwn_but_2);


/*****************          BARRA           ************************/

XtSetArg(args[0], XtNlength,500);
XtSetArg(args[1], XtNthickness,30);
XtSetArg(args[2], XtNorientation,XtorientHorizontal);

*st_cur_ruler_2 = XtCreateWidget ( "st_cur_ruler_2", scrollbarWidgetClass,
                                 steerer_box, args, 3);

XtOverrideTranslations( *st_cur_ruler_2,
                        XtParseTranslationTable(SCROLLTRANS));


steerer_supply[steerer[numero_steerer].alim2].x_oppure_y = 2 ;



XtAddCallback( *st_cur_ruler_2 , XtNjumpProc,
               st_jump_act, steerer[numero_steerer].alim2 ) ;


XtAddCallback( *st_cur_ruler_2, XtNscrollProc,
               st_scroll_act,steerer[numero_steerer].alim2 ) ;

XtSetArg(args[0], XtNfromHoriz, *st_curdwn_but_2);
XtSetArg(args[1], XtNfromVert, *st_incdec_2);
XtSetValues(*st_cur_ruler_2, args,2);
XtManageChild(*st_cur_ruler_2);



/*****************          BOTTONE        ************************/
*st_curup_but_2 = XtCreateWidget("st_curup_but_2",
                commandWidgetClass, steerer_box,NULL,0);

XtAddCallback( *st_curup_but_2 , XtNcallback,
               st_curup_act, steerer[numero_steerer].alim2 ) ;

XtSetArg(args[0], XtNfromVert, *st_incdec_2);
XtSetArg(args[1], XtNfromHoriz, *st_cur_ruler_2);
/*XtSetArg(args[2], XtNbitmap,bottgiu);*/
XtSetValues(*st_curup_but_2, args, 2);
XtManageChild(*st_curup_but_2);



/*****************          BOTTONE        ************************/
*st_curup_fast_but_2 = XtCreateWidget("st_curup_fast_but_2",
                     commandWidgetClass, steerer_box,NULL,0);

XtAddCallback(*st_curup_fast_but_2 , XtNcallback,
              st_curup_fast_act, steerer[numero_steerer].alim2 ) ;

XtSetArg(args[0], XtNfromVert, *st_incdec_2);
XtSetArg(args[1], XtNfromHoriz, *st_curup_but_2);
XtSetValues(*st_curup_fast_but_2, args, 2);
XtManageChild(*st_curup_fast_but_2);

}



/* STEFANIA2 (il ritorno) : modificato per pag. di tipo 7 e 8 */
crea_steerer_page( flag )
int flag ; /* flag==0 => sottopagina steerer,
              flag==1 => pagina magneti, 2 steerer
              flag==2 => pagina magneti, 1 steerer */
{

Arg args[10] ;

XtSetArg( args[0], XtNiconName, "steerers" ) ;

if ( flag == 0 ) /* crea una nuova finestra */
    {
    steerer_shell = XtCreateApplicationShell ( "steerer_shell",
                                               applicationShellWidgetClass,
                                               args, 1);

    /* STEFANIA2, scatoletta!!!, ma andrea ... */
    scatoletta = XtCreateWidget( "scatoletta", formWidgetClass,
                                  steerer_shell, NULL, 0 ) ;
    }
else /* usa la finestra "magneti" */
    {
    scatoletta = XtCreateWidget( "scatoletta", formWidgetClass,
                                  ma_setup_box, NULL, 0 ) ;
    /* ma_setup_box e' la box X bottoniera in cima alla finestra */
    XtSetArg( args[0], XtNfromVert, ma_magexit_but );
    XtSetValues(scatoletta , args, 1 ) ;
    }

/* in ogni caso la scatola A va' fatta */
XtManageChild(scatoletta);
steerer_box_a = XtCreateWidget( "steerer_box_a", formWidgetClass,
                                scatoletta, NULL, 0 ) ;
XtManageChild(steerer_box_a);

steerer_supply[steerer[button[numero_bottone].obj1].alim1].finestra = 1;
steerer_supply[steerer[button[numero_bottone].obj1].alim2].finestra = 1;

/* la scatola B va' fatta solo nei casi 0 e 1 */
if ( flag == 0 || flag == 1 )
    {
    steerer_box_b = XtCreateWidget( "steerer_box_b", formWidgetClass,
                                    scatoletta, NULL, 0 ) ;
    XtSetArg( args[0], XtNfromVert, steerer_box_a  ) ;
    XtSetValues(steerer_box_b , args, 1 ) ;
    XtManageChild(steerer_box_b);
    }


/* in ogni caso la scatola A va' riempita */
st_fill_box(
button[numero_bottone].obj1,
steerer_box_a, 
&st_name_lab_a,
&st_onoff_but_a,
&st_display_lab_a,
&st_incdec_a_1, &st_incdec_a_2,
&st_real_a_1, &st_real_a_2,
&st_grip_a_1, &st_grip_a_2,
&st_curdwn_fast_but_a_1, &st_curdwn_fast_but_a_2,
&st_curdwn_but_a_1, &st_curdwn_but_a_2,
&st_cur_ruler_a_1, &st_cur_ruler_a_2,
&st_curup_but_a_1, &st_curup_but_a_2,
&st_curup_fast_but_a_1, &st_curup_fast_but_a_2);

/* la scatola B va' riempita solo nei casi 0 e 1 */
if ( flag == 0 || flag == 1 )
    {
    st_fill_box(
    button[numero_bottone].obj3,
    steerer_box_b,
    &st_name_lab_b,
    &st_onoff_but_b,
    &st_display_lab_b,
    &st_incdec_b_1, &st_incdec_b_2,
    &st_real_b_1, &st_real_b_2,
    &st_grip_b_1, &st_grip_b_2,
    &st_curdwn_fast_but_b_1,&st_curdwn_fast_but_b_2,
    &st_curdwn_but_b_1,&st_curdwn_but_b_2,
    &st_cur_ruler_b_1, &st_cur_ruler_b_2,
    &st_curup_but_b_1, &st_curup_but_b_2,
    &st_curup_fast_but_b_1, &st_curup_fast_but_b_2);

    steerer_supply[steerer[button[numero_bottone].obj3].alim1].finestra = 2;
    steerer_supply[steerer[button[numero_bottone].obj3].alim2].finestra = 2;

    }

if ( flag == 0 ) /* se "old style", realizza la nuova finestra */
    {
    XtRealizeWidget(steerer_shell);
    }
else
    {
    XtRealizeWidget(ma_comm_shell); /* realizza la finestra magneti */
    }

attributes.backing_store = Always;
valuemask =  CWBackingStore ;

XChangeWindowAttributes(XtDisplay(st_display_lab_a),
                        XtWindow(st_display_lab_a), valuemask, &attributes);

/* la scatola B va' modificata solo nei casi 0 e 1 */
if ( flag == 0 || flag == 1 )
    {
    XChangeWindowAttributes(XtDisplay(st_display_lab_b),
                            XtWindow(st_display_lab_b), valuemask, &attributes);
    }

}



st_setting_act( st_display_lab, tag, st_display_dat )
    Widget st_display_lab ;
    caddr_t tag ; 
    caddr_t st_display_dat ; 
{
Display *display;
Window w;
Window root_return, child_return;
int root_x_return, root_y_return;
int win_x_return, win_y_return;
unsigned int mask_return;

int x, y;
extern disegna();



display = XtDisplay(st_display_lab);



w = XtWindow(st_display_lab);



XQueryPointer(display, w, 
              &root_return, &child_return, &root_x_return, &root_y_return, 
              &win_x_return, &win_y_return, &mask_return);
/*
printf ("root_x_return=%d root_y_return=%d win_x_return=%d win_y_return=%d\n",
        root_x_return, root_y_return, win_x_return, win_y_return); */

disegna(steerer[(int)tag].alim1);

}


stubizza(supply)
int supply;
{

/*printf ("alimentatore = %d, finestart = %d", supply, steerer_supply[supply].finestra);*/

switch ( steerer_supply[supply].finestra )
        {
        case 1:
        if (steerer_supply[supply].mode == 0 ) 
        XtSetArg(args[0], XtNbackground, pixel_black);
        if (steerer_supply[supply].mode == 2 ) 
        XtSetArg(args[0], XtNbackground, pixel_green);


        XtSetValues(steerer_box_a, args, 1);


        XtSetArg(args[0], XtNbackground, pixel_black);
        XtSetArg(args[1], XtNforeground, pixel_yellow);

        XtSetValues(st_name_lab_a, args, 2);
        XtSetValues(st_display_lab_a, args, 1);
        XtSetValues(st_incdec_a_1, args, 1);
        XtSetValues(st_incdec_a_2, args, 1);
        XtSetValues(st_grip_a_1, args, 1);
        XtSetValues(st_curdwn_fast_but_a_1, args, 2);
        XtSetValues(st_curdwn_but_a_1, args, 2);
        XtSetValues(st_cur_ruler_a_1, args, 2);
        XtSetValues(st_curup_but_a_1, args, 2);
        XtSetValues(st_curup_fast_but_a_1, args, 2);
        XtSetValues(st_grip_a_2, args, 1);
        XtSetValues(st_curdwn_fast_but_a_2, args, 2);
        XtSetValues(st_curdwn_but_a_2, args, 2);
        XtSetValues(st_cur_ruler_a_2, args, 2);
        XtSetValues(st_curup_but_a_2, args, 2);
        XtSetValues(st_curup_fast_but_a_2, args, 2);


        break;

        case 2:

        if (steerer_supply[supply].mode == 0 ) 
        XtSetArg(args[0], XtNbackground, pixel_black);
        if (steerer_supply[supply].mode == 2 ) 
        XtSetArg(args[0], XtNbackground, pixel_green);


        XtSetValues(steerer_box_b, args, 1);

        XtSetArg(args[0], XtNbackground, pixel_black);
        XtSetArg(args[1], XtNforeground, pixel_yellow);

        XtSetValues(st_name_lab_b, args, 2);
        XtSetValues(st_display_lab_b, args, 1);
        XtSetValues(st_incdec_b_1, args, 1);
        XtSetValues(st_incdec_b_2, args, 1);
        XtSetValues(st_grip_b_1, args, 1);
        XtSetValues(st_curdwn_fast_but_b_1, args, 2);
        XtSetValues(st_curdwn_but_b_1, args, 2);
        XtSetValues(st_cur_ruler_b_1, args, 2);
        XtSetValues(st_curup_but_b_1, args, 2);
        XtSetValues(st_curup_fast_but_b_1, args, 2);
        XtSetValues(st_grip_b_2, args, 1);
        XtSetValues(st_curdwn_fast_but_b_2, args, 2);
        XtSetValues(st_curdwn_but_b_2, args, 2);
        XtSetValues(st_cur_ruler_b_2, args, 2);
        XtSetValues(st_curup_but_b_2, args, 2);
        XtSetValues(st_curup_fast_but_b_2, args, 2);

        break;
        }

}


st_refresh_b()
{
Arg arg[9];
gc=(GContext)XtGetGC(st_display_lab_b, 0, &val_gc);


xgcv.function = GXcopy;
XChangeGC(XtDisplay(st_display_lab_b), gc, GCFunction , &xgcv);

XSetForeground(XtDisplay(st_display_lab_b), gc, pixel_red);

XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 0, 152, 290, 152);
XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 290, 142, 300, 152);
XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 290, 142, 290, 162);
XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 300, 152, 290, 162);

XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 152, 10, 152, 300);
XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 152, 0, 162, 10);
XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 162, 10, 142, 10);
XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 142, 10, 152, 0);

dimensione_cursore = 0.01;
posizione_cursore = (steerer_supply[steerer[button[numero_bottone].obj3].alim1].current + CURRENT_LIMIT )/(2 * CURRENT_LIMIT );
        if (sizeof(float) > sizeof(XtArgVal))
                {
                XtSetArg(arg[0], XtNshown, &dimensione_cursore);
                XtSetArg(arg[1], XtNtopOfThumb, &posizione_cursore);
                }
        else {
             XtArgVal * I_dimensione_cursore = (XtArgVal *) &dimensione_cursore;
             XtArgVal * I_posizione_cursore = (XtArgVal *) &posizione_cursore;
             XtSetArg(arg[0], XtNshown, *I_dimensione_cursore);
             XtSetArg(arg[1], XtNtopOfThumb, *I_posizione_cursore);
             }
             XtSetArg(arg[2], XtNlength, 500);
             XtSetValues(st_cur_ruler_b_1, arg,3);

dimensione_cursore = 0.01;
posizione_cursore = (steerer_supply[steerer[button[numero_bottone].obj3].alim2].current + CURRENT_LIMIT )/(2 * CURRENT_LIMIT );
        if (sizeof(float) > sizeof(XtArgVal))
                {
                XtSetArg(arg[0], XtNshown, &dimensione_cursore);
                XtSetArg(arg[1], XtNtopOfThumb, &posizione_cursore);
                }
        else {
             XtArgVal * I_dimensione_cursore = (XtArgVal *) &dimensione_cursore;
             XtArgVal * I_posizione_cursore = (XtArgVal *) &posizione_cursore;
             XtSetArg(arg[0], XtNshown, *I_dimensione_cursore);
             XtSetArg(arg[1], XtNtopOfThumb, *I_posizione_cursore);
             }
             XtSetArg(arg[2], XtNlength, 500);
             XtSetValues(st_cur_ruler_b_2, arg,3);



sprintf(string, "%4.2f %%",100.0 * steerer_supply[steerer[button[numero_bottone].obj3].alim1].current/CURRENT_LIMIT);
XtSetArg(args[0], XtNlabel, string);
XtSetValues(st_incdec_b_1 , args, 1 ) ;

sprintf(string, "%4.2f %%",100.0 * steerer_supply[steerer[button[numero_bottone].obj3].alim2].current/CURRENT_LIMIT);
XtSetArg(args[0], XtNlabel, string);
XtSetValues(st_incdec_b_2 , args, 1 ) ;


b_x_center = 150 + (int)((steerer_supply[steerer[button[numero_bottone].obj3].alim1].current/CURRENT_LIMIT)*150.0);
b_y_center = 150 + (int)((steerer_supply[steerer[button[numero_bottone].obj3].alim2].current/CURRENT_LIMIT)*150.0);

XSetForeground(XtDisplay(st_display_lab_b), gc, pixel_black);
XFillArc(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, b_x_center - 1, 300 - b_y_center - 1, 7, 7, 0, 360*64);

if (steerer_supply[steerer[button[numero_bottone].obj3].alim1].mode == 0)
    stubizza(steerer[button[numero_bottone].obj3].alim1);

/* tolto sett. 2001 perche non agg. correttamente etichetta
if (steerer_supply[steerer[button[numero_bottone].obj3].alim1].mode == 2)
    agg_etich( steerer[button[numero_bottone].obj3].alim1, 1);
if (steerer_supply[steerer[button[numero_bottone].obj3].alim1].mode != 2)
    agg_etich( steerer[button[numero_bottone].obj3].alim1, 0);


if (steerer_supply[steerer[button[numero_bottone].obj3].alim2].mode == 2)
    agg_etich( steerer[button[numero_bottone].obj3].alim2, 1);
if (steerer_supply[steerer[button[numero_bottone].obj3].alim2].mode != 2)
    agg_etich( steerer[button[numero_bottone].obj3].alim2, 0);

*/
}



st_refresh_a()
{
Arg arg[9];

gc=(GContext)XtGetGC(st_display_lab_a, 0, &val_gc);


xgcv.function = GXcopy;
XChangeGC(XtDisplay(st_display_lab_a), gc, GCFunction , &xgcv);

XSetForeground(XtDisplay(st_display_lab_a), gc, pixel_red);

XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 0, 152, 290, 152);
XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 290, 142, 300, 152);
XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 290, 142, 290, 162);
XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 300, 152, 290, 162);

XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 152, 10, 152, 300);
XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 152, 0, 162, 10);
XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 162, 10, 142, 10);
XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 142, 10, 152, 0);


dimensione_cursore = 0.01;
posizione_cursore = (steerer_supply[steerer[button[numero_bottone].obj1].alim1].current + CURRENT_LIMIT )/(2 * CURRENT_LIMIT);
        if (sizeof(float) > sizeof(XtArgVal))
                {
                XtSetArg(arg[0], XtNshown, &dimensione_cursore);
                XtSetArg(arg[1], XtNtopOfThumb, &posizione_cursore);
                }
        else {
             XtArgVal * I_dimensione_cursore = (XtArgVal *) &dimensione_cursore;
             XtArgVal * I_posizione_cursore = (XtArgVal *) &posizione_cursore;
             XtSetArg(arg[0], XtNshown, *I_dimensione_cursore);
             XtSetArg(arg[1], XtNtopOfThumb, *I_posizione_cursore);
             }
             XtSetArg(arg[2], XtNlength, 500);
             XtSetValues(st_cur_ruler_a_1, arg,3);

dimensione_cursore = 0.01;
posizione_cursore = (steerer_supply[steerer[button[numero_bottone].obj1].alim2].current + CURRENT_LIMIT )/(2 * CURRENT_LIMIT);
        if (sizeof(float) > sizeof(XtArgVal))
                {
                XtSetArg(arg[0], XtNshown, &dimensione_cursore);
                XtSetArg(arg[1], XtNtopOfThumb, &posizione_cursore);
                }
        else {
             XtArgVal * I_dimensione_cursore = (XtArgVal *) &dimensione_cursore;
             XtArgVal * I_posizione_cursore = (XtArgVal *) &posizione_cursore;
             XtSetArg(arg[0], XtNshown, *I_dimensione_cursore);
             XtSetArg(arg[1], XtNtopOfThumb, *I_posizione_cursore);
             }
             XtSetArg(arg[2], XtNlength, 500);
             XtSetValues(st_cur_ruler_a_2, arg,3);



sprintf(string, "%4.2f %%",100.0 * steerer_supply[steerer[button[numero_bottone].obj1].alim1].current/CURRENT_LIMIT);
XtSetArg(args[0], XtNlabel, string);
XtSetValues(st_incdec_a_1 , args, 1 ) ;

sprintf(string, "%4.2f %%",100.0 * steerer_supply[steerer[button[numero_bottone].obj1].alim2].current/CURRENT_LIMIT);
XtSetArg(args[0], XtNlabel, string);
XtSetValues(st_incdec_a_2 , args, 1 ) ;


a_x_center = 150 + (int)((steerer_supply[steerer[button[numero_bottone].obj1].alim1].current/CURRENT_LIMIT)*150.0);
a_y_center = 150 + (int)((steerer_supply[steerer[button[numero_bottone].obj1].alim2].current/CURRENT_LIMIT)*150.0);

XSetForeground(XtDisplay(st_display_lab_a), gc, pixel_black);
XFillArc(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, a_x_center - 1, 300 - a_y_center - 1, 7, 7, 0, 360*64);

if (steerer_supply[steerer[button[numero_bottone].obj1].alim1].mode == 0)
        {
        /*printf("chiamo stubizza da st_callbacks:637\n");*/
        stubizza(steerer[button[numero_bottone].obj1].alim1);
        }


/* tolto sett. 2001 perche non agg. correttamente etichetta
if (steerer_supply[steerer[button[numero_bottone].obj1].alim1].mode == 2)
        agg_etich( steerer[button[numero_bottone].obj1].alim1, 1);
if (steerer_supply[steerer[button[numero_bottone].obj1].alim1].mode != 2)
        agg_etich( steerer[button[numero_bottone].obj1].alim1, 0);


if (steerer_supply[steerer[button[numero_bottone].obj1].alim2].mode == 2)
        agg_etich( steerer[button[numero_bottone].obj1].alim2, 1);
if (steerer_supply[steerer[button[numero_bottone].obj1].alim2].mode != 2)
        agg_etich( steerer[button[numero_bottone].obj1].alim2, 0);

*/

}

