
#include "alpi.h"
#include "ma_const.h"
#include "ma_ext_vars.h"

#define S8000_TIMEOUT 1500 /* era 1500 */

int timeout_proc()
{
int type, supply_number ;
char string[30];
static int counter = 0;
int semop_retval;


struct sembuf semaphore_struct ;



/* structure used to clear the semaphore (red light) */
semaphore_struct.sem_num = 0 ;
semaphore_struct.sem_op = -1 ;
semaphore_struct.sem_flg = 0 ;






//==============================================================



                semop_retval = semop( semaphore_id, &semaphore_struct, 1 ) ;
                if( semop_retval == -1 )
                        printf( "*1* error in semaphore operation\n" ) ;
                else
                        {


/* if display is off, then return */
if (refresh_onoff == 0) return;

type = button[numero_bottone].type  ;

counter++;
if ( counter == 60 ) /* ciclo di 60 secondi */
    counter = 0 ;
switch ( type )
    {

    case 1:

    supply_number = magnet[button[numero_bottone].obj2].alim1;
    check_ma_alim_a( supply_number, counter ) ;

    break;
        


    case 2:
    case 4:

    supply_number = magnet[button[numero_bottone].obj2].alim1;
    check_ma_alim_a( supply_number, counter ) ;

    supply_number = magnet[button[numero_bottone].obj2].alim2;
    check_ma_alim_b( supply_number, counter ) ;

    break;



    case 3:

    supply_number = magnet[button[numero_bottone].obj1].alim1;
    check_ma_alim_a( supply_number, counter ) ;

    supply_number = magnet[button[numero_bottone].obj2].alim1;
    check_ma_alim_b( supply_number, counter ) ;

    supply_number = magnet[button[numero_bottone].obj3].alim1;
    check_ma_alim_c( supply_number, counter ) ;

    break;
    }


//============================================================

    /* turn on the green light */
                semop_retval = semctl( semaphore_id,  0, SETVAL, 1 ) ;

}

if ( refresh_onoff == 1 )
     interval_id = XtAddTimeOut ( S8000_TIMEOUT, timeout_proc, NULL ) ;

}

/* check supply_number in box A, refresh labels, and (if adc_index)
   check also ADCs */

int check_ma_alim_a( supply_number, adc_index )
int supply_number, adc_index ;
{

int adc ;
char string[30];
Arg arg, args[3], noal_args[2] ;

/* check degli adc, da fare ogni tanto */
switch ( adc_index )
    {

    case 2:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 2", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf( string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_volt_perc_lab_a , &arg, 1 ) ;
    sprintf(string, "%f",
    (float)(adc)*(float)(supply[supply_number].maxvoltage)/100.0);
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_volt_lab_a , &arg, 1 ) ;

    break ;

    case 3:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 3", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_v1_lab_a , &arg, 1 ) ;

    break ;

    case 4:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 4", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_v2_lab_a , &arg, 1 ) ;

    break ;

    case 5:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 5", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_v3_lab_a , &arg, 1 ) ;

    break ;

    case 6:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 6", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_temp_lab_a , &arg, 1 ) ;

    break ;

    case 7:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 7", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_trans_lab_a , &arg, 1 ) ;

    break ;

    }



/* check della corrente, da fare sempre */
sprintf ( to_server ,"ma comm : ALIM = %2d !AD 0",
          supply_number);
write( server_channel, to_server, BUF_SIZE ) ;
read( server_channel, from_server, BUF_SIZE ) ;
sscanf( from_server, "%d", &adc ) ;

sprintf(string, "%d", adc ) ;
XtSetArg( arg, XtNlabel, string ) ;
XtSetValues(ma_curatt_perc_lab_a , &arg, 1 ) ;

sprintf(string, "%f",
       (float)(adc)*(float)(supply[supply_number].maxcurrent)/100.0);
XtSetArg( arg, XtNlabel, string ) ;
XtSetValues(ma_curatt_lab_a , &arg, 1 ) ;



/* check degli allarmi, da fare sempre */
sprintf ( to_server ,"ma comm : ALIM = %2d !S1", supply_number);
write( server_channel, to_server, BUF_SIZE ) ;
read( server_channel, from_server, BUF_SIZE ) ;

        XtSetArg( args[0], XtNbackground, pixel_lightsteelblue3 ) ; /* agg. sett. 2001 */
        XtSetValues(ma_box_a , args, 1 ) ; /* agg. sett. 2001 */

