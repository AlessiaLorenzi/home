
#include "alpi.h"
#include "ma_const.h"
#include "ma_ext_vars.h"
#include "ma.bit"
#include "bottsu"
#include "bottgiu"
#include <Shell.h>
#include <Grip.h>

#define MA_WIDTH      1022
#define MA_HEIGHT      516

#define CURRENT_LIMIT 3.5


#define MA_STEP                                1
#define MA_MIN_WA                      MA_STEP-1
#define MA_MAX_WA              (1000000-MA_STEP)

#define MA_STEP_FAST                          50
#define MA_MIN_WA_FAST            MA_STEP_FAST-1
#define MA_MAX_WA_FAST     (1000000-MA_STEP_FAST)


#define MA_STEP_KEY                           50
#define MA_MIN_WA_KEY              MA_STEP_KEY-1
#define MA_MAX_WA_KEY      (1000000-MA_STEP_KEY)

#define SCROLLTRANS "\
<BtnDown>: StartScroll(Forward) MoveThumb() NotifyScroll(Proportional)\n\
<BtnMotion>: MoveThumb() NotifyScroll(Proportional)\n\
<BtnUp>: MoveThumb() NotifyThumb() EndScroll()"

static ma_step_key = MA_STEP_KEY ;
static ma_min_wa_key = MA_MIN_WA_KEY ;
static ma_max_wa_key = MA_MAX_WA_KEY ;

int nl;
float ci, cl, vl;
char s1[20], s2[20] ,s3[20];



XWindowAttributes attributi_finestra;
GContext gc;
XGCValues xgcv;
XGCValues val_gc;

unsigned long valuemask;
XSetWindowAttributes attributes, attributes_1;


void st_current_cmd( supply_number, value_to_send )
int supply_number ;
float value_to_send ;

{
extern int verbose ;
extern stubizza();

sprintf ( to_server ,"st comm : ALIM = %2d !WA %3.2f",
          supply_number, value_to_send);
if ( verbose )
    printf ("ALPI send: %s\n", to_server);
write( server_channel, to_server, BUF_SIZE ) ;
read( server_channel, from_server, BUF_SIZE ) ;

    /*printf ("  ++  ALPI receive: %s\n", from_server);*/

if (strncmp(from_server, "STUB", 4) == 0)
	{
	steerer_supply[supply_number].mode = 0 ;
	/*printf("chiamo stubizza da st_act:75\n");*/
	stubizza(supply_number);
	}

else
	sscanf (from_server+2, "%s %f %s %f %s %f", s1, &ci, s2, &cl, s3, &vl);


/*printf("  cl === %f\n", cl);*/

};


