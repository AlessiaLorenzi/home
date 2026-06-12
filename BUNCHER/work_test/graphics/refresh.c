
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/BulletinB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include <Xm/ArrowB.h>
#include <sys/types.h>l
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#include "creation-c.h"
#include "defs.h"
#include "w_defs.h"

refresh ()
{

char cmd[64];
char stringa[64];
char stringa1[64];
char namestring[64];
char frame_to_r[1024];

Arg args[10];
Arg arrg[10];
Arg arrg1[10];
Boolean  argok=False;
int ac = 0;
int value;
int      slider_size;
int      increment;
int      page_increment;
Boolean  notify;
extern display_error();



XtSetArg(args[2], XmNbackground, CONVERT(pushButton29, "#00ff00", XmRPixel, 0, &argok));

if ( debug_verbose ) printf ("\n>>>>>>>  sono in refresh: pdr = %d  tipo = %d \n", pdr, sottop_tipo[pdr]);
if ( soft_verbose ) printf ("\n>>>>>>>  sono in refresh: pdr = %d  tipo = %d  nome = %s\n", pdr, sottop_tipo[pdr], etichetta[pdr]);

/*
if ( displ_errors_onoff == 1 )
	display_interval = XtAppAddTimeOut(XtWidgetToApplicationContext(bulletinBoard), 5, display_error, NULL);
*/
if ( sottop_tipo[pdr] == 0 )
        {
	/* BUNCHER */




if (rf_is_alive[pdr] == 0 )
        {
        XtSetArg(args[0], XmNlabelString, XmStringCreate(primo_messaggio[pdr], XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label140, args, 1);
        XtUnmanageChild(form3);
        XtUnmanageChild(bulletinBoard22);
        XtUnmanageChild(bulletinBoard20);
        XtManageChild(form20);
        }
        else
        {
        XtUnmanageChild(form20);
        XtManageChild(bulletinBoard22);
        XtManageChild(bulletinBoard20);
        XtManageChild(form3);
        }
if ( tu_is_alive[pdr] == 0 )
        {
        XtSetArg(args[0], XmNlabelString, XmStringCreate(tu_primo_messaggio[pdr], XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label186, args, 1);
        XtUnmanageChild(form19);
        XtManageChild(form24);
        }
else
        {
        XtManageChild(form19);
        XtUnmanageChild(form24);
        }



	/* etichetta col nome */
	XtSetArg(args[0], XmNlabelString, XmStringCreate(etichetta[pdr], XmFONTLIST_DEFAULT_TAG));
	XtSetValues(label48, args, 1);


	/* field */
	sprintf(stringa1,"%3.0f",field[pdr] * max_field[pdr]+min_field[pdr]);
	strcpy(namestring,"ampl. (V): ");
	strcat(namestring,stringa1);
        XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label62, args, 1);

	XmScrollBarGetValues (scrollBar15, &value, &slider_size, &increment, &page_increment);
	XmScrollBarSetValues (scrollBar15, (int)(field[pdr]*4096), slider_size, increment, page_increment, FALSE); 

	/* loop phase */
	sprintf(stringa1,"%4d",loop[pdr]);
	strcpy(namestring,"loop ph. : ");
	strcat(namestring,stringa1);
        XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label67, args, 1);

        XmScrollBarGetValues (scrollBar16, &value, &slider_size, &increment, &page_increment);
        XmScrollBarSetValues (scrollBar16, loop[pdr], slider_size, increment, page_increment, FALSE);

	/* quiescent power */ 
	sprintf(stringa1,"%4d",power[pdr]*100/500);
	strcpy(namestring,"power : ");
	strcat(namestring,stringa1);
        XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label71, args, 1);

        XmScrollBarGetValues (scrollBar17, &value, &slider_size, &increment, &page_increment);
        XmScrollBarSetValues (scrollBar17, power[pdr], slider_size, increment, page_increment, FALSE);
	
	/* attenuazione */
	sprintf(stringa1,"-%d",64-attenuation[pdr]*2);
	strcpy(namestring,"atten. (dB): ");
	strcat(namestring,stringa1);
        XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label75, args, 1);

        XmScrollBarGetValues (scrollBar18, &value, &slider_size, &increment, &page_increment);
        XmScrollBarSetValues (scrollBar18, attenuation[pdr], slider_size, increment, page_increment, FALSE);

	/* amplitude gain */
	sprintf(stringa1,"%4d",a_gain[pdr]);
	strcpy(namestring,"ampl gain : ");
	strcat(namestring,stringa1);
        XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label79, args, 1);

        XmScrollBarGetValues (scrollBar19, &value, &slider_size, &increment, &page_increment);
        XmScrollBarSetValues (scrollBar19, a_gain[pdr], slider_size, increment, page_increment, FALSE);

	/* phase gain */
	sprintf(stringa1,"%4d",f_gain[pdr]);
	strcpy(namestring," phase gain : ");
	strcat(namestring,stringa1);
        XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label83, args, 1);

        XmScrollBarGetValues (scrollBar20, &value, &slider_size, &increment, &page_increment);
        XmScrollBarSetValues (scrollBar20, f_gain[pdr], slider_size, increment, page_increment, FALSE);

	/* tasto loop power */
        if ( lp_onoff[pdr] == SPENTO )
                {
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("loop pow.:OFF", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton29, ":::t\"loop power\"\"OFF\"", XmRXmString, 0, &argok));
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton29, "#00ff00", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton29, "black", XmRPixel, 0, &argok));
                }

        else
                {
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("loop pow.:ON", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton29, ":::t\"loop power\"\"ON\"", XmRXmString, 0, &argok));
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton29, "red", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton29, "black", XmRPixel, 0, &argok));
                }

	XtSetValues(pushButton29, args, 3);
	

        /* tasto amplitude feedback */
        if ( af_onoff[pdr] == SPENTO )
                {
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("ampl.feedb.:OFF", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton30, ":::t\"amplitude feedb.\"\"OFF\"", XmRXmString, 0, &argok));
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton30, "#00ff00", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton30, "black", XmRPixel, 0, &argok));
                }

        else
                {
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("ampl.feedb.:ON", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton30, ":::t\"amplitude feedb.\"\"ON\"", XmRXmString, 0, &argok));
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton30, "red", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton30, "black", XmRPixel, 0, &argok));
                }

        XtSetValues(pushButton30, args, 3);

        /* tasto phase feedback */
        if ( pf_onoff[pdr] == SPENTO )
                {
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("ph.feedb.:OFF", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton31, ":::t\"phase feedb.\"\"OFF\"", XmRXmString, 0, &argok));
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton31, "#00ff00", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton31, "black", XmRPixel, 0, &argok));
                }

        else
                {
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("ph.feedb.:ON", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton31, ":::t\"phase feedb.\"\"ON\"", XmRXmString, 0, &argok));
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton31, "red", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton31, "black", XmRPixel, 0, &argok));
                }

        XtSetValues(pushButton31, args, 3);

        /* tasto amplificatore */
        if ( amplifier_onoff[pdr] == SPENTO )
          {
          XtSetArg(args[0], XmNlabelString, XmStringCreate("amplif.:OFF", XmFONTLIST_DEFAULT_TAG));
          XtSetArg(args[2], XmNbackground, CONVERT(pushButton35, "#00ff00", XmRPixel, 0, &argok));
          XtSetArg(args[1], XmNforeground, CONVERT(pushButton35, "black", XmRPixel, 0, &argok));
          }

        else
          {
          XtSetArg(args[0], XmNlabelString, XmStringCreate("amplif.:ON", XmFONTLIST_DEFAULT_TAG));
          XtSetArg(args[2], XmNbackground, CONVERT(pushButton35, "red", XmRPixel, 0, &argok));
          XtSetArg(args[1], XmNforeground, CONVERT(pushButton35, "black", XmRPixel, 0, &argok));
          }

        XtSetValues(pushButton35, args, 3);




	/* tasto soft tuning */

	if ( zero_f_onoff[pdr] == ACCESO )
		{
        	XtSetArg(args[2], XmNbackground, CONVERT(pushButton32, "red", XmRPixel, 0, &argok));
        	XtSetArg(args[1], XmNforeground, CONVERT(pushButton32, "black", XmRPixel, 0, &argok));
        	/*XtSetArg(args[0], XmNlabelString, XmStringCreate("soft tuning:ON", XmFONTLIST_DEFAULT_TAG));*/
 		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton19, ":::t\"soft tuning\"\"ON\"", XmRXmString, 0, &argok));
		}

        if ( zero_f_onoff[pdr] == SPENTO )
                {
        	XtSetArg(args[2], XmNbackground, CONVERT(pushButton32, "#00ff00", XmRPixel, 0, &argok));
        	XtSetArg(args[1], XmNforeground, CONVERT(pushButton32, "black", XmRPixel, 0, &argok));
        	/*XtSetArg(args[0], XmNlabelString, XmStringCreate("soft tuning:OFF", XmFONTLIST_DEFAULT_TAG));*/
 		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton19, ":::t\"soft tuning\"\"OFF\"", XmRXmString, 0, &argok));
		}

        XtSetValues(pushButton32, args, 3);

        /* tasto zero ampl. err. */

        if ( zero_a_onoff[pdr] == ACCESO )
                {
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton81, "red", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton81, "black", XmRPixel, 0, &argok));
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("zero ampl.err.:ON", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton81, ":::t\"zero ampl. err.\"\"ON\"", XmRXmString, 0, &argok));
                }

        if ( zero_a_onoff[pdr] == SPENTO )
                {
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton81, "#00ff00", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton81, "black", XmRPixel, 0, &argok));
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("zero ampl.err.:OFF", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton81, ":::t\"zero ampl. err.\"\"OFF\"", XmRXmString, 0, &argok));
                }

        XtSetValues(pushButton81, args, 3);




        if ( debug_verbose ) printf ("** position = %d  %d  %s",(int)field[pdr]*4096,  pdr, etichetta[pdr]);




    switch ( displ_errors_onoff )
        {
        case ACCESO  :

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton61, "red", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton61, "black", XmRPixel, 0, &argok));
        /*XtSetArg(args[0], XmNlabelString, XmStringCreate("errors:ON", XmFONTLIST_DEFAULT_TAG));*/
	XtSetArg(args[0], XmNlabelString, CONVERT(pushButton61, ":::t\"errors reading\"\"ON\"", XmRXmString, 0, &argok));

        XtSetValues(pushButton61, args, 3);

        break;

        case SPENTO :

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton61, "#00ff00", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton61, "black", XmRPixel, 0, &argok));
        /*XtSetArg(args[0], XmNlabelString, XmStringCreate("erroros:OFF", XmFONTLIST_DEFAULT_TAG));*/
	XtSetArg(args[0], XmNlabelString, CONVERT(pushButton61, ":::t\"errors reading\"\"OFF\"", XmRXmString, 0, &argok));

        XtSetValues(pushButton61, args, 3);

        break;

        }





	}