/* status STUB/ON/OFF */
if ( from_server[0] == 'S')
    { /* STUB */
    XtSetArg(args[0], XtNlabel, "STUB" ) ;
    XtSetArg(args[1], XtNbackground, pixel_white ) ;
    XtSetArg(args[2], XtNforeground, pixel_black ) ;
    XtSetValues(ma_onoff_but_a , args, 3 ) ;
    supply[supply_number].on_off = STUB;

        XtSetArg( args[0], XtNbackground, pixel_black ) ;
        XtSetValues(ma_box_a , args, 1 ) ;
    }
else
    if ( from_server[0] == '!')
        { /* OFF */
        XtSetArg( args[0], XtNlabel, "OFF" ) ;
        XtSetArg( args[1], XtNbackground, pixel_grey ) ;
        XtSetValues(ma_onoff_but_a , args, 2 ) ;
        supply[supply_number].on_off = OFF;
        }
    else
        { /* ON */
        XtSetArg(args[0], XtNlabel, "ON" ) ;
        XtSetArg(args[1], XtNbackground, pixel_orange ) ;
        XtSetValues(ma_onoff_but_a , args, 2 ) ;
        supply[supply_number].on_off = ON;
        }



XtSetArg( noal_args[0], XtNlabel, "" ) ;
XtSetArg( noal_args[1], XtNbackground, pixel_grey);