st_timeout_proc()
{
int nl1, al1;
float ci1, cl1, vl1;
char s1[20], s2[20] ,s3[20];
extern agg_etich ();
char from_server1[1024];


//printf("st_proc: gli alimentatori sono  %d  %d %d %d \n",
//steerer[button[numero_bottone].obj1].alim1,
//steerer[button[numero_bottone].obj1].alim2 ,  
//steerer[button[numero_bottone].obj3].alim1,
//steerer[button[numero_bottone].obj3].alim2
//);


al1 = steerer[button[numero_bottone].obj1].alim1;
sprintf ( to_server ,"st comm : ALIM = %2d #", al1);
if ( verbose )
    	printf ("ALPI send: %s\n", to_server);
write( server_channel, to_server, BUF_SIZE ) ;
read( server_channel, from_server1, BUF_SIZE ) ;

/*printf ("     --    ALPI receive: %s\n", from_server1);*/

if ( strncmp ( from_server1, "STUB", 4 ) != 0 )
	{
	sscanf (from_server1+2, "%s %f %s %f %s %f", s1, &ci1, s2, &cl1, s3, &vl1);
	/*printf("@@@@@@@@@@@@ alim = %d   differenza percentuale %f\n",al1,
	((cl1-ci1)/ci1)*100.0);*/

	if ( (((cl1-ci1)/ci1)*100.0 > 2.0 || ((cl1-ci1)/ci1)*100.0 < -2.0))

		{
		printf("[01;35m l'alimentatore numero %d e' fuori specifiche ! [0m\n", al1);
		steerer_supply[al1].mode = 2;
		}
	agg_etich ( al1, cl1 );

	}

al1 = steerer[button[numero_bottone].obj1].alim2;
sprintf ( to_server ,"st comm : ALIM = %2d #", al1);
if ( verbose )
    	printf ("ALPI send: %s\n", to_server);
write( server_channel, to_server, BUF_SIZE ) ;
read( server_channel, from_server1, BUF_SIZE ) ;

/*printf ("     --    ALPI receive: %s\n", from_server1);*/

if ( strncmp ( from_server1, "STUB", 4 ) != 0 )
	{
	sscanf (from_server1+2, "%s %f %s %f %s %f", s1, &ci1, s2, &cl1, s3, &vl1);
	/*printf("@@@@@@@@@@@@ alim = %d   differenza percentuale %f\n",al1,
	((cl1-ci1)/ci1)*100.0);*/

	if ( (((cl1-ci1)/ci1)*100.0 > 2.0 || ((cl1-ci1)/ci1)*100.0 < -2.0))

		{
		printf("[01;35m l'alimentatore numero %d e' fuori specifiche ! [0m\n", al1);
		steerer_supply[al1].mode = 2;
		}
	agg_etich ( al1, cl1 );
	}


al1 = steerer[button[numero_bottone].obj3].alim1;
sprintf ( to_server ,"st comm : ALIM = %2d #", al1);
if ( verbose )
    	printf ("ALPI send: %s\n", to_server);
write( server_channel, to_server, BUF_SIZE ) ;
read( server_channel, from_server1, BUF_SIZE ) ;

/*printf ("     --    ALPI receive: %s\n", from_server1);*/

if ( strncmp ( from_server1, "STUB", 4 ) != 0 )
	{
	sscanf (from_server1+2, "%s %f %s %f %s %f", s1, &ci1, s2, &cl1, s3, &vl1);
	/*printf("@@@@@@@@@@@@ alim = %d   differenza percentuale %f\n",al1,
	((cl1-ci1)/ci1)*100.0);*/

	if ( (((cl1-ci1)/ci1)*100.0 > 2.0 || ((cl1-ci1)/ci1)*100.0 < -2.0))

		{
		printf("[01;35m l'alimentatore numero %d e' fuori specifiche ! [0m\n", al1);
		steerer_supply[al1].mode = 2;
		}
	agg_etich ( al1, cl1 );
	}

al1 = steerer[button[numero_bottone].obj3].alim2;
sprintf ( to_server ,"st comm : ALIM = %2d #", al1);
if ( verbose )
    	printf ("ALPI send: %s\n", to_server);
write( server_channel, to_server, BUF_SIZE ) ;
read( server_channel, from_server1, BUF_SIZE ) ;

/*printf ("     --    ALPI receive: %s\n", from_server1);*/

if ( strncmp ( from_server1, "STUB", 4 ) != 0 )
	{
	sscanf (from_server1+2, "%s %f %s %f %s %f", s1, &ci1, s2, &cl1, s3, &vl1);
	/*printf("@@@@@@@@@@@@ alim = %d   differenza percentuale %f\n",al1,
	((cl1-ci1)/ci1)*100.0);*/

	if ( (((cl1-ci1)/ci1)*100.0 > 2.0 || ((cl1-ci1)/ci1)*100.0 < -2.0))

		{
		printf("[01;35m l'alimentatore numero %d e' fuori specifiche ! [0m\n", al1);
		steerer_supply[al1].mode = 2;
		}
	agg_etich ( al1, cl1 );
	}




if (st_refresh_onoff == 1)
        st_interval_id = XtAddTimeOut ( 2000, st_timeout_proc, NULL);

}