else
        {
	/* POSTCHOPPER */

if (rf_is_alive[pdr] == 0 ) 
	{
	XtSetArg(args[0], XmNlabelString, XmStringCreate(primo_messaggio[pdr], XmFONTLIST_DEFAULT_TAG));
	XtSetValues(label123, args, 1);
	XtUnmanageChild(form7);
	XtUnmanageChild(bulletinBoard21);
	XtUnmanageChild(bulletinBoard19);
	XtManageChild(form21);
	}
	else 
	{	
	XtUnmanageChild(form21);
	XtManageChild(form7);
	XtManageChild(bulletinBoard21);
	XtManageChild(bulletinBoard19);
	}
if ( tu_is_alive[pdr] == 0 )
	{
        XtSetArg(args[0], XmNlabelString, XmStringCreate(tu_primo_messaggio[pdr], XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label182, args, 1);
	XtUnmanageChild(form9);
	XtManageChild(form23);
	}
else
        {
	XtManageChild(form9);
	XtUnmanageChild(form23);
        }

/*
XtSetValues(pushButton12, arrg1, 1);
XtSetValues(pushButton18, arrg1, 1);
XtSetValues(pushButton17, arrg1, 1);
XtSetValues(pushButton10, arrg1, 1);
*/

        /* etichetta col nome */
	XtSetArg(args[0], XmNlabelString, XmStringCreate(etichetta[pdr], XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label1, args, 1);
	
	/* cursore field */
	sprintf(stringa1,"%3.0f",field[pdr] * max_field[pdr]+min_field[pdr]);
	strcpy(namestring,"amplitude (V): ");
	strcat(namestring,stringa1);
        XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label133, args, 1);

	XmScrollBarGetValues (scrollBar11, &value, &slider_size, &increment, &page_increment);
	XmScrollBarSetValues (scrollBar11, (int)(field[pdr]*4096), slider_size, increment, page_increment, FALSE);


        /* cursore loop phase */
        sprintf(stringa1,"%4d",loop[pdr]);
        strcpy(namestring,"loop ph. : ");
        strcat(namestring,stringa1);
        XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label129, args, 1);

        XmScrollBarGetValues (scrollBar7, &value, &slider_size, &increment, &page_increment);
        XmScrollBarSetValues (scrollBar7, loop[pdr], slider_size, increment, page_increment, FALSE);


        /* cursore quiescent power */ 
        sprintf(stringa1,"%4d",power[pdr]*100/500);
        strcpy(namestring,"power : ");
        strcat(namestring,stringa1);
        XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label125, args, 1);

        XmScrollBarGetValues (scrollBar6, &value, &slider_size, &increment, &page_increment);
        XmScrollBarSetValues (scrollBar6, power[pdr], slider_size, increment, page_increment, FALSE);


        /* cursore amplitude gain */
        sprintf(stringa1,"%4d",a_gain[pdr]);
        strcpy(namestring,"ampl gain : ");
        strcat(namestring,stringa1);
        XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label37, args, 1);

        XmScrollBarGetValues (scrollBar2, &value, &slider_size, &increment, &page_increment);
        XmScrollBarSetValues (scrollBar2, a_gain[pdr], slider_size, increment, page_increment, FALSE);


        /* cursore phase gain */
        sprintf(stringa1,"%4d",f_gain[pdr]);
        strcpy(namestring," phase gain : ");
        strcat(namestring,stringa1);
        XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
        XtSetValues(label30, args, 1);

        XmScrollBarGetValues (scrollBar1, &value, &slider_size, &increment, &page_increment);
        XmScrollBarSetValues (scrollBar1, f_gain[pdr], slider_size, increment, page_increment, FALSE);


        /* tasto loop power */
        if ( lp_onoff[pdr] == SPENTO )
		{
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("loop pow.:OFF", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton58, ":::t\"loop power\"\"OFF\"", XmRXmString, 0, &argok));
	        XtSetArg(args[2], XmNbackground, CONVERT(pushButton58, "#00ff00", XmRPixel, 0, &argok));
		XtSetArg(args[1], XmNforeground, CONVERT(pushButton58, "black", XmRPixel, 0, &argok));
		}

        else
		{
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("loop pow.:ON", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton58, ":::t\"loop power\"\"ON\"", XmRXmString, 0, &argok));
	        XtSetArg(args[2], XmNbackground, CONVERT(pushButton58, "red", XmRPixel, 0, &argok));
	        XtSetArg(args[1], XmNforeground, CONVERT(pushButton58, "black", XmRPixel, 0, &argok));
		}


        XtSetValues(pushButton58, args, 3);



        /* tasto amplitude feedback */
        if ( af_onoff[pdr] == SPENTO )
                {
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("ampl.feedb.:OFF", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton57, ":::t\"amplitude feedb.\"\"OFF\"", XmRXmString, 0, &argok));
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton57, "#00ff00", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton57, "black", XmRPixel, 0, &argok));
                }

        else
                {
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("ampl.feedb.:ON", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton57, ":::t\"amplitude feedb.\"\"ON\"", XmRXmString, 0, &argok));
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton57, "red", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton57, "black", XmRPixel, 0, &argok));
                }


        XtSetValues(pushButton57, args, 3);



        /* tasto phase feedback */
        if ( pf_onoff[pdr] == SPENTO )
                {
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("ph.feedb.:OFF", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton21, ":::t\"phase feedb.\"\"OFF\"", XmRXmString, 0, &argok));
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton21, "#00ff00", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton21, "black", XmRPixel, 0, &argok));
                }

        else
                {
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("ph.feedb.:ON", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton21, ":::t\"phase feedb.\"\"ON\"", XmRXmString, 0, &argok));
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton21, "red", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton21, "black", XmRPixel, 0, &argok));
                }


        XtSetValues(pushButton21, args, 3);



        /* tasto soft tuning */

        if ( zero_f_onoff[pdr] == ACCESO )
                {
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton19, "red", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton19, "black", XmRPixel, 0, &argok));
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("soft tuning:ON", XmFONTLIST_DEFAULT_TAG));*/
 		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton19, ":::t\"soft tuning\"\"ON\"", XmRXmString, 0, &argok));
                }

        if ( zero_f_onoff[pdr] == SPENTO )
                {
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton19, "#00ff00", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton19, "black", XmRPixel, 0, &argok));
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("soft tuning:OFF", XmFONTLIST_DEFAULT_TAG));*/
 		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton19, ":::t\"soft tuning\"\"OFF\"", XmRXmString, 0, &argok));
                }

        XtSetValues(pushButton19, args, 3);


        /* tasto zero ampl. err. */

        if ( zero_a_onoff[pdr] == ACCESO )
                {
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton80, "red", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton80, "black", XmRPixel, 0, &argok));
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("zero ampl.err.:ON", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton80, ":::t\"zero ampl. err.\"\"ON\"", XmRXmString, 0, &argok));
                }

        if ( zero_a_onoff[pdr] == SPENTO )
                {
                XtSetArg(args[2], XmNbackground, CONVERT(pushButton80, "#00ff00", XmRPixel, 0, &argok));
                XtSetArg(args[1], XmNforeground, CONVERT(pushButton80, "black", XmRPixel, 0, &argok));
                /*XtSetArg(args[0], XmNlabelString, XmStringCreate("zero ampl.err.:OFF", XmFONTLIST_DEFAULT_TAG));*/
		XtSetArg(args[0], XmNlabelString, CONVERT(pushButton80, ":::t\"zero ampl. err.\"\"OFF\"", XmRXmString, 0, &argok));
                }

        XtSetValues(pushButton80, args, 3);





        if ( debug_verbose ) printf ("**  position = %d  %d  %s", (int)field[pdr]*4096, pdr, etichetta[pdr]);





    switch ( displ_errors_onoff )
        {
        case ACCESO :

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton60, "red", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton60, "black", XmRPixel, 0, &argok));
        /*XtSetArg(args[0], XmNlabelString, XmStringCreate("errors:ON", XmFONTLIST_DEFAULT_TAG));*/
	XtSetArg(args[0], XmNlabelString, CONVERT(pushButton60, ":::t\"errors reading\"\"ON\"", XmRXmString, 0, &argok));

        XtSetValues(pushButton60, args, 3);

        break;

        case SPENTO :

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton60, "#00ff00", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton60, "black", XmRPixel, 0, &argok));
        /*XtSetArg(args[0], XmNlabelString, XmStringCreate("erroros:OFF", XmFONTLIST_DEFAULT_TAG));*/
	XtSetArg(args[0], XmNlabelString, CONVERT(pushButton60, ":::t\"errors reading\"\"OFF\"", XmRXmString, 0, &argok));

        XtSetValues(pushButton60, args, 3);

        break;

        }




        }

}