if ( from_server[2] == '!')
    {
    XtSetArg( args[0], XtNlabel, "pol. rev." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_1_a, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_1_a, noal_args, 2 ) ;


if ( from_server[3] == '!')
    {
    XtSetArg( args[0], XtNlabel, "reg. tr. <> 0" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_2_a, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_2_a, noal_args, 2 ) ;


if ( from_server[8] == '!')
    {
    XtSetArg( args[0], XtNlabel, "trans. fault" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_3_a, args, 2 ) ;
    }
else
    {
    XtSetArg( args[0], XtNlabel, "" ) ;
    XtSetArg( args[1], XtNbackground, pixel_grey);
    XtSetValues(ma_status_lab_3_a, noal_args, 2 ) ;
    }


if ( from_server[10] == '!')
    {
    XtSetArg( args[0], XtNlabel, "DC overcurrent" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_4_a, noal_args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_4_a, noal_args, 2 ) ;


if ( from_server[11] == '!')
    {
    XtSetArg( args[0], XtNlabel, "DC overload" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_5_a, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_5_a, noal_args, 2 ) ;


if ( from_server[12] == '!')
    {
    XtSetArg( args[0], XtNlabel, "reg. mod. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_6_a, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_6_a, noal_args, 2 ) ;


if ( from_server[13] == '!')
    {
    XtSetArg( args[0], XtNlabel, "prereg. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_7_a, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_7_a, noal_args, 2 ) ;


if ( from_server[14] == '!')
    {
    XtSetArg( args[0], XtNlabel, "phase fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_8_a, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_8_a, noal_args, 2 ) ;


if ( from_server[15] == '!')
    {
    XtSetArg( args[0], XtNlabel, "MPS wat. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_9_a, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_9_a, noal_args, 2 ) ;


if ( from_server[16] == '!')
    {
    XtSetArg( args[0], XtNlabel, "eath lek. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_10_a, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_10_a, noal_args, 2 ) ;


if ( from_server[17] == '!')
    {
    XtSetArg( args[0], XtNlabel, "th. br./fu." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_11_a, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_11_a, noal_args, 2 ) ;


if ( from_server[18] == '!')
    {
    XtSetArg( args[0], XtNlabel, "MPS overtemp." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_12_a, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_12_a, noal_args, 2 ) ;


if ( from_server[19] == '!')
    {
    XtSetArg( args[0], XtNlabel, "pan. but./door" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_13_a, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_13_a, noal_args, 2 ) ;


if ( from_server[20] == '!')
    {
    XtSetArg( args[0], XtNlabel, "mag w. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_14_a, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_14_a, noal_args, 2 ) ;


if ( from_server[21] == '!')
    {
    XtSetArg( args[0], XtNlabel, "mag. overt." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_15_a, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_15_a, noal_args, 2 ) ;


if ( from_server[22] == '!')
    {
    XtSetArg( args[0], XtNlabel, "MPS not ready" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_16_a, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_16_a, noal_args, 2 ) ;


}

/* check supply_number in box B, refresh labels, and (if adc_index)
   check also ADCs */

int check_ma_alim_b( supply_number, adc_index )
int supply_number, adc_index ;
{

int adc ;
char string[30];
Arg arg, args[3], noal_args[2] ;

/* check degli adc, da fare ogni tanto */
switch ( adc_index )
    {

    case 2:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 2", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf( string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_volt_perc_lab_b , &arg, 1 ) ;
    sprintf(string, "%f",
    (float)(adc)*(float)(supply[supply_number].maxvoltage)/100.0);
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_volt_lab_b , &arg, 1 ) ;

    break ;

    case 3:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 3", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_v1_lab_b , &arg, 1 ) ;

    break ;

    case 4:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 4", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_v2_lab_b , &arg, 1 ) ;

    break ;

    case 5:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 5", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_v3_lab_b , &arg, 1 ) ;

    break ;

    case 6:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 6", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_temp_lab_b , &arg, 1 ) ;

    break ;

    case 7:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 7", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_trans_lab_b , &arg, 1 ) ;

    break ;

    }



/* check della corrente, da fare sempre */
sprintf ( to_server ,"ma comm : ALIM = %2d !AD 0",
          supply_number);
write( server_channel, to_server, BUF_SIZE ) ;
read( server_channel, from_server, BUF_SIZE ) ;
sscanf( from_server, "%d", &adc ) ;

sprintf(string, "%d", adc ) ;
XtSetArg( arg, XtNlabel, string ) ;
XtSetValues(ma_curatt_perc_lab_b , &arg, 1 ) ;

sprintf(string, "%f",
       (float)(adc)*(float)(supply[supply_number].maxcurrent)/100.0);
XtSetArg( arg, XtNlabel, string ) ;
XtSetValues(ma_curatt_lab_b , &arg, 1 ) ;



/* check degli allarmi, da fare sempre */
sprintf ( to_server ,"ma comm : ALIM = %2d !S1", supply_number);
write( server_channel, to_server, BUF_SIZE ) ;
read( server_channel, from_server, BUF_SIZE ) ;



/* status STUB/ON/OFF */
if ( from_server[0] == 'S')
    { /* STUB */
    XtSetArg(args[0], XtNlabel, "STUB" ) ;
    XtSetArg(args[1], XtNbackground, pixel_white ) ;
    XtSetArg(args[2], XtNforeground, pixel_black ) ;
    XtSetValues(ma_onoff_but_b , args, 3 ) ;
    supply[supply_number].on_off = STUB;

        XtSetArg( args[0], XtNbackground, pixel_black ) ;
        XtSetValues(ma_box_b , args, 1 ) ;
    }
else
    if ( from_server[0] == '!')
        { /* OFF */
        XtSetArg( args[0], XtNlabel, "OFF" ) ;
        XtSetArg( args[1], XtNbackground, pixel_grey ) ;
        XtSetValues(ma_onoff_but_b , args, 2 ) ;
        supply[supply_number].on_off = OFF;
        }
    else
        { /* ON */
        XtSetArg(args[0], XtNlabel, "ON" ) ;
        XtSetArg(args[1], XtNbackground, pixel_orange ) ;
        XtSetValues(ma_onoff_but_b , args, 2 ) ;
        supply[supply_number].on_off = ON;
        }



XtSetArg( noal_args[0], XtNlabel, "" ) ;
XtSetArg( noal_args[1], XtNbackground, pixel_grey);

if ( from_server[2] == '!')
    {
    XtSetArg( args[0], XtNlabel, "pol. rev." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_1_b, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_1_b, noal_args, 2 ) ;


if ( from_server[3] == '!')
    {
    XtSetArg( args[0], XtNlabel, "reg. tr. <> 0" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_2_b, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_2_b, noal_args, 2 ) ;


if ( from_server[8] == '!')
    {
    XtSetArg( args[0], XtNlabel, "trans. fault" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_3_b, args, 2 ) ;
    }
else
    {
    XtSetArg( args[0], XtNlabel, "" ) ;
    XtSetArg( args[1], XtNbackground, pixel_grey);
    XtSetValues(ma_status_lab_3_b, noal_args, 2 ) ;
    }


if ( from_server[10] == '!')
    {
    XtSetArg( args[0], XtNlabel, "DC overcurrent" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_4_b, noal_args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_4_b, noal_args, 2 ) ;


if ( from_server[11] == '!')
    {
    XtSetArg( args[0], XtNlabel, "DC overload" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_5_b, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_5_b, noal_args, 2 ) ;


if ( from_server[12] == '!')
    {
    XtSetArg( args[0], XtNlabel, "reg. mod. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_6_b, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_6_b, noal_args, 2 ) ;


if ( from_server[13] == '!')
    {
    XtSetArg( args[0], XtNlabel, "prereg. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_7_b, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_7_b, noal_args, 2 ) ;


if ( from_server[14] == '!')
    {
    XtSetArg( args[0], XtNlabel, "phase fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_8_b, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_8_b, noal_args, 2 ) ;


if ( from_server[15] == '!')
    {
    XtSetArg( args[0], XtNlabel, "MPS wat. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_9_b, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_9_b, noal_args, 2 ) ;


if ( from_server[16] == '!')
    {
    XtSetArg( args[0], XtNlabel, "eath lek. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_10_b, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_10_b, noal_args, 2 ) ;


if ( from_server[17] == '!')
    {
    XtSetArg( args[0], XtNlabel, "th. br./fu." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_11_b, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_11_b, noal_args, 2 ) ;


if ( from_server[18] == '!')
    {
    XtSetArg( args[0], XtNlabel, "MPS overtemp." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_12_b, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_12_b, noal_args, 2 ) ;


if ( from_server[19] == '!')
    {
    XtSetArg( args[0], XtNlabel, "pan. but./door" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_13_b, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_13_b, noal_args, 2 ) ;


if ( from_server[20] == '!')
    {
    XtSetArg( args[0], XtNlabel, "mag w. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_14_b, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_14_b, noal_args, 2 ) ;


if ( from_server[21] == '!')
    {
    XtSetArg( args[0], XtNlabel, "mag. overt." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_15_b, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_15_b, noal_args, 2 ) ;


if ( from_server[22] == '!')
    {
    XtSetArg( args[0], XtNlabel, "MPS not ready" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_16_b, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_16_b, noal_args, 2 ) ;


}

/* check supply_number in box C, refresh labels, and (if adc_index)
   check also ADCs */

int check_ma_alim_c( supply_number, adc_index )
int supply_number, adc_index ;
{

int adc ;
char string[30];
Arg arg, args[3], noal_args[2] ;

/* check degli adc, da fare ogni tanto */
switch ( adc_index )
    {

    case 2:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 2", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf( string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_volt_perc_lab_c , &arg, 1 ) ;
    sprintf(string, "%f",
    (float)(adc)*(float)(supply[supply_number].maxvoltage)/100.0);
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_volt_lab_c , &arg, 1 ) ;

    break ;

    case 3:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 3", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_v1_lab_c , &arg, 1 ) ;

    break ;

    case 4:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 4", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_v2_lab_c , &arg, 1 ) ;

    break ;

    case 5:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 5", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_v3_lab_c , &arg, 1 ) ;

    break ;

    case 6:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 6", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_temp_lab_c , &arg, 1 ) ;

    break ;

    case 7:

    sprintf ( to_server ,"ma comm : ALIM = %2d !AD 7", supply_number ) ;
    write( server_channel, to_server, BUF_SIZE ) ;
    read( server_channel, from_server, BUF_SIZE ) ;
    sscanf ( from_server, " %d", &adc ) ;
    sprintf(string, "%d", adc ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    XtSetValues(ma_trans_lab_c , &arg, 1 ) ;

    break ;

    }



/* check della corrente, da fare sempre */
sprintf ( to_server ,"ma comm : ALIM = %2d !AD 0",
          supply_number);
write( server_channel, to_server, BUF_SIZE ) ;
read( server_channel, from_server, BUF_SIZE ) ;
sscanf( from_server, "%d", &adc ) ;

sprintf(string, "%d", adc ) ;
XtSetArg( arg, XtNlabel, string ) ;
XtSetValues(ma_curatt_perc_lab_c , &arg, 1 ) ;

sprintf(string, "%f",
       (float)(adc)*(float)(supply[supply_number].maxcurrent)/100.0);
XtSetArg( arg, XtNlabel, string ) ;
XtSetValues(ma_curatt_lab_c , &arg, 1 ) ;



/* check degli allarmi, da fare sempre */
sprintf ( to_server ,"ma comm : ALIM = %2d !S1", supply_number);
write( server_channel, to_server, BUF_SIZE ) ;
read( server_channel, from_server, BUF_SIZE ) ;



/* status STUB/ON/OFF */
if ( from_server[0] == 'S')
    { /* STUB */
    XtSetArg(args[0], XtNlabel, "STUB" ) ;
    XtSetArg(args[1], XtNbackground, pixel_white ) ;
    XtSetArg(args[2], XtNforeground, pixel_black ) ;
    XtSetValues(ma_onoff_but_c , args, 3 ) ;
    supply[supply_number].on_off = STUB;
            XtSetArg( args[0], XtNbackground, pixel_black ) ;
            XtSetValues(ma_box_c , args, 1 ) ;
    }
else
    if ( from_server[0] == '!')
        { /* OFF */
        XtSetArg( args[0], XtNlabel, "OFF" ) ;
        XtSetArg( args[1], XtNbackground, pixel_grey ) ;
        XtSetValues(ma_onoff_but_c , args, 2 ) ;
        supply[supply_number].on_off = OFF;
        }
    else
        { /* ON */
        XtSetArg(args[0], XtNlabel, "ON" ) ;
        XtSetArg(args[1], XtNbackground, pixel_orange ) ;
        XtSetValues(ma_onoff_but_c , args, 2 ) ;
        supply[supply_number].on_off = ON;
        }



XtSetArg( noal_args[0], XtNlabel, "" ) ;
XtSetArg( noal_args[1], XtNbackground, pixel_grey);

if ( from_server[2] == '!')
    {
    XtSetArg( args[0], XtNlabel, "pol. rev." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_1_c, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_1_c, noal_args, 2 ) ;


if ( from_server[3] == '!')
    {
    XtSetArg( args[0], XtNlabel, "reg. tr. <> 0" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_2_c, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_2_c, noal_args, 2 ) ;


if ( from_server[8] == '!')
    {
    XtSetArg( args[0], XtNlabel, "trans. fault" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_3_c, args, 2 ) ;
    }
else
    {
    XtSetArg( args[0], XtNlabel, "" ) ;
    XtSetArg( args[1], XtNbackground, pixel_grey);
    XtSetValues(ma_status_lab_3_c, noal_args, 2 ) ;
    }


if ( from_server[10] == '!')
    {
    XtSetArg( args[0], XtNlabel, "DC overcurrent" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_4_c, noal_args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_4_c, noal_args, 2 ) ;


if ( from_server[11] == '!')
    {
    XtSetArg( args[0], XtNlabel, "DC overload" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_5_c, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_5_c, noal_args, 2 ) ;


if ( from_server[12] == '!')
    {
    XtSetArg( args[0], XtNlabel, "reg. mod. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_6_c, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_6_c, noal_args, 2 ) ;


if ( from_server[13] == '!')
    {
    XtSetArg( args[0], XtNlabel, "prereg. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_7_c, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_7_c, noal_args, 2 ) ;


if ( from_server[14] == '!')
    {
    XtSetArg( args[0], XtNlabel, "phase fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_8_c, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_8_c, noal_args, 2 ) ;


if ( from_server[15] == '!')
    {
    XtSetArg( args[0], XtNlabel, "MPS wat. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_9_c, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_9_c, noal_args, 2 ) ;


if ( from_server[16] == '!')
    {
    XtSetArg( args[0], XtNlabel, "eath lek. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_10_c, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_10_c, noal_args, 2 ) ;


if ( from_server[17] == '!')
    {
    XtSetArg( args[0], XtNlabel, "th. br./fu." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_11_c, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_11_c, noal_args, 2 ) ;


if ( from_server[18] == '!')
    {
    XtSetArg( args[0], XtNlabel, "MPS overtemp." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_12_c, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_12_c, noal_args, 2 ) ;


if ( from_server[19] == '!')
    {
    XtSetArg( args[0], XtNlabel, "pan. but./door" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_13_c, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_13_c, noal_args, 2 ) ;


if ( from_server[20] == '!')
    {
    XtSetArg( args[0], XtNlabel, "mag w. fail." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_14_c, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_14_c, noal_args, 2 ) ;


if ( from_server[21] == '!')
    {
    XtSetArg( args[0], XtNlabel, "mag. overt." ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_15_c, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_15_c, noal_args, 2 ) ;


if ( from_server[22] == '!')
    {
    XtSetArg( args[0], XtNlabel, "MPS not ready" ) ;
    XtSetArg( args[1], XtNbackground, pixel_red);
    XtSetValues(ma_status_lab_16_c, args, 2 ) ;
    }
else
    XtSetValues(ma_status_lab_16_c, noal_args, 2 ) ;


}