void st_grip_act(w, index, call_data)
    Widget w;
    int index;
    GripCallDataRec *call_data;
{
    extern int pixel_white, pixel_wheat, pixel_coral ;
    static int inside = FALSE, grabbed = FALSE;
    Arg arg;

    if (!strncmp(*(call_data->params), "enter", 5))  {
       inside = TRUE;
       XSetInputFocus( XtDisplay(w), XtWindow(w),
                       RevertToParent, CurrentTime ) ;
       XtSetArg(arg, XtNforeground, pixel_white );
       XtSetValues(w, &arg, 1);
       XSync(XtDisplay(w), FALSE);
       }
    if (!strncmp(*(call_data->params), "leave", 5))  {
       inside = FALSE;
       XSync(XtDisplay(w), FALSE);
       XSetInputFocus( XtDisplay(w),
                       XDefaultRootWindow( XtDisplay(w) ),
                       RevertToParent, CurrentTime );
       XtSetArg( arg, XtNforeground, pixel_wheat ) ;
       XtSetValues(w, &arg, 1);
       XSync(XtDisplay(w), FALSE);
       }


//    if (!strncmp(*(call_data->params), "press", 5))  {
//       }
//    if (!strncmp(*(call_data->params), "release", 7))  {
//       if (inside) 
//           if (grabbed)  {
//               grabbed = FALSE;
//               XUngrabPointer( XtDisplay(w), CurrentTime);
//               XUngrabKeyboard( XtDisplay(w), CurrentTime) ;
//               XtSetArg( arg, XtNforeground, pixel_white ) ;
//               XtSetValues(w, &arg, 1);
//               XSync( XtDisplay(w), FALSE ) ;
//
//                   /*printf("ungrabbed\n");*/
//               }
//           else {
//               grabbed = TRUE;
//               XSync( XtDisplay(w), FALSE ) ;
//               XGrabPointer( XtDisplay(w), XtWindow(w),
//                             TRUE, ~NoEventMask,
//                             GrabModeSync, GrabModeSync,
//                             XtWindow(w), None, CurrentTime);
//               XAllowEvents( XtDisplay(w), AsyncBoth, CurrentTime ) ;
//               XtSetArg( arg, XtNforeground, pixel_coral ) ;
//               XtSetValues(w, &arg, 1);
//               XSync( XtDisplay(w), FALSE ) ;
//
//                   /*printf("grabbed\n");*/
//               }
//       }


    if (!strncmp(*(call_data->params), "right", 5))  
	{
	XSync( XtDisplay(w), TRUE);
	st_curup_act( w, index, NULL );
	/*printf( "up alim %d\n", index ) ;*/
	}

    if (!strncmp(*(call_data->params), "left", 4))  
	{
	XSync( XtDisplay(w), TRUE);
	st_curdwn_act( w, index, NULL );
	/*printf( "down alim %d\n", index ) ;*/
	}


}

agg_etich ( alimentatore, on_off ) 
int alimentatore;
float on_off;
{
Arg args[10];
char string[100];

//printf("      in agg_etich alim = %d corrente = %6.4f\n\n", alimentatore, on_off);
/*printf("      in agg alim = %d corr = %4.2f\n\n", alimentatore, current);*/
sprintf ( string, "VALUE READ FROM SUPPLY : %6.2f %%", 100.0 * on_off/CURRENT_LIMIT );

if ( steerer_supply[alimentatore].finestra == 1 ) 
	{
	if ( steerer_supply[alimentatore].x_oppure_y == 1 )
		{
		XtSetArg(args[0], XtNlabel, string);
		XtSetArg(args[1], XtNforeground, pixel_red);
		XtSetValues(st_real_a_1 , args, 2 ) ;
		}

	if ( steerer_supply[alimentatore].x_oppure_y == 2 )
		{
		XtSetArg(args[0], XtNlabel, string);
		XtSetArg(args[1], XtNforeground, pixel_red);
		XtSetValues(st_real_a_2 , args, 1 ) ;
		}
	}

if ( steerer_supply[alimentatore].finestra == 2 )
	{ 

	if ( steerer_supply[alimentatore].x_oppure_y == 1 )
		{
		XtSetArg(args[0], XtNlabel, string);
		XtSetArg(args[1], XtNforeground, pixel_red);
		XtSetValues(st_real_b_1 , args, 1 ) ;
		}

	if ( steerer_supply[alimentatore].x_oppure_y == 2 )
		{
		XtSetArg(args[0], XtNlabel, string);
		XtSetArg(args[1], XtNforeground, pixel_red);
		XtSetValues(st_real_b_2 , args, 1 ) ;
		}

	}
}