chopper_refresh ()
{

char cmd[64];
char stringa[64];
char stringa1[64];
char namestring[64];
char frame_to_r[1024];

Arg args[10];
Arg arrg[10];
Arg arrg1[10];
Boolean  argok=False;
int ac = 0;
int value;
int valore;
int      slider_size;
int      increment;
int      page_increment;
Boolean  notify;

if (debug_verbose) printf ("onoff = %d  intext = %d\n", chopper_onoff, chopper_intext );


if ( chopper_is_alive == 0 )
        {
        XtUnmanageChild(bulletinBoard16);
        XtManageChild(bulletinBoard17);
        }
else
        {
        XtUnmanageChild(bulletinBoard17);
        XtManageChild(bulletinBoard16);
        }




switch ( chopper_onoff )
        {
        case ACCESO :

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton24, "red", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton24, "black", XmRPixel, 0, &argok));
        XtSetArg(args[0], XmNlabelString, XmStringCreate("chopper:ON", XmFONTLIST_DEFAULT_TAG));

        XtSetValues(pushButton24, args, 3);

        break;

        case SPENTO :

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton24, "#00ff00", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton24, "black", XmRPixel, 0, &argok));
        XtSetArg(args[0], XmNlabelString, XmStringCreate("chopper:OFF", XmFONTLIST_DEFAULT_TAG));

        XtSetValues(pushButton24, args, 3);

        break;

	}
