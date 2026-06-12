

#include "alpi.h"
#include "ma_const.h"
#include "ma_ext_vars.h"
#include <Shell.h>
#include <Grip.h>

#define MA_WIDTH      1022
#define MA_HEIGHT      516

#define MA_STEP                                1
#define MA_MIN_WA                      MA_STEP-1
#define MA_MAX_WA              (1000000-MA_STEP)

#define MA_STEP_FAST                          50
#define MA_MIN_WA_FAST            MA_STEP_FAST-1
#define MA_MAX_WA_FAST     (1000000-MA_STEP_FAST)

#define MA_STEP_KEY                           50
#define MA_MIN_WA_KEY              MA_STEP_KEY-1
#define MA_MAX_WA_KEY      (1000000-MA_STEP_KEY)

static ma2q3q_step_Ik = MA_STEP_KEY ;
static ma2q3q_min_wa_Ik = MA_MIN_WA_KEY ;
static ma2q3q_max_wa_Ik = MA_MAX_WA_KEY ;

static ma2q3q_step_Dk = MA_STEP_KEY ;
static ma2q3q_min_wa_Dk = MA_MIN_WA_KEY ;
static ma2q3q_max_wa_Dk = MA_MAX_WA_KEY ;

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

Arg args[9];
/* callbacks per i widgets unificanti per 2q e 3q */

/* callbacks del ruler di I : */

void ma2q3q_I_jump( ma2q3q_I_ruler, magnet_number, position )
    Widget ma2q3q_I_ruler ;
    int magnet_number ; 
    caddr_t position;

{
extern void ma_current_upd(), ma_current_cmd() ;

float d_value, i_value ;

int i_int_value, limit, new_i1, new_i2, supply_number ;

d_value = magnet[ magnet_number ].d_value ;

if ( d_value == 0.0 )
    limit = 999999 ;
else {
    if ( d_value > 0.0 )
        limit = (int)( 1000000.0 / ( 1.0 + d_value ) + 0.5 ) ;
    else
        limit = (int)( 1000000.0 / ( 1.0 - d_value ) + 0.5 ) ;
    if ( limit > 999999 ) limit = 999999 ;
    }

i_value = *(float *)position; /* 0.0 <= i_value <= 1.0 */
i_int_value = (int)( i_value * 1000000.0 + 0.5 ) ;
if ( i_int_value > limit ) i_int_value = limit ;
i_value = (float)i_int_value / 1000000.0 ;
new_i1 = (int) ( (float)i_int_value * ( 1.0 + d_value ) + 0.5 ) ;
new_i2 = (int) ( (float)i_int_value * ( 1.0 - d_value ) + 0.5 ) ;
if ( new_i1 >= 999999 ) new_i1 = 999999 ;
if ( new_i2 >= 999999 ) new_i2 = 999999 ;
if ( new_i1 < 0 ) new_i1 = 0 ;
if ( new_i2 < 0 ) new_i2 = 0 ;

dimensione_cursore = 0.01;
posizione_cursore = i_value ;
        if (sizeof(float) > sizeof(XtArgVal))
                {
                XtSetArg(args[0], XtNshown, &dimensione_cursore);
                XtSetArg(args[1], XtNtopOfThumb, &posizione_cursore);
                }
        else {
             XtArgVal * I_dimensione_cursore = (XtArgVal *) &dimensione_cursore;
             XtArgVal * I_posizione_cursore = (XtArgVal *) &posizione_cursore;
             XtSetArg(args[0], XtNshown, *I_dimensione_cursore);
             XtSetArg(args[1], XtNtopOfThumb, *I_posizione_cursore);
             }
             XtSetArg(args[2], XtNlength, 500);
             XtSetValues(ma2q3q_I_ruler, args,3);


magnet[ magnet_number ].i_value = i_value ;

/*printf( "ma 2q 3q I jump: %d i1: %d i2: %d\n",
        i_int_value, new_i1, new_i2 ) ;*/

supply_number = magnet[ magnet_number ].alim1  ;
supply[ supply_number ].wa_current = new_i1 ;
supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
ma_current_cmd( supply_number, new_i1 ) ;
ma_current_upd( supply_number ) ;

supply_number = magnet[ magnet_number ].alim2  ;
supply[ supply_number ].wa_current = new_i2 ;
supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
ma_current_cmd( supply_number, new_i2 ) ;
ma_current_upd( supply_number ) ;

}

void ma2q3q_I_scroll( ma2q3q_I_ruler, magnet_number, ma2q3q_I_dat)
    Widget ma2q3q_I_ruler ;
    int magnet_number ;
    int ma2q3q_I_dat ;

{
extern Widget ma2q3q_I_lab ;

float d_value, i_value, limit ;
char string[30];
Arg arg ;

d_value = magnet[ magnet_number ].d_value ;

if ( d_value == 0.0 )
    limit = 99.9999 ;
else
    if ( d_value > 0.0 )
        limit = 99.9999 / ( 1.0 + d_value ) ;
    else
        limit = 99.9999 / ( 1.0 - d_value ) ;

i_value = (float)ma2q3q_I_dat/5.0 ;
if ( i_value > limit ) i_value = limit ;

sprintf( string, " Actual  I  :  %7.2f%%", i_value ) ;
XtSetArg( arg, XtNlabel, string ) ;
XtSetValues( ma2q3q_I_lab, &arg, 1 ) ;

}

/* callbacks del ruler di D : */

void ma2q3q_D_jump( ma2q3q_D_ruler, magnet_number, position )
    Widget ma2q3q_D_ruler ;
    int magnet_number ; 
    caddr_t position;