disegna(alimentatore)
int alimentatore;
{
Arg args[10];
Arg arg[10];
char string[100];
/*printf (" sono in disegna con alim = %d\n", alimentatore);*/


if ( steerer_supply[alimentatore].finestra == 1 ) 
	{
	gc=(GContext)XtGetGC(st_display_lab_a,0,&val_gc);

	XSetForeground(XtDisplay(st_display_lab_a), gc, pixel_white);

	xgcv.function = GXcopy;

	XChangeGC(XtDisplay(st_display_lab_a), gc, GCFunction , &xgcv);


	XFillArc(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, a_x_center - 1, 300 - a_y_center - 1, 7, 7, 0, 360*64);


	if ( steerer_supply[alimentatore].x_oppure_y == 1 )
		{
		a_x_center = 150 + (int)((steerer_supply[alimentatore].current/5.0)*150.0);
        dimensione_cursore = 0.01;
        posizione_cursore = (steerer_supply[alimentatore].current + CURRENT_LIMIT)/(2 * CURRENT_LIMIT);
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
             XtSetValues(st_cur_ruler_a_1, arg,2);

		sprintf(string, "%4.2f %%",100.0 * steerer_supply[alimentatore].current/CURRENT_LIMIT);
		XtSetArg(args[0], XtNlabel, string);
		XtSetValues(st_incdec_a_1 , args, 1 ) ;

		}

	if ( steerer_supply[alimentatore].x_oppure_y == 2 )
		{
		a_y_center = 150 + (int)((steerer_supply[alimentatore].current/5.0)*150.0);
        dimensione_cursore = 0.01;
        posizione_cursore =  (steerer_supply[alimentatore].current + CURRENT_LIMIT)/(2 * CURRENT_LIMIT);
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
             XtSetValues(st_cur_ruler_a_2, arg,2);

		sprintf(string, "%4.2f %%",100.0 * steerer_supply[alimentatore].current/CURRENT_LIMIT);
		XtSetArg(args[0], XtNlabel, string);
		XtSetValues(st_incdec_a_2 , args, 1 ) ;
		}

	XSetForeground(XtDisplay(st_display_lab_a), gc, pixel_red);
	xgcv.function = GXcopy;
	XChangeGC(XtDisplay(st_display_lab_a), gc, GCFunction , &xgcv);
	XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 0, 152, 290, 152);
	XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 290, 142, 300, 152);
	XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 290, 142, 290, 162);
	XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 300, 152, 290, 162);

	XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 152, 10, 152, 300);
	XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 152, 0, 162, 10);
	XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 162, 10, 142, 10);
	XDrawLine(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, 142, 10, 152, 0);

	XSetForeground(XtDisplay(st_display_lab_a), gc, pixel_black);
	XFillArc(XtDisplay(st_display_lab_a), XtWindow(st_display_lab_a), gc, a_x_center - 1, 300 - a_y_center - 1, 7, 7, 0, 360*64);

	}


if ( steerer_supply[alimentatore].finestra == 2 )
	{ 
	gc=(GContext)XtGetGC(st_display_lab_b, 0, &val_gc);

	XSetForeground(XtDisplay(st_display_lab_b), gc, pixel_white);

	xgcv.function = GXcopy;

	XChangeGC(XtDisplay(st_display_lab_b), gc, GCFunction , &xgcv);

	XFillArc(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, b_x_center - 1, 300 - b_y_center - 1, 7, 7, 0, 360*64);

	if ( steerer_supply[alimentatore].x_oppure_y == 1 )
		{
		b_x_center = 150 + (int)((steerer_supply[alimentatore].current/5.0)*150.0);
        dimensione_cursore = 0.01;
        posizione_cursore = (steerer_supply[alimentatore].current + CURRENT_LIMIT)/(2 * CURRENT_LIMIT);
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
             XtSetValues(st_cur_ruler_b_1, arg,2);

		sprintf(string, "%4.2f %%",100.0 * steerer_supply[alimentatore].current/CURRENT_LIMIT);
		XtSetArg(args[0], XtNlabel, string);
		XtSetValues(st_incdec_b_1 , args, 1 ) ;
		}

	if ( steerer_supply[alimentatore].x_oppure_y == 2 )
		{
		b_y_center = 150 + (int)((steerer_supply[alimentatore].current/5.0)*150.0);
        dimensione_cursore = 0.01;
        posizione_cursore = (steerer_supply[alimentatore].current + CURRENT_LIMIT)/(2 * CURRENT_LIMIT);
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
             XtSetValues(st_cur_ruler_b_2, arg,2);

		sprintf(string, "%4.2f %%",100.0 * steerer_supply[alimentatore].current/CURRENT_LIMIT);
		XtSetArg(args[0], XtNlabel, string);
		XtSetValues(st_incdec_b_2 , args, 1 ) ;
		}

	XSetForeground(XtDisplay(st_display_lab_a), gc, pixel_red);
	xgcv.function = GXcopy;
	XChangeGC(XtDisplay(st_display_lab_b), gc, GCFunction , &xgcv);
	XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 0, 152, 290, 152);
	XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 290, 142, 300, 152);
	XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 290, 142, 290, 162);
	XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 300, 152, 290, 162);

	XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 152, 10, 152, 300);
	XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 152, 0, 162, 10);
	XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 162, 10, 142, 10);
	XDrawLine(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, 142, 10, 152, 0);

	XSetForeground(XtDisplay(st_display_lab_a), gc, pixel_black);
	XFillArc(XtDisplay(st_display_lab_b), XtWindow(st_display_lab_b), gc, b_x_center - 1, 300 - b_y_center - 1, 7, 7, 0, 360*64);

	
	}


}