switch ( chopper_intext )
        {
        case ACCESO :
        /* esterno */

                XtUnmanageChild(scrollBar13);
                XtUnmanageChild(label45);
                XtUnmanageChild(label121);
                XtUnmanageChild(label38);
                XtUnmanageChild(label39);
                XtUnmanageChild(label40);
                XtUnmanageChild(label41);
                XtUnmanageChild(label42);
                XtUnmanageChild(label43);
                XtUnmanageChild(arrowButton);
                XtUnmanageChild(arrowButton1);
                XtUnmanageChild(scrollBar14);
                XtUnmanageChild(label46);
                XtUnmanageChild(label122);

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton25, "red", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton25, "black", XmRPixel, 0, &argok));
        XtSetArg(args[0], XmNlabelString, XmStringCreate("chopper:ext", XmFONTLIST_DEFAULT_TAG));

        XtSetValues(pushButton25, args, 3);

        break;

        case SPENTO :
        /* interno */

                XtUnmanageChild(scrollBar14);
                XtUnmanageChild(label46);
                XtUnmanageChild(label122);
                XtManageChild(label38);
                XtManageChild(label39);
                XtManageChild(label40);
                XtManageChild(label41);
                XtManageChild(label42);
                XtManageChild(label43);
                XtManageChild(arrowButton);
                XtManageChild(arrowButton1);