{
extern void ma_current_upd(), ma_current_cmd() ;

int i1, i2, new_i1, new_i2, supply_number ;
float i_value, d_value, limit ;

float value ;

i_value = magnet[ magnet_number ].i_value ;

if ( i_value > 0.5 )
    limit =   ( 1.0 / i_value ) - 1.0 ;
else
    limit =  1.0 ;

d_value = *(float *)position; /* 0.0 <= d_value <= 1.0 */
d_value = 2.0 * ( d_value - 0.500000 ) ; /* -1, +1 */

if ( d_value >= 0 ) {
    if ( d_value > limit )
        d_value = limit ;
    }
else {
    if ( d_value < ((-1.0)*limit) )
        d_value = ((-1.0)*limit) ;
    }

new_i1 = (int) ( 1000000.0 * i_value * ( 1.0 + d_value ) + 0.5 ) ;
new_i2 = (int) ( 1000000.0 * i_value * ( 1.0 - d_value ) + 0.5 ) ;
if ( new_i1 > 999999 ) new_i1 = 999999 ;
if ( new_i2 > 999999 ) new_i2 = 999999 ;
if ( new_i1 < 0 ) new_i1 = 0 ;
if ( new_i2 < 0 ) new_i2 = 0 ;

dimensione_cursore = 0.01;
posizione_cursore = ( d_value + 1.0 ) / 2.0 ;
        if (sizeof(float) > sizeof(XtArgVal))
                {
                XtSetArg(args[0], XtNshown, &dimensione_cursore);
                XtSetArg(args[1], XtNtopOfThumb, &posizione_cursore);
                }
        else {
             XtArgVal * I_dimensione_cursore = (XtArgVal *) &dimensione_cursore;
             XtArgVal * I_posizione_cursore = (XtArgVal *) &posizione_cursore;
             XtSetArg(args[0], XtNshown, *I_dimensione_cursore);
             XtSetArg(args[1], XtNtopOfThumb, *I_posizione_cursore);
             }
             XtSetArg(args[2], XtNlength, 500);
             XtSetValues(ma2q3q_D_ruler, args,3);


magnet[ magnet_number ].d_value = d_value ;

/*printf( "ma 2q 3q D jump: %f i1: %d i2: %d\n",
        d_value, new_i1, new_i2 ) ;*/

supply_number = magnet[ magnet_number ].alim1  ;
supply[ supply_number ].wa_current = new_i1 ;
supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
ma_current_cmd( supply_number, new_i1 ) ;
ma_current_upd( supply_number ) ;

supply_number = magnet[ magnet_number ].alim2  ;
supply[ supply_number ].wa_current = new_i2 ;
supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
ma_current_cmd( supply_number, new_i2 ) ;
ma_current_upd( supply_number ) ;

}

void ma2q3q_D_scroll( ma2q3q_D_ruler, magnet_number, ma2q3q_D_dat)
    Widget ma2q3q_D_ruler ;
    int magnet_number ;
    int ma2q3q_D_dat ;

{
extern Widget ma2q3q_D_lab ;


float i_value, d_value, limit ;
char string[30];
Arg arg ;

i_value = magnet[ magnet_number ].i_value ;

if ( i_value > 0.5 )
    limit =   ( 1.0 / i_value ) - 1.0 ;
else
    limit =  1.0 ;

d_value = (float)ma2q3q_D_dat/250.0 - 1.0 ;
if ( d_value >= 0 ) {
    if ( d_value > limit )
        d_value = limit ;
    }
else {
    if ( d_value < ((-1.0)*limit) )
        d_value = ((-1.0)*limit) ;
    }

sprintf( string, " Actual  D  :  %+7.4f", d_value ) ;
XtSetArg( arg, XtNlabel, string ) ;
XtSetValues( ma2q3q_D_lab, &arg, 1 ) ;

}

/* call back della grip area per I  */

void ma2q3q_I_grip_act(w, index, call_data)
    Widget w;
    int index;
    GripCallDataRec *call_data;
{
    extern int verbose ;
    void ma2q3q_I_right(), ma2q3q_I_left() ;
    void ma2q3q_I_up(), ma2q3q_I_down() ;
    extern int pixel_white, pixel_wheat, pixel_coral ;
    static int inside = FALSE, grabbed = FALSE;
    Arg arg;
    XKeyboardControl valore;
    if (!strncmp(*(call_data->params), "enter", 5))  {
       inside = TRUE;
       valore.key_click_percent = 100;
/*
       XChangeKeyboardControl( XtDisplay(w), KBKeyClickPercent, &valore);
*/
       XSetInputFocus( XtDisplay(w), XtWindow(w),
                       RevertToParent, CurrentTime ) ;
       XtSetArg(arg, XtNforeground, pixel_white );
       XtSetValues(w, &arg, 1);
       XSync(XtDisplay(w), FALSE);
       }
    if (!strncmp(*(call_data->params), "leave", 5))  {
       inside = FALSE;
       valore.key_click_percent = 0;
/*
       XChangeKeyboardControl( XtDisplay(w), KBKeyClickPercent, &valore);
*/
       XSync(XtDisplay(w), FALSE);
       XSetInputFocus( XtDisplay(w),
                       XDefaultRootWindow( XtDisplay(w) ),
                       RevertToParent, CurrentTime );
       XtSetArg( arg, XtNforeground, pixel_wheat ) ;
       XtSetValues(w, &arg, 1);
       XSync(XtDisplay(w), FALSE);
       }
    if (!strncmp(*(call_data->params), "right", 5))  {
       XSync( XtDisplay(w), TRUE);
       if (verbose)
           printf( "up magnet %d\n", index ) ;
       ma2q3q_I_right( index ) ;
       }
    if (!strncmp(*(call_data->params), "left", 4))  {
       XSync( XtDisplay(w), TRUE);
       if (verbose)
           printf( "down magnet %d\n", index ) ;
       ma2q3q_I_left( index ) ;
       }
    if (!strncmp(*(call_data->params), "up", 2))  {
       XSync( XtDisplay(w), TRUE);
       if (verbose)
           printf( "up inc\n" ) ;
       ma2q3q_I_up( index ) ;
       }
    if (!strncmp(*(call_data->params), "down", 4))  {
       XSync( XtDisplay(w), TRUE);
       if (verbose)
           printf( "down inc\n" ) ;
       ma2q3q_I_down( index ) ;
       }
}