st_curdwn_act( st_curup_but, tag, st_curup_dat )
    Widget st_curup_but ;
    caddr_t tag ; 
    caddr_t st_curup_dat ; 
{
if ( steerer_supply[(int)tag].current > - CURRENT_LIMIT )
	steerer_supply[(int)tag].current = steerer_supply[(int)tag].current - 0.01 ;

/*printf("alimentatore = %d  corrente = %f\n", (int)tag, steerer_supply[(int)tag].current);*/

st_current_cmd ( (int)tag, steerer_supply[(int)tag].current );


disegna ( (int)tag ) ;

}


st_curdwn_fast_act( st_curup_but, tag, st_curup_dat )
    Widget st_curup_but ;
    caddr_t tag ; 
    caddr_t st_curup_dat ; 
{
if ( steerer_supply[(int)tag].current > - CURRENT_LIMIT )
	steerer_supply[(int)tag].current = steerer_supply[(int)tag].current - 0.2 ;

/*printf("alimentatore = %d  corrente = %f\n", (int)tag, steerer_supply[(int)tag].current);*/

st_current_cmd ( (int)tag, steerer_supply[(int)tag].current );

disegna ( (int)tag ) ;


}


st_curup_act( st_curup_but, tag, st_curup_dat )
    Widget st_curup_but ;
    caddr_t tag ; 
    caddr_t st_curup_dat ; 
{
if ( steerer_supply[(int)tag].current < CURRENT_LIMIT )
	steerer_supply[(int)tag].current = steerer_supply[(int)tag].current + 0.01 ;

/*printf("alimentatore = %d  corrente = %f\n", (int)tag, steerer_supply[(int)tag].current);*/

st_current_cmd ( (int)tag, steerer_supply[(int)tag].current );


disegna ( (int)tag ) ;
}


st_curup_fast_act( st_curup_but, tag, st_curup_dat )
    Widget st_curup_but ;
    caddr_t tag ; 
    caddr_t st_curup_dat ; 
{
if ( steerer_supply[(int)tag].current < CURRENT_LIMIT )
	steerer_supply[(int)tag].current = steerer_supply[(int)tag].current + 0.2 ;

/*printf("alimentatore = %d  corrente = %f\n", (int)tag, steerer_supply[(int)tag].current);*/

st_current_cmd ( (int)tag, steerer_supply[(int)tag].current );



disegna ( (int)tag ) ;

}


st_jump_act( ma_cur_ruler, tag, percento )

    Widget ma_cur_ruler ;
    caddr_t tag ; 
    caddr_t percento;
{
int corrente;
Arg arg[10];
char string[30];
float percent;

int supply_number ;
supply_number = (int)tag;
percent = *(float *)percento;
steerer_supply[supply_number].current = 2.0 * CURRENT_LIMIT * ( percent - 0.5 ) ;

if ( steerer_supply[supply_number].current > CURRENT_LIMIT ) steerer_supply[supply_number].current = CURRENT_LIMIT ;
if ( steerer_supply[supply_number].current < - CURRENT_LIMIT ) steerer_supply[supply_number].current = - CURRENT_LIMIT ;

/*printf("alimentatore = %d  corrente = %f\n", supply_number, steerer_supply[supply_number].current);*/

st_current_cmd ( supply_number, steerer_supply[supply_number].current );


disegna(supply_number);
}



st_scroll_act( ma_cur_ruler, tag, percento )

    Widget ma_cur_ruler ;
    caddr_t tag ; 
    int percento;
{
Arg arg[10];
char string[30];
float percent;

int supply_number;
supply_number = (int)tag;
percent = (float)percento/500.0;
steerer_supply[supply_number].current = 2.0 * CURRENT_LIMIT * ( percent - 0.5 ) ;
if ( steerer_supply[supply_number].current > CURRENT_LIMIT ) steerer_supply[supply_number].current = CURRENT_LIMIT ;
if ( steerer_supply[supply_number].current < - CURRENT_LIMIT ) steerer_supply[supply_number].current = - CURRENT_LIMIT ;
/*printf("alimentatore = %d  corrente = %f\n", supply_number, steerer_supply[supply_number].current);*/


disegna(supply_number);

/*printf ( "scroll : a_x = %d  a_y = %d \n", a_x_center, a_y_center );
printf ( "scroll : b_x = %d  b_y = %d \n", b_x_center, b_y_center );*/

}


st_remloc_act()
{
}


st_reset_act()
{
}