if ( chopper_denom <= 7 )
        {
        /* fast pulsing */
        XtManageChild(scrollBar13);
        XtManageChild(label45);
        XtManageChild(label121);
        }

else
        {
        /* slow pulsiong */
        XtManageChild(scrollBar14);
        XtManageChild(label46);
        XtManageChild(label122);
        }

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton25, "#00ff00", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton25, "black", XmRPixel, 0, &argok));
        XtSetArg(args[0], XmNlabelString, XmStringCreate("chopper:int", XmFONTLIST_DEFAULT_TAG));

        XtSetValues(pushButton25, args, 3);

        break;

        }




sprintf(stringa1,"%3d",chopper_denom);
strcpy(namestring,"5/2^");
strcat(namestring,stringa1);
XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
XtSetValues(label39, args, 1);



if ( chopper_denom >= 8 )
       {
       /* aggiorno etichetta */
       XtSetArg(args[2], XmNbackground, CONVERT(label40, "pink", XmRPixel, 0, &argok));
       XtSetArg(args[1], XmNforeground, CONVERT(label40, "black", XmRPixel, 0, &argok));
       XtSetArg(args[0], XmNlabelString, XmStringCreate("slow pulsing", XmFONTLIST_DEFAULT_TAG));
       XtSetValues(label40, args, 3);
       }