/* end call back della grip area per I */

/* routine associata al tasto "arrow =>" per I (right) */

void ma2q3q_I_right( magnet_number )
    int magnet_number ;

{
extern int verbose ;

extern void ma_current_upd(), ma_current_cmd() ;

float d_value, i_value ;

int i_int_value, limit, new_i1, new_i2, supply_number ;

char string[30];
Arg arg ;

d_value = magnet[ magnet_number ].d_value ;

if ( verbose )
    printf("I right\n") ;

if ( d_value == 0.0 )
    limit = 999999 ;
else {
    if ( d_value > 0.0 )
        limit = (int)( 1000000.0 / ( 1.0 + d_value ) + 0.5 ) ;
    else
        limit = (int)( 1000000.0 / ( 1.0 - d_value ) + 0.5 ) ;
    if ( limit > 999999 ) limit = 999999 ;
    }

i_value = magnet[ magnet_number ].i_value +
          (float)ma2q3q_step_Ik / 10000.0 ; /* in 0.0-1.0 */
if ( i_value > 1.0 ) i_value = 1.0 ;
i_int_value = (int)( i_value * 1000000.0 + 0.5 ) ;
if ( i_int_value > limit ) i_int_value = limit ;
i_value = (float)i_int_value / 1000000.0 ;
new_i1 = (int) ( (float)i_int_value * ( 1.0 + d_value ) + 0.5 ) ;
new_i2 = (int) ( (float)i_int_value * ( 1.0 - d_value ) + 0.5 ) ;
if ( new_i1 >= 999999 ) new_i1 = 999999 ;
if ( new_i2 >= 999999 ) new_i2 = 999999 ;
if ( new_i1 < 0 ) new_i1 = 0 ;
if ( new_i2 < 0 ) new_i2 = 0 ;

magnet[ magnet_number ].i_value = i_value ;
sprintf( string, " Actual  I  :  %7.2f%%", i_value*100.0 ) ;
XtSetArg( arg, XtNlabel, string ) ;
XtSetValues( ma2q3q_I_lab, &arg, 1 ) ;

dimensione_cursore = 0.01;
posizione_cursore = i_value ;
        if (sizeof(float) > sizeof(XtArgVal))
                {
                XtSetArg(args[0], XtNshown, &dimensione_cursore);
                XtSetArg(args[1], XtNtopOfThumb, &posizione_cursore);
                }
        else {
             XtArgVal * I_dimensione_cursore = (XtArgVal *) &dimensione_cursore;
             XtArgVal * I_posizione_cursore = (XtArgVal *) &posizione_cursore;
             XtSetArg(args[0], XtNshown, *I_dimensione_cursore);
             XtSetArg(args[1], XtNtopOfThumb, *I_posizione_cursore);
             }
             XtSetArg(args[2], XtNlength, 500);
             XtSetValues( ma2q3q_I_ruler, args,3);


/*printf( "ma 2q 3q I right: %d i1: %d i2: %d\n",
        i_int_value, new_i1, new_i2 ) ;*/

supply_number = magnet[ magnet_number ].alim1  ;
supply[ supply_number ].wa_current = new_i1 ;
supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
ma_current_cmd( supply_number, new_i1 ) ;
ma_current_upd( supply_number ) ;

supply_number = magnet[ magnet_number ].alim2  ;
supply[ supply_number ].wa_current = new_i2 ;
supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
ma_current_cmd( supply_number, new_i2 ) ;
ma_current_upd( supply_number ) ;

};


/* routine associata al tasto "arrow <=" per I (left) */

void ma2q3q_I_left( magnet_number )
    int magnet_number ;