else
      {
      /* aggiorno etichetta */
      XtSetArg(args[2], XmNbackground, CONVERT(label40, "#aaaaff", XmRPixel, 0, &argok));
      XtSetArg(args[1], XmNforeground, CONVERT(label40, "black", XmRPixel, 0, &argok));
      XtSetArg(args[0], XmNlabelString, XmStringCreate("fast pulsing", XmFONTLIST_DEFAULT_TAG));
      XtSetValues(label40, args, 3);
      }


/* multiplate delay */

valore=(int)chopper_m_d;
sprintf(stringa,"%4d",valore);
sprintf(namestring,"%4d",chopper_m_d);
XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
XtSetValues(label120, args, 1);

XmScrollBarGetValues (scrollBar, &value, &slider_size, &increment, &page_increment);
XmScrollBarSetValues (scrollBar, valore, slider_size, increment, page_increment, FALSE);


valore=(int)chopper_p_w;
sprintf(stringa,"%4d",valore);
sprintf(namestring,"%4d",chopper_p_w);
XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
XtSetValues(label121, args, 1);

XmScrollBarGetValues (scrollBar13, &value, &slider_size, &increment, &page_increment);
XmScrollBarSetValues (scrollBar13, valore, slider_size, increment, page_increment, FALSE);


valore=(int)chopper_d_c;
sprintf(stringa,"%4d",valore);
sprintf(namestring,"%4d",chopper_d_c);
XtSetArg(args[0], XmNlabelString, XmStringCreate(namestring, XmFONTLIST_DEFAULT_TAG));
XtSetValues(label122, args, 1);

XmScrollBarGetValues (scrollBar14, &value, &slider_size, &increment, &page_increment);
XmScrollBarSetValues (scrollBar14, valore, slider_size, increment, page_increment, FALSE);



}



buncher_small_refresh()
{
char cmd[64];
char stringa[64];
char stringa1[64];
char namestring[64];
char frame_to_r[1024];

Arg args[10];
Arg arrg[10];
Arg arrg1[10];
Boolean  argok=False;
int ac = 0;
int value;
int valore;
int      slider_size;
int      increment;
int      page_increment;
Boolean  notify;


valore = (int)(5000.0*v_centro_new[pdr]);

XmScrollBarGetValues (scrollBar8, &value, &slider_size, &increment, &page_increment);
XmScrollBarSetValues (scrollBar8, valore, slider_size, increment, page_increment, FALSE);

valore = (int)(2500.0*v_delta_new[pdr]);

XmScrollBarGetValues (scrollBar9, &value, &slider_size, &increment, &page_increment);
XmScrollBarSetValues (scrollBar9, valore, slider_size, increment, page_increment, FALSE);


sprintf(stringa1,"%6.4f", v_centro_new[pdr]);
XtSetArg(args[0], XmNlabelString, XmStringCreate(stringa1, XmFONTLIST_DEFAULT_TAG));
XtSetValues(label23, args, 1);

sprintf(stringa1,"%6.4f", v_delta_new[pdr]);
XtSetArg(args[0], XmNlabelString, XmStringCreate(stringa1, XmFONTLIST_DEFAULT_TAG));
XtSetValues(label26, args, 1);


switch ( tubi[pdr] )
    {
    case ACCESO :

    XtSetArg(args[2], XmNbackground, CONVERT(pushButton3, "red", XmRPixel, 0, &argok));
    XtSetArg(args[1], XmNforeground, CONVERT(pushButton3, "black", XmRPixel, 0, &argok));
    XtSetArg(args[0], XmNlabelString, XmStringCreate("tubi lunghi", XmFONTLIST_DEFAULT_TAG));

    XtSetValues(pushButton3, args, 3);

    break;

    case SPENTO :

    XtSetArg(args[2], XmNbackground, CONVERT(pushButton3, "#00ff00", XmRPixel, 0, &argok));
    XtSetArg(args[1], XmNforeground, CONVERT(pushButton3, "black", XmRPixel, 0, &argok));
    XtSetArg(args[0], XmNlabelString, XmStringCreate("tubi corti", XmFONTLIST_DEFAULT_TAG));

    XtSetValues(pushButton3, args, 3);

    break;

    }



XmScrollBarGetValues (scrollBar10, &value, &slider_size, &increment, &page_increment);
XmScrollBarSetValues (scrollBar10,buncher_ph[sub_page_index[pdr]], slider_size, increment, page_increment, FALSE);
sprintf(stringa1,"%d",buncher_ph[sub_page_index[pdr]]);

XtVaSetValues(label29, XmNlabelString, XmStringCreate(stringa1, XmFONTLIST_DEFAULT_TAG), NULL);


XtVaSetValues(label141, XmNlabelString, XmStringCreate(nome_dispositivo[pdr], XmFONTLIST_DEFAULT_TAG), NULL);

}