{
extern int verbose ;

extern void ma_current_upd(), ma_current_cmd() ;

float d_value, i_value ;

int i_int_value, limit, new_i1, new_i2, supply_number ;

char string[30];
Arg arg ;

d_value = magnet[ magnet_number ].d_value ;

if ( verbose )
    printf("I left\n") ;

if ( d_value == 0.0 )
    limit = 999999 ;
else {
    if ( d_value > 0.0 )
        limit = (int)( 1000000.0 / ( 1.0 + d_value ) + 0.5 ) ;
    else
        limit = (int)( 1000000.0 / ( 1.0 - d_value ) + 0.5 ) ;
    if ( limit > 999999 ) limit = 999999 ;
    }

i_value = magnet[ magnet_number ].i_value -
          (float)ma2q3q_step_Ik / 10000.0 ; /* in 0.0-1.0 */
if ( i_value < 0.0 ) i_value = 0.0 ;
i_int_value = (int)( i_value * 1000000.0 + 0.5 ) ;
if ( i_int_value > limit ) i_int_value = limit ;
i_value = (float)i_int_value / 1000000.0 ;
new_i1 = (int) ( (float)i_int_value * ( 1.0 + d_value ) + 0.5 ) ;
new_i2 = (int) ( (float)i_int_value * ( 1.0 - d_value ) + 0.5 ) ;
if ( new_i1 >= 999999 ) new_i1 = 999999 ;
if ( new_i2 >= 999999 ) new_i2 = 999999 ;
if ( new_i1 < 0 ) new_i1 = 0 ;
if ( new_i2 < 0 ) new_i2 = 0 ;

magnet[ magnet_number ].i_value = i_value ;
sprintf( string, " Actual  I  :  %7.2f%%", i_value*100.0 ) ;
XtSetArg( arg, XtNlabel, string ) ;
XtSetValues( ma2q3q_I_lab, &arg, 1 ) ;
dimensione_cursore = 0.01;
posizione_cursore = i_value ;
        if (sizeof(float) > sizeof(XtArgVal))
                {
                XtSetArg(args[0], XtNshown, &dimensione_cursore);
                XtSetArg(args[1], XtNtopOfThumb, &posizione_cursore);
                }
        else {
             XtArgVal * I_dimensione_cursore = (XtArgVal *) &dimensione_cursore;
             XtArgVal * I_posizione_cursore = (XtArgVal *) &posizione_cursore;
             XtSetArg(args[0], XtNshown, *I_dimensione_cursore);
             XtSetArg(args[1], XtNtopOfThumb, *I_posizione_cursore);
             }
             XtSetArg(args[2], XtNlength, 500);
             XtSetValues(ma2q3q_I_ruler, args,3);


/*printf( "ma 2q 3q I right: %d i1: %d i2: %d\n",
        i_int_value, new_i1, new_i2 ) ;*/

supply_number = magnet[ magnet_number ].alim1  ;
supply[ supply_number ].wa_current = new_i1 ;
supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
ma_current_cmd( supply_number, new_i1 ) ;
ma_current_upd( supply_number ) ;

supply_number = magnet[ magnet_number ].alim2  ;
supply[ supply_number ].wa_current = new_i2 ;
supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
ma_current_cmd( supply_number, new_i2 ) ;
ma_current_upd( supply_number ) ;

};

/* routine associata al tasto "arrow ^" per I (up) */

void ma2q3q_I_up( magnet_number )
    int magnet_number ;