postchopper_small_refresh()
{
char cmd[64];
char stringa[64];
char stringa1[64];
char namestring[64];
char frame_to_r[1024];

Arg args[10];
Arg arrg[10];
Arg arrg1[10];
Boolean  argok=False;
int ac = 0;
int value;
int valore;
int      slider_size;
int      increment;
int      page_increment;
Boolean  notify;

XmScrollBarGetValues (scrollBar4, &value, &slider_size, &increment, &page_increment);
XmScrollBarSetValues (scrollBar4,(int)(field[sub_page_index[pdr]] * 4095), slider_size, increment, page_increment, FALSE);

        sprintf(stringa1,"%3.0f",field[sub_page_index[pdr]] * max_field[sub_page_index[pdr]]+min_field[sub_page_index[pdr]]);
        XtVaSetValues(label11, XmNlabelString, XmStringCreate(stringa1, XmFONTLIST_DEFAULT_TAG), NULL);


XmScrollBarGetValues (scrollBar5, &value, &slider_size, &increment, &page_increment);
XmScrollBarSetValues (scrollBar5,postchopper_ph[sub_page_index[pdr]], slider_size, increment, page_increment, FALSE);
sprintf(stringa1,"%d",postchopper_ph[sub_page_index[pdr]]);

XtVaSetValues(label14, XmNlabelString, XmStringCreate(stringa1, XmFONTLIST_DEFAULT_TAG), NULL);


XtVaSetValues(label47, XmNlabelString, XmStringCreate(nome_dispositivo[pdr], XmFONTLIST_DEFAULT_TAG), NULL);
}



phase_refresh()
{
char cmd[64];
char stringa[64];
char stringa1[64];
char namestring[64];
char frame_to_r[1024];

Arg args[10];
Arg arrg[10];
Arg arrg1[10];
Boolean  argok=False;
int ac = 0;
int value;
int valore;
int      slider_size;
int      increment;
int      page_increment;
Boolean  notify;


sprintf(stringa, "%6.2f", mass_A);

XtSetArg(args[0], XmNlabelString, XmStringCreate(stringa, XmFONTLIST_DEFAULT_TAG));
XtSetValues(label114, args, 1);

sprintf(stringa, "%6.2f", charge_Q);

XtSetArg(args[0], XmNlabelString, XmStringCreate(stringa, XmFONTLIST_DEFAULT_TAG));
XtSetValues(label115, args, 1);

sprintf(stringa, "%6.2f", energy_E);

XtSetArg(args[0], XmNlabelString, XmStringCreate(stringa, XmFONTLIST_DEFAULT_TAG));
XtSetValues(label116, args, 1);



XmScrollBarGetValues (scrollBar23, &value, &slider_size, &increment, &page_increment);
XmScrollBarSetValues (scrollBar23, pha_loop, slider_size, increment, page_increment, FALSE);
sprintf(stringa1,"%d", pha_loop);

XtVaSetValues(label102, XmNlabelString, XmStringCreate(stringa1, XmFONTLIST_DEFAULT_TAG), NULL);



XmScrollBarGetValues (scrollBar24, &value, &slider_size, &increment, &page_increment);
XmScrollBarSetValues (scrollBar24, pha_f_gain, slider_size, increment, page_increment, FALSE);
sprintf(stringa1,"%d", pha_f_gain);

XtVaSetValues(label106, XmNlabelString, XmStringCreate(stringa1, XmFONTLIST_DEFAULT_TAG), NULL);




XmScrollBarGetValues (scrollBar21, &value, &slider_size, &increment, &page_increment);
XmScrollBarSetValues (scrollBar21, general_phase, slider_size, increment, page_increment, FALSE);
sprintf(stringa1,"gen. ph. %4d", general_phase);

XtVaSetValues(label92, XmNlabelString, XmStringCreate(stringa1, XmFONTLIST_DEFAULT_TAG), NULL);

XmScrollBarGetValues (scrollBar22, &value, &slider_size, &increment, &page_increment);
XmScrollBarSetValues (scrollBar22, le_f_gain, slider_size, increment, page_increment, FALSE);
sprintf(stringa1,"phase gain : %d", le_f_gain);

XtVaSetValues(label97, XmNlabelString, XmStringCreate(stringa1, XmFONTLIST_DEFAULT_TAG), NULL);

XtVaSetValues(label204, XmNlabelString, XmStringCreate(ph_objects[selected_phase_position - 1], XmFONTLIST_DEFAULT_TAG), NULL);





    switch ( ph_stab_onoff )

        {

        case ACCESO :


        XtSetArg(args[2], XmNbackground, CONVERT(pushButton93, "Chocolate3", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton93, "black", XmRPixel, 0, &argok));
        XtSetArg(args[0], XmNlabelString, XmStringCreate("ph. stab on", XmFONTLIST_DEFAULT_TAG));

        XtSetValues(pushButton93, args, 3);
        XtSetValues(pushButton102, args, 3);

        break;

        case SPENTO :


        XtSetArg(args[2], XmNbackground, CONVERT(pushButton93, "Aquamarine4", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton93, "black", XmRPixel, 0, &argok));
        XtSetArg(args[0], XmNlabelString, XmStringCreate("ph. stab off", XmFONTLIST_DEFAULT_TAG));

        XtSetValues(pushButton93, args, 3);
        XtSetValues(pushButton102, args, 3);

        break;

        }



    switch ( ph_lock_onoff )

        {

        case ACCESO :

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton46, "Chocolate3", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton46, "black", XmRPixel, 0, &argok));
        XtSetArg(args[0], XmNlabelString, XmStringCreate("phase locked", XmFONTLIST_DEFAULT_TAG));

        XtSetValues(pushButton46, args, 3);

      break;



        case SPENTO :

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton46, "Aquamarine4", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton46, "black", XmRPixel, 0, &argok));
        XtSetArg(args[0], XmNlabelString, XmStringCreate("phase unlocked", XmFONTLIST_DEFAULT_TAG));

        XtSetValues(pushButton46, args, 3);
      break;

        }


sprintf(stringa, "beta = %f", beta);

XtSetArg(args[0], XmNlabelString, XmStringCreate(stringa, XmFONTLIST_DEFAULT_TAG));
XtSetValues(label117, args, 1);




    switch ( ref_intext )

        {

        case ACCESO :

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton94, "Chocolate3", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton94, "black", XmRPixel, 0, &argok));
        XtSetArg(args[0], XmNlabelString, XmStringCreate("Reference signal : INTERNAL", XmFONTLIST_DEFAULT_TAG));

        XtSetValues(pushButton94, args, 3);

      break;



        case SPENTO :

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton94, "Aquamarine4", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton94, "black", XmRPixel, 0, &argok));
        XtSetArg(args[0], XmNlabelString, XmStringCreate("Reference signal : ALPI", XmFONTLIST_DEFAULT_TAG));

        XtSetValues(pushButton94, args, 3);
      break;

        }


}


chopper_small_refresh()
{
char cmd[64];
char stringa[64];
char stringa1[64];
char namestring[64];
char frame_to_r[1024];
int nb_to_s,nb_to_r;
Arg args[10];
int t_fildes;
int t_par_port;
int valore;
int value;
int      slider_size;
int      increment;
int      page_increment;
Boolean  notify;
Boolean  argok=False;


valore=chopper_phase;
sprintf(stringa1,"%4d",valore);
XtVaSetValues(label35, XmNlabelString, XmStringCreate(stringa1, XmFONTLIST_DEFAULT_TAG), NULL);

XmScrollBarGetValues (scrollBar12, &value, &slider_size, &increment, &page_increment);
XmScrollBarSetValues (scrollBar12, chopper_phase, slider_size, increment, page_increment, FALSE);


switch ( chopper_onoff )
        {
        case ACCESO :

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton20, "red", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton20, "black", XmRPixel, 0, &argok));
        XtSetArg(args[0], XmNlabelString, XmStringCreate("chopper:ON", XmFONTLIST_DEFAULT_TAG));

        XtSetValues(pushButton20, args, 3);

        break;

        case SPENTO :

        XtSetArg(args[2], XmNbackground, CONVERT(pushButton20, "#00ff00", XmRPixel, 0, &argok));
        XtSetArg(args[1], XmNforeground, CONVERT(pushButton20, "black", XmRPixel, 0, &argok));
        XtSetArg(args[0], XmNlabelString, XmStringCreate("chopper:OFF", XmFONTLIST_DEFAULT_TAG));

        XtSetValues(pushButton20, args, 3);

        break;

        }

}