{
    Arg arg;
    char string[8] ;

    if ( ma2q3q_step_Ik < 25 ) {
        switch ( ma2q3q_step_Ik ) {
            case 10:
            ma2q3q_step_Ik = 25 ;
            break ;
            case 5:
            ma2q3q_step_Ik = 10 ;
            break ;
            case 2:
            ma2q3q_step_Ik = 5 ;
            break ;
            default:
            ma2q3q_step_Ik = 2 ;
            break ;
            }
        ma2q3q_min_wa_Ik =           ma2q3q_step_Ik - 1 ;
        ma2q3q_max_wa_Ik = ( 1000000 - ma2q3q_step_Ik ) ;
        }
    else
        if ( ma2q3q_step_Ik < 2000 ) {
            ma2q3q_step_Ik   =           ma2q3q_step_Ik * 2 ;
            ma2q3q_min_wa_Ik =           ma2q3q_step_Ik - 1 ;
            ma2q3q_max_wa_Ik = ( 1000000 - ma2q3q_step_Ik ) ;
            }

    /* aggiornamento label inc/dec */
    sprintf( string, "%5.2f%%", (float)ma2q3q_step_Ik/100.0 ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues( ma2q3q_I_griplab, &arg, 1 ) ;

};


/* routine associata al tasto "arrow v" per I (down) */

void ma2q3q_I_down( magnet_number )
    int magnet_number ;

{
    Arg arg;
    char string[8] ;

    if ( ma2q3q_step_Ik < 50 ) {
        switch ( ma2q3q_step_Ik ) {
            case 25:
            ma2q3q_step_Ik = 10 ;
            break ;
            case 10:
            ma2q3q_step_Ik = 5 ;
            break ;
            case 5:
            ma2q3q_step_Ik = 2 ;
            break ;
            default:
            ma2q3q_step_Ik = 1 ;
            break ;
            }
        ma2q3q_min_wa_Ik =           ma2q3q_step_Ik - 1 ;
        ma2q3q_max_wa_Ik = ( 1000000 - ma2q3q_step_Ik ) ;
        }
    else {
        ma2q3q_step_Ik   =           ma2q3q_step_Ik / 2 ;
        ma2q3q_min_wa_Ik =           ma2q3q_step_Ik - 1 ;
        ma2q3q_max_wa_Ik = ( 1000000 - ma2q3q_step_Ik ) ;
        }

    /* aggiornamento label inc/dec */
    sprintf( string, "%5.2f%%", (float)ma2q3q_step_Ik/100.0 ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues( ma2q3q_I_griplab, &arg, 1 ) ;

};

/* call back della grip area per D  */

void ma2q3q_D_grip_act(w, index, call_data)
    Widget w;
    int index;
    GripCallDataRec *call_data;
{
    XKeyboardControl valore;
    extern int verbose ;
    void ma2q3q_D_right(), ma2q3q_D_left() ;
    void ma2q3q_D_up(), ma2q3q_D_down() ;
    extern int pixel_white, pixel_wheat, pixel_coral ;
    static int inside = FALSE, grabbed = FALSE;
    Arg arg;
    if (!strncmp(*(call_data->params), "enter", 5))  {
       inside = TRUE;
       valore.key_click_percent = 100;
/*
       XChangeKeyboardControl( XtDisplay(w), KBKeyClickPercent, &valore);
*/
       XSetInputFocus( XtDisplay(w), XtWindow(w),
                       RevertToParent, CurrentTime ) ;
       XtSetArg(arg, XtNforeground, pixel_white );
       XtSetValues(w, &arg, 1);
       XSync(XtDisplay(w), FALSE);
       }
    if (!strncmp(*(call_data->params), "leave", 5))  {
       inside = FALSE;
       valore.key_click_percent = 0;
/*
       XChangeKeyboardControl( XtDisplay(w), KBKeyClickPercent, &valore);
*/
       XSync(XtDisplay(w), FALSE);
       XSetInputFocus( XtDisplay(w),
                       XDefaultRootWindow( XtDisplay(w) ),
                       RevertToParent, CurrentTime );
       XtSetArg( arg, XtNforeground, pixel_wheat ) ;
       XtSetValues(w, &arg, 1);
       XSync(XtDisplay(w), FALSE);
       }
    if (!strncmp(*(call_data->params), "right", 5))  {
       XSync( XtDisplay(w), TRUE);
       if (verbose)
           printf( "up magnet %d\n", index ) ;
       ma2q3q_D_right( index ) ;
       }
    if (!strncmp(*(call_data->params), "left", 4))  {
       XSync( XtDisplay(w), TRUE);
       if (verbose)
           printf( "down magnet %d\n", index ) ;
       ma2q3q_D_left( index ) ;
       }
    if (!strncmp(*(call_data->params), "up", 2))  {
       XSync( XtDisplay(w), TRUE);
       if (verbose)
           printf( "up inc\n" ) ;
       ma2q3q_D_up( index ) ;
       }
    if (!strncmp(*(call_data->params), "down", 4))  {
       XSync( XtDisplay(w), TRUE);
       if (verbose)
           printf( "down inc\n" ) ;
       ma2q3q_D_down( index ) ;
       }
}

/* end call back della grip area per D */

/* routine associata al tasto "arrow =>" per D (right) */

void ma2q3q_D_right( magnet_number )
    int magnet_number ;

{
extern void ma_current_upd(), ma_current_cmd() ;

int i1, i2, new_i1, new_i2, supply_number ;
float i_value, d_value, limit ;

float value ;

char string[30];
Arg arg ;

if ( verbose )
    printf("D right\n") ;

i_value = magnet[ magnet_number ].i_value ;

if ( i_value > 0.5 )
    limit =   ( 1.0 / i_value ) - 1.0 ;
else
    limit =  1.0 ;

d_value = magnet[ magnet_number ].d_value +
          (float)ma2q3q_step_Dk / 10000.0 ;
if ( d_value > 1.0 ) d_value = 1.0 ;

if ( d_value >= 0 ) {
    if ( d_value > limit )
        d_value = limit ;
    }
else {
    if ( d_value < ((-1.0)*limit) )
        d_value = ((-1.0)*limit) ;
    }

new_i1 = (int) ( 1000000.0 * i_value * ( 1.0 + d_value ) + 0.5 ) ;
new_i2 = (int) ( 1000000.0 * i_value * ( 1.0 - d_value ) + 0.5 ) ;
if ( new_i1 > 999999 ) new_i1 = 999999 ;
if ( new_i2 > 999999 ) new_i2 = 999999 ;
if ( new_i1 < 0 ) new_i1 = 0 ;
if ( new_i2 < 0 ) new_i2 = 0 ;

magnet[ magnet_number ].d_value = d_value ;

dimensione_cursore = 0.01;
posizione_cursore =  ( d_value + 1.0 ) / 2.0 ;
        if (sizeof(float) > sizeof(XtArgVal))
                {
                XtSetArg(args[0], XtNshown, &dimensione_cursore);
                XtSetArg(args[1], XtNtopOfThumb, &posizione_cursore);
                }
        else {
             XtArgVal * I_dimensione_cursore = (XtArgVal *) &dimensione_cursore;
             XtArgVal * I_posizione_cursore = (XtArgVal *) &posizione_cursore;
             XtSetArg(args[0], XtNshown, *I_dimensione_cursore);
             XtSetArg(args[1], XtNtopOfThumb, *I_posizione_cursore);
             }
             XtSetArg(args[2], XtNlength, 500);
             XtSetValues(ma2q3q_D_ruler, args,3);

sprintf( string, " Actual  D  :  %+7.4f", d_value ) ;
XtSetArg( arg, XtNlabel, string ) ;
XtSetValues( ma2q3q_D_lab, &arg, 1 ) ;

/*printf( "ma 2q 3q D right: %f i1: %d i2: %d\n",
        d_value, new_i1, new_i2 ) ;*/

supply_number = magnet[ magnet_number ].alim1  ;
supply[ supply_number ].wa_current = new_i1 ;
supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
ma_current_cmd( supply_number, new_i1 ) ;
ma_current_upd( supply_number ) ;

supply_number = magnet[ magnet_number ].alim2  ;
supply[ supply_number ].wa_current = new_i2 ;
supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
ma_current_cmd( supply_number, new_i2 ) ;
ma_current_upd( supply_number ) ;

};


/* routine associata al tasto "arrow <=" per D (left) */

void ma2q3q_D_left( magnet_number )
    int magnet_number ;

{
extern void ma_current_upd(), ma_current_cmd() ;

int i1, i2, new_i1, new_i2, supply_number ;
float i_value, d_value, limit ;

float value ;

char string[30];
Arg arg ;

if ( verbose )
    printf("D left\n") ;

i_value = magnet[ magnet_number ].i_value ;

if ( i_value > 0.5 )
    limit =   ( 1.0 / i_value ) - 1.0 ;
else
    limit =  1.0 ;

d_value = magnet[ magnet_number ].d_value -
          (float)ma2q3q_step_Dk / 10000.0 ;
if ( d_value < -1.0 ) d_value = -1.0 ;

if ( d_value >= 0 ) {
    if ( d_value > limit )
        d_value = limit ;
    }
else {
    if ( d_value < ((-1.0)*limit) )
        d_value = ((-1.0)*limit) ;
    }

new_i1 = (int) ( 1000000.0 * i_value * ( 1.0 + d_value ) + 0.5 ) ;
new_i2 = (int) ( 1000000.0 * i_value * ( 1.0 - d_value ) + 0.5 ) ;
if ( new_i1 > 999999 ) new_i1 = 999999 ;
if ( new_i2 > 999999 ) new_i2 = 999999 ;
if ( new_i1 < 0 ) new_i1 = 0 ;
if ( new_i2 < 0 ) new_i2 = 0 ;

magnet[ magnet_number ].d_value = d_value ;

dimensione_cursore = 0.01;
posizione_cursore = ( d_value + 1.0 ) / 2.0 ;
        if (sizeof(float) > sizeof(XtArgVal))
                {
                XtSetArg(args[0], XtNshown, &dimensione_cursore);
                XtSetArg(args[1], XtNtopOfThumb, &posizione_cursore);
                }
        else {
             XtArgVal * I_dimensione_cursore = (XtArgVal *) &dimensione_cursore;
             XtArgVal * I_posizione_cursore = (XtArgVal *) &posizione_cursore;
             XtSetArg(args[0], XtNshown, *I_dimensione_cursore);
             XtSetArg(args[1], XtNtopOfThumb, *I_posizione_cursore);
             }
             XtSetArg(args[2], XtNlength, 500);
             XtSetValues(ma2q3q_D_ruler, args,3);

sprintf( string, " Actual  D  :  %+7.4f", d_value ) ;
XtSetArg( arg, XtNlabel, string ) ;
XtSetValues( ma2q3q_D_lab, &arg, 1 ) ;

/*printf( "ma 2q 3q D right: %f i1: %d i2: %d\n",
        d_value, new_i1, new_i2 ) ;*/

supply_number = magnet[ magnet_number ].alim1  ;
supply[ supply_number ].wa_current = new_i1 ;
supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
ma_current_cmd( supply_number, new_i1 ) ;
ma_current_upd( supply_number ) ;

supply_number = magnet[ magnet_number ].alim2  ;
supply[ supply_number ].wa_current = new_i2 ;
supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
ma_current_cmd( supply_number, new_i2 ) ;
ma_current_upd( supply_number ) ;

};

/* routine associata al tasto "arrow ^" per D (up) */

void ma2q3q_D_up( magnet_number )
    int magnet_number ;

{
    Arg arg;
    char string[8] ;

    if ( ma2q3q_step_Dk < 25 ) {
        switch ( ma2q3q_step_Dk ) {
            case 10:
            ma2q3q_step_Dk = 25 ;
            break ;
            case 5:
            ma2q3q_step_Dk = 10 ;
            break ;
            case 2:
            ma2q3q_step_Dk = 5 ;
            break ;
            default:
            ma2q3q_step_Dk = 2 ;
            break ;
            }
        ma2q3q_min_wa_Dk =           ma2q3q_step_Dk - 1 ;
        ma2q3q_max_wa_Dk = ( 1000000 - ma2q3q_step_Dk ) ;
        }
    else
        if ( ma2q3q_step_Dk < 2000 ) {
            ma2q3q_step_Dk   =           ma2q3q_step_Dk * 2 ;
            ma2q3q_min_wa_Dk =           ma2q3q_step_Dk - 1 ;
            ma2q3q_max_wa_Dk = ( 1000000 - ma2q3q_step_Dk ) ;
            }

    /* aggiornamento label inc/dec */
    sprintf( string, "%6.4f", (float)ma2q3q_step_Dk/10000.0 ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues( ma2q3q_D_griplab, &arg, 1 ) ;

};


/* routine associata al tasto "arrow v" per D (down) */

void ma2q3q_D_down( magnet_number )
    int magnet_number ;

{
    Arg arg;
    char string[8] ;

    if ( ma2q3q_step_Dk < 50 ) {
        switch ( ma2q3q_step_Dk ) {
            case 25:
            ma2q3q_step_Dk = 10 ;
            break ;
            case 10:
            ma2q3q_step_Dk = 5 ;
            break ;
            case 5:
            ma2q3q_step_Dk = 2 ;
            break ;
            default:
            ma2q3q_step_Dk = 1 ;
            break ;
            }
        ma2q3q_min_wa_Dk =           ma2q3q_step_Dk - 1 ;
        ma2q3q_max_wa_Dk = ( 1000000 - ma2q3q_step_Dk ) ;
        }
    else {
        ma2q3q_step_Dk   =           ma2q3q_step_Dk / 2 ;
        ma2q3q_min_wa_Dk =           ma2q3q_step_Dk - 1 ;
        ma2q3q_max_wa_Dk = ( 1000000 - ma2q3q_step_Dk ) ;
        }

    /* aggiornamento label inc/dec */
    sprintf( string, "%6.4f", (float)ma2q3q_step_Dk/10000.0 ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues( ma2q3q_D_griplab, &arg, 1 ) ;

};

void ma2q3q_refresh()
{
extern int verbose ;
int i1, i2 ;
float i_value, d_value ;
char string[30];
Arg arg;

if ( page_type == -1 ) return ;


i1 = supply[magnet[numero_bottone].alim1].wa_current ;
i2 = supply[magnet[numero_bottone].alim2].wa_current ;
i_value = (float)( i1 + i2 ) / 2000000.0 ; /* in [0.0, 1.0] */
d_value = (float)( i1 - i2 ) ;
if ( d_value != 0.0 ) d_value = d_value / (float)( i1 + i2 ) ;
magnet[numero_bottone].i_value = i_value ;
magnet[numero_bottone].d_value = d_value ;

if ( verbose ) {
    printf( "ma2q3q refresh: magnet is %d\n", numero_bottone ) ;
    printf( "ma2q3q refresh: current 1 is %d\n",
    supply[magnet[numero_bottone].alim1].wa_current ) ;
    printf( "ma2q3q refresh: current 2 is %d\n",
    supply[magnet[numero_bottone].alim2].wa_current ) ;
    printf( "ma2q3q refresh: I is %f, D is %f\n", i_value, d_value ) ;
    }

dimensione_cursore = 0.01;
posizione_cursore = i_value ;
        if (sizeof(float) > sizeof(XtArgVal))
                {
                XtSetArg(args[0], XtNshown, &dimensione_cursore);
                XtSetArg(args[1], XtNtopOfThumb, &posizione_cursore);
                }
        else {
             XtArgVal * I_dimensione_cursore = (XtArgVal *) &dimensione_cursore;
             XtArgVal * I_posizione_cursore = (XtArgVal *) &posizione_cursore;
             XtSetArg(args[0], XtNshown, *I_dimensione_cursore);
             XtSetArg(args[1], XtNtopOfThumb, *I_posizione_cursore);
             }
             XtSetArg(args[2], XtNlength, 500);
             XtSetValues( ma2q3q_I_ruler, args,3);


dimensione_cursore = 0.01;
posizione_cursore =  ( d_value + 1.0 ) / 2.0 ;
        if (sizeof(float) > sizeof(XtArgVal))
                {
                XtSetArg(args[0], XtNshown, &dimensione_cursore);
                XtSetArg(args[1], XtNtopOfThumb, &posizione_cursore);
                }
        else {
             XtArgVal * I_dimensione_cursore = (XtArgVal *) &dimensione_cursore;
             XtArgVal * I_posizione_cursore = (XtArgVal *) &posizione_cursore;
             XtSetArg(args[0], XtNshown, *I_dimensione_cursore);
             XtSetArg(args[1], XtNtopOfThumb, *I_posizione_cursore);
             }
             XtSetArg(args[2], XtNlength, 500);
             XtSetValues( ma2q3q_D_ruler, args,3);


sprintf( string, " Actual  I  :  %7.2f%%", i_value*100.0 ) ;
XtSetArg( arg, XtNlabel, string ) ;
XtSetValues( ma2q3q_I_lab, &arg, 1 ) ;
sprintf( string, " Actual  D  :  %+7.4f", d_value ) ;
XtSetArg( arg, XtNlabel, string ) ;
XtSetValues( ma2q3q_D_lab, &arg, 1 ) ;

} ;

/**********************************************************************/

mafill_2q3qbox( ma_ps1,
                ma_ps2,
                ma2q3qname,
                ma2q3q_box,
                ma2q3q_label,
                ma2q3q_I_lab,
                ma2q3q_D_lab,
                ma2q3q_I_ruler,
                ma2q3q_D_ruler,
                ma2q3q_I_grip,
                ma2q3q_D_grip,
                ma2q3q_I_griplab,
                ma2q3q_D_griplab )

int ma_ps1, ma_ps2 ;
char *ma2q3qname ;
Widget ma2q3q_box,
       *ma2q3q_label,
       *ma2q3q_I_lab,
       *ma2q3q_D_lab,
       *ma2q3q_I_ruler,
       *ma2q3q_D_ruler,
       *ma2q3q_I_grip,
       *ma2q3q_D_grip,
       *ma2q3q_I_griplab,
       *ma2q3q_D_griplab ;

{
Arg arg, args[4] ;
char s[32] ;

XtSetArg( args[0], XtNlength, 500 ) ;
XtSetArg( args[1], XtNthickness, 30 ) ;
XtSetArg( args[2], XtNorientation, XtorientHorizontal ) ;
XtSetArg( args[3], XtNresize, FALSE ) ;

*ma2q3q_label = XtCreateWidget ( "ma2q3q_label",
                                 labelWidgetClass,
                                 ma2q3q_box, NULL, 0 ) ;
s[0] = NULL ;
strcat( s, ma2q3qname ) ;
strcat( s, "  -  I  D" ) ;
XtSetArg( arg, XtNlabel, s ) ;
XtSetValues( *ma2q3q_label, &arg, 1 ) ;
XtManageChild( *ma2q3q_label ) ;

*ma2q3q_I_lab = XtCreateWidget ( "ma2q3q_I_lab",
                                 labelWidgetClass,
                                 ma2q3q_box, NULL, 0 ) ;
XtManageChild( *ma2q3q_I_lab ) ;

*ma2q3q_I_ruler = XtCreateWidget ( "ma2q3q_I_ruler",
                                   scrollbarWidgetClass,
                                   ma2q3q_box, args, 4 );
XtOverrideTranslations( *ma2q3q_I_ruler,
                        XtParseTranslationTable(SCROLLTRANS));
XtAddCallback( *ma2q3q_I_ruler , XtNjumpProc,
               ma2q3q_I_jump, numero_bottone ) ;
XtAddCallback( *ma2q3q_I_ruler , XtNscrollProc,
               ma2q3q_I_scroll, numero_bottone ) ;
XtManageChild( *ma2q3q_I_ruler );

*ma2q3q_D_lab = XtCreateWidget ( "ma2q3q_D_lab",
                                 labelWidgetClass,
                                 ma2q3q_box, NULL, 0 ) ;

XtManageChild( *ma2q3q_D_lab ) ;

*ma2q3q_D_ruler = XtCreateWidget ( "ma2q3q_D_ruler",
                                   scrollbarWidgetClass,
                                   ma2q3q_box, args, 4 );
XtOverrideTranslations( *ma2q3q_D_ruler,
                        XtParseTranslationTable(SCROLLTRANS));
XtAddCallback( *ma2q3q_D_ruler , XtNjumpProc,
               ma2q3q_D_jump, numero_bottone ) ;
XtAddCallback( *ma2q3q_D_ruler , XtNscrollProc,
               ma2q3q_D_scroll, numero_bottone ) ;

dimensione_cursore = 0.01;
posizione_cursore = 0.5 ;
        if (sizeof(float) > sizeof(XtArgVal))
                {
                XtSetArg(args[0], XtNshown, &dimensione_cursore);
                XtSetArg(args[1], XtNtopOfThumb, &posizione_cursore);
                }
        else {
             XtArgVal * I_dimensione_cursore = (XtArgVal *) &dimensione_cursore;
             XtArgVal * I_posizione_cursore = (XtArgVal *) &posizione_cursore;
             XtSetArg(args[0], XtNshown, *I_dimensione_cursore);
             XtSetArg(args[1], XtNtopOfThumb, *I_posizione_cursore);
             }
             XtSetArg(args[2], XtNlength, 500);
             XtSetValues( *ma2q3q_D_ruler, args,3);


XtManageChild( *ma2q3q_D_ruler );

*ma2q3q_I_grip = XtCreateWidget ( "ma2q3q_I_grip",
                                   gripWidgetClass, ma2q3q_box,
                                   NULL, 0 ) ;
XtManageChild( *ma2q3q_I_grip ) ;
XtOverrideTranslations( *ma2q3q_I_grip,
                        XtParseTranslationTable(GRIP_TRANS) ) ;
XtAddCallback( *ma2q3q_I_grip, XtNcallback,
               ma2q3q_I_grip_act, numero_bottone ) ;
*ma2q3q_I_griplab = XtCreateWidget ( "ma2q3q_I_griplab",
                                     labelWidgetClass, ma2q3q_box,
                                     NULL, 0 ) ;
XtManageChild( *ma2q3q_I_griplab ) ;

*ma2q3q_D_grip = XtCreateWidget ( "ma2q3q_D_grip",
                                   gripWidgetClass, ma2q3q_box,
                                   NULL, 0 ) ;
XtManageChild( *ma2q3q_D_grip ) ;
XtOverrideTranslations( *ma2q3q_D_grip,
                        XtParseTranslationTable(GRIP_TRANS) ) ;
XtAddCallback( *ma2q3q_D_grip, XtNcallback,
               ma2q3q_D_grip_act, numero_bottone ) ;
*ma2q3q_D_griplab = XtCreateWidget ( "ma2q3q_D_griplab",
                                     labelWidgetClass, ma2q3q_box,
                                     NULL, 0 ) ;
XtManageChild( *ma2q3q_D_griplab ) ;

XtSetArg( arg, XtNfromVert, *ma2q3q_label );
XtSetValues( *ma2q3q_I_grip , &arg, 1 ) ;
XtSetValues( *ma2q3q_I_griplab , &arg, 1 ) ;
XtSetValues( *ma2q3q_I_lab , &arg, 1 ) ;
XtSetValues( *ma2q3q_I_ruler , &arg, 1 ) ;
XtSetArg( arg, XtNfromHoriz, *ma2q3q_I_grip );
XtSetValues( *ma2q3q_I_griplab , &arg, 1 ) ;
XtSetArg( arg, XtNfromHoriz, *ma2q3q_I_griplab );
XtSetValues( *ma2q3q_I_lab , &arg, 1 ) ;
XtSetArg( arg, XtNfromHoriz, *ma2q3q_I_lab );
XtSetValues( *ma2q3q_I_ruler , &arg, 1 ) ;

XtSetArg( arg, XtNfromVert, *ma2q3q_I_grip );
XtSetValues( *ma2q3q_D_grip , &arg, 1 ) ;
XtSetValues( *ma2q3q_D_griplab , &arg, 1 ) ;
XtSetValues( *ma2q3q_D_lab , &arg, 1 ) ;
XtSetValues( *ma2q3q_D_ruler , &arg, 1 ) ;
XtSetArg( arg, XtNfromHoriz, *ma2q3q_D_grip );
XtSetValues( *ma2q3q_D_griplab , &arg, 1 ) ;
XtSetArg( arg, XtNfromHoriz, *ma2q3q_D_griplab );
XtSetValues( *ma2q3q_D_lab , &arg, 1 ) ;
XtSetArg( arg, XtNfromHoriz, *ma2q3q_D_lab );
XtSetValues( *ma2q3q_D_ruler , &arg, 1 ) ;

sprintf( s, "%5.2f%%", (float)ma2q3q_step_Ik/100.0 ) ;
XtSetArg( arg, XtNlabel, s ) ;
XtSetValues( *ma2q3q_I_griplab, &arg, 1 ) ;
sprintf( s, "%6.4f", (float)ma2q3q_step_Dk/10000.0 ) ;
XtSetArg( arg, XtNlabel, s ) ;
XtSetValues( *ma2q3q_D_griplab, &arg, 1 ) ;

}
