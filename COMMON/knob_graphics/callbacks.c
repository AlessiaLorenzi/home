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

char comando_di_stampa[50];
int cnt;
extern void ma2q3q_refresh();
FILE *file_to_write;
FILE *file_to_print;
FILE *command_file;

static ma_step_key = MA_STEP_KEY ;
static ma_min_wa_key = MA_MIN_WA_KEY ;
static ma_max_wa_key = MA_MAX_WA_KEY ;




void pr_maread_data(string)
char *string;
{
int cnt, cnt1, value_to_send, corrente ;
FILE *file_to_read;
char stringa[100];

sprintf(stringa, "%s%s", "./dati_fascio/", string);
file_to_read = fopen(stringa,"r");


if ( file_to_read != NULL )
        {
        for (cnt = 0; cnt<100; cnt++)
                {
                fscanf( file_to_read, "%d%f%d%d\n",&cnt1,
                        &(pr_supply[cnt].current),
                (&pr_supply[cnt].on_off), &(pr_supply[cnt].mode) );
                }

        for (cnt = 0; cnt<100; cnt++)
                {
                fscanf( file_to_read, "%d%f%d\n",&cnt1,
                &(pr_steerer_supply[cnt].current),
                &(pr_steerer_supply[cnt].mode) );
                }

        fclose(file_to_read);
        }
if ( file_to_read == NULL )
        printf ("\007WARNING : FILE DOES NOT EXIST \n");


}





void maprint_data(string)
char *string;
{

char comando_di_stampa[90], form1[90], form2[90], form3[90];
int cnt;
FILE *file_to_print;
FILE *command_file;


command_file = fopen("print_commands","r");

fgets( comando_di_stampa, 90, command_file );

fgets( form1, 90, command_file );
fgets( form2, 90, command_file );
fgets( form3, 90, command_file );
printf("  >>>>>>> %s\n%s\n%s\n", form1, form2, form3 );

fclose(command_file);


pr_maread_data(string);


system("date > file_to_print");

file_to_print = fopen("file_to_print","a");
fprintf(file_to_print, "\n");

fprintf(file_to_print,"MAGNETS :\n\n");


for (cnt = 1; cnt<numero_bottoni+1 ; cnt++) /* SOSTITUITO */
	{
	if ( ( magnet[cnt].alim2 != 9999 ) && ( magnet[cnt].magnet != 999999 ) )
		{
		if (pr_supply[magnet[cnt].alim1].mode == 1 && pr_supply[magnet[cnt].alim1].on_off == 1 &&
		pr_supply[magnet[cnt].alim2].mode == 1 && pr_supply[magnet[cnt].alim2].on_off == 1)
			{
			if (     (pr_supply[magnet[cnt].alim1].current + pr_supply[magnet[cnt].alim2].current) != 0.0)
			fprintf(file_to_print,"%8s I  :  %7.2f%%  D  :  %+7.4f \n",magnet[cnt].name, 
			100.0*(pr_supply[magnet[cnt].alim1].current + pr_supply[magnet[cnt].alim2].current)/
			(2.0 * supply[magnet[cnt].alim1].maxcurrent),
			(pr_supply[magnet[cnt].alim1].current - pr_supply[magnet[cnt].alim2].current)
			/(pr_supply[magnet[cnt].alim1].current + pr_supply[magnet[cnt].alim2].current));
			else
			fprintf(file_to_print,"%8s   alimentat. entrambi a 0 \n", magnet[cnt].name);
			fprintf(file_to_print,"\n");
			}
		else
			{
			if (pr_supply[magnet[cnt].alim1].mode == 1 &&
			pr_supply[magnet[cnt].alim1].on_off == 1)
				fprintf(file_to_print,form1, 
				magnet[cnt].n_alim1, pr_supply[magnet[cnt].alim1].current,
				pr_supply[magnet[cnt].alim1].current*100.0/supply[magnet[cnt].alim1].maxcurrent);
                    
			if (pr_supply[magnet[cnt].alim1].mode == 1 &&
			pr_supply[magnet[cnt].alim1].on_off == 0)
				fprintf(file_to_print,form2, 
				magnet[cnt].n_alim1, pr_supply[magnet[cnt].alim1].current,
				pr_supply[magnet[cnt].alim1].current*100.0/supply[magnet[cnt].alim1].maxcurrent);

			if (pr_supply[magnet[cnt].alim1].mode == 0)
				fprintf(file_to_print, form3, magnet[cnt].n_alim1);


			if (pr_supply[magnet[cnt].alim2].mode == 1 &&
			pr_supply[magnet[cnt].alim2].on_off == 1)
				fprintf(file_to_print,form1, 
				magnet[cnt].n_alim2, pr_supply[magnet[cnt].alim2].current,
				pr_supply[magnet[cnt].alim2].current*100.0/supply[magnet[cnt].alim2].maxcurrent);
                                
			if (pr_supply[magnet[cnt].alim2].mode == 1 &&
			pr_supply[magnet[cnt].alim2].on_off == 0)
				fprintf(file_to_print,form2, 
				magnet[cnt].n_alim2, pr_supply[magnet[cnt].alim2].current,
				pr_supply[magnet[cnt].alim2].current*100.0/supply[magnet[cnt].alim2].maxcurrent);

			if (pr_supply[magnet[cnt].alim2].mode == 0)
				fprintf(file_to_print,form3, magnet[cnt].n_alim2);

			fprintf(file_to_print, "\n");

			}
		}
	else
		{
		if (pr_supply[magnet[cnt].alim1].mode == 1 &&
		pr_supply[magnet[cnt].alim1].on_off == 1)
			fprintf(file_to_print,form1, 
			magnet[cnt].n_alim1, pr_supply[magnet[cnt].alim1].current,
			pr_supply[magnet[cnt].alim1].current*100.0/supply[magnet[cnt].alim1].maxcurrent);
                    
		if (pr_supply[magnet[cnt].alim1].mode == 1 &&
		pr_supply[magnet[cnt].alim1].on_off == 0)
			fprintf(file_to_print,form2, 
			magnet[cnt].n_alim1, pr_supply[magnet[cnt].alim1].current,
			pr_supply[magnet[cnt].alim1].current*100.0/supply[magnet[cnt].alim1].maxcurrent);

		if (pr_supply[magnet[cnt].alim1].mode == 0)
			fprintf(file_to_print, form3, magnet[cnt].n_alim1);
		}
	}






fprintf(file_to_print,"\n\nSTEERERS :\n\n");

for ( cnt = 1; cnt < numero_bottoni + 1; cnt++ )
        
    {

    if ( button[cnt].type == 2 )
        {
        if ( button[cnt].obj1 != 9999 &&
	     button[cnt].obj1 != 999999 )
            {
            if ( pr_steerer_supply[steerer[button[cnt].obj1].alim1].mode == 0 )
                fprintf(file_to_print, "%-10s  STUB\n", 
                        steerer[button[cnt].obj1].name);

            else
                fprintf(file_to_print, "%-10s  %-5.2f\n", 
                    steerer[button[cnt].obj1].name,
                    pr_steerer_supply[steerer[button[cnt].obj1].alim1].current);


            if ( pr_steerer_supply[steerer[button[cnt].obj1].alim2].mode == 0 )
                fprintf(file_to_print, "            STUB\n");


            else
                fprintf(file_to_print, "            %-5.2f\n", 
                    pr_steerer_supply[steerer[button[cnt].obj1].alim2].current);

 
            }
    
        if ( button[cnt].obj3 != 9999 && 
	     button[cnt].obj3 != 999999 )
            {
            if ( pr_steerer_supply[steerer[button[cnt].obj3].alim1].mode == 0 )
                fprintf(file_to_print, "%-10s  STUB\n", 
                        steerer[button[cnt].obj3].name);
            
            else
                fprintf(file_to_print, "%-10s  %-5.2f\n", 
                    steerer[button[cnt].obj3].name,
                    pr_steerer_supply[steerer[button[cnt].obj3].alim1].current);

            if ( pr_steerer_supply[steerer[button[cnt].obj3].alim2].mode == 0 )
                fprintf(file_to_print, "            STUB\n");


            else
                fprintf(file_to_print, "            %-5.2f\n", 
                    pr_steerer_supply[steerer[button[cnt].obj3].alim2].current);



            }


        }

    }        



fprintf(file_to_print,"\ndati stampati dal file %s", string);

fclose(file_to_print);


printf(" * * 7.5 * * %s\n",comando_di_stampa );

system(comando_di_stampa);


}



void masave_data(string)
char *string;
{

char comando_di_stampa[90], form1[90], form2[90], form3[90];
int cnt;
FILE *file_to_write;
FILE *file_to_print;
char stringa[100];
    sprintf(stringa, "%s%s", "./dati_fascio/", string);
    file_to_write = fopen(stringa,"w");
    for (cnt = 0; cnt<100; cnt++)
        {
        fprintf(file_to_write, "%d %f %d %d \n",cnt,
        supply[cnt].current,
        supply[cnt].on_off, supply[cnt].mode);
        }
    for (cnt = 0; cnt<100; cnt++)
        {
        fprintf(file_to_write, "%d %f %d \n",cnt,
        steerer_supply[cnt].current,
        steerer_supply[cnt].mode);
        }

    fclose(file_to_write);
}



void maread_data(string)
char *string;
{
int cnt, cnt1, cnt2, value_to_send, corrente ;
int oldvalue;
float oldcurrent;
FILE *file_to_read;
float read_current;
char stringa[100];
char warn_string[1000];
Arg arg[8];

sprintf(stringa, "%s%s", "./dati_fascio/", string);
file_to_read = fopen(stringa,"r");

strcpy(stringa," ");
strcpy(warn_string,"NOT LOADED");

XtSetArg( arg[0], XtNforeground, pixel_brown );
XtSetValues( mawarning_lab, &arg, 1 ) ;
XtSetValues( mawarning_but, &arg, 1 ) ;

if ( file_to_read != NULL )
        {
        for (cnt = 0; cnt<100; cnt++)
                {
		oldvalue=supply[cnt].on_off;
		oldcurrent=supply[cnt].current;
                fscanf( file_to_read, "%d%f%d%d\n",&cnt1,
                        &(supply[cnt].current),
                (&supply[cnt].on_off), &(supply[cnt].mode) );
		if ( supply[cnt].on_off != oldvalue )
			{
			supply[cnt].on_off = oldvalue;
			supply[cnt].current = oldcurrent;
			//strcpy(warn_string, "STATUS CHANGED ON:");
			for ( cnt2 = 0 ; cnt2 < 100 ; cnt2++ )
				{
				if ( magnet[cnt2].alim1 == cnt )
					{
					sprintf(stringa, " %s", magnet[cnt2].n_alim1);
					strcat(warn_string, stringa);
					}
				if ( magnet[cnt2].alim2 == cnt )
					{
					sprintf(stringa, " %s", magnet[cnt2].n_alim2);
					strcat(warn_string, stringa);
					}
				}
        		XtSetArg( arg[0], XtNbackground, pixel_red );
        		XtSetArg( arg[1], XtNforeground, pixel_black );
       			XtSetValues( mawarning_lab, &arg, 2 ) ;
			XtSetValues( mawarning_but, &arg, 2 ) ;
			}
                }
       		XtSetArg( arg[0], XtNlabel, warn_string);
       		XtSetValues( mawarning_lab, &arg, 1 ) ;
       		XtSetArg( arg[0], XtNlabel, "OK");
       		XtSetValues( mawarning_but, &arg, 1 ) ;

        for (cnt = 0; cnt<100; cnt++)
                {
                fscanf( file_to_read, "%d%f%d\n",&cnt1,
                &(steerer_supply[cnt].current),
                &(steerer_supply[cnt].mode) );
                }

        for (cnt = 1; cnt < numero_bottoni + 1; cnt++)
                {
		if ( button[cnt].type == 2 )
			{
			st_current_cmd(steerer[button[cnt].obj1].alim1,
			steerer_supply[steerer[button[cnt].obj1].alim1].current);
			st_current_cmd(steerer[button[cnt].obj1].alim2,
			steerer_supply[steerer[button[cnt].obj1].alim2].current);
			st_current_cmd(steerer[button[cnt].obj3].alim1,
			steerer_supply[steerer[button[cnt].obj3].alim1].current);
			st_current_cmd(steerer[button[cnt].obj3].alim2,
			steerer_supply[steerer[button[cnt].obj3].alim2].current);
			}
                }

/* agg. sett 2001 */
 if ( button[ numero_bottone ].type == 2 || button[ numero_bottone ].type == 4 )
	{
 	ma2q3q_refresh() ;  
	}
                        if ( pagina_steerers == 1 )
                                {
                                st_refresh_a();
                                st_refresh_b();
                                }
/* fine agg. sett 2001 */

        fclose(file_to_read);
        }
if ( file_to_read == NULL )
        printf ("\007WARNING : FILE DOES NOT EXIST \n");

for (cnt = 0 ; cnt<100 ; cnt++ )
        {
        if ( magnet[cnt].alim1 > 0 && magnet[cnt].alim1 < 100 )
                {
		printf ("modificato valore di %6s (%d)\n", magnet[cnt].n_alim1, magnet[cnt].alim1 );

                value_to_send =
                (int)(supply[magnet[cnt].alim1].current * 1000000.0 / 
                 supply[magnet[cnt].alim1].maxcurrent);

                /* STEFANIA: aggiornamento anche
                   di wa_current e ra_current */
                supply[magnet[cnt].alim1].wa_current = value_to_send ;
                sprintf ( to_server ,"ma comm : ALIM = %2d !WA %06d", 
                          magnet[cnt].alim1, value_to_send);
                write( server_channel, to_server, BUF_SIZE ) ;
                read( server_channel, from_server, BUF_SIZE ) ;
                sscanf( from_server, "%d",&corrente);
                supply[magnet[cnt].alim1].ra_current = corrente ;

                }
        if ( magnet[cnt].alim2 > 0 && magnet[cnt].alim2 < 100 )
                {
	//	printf ("modificato valore di %6s (%d)\n", magnet[cnt].n_alim2, magnet[cnt].alim2 );

                value_to_send = (int)(supply[magnet[cnt].alim2].current *
                                      1000000.0 / 
                                      supply[magnet[cnt].alim2].maxcurrent);

                /* STEFANIA: aggiornamento anche di wa_current e ra_current */
                supply[magnet[cnt].alim2].wa_current = value_to_send ;
                sprintf ( to_server ,"ma comm : ALIM = %2d !WA %06d", 
                          magnet[cnt].alim2, value_to_send);
                write( server_channel, to_server, BUF_SIZE ) ;
                read( server_channel, from_server, BUF_SIZE ) ;
                sscanf( from_server, "%d",&corrente);
                supply[magnet[cnt].alim2].ra_current = corrente ;

                }
        }
}


void ma_reset_act(ma_reset_but_1, tag, ma_reset_dat)
Widget ma_reset_but_1;
caddr_t tag ;
caddr_t ma_reset_dat;
{
int supply_number, value_to_send;

/*printf ("reset\n");*/
supply_number = (int)tag;

/*printf("onoff : alimentatore = %d valore =%d\n",
supply_number,supply[supply_number].on_off);*/

sprintf ( to_server ,"ma comm : ALIM = %2d !RS", supply_number);
/*printf ("messaggi inviato al netm  : %s\n", to_server);*/
write( server_channel, to_server, BUF_SIZE ) ;

read( server_channel, from_server, BUF_SIZE ) ;
/*printf ("messaggio ricevuto dal netm  : %s\n", from_server);*/

}


void ma_onoff_act(ma_onoff_but_1, tag, ma_onoff_dat)
Widget ma_onoff_but_1;
caddr_t tag ;
caddr_t ma_onoff_dat;
{
extern int pixel_orange, pixel_grey ;

Arg arg;
int supply_number, value_to_send;

supply_number = (int)tag;


        if  ( supply[supply_number].on_off == ON )
            {
            XtSetArg( arg, XtNlabel, "OFF" ) ;
            XtSetValues(ma_onoff_but_1 , &arg, 1 ) ;
            XtSetArg( arg, XtNbackground, pixel_grey ) ;
            XtSetValues(ma_onoff_but_1 , &arg, 1 ) ;

            sprintf ( to_server ,"ma comm : ALIM = %2d !F",
                      supply_number);
            write( server_channel, to_server, BUF_SIZE ) ;
            read( server_channel, from_server, BUF_SIZE ) ;
            supply[supply_number].on_off = OFF;
            }
        else
        if  ( supply[supply_number].on_off == OFF )
            {
            /* prima spegnerlo perche' poi si accenda */
            sprintf ( to_server ,"ma comm : ALIM = %2d !F",
                      supply_number);
            write( server_channel, to_server, BUF_SIZE ) ;
            read( server_channel, from_server, BUF_SIZE ) ;
sleep(1);

            sprintf ( to_server, "ma comm : ALIM = %2d !N",
                      supply_number);
            write( server_channel, to_server, BUF_SIZE ) ;
            read( server_channel, from_server, BUF_SIZE ) ;
            sleep(1);

            sprintf( to_server, "ma comm : ALIM = %2d !S1",
                      supply_number);
            write(server_channel, to_server, BUF_SIZE ) ;
            read(server_channel, from_server, BUF_SIZE ) ;
            if (from_server[0] ==  '.' )
                {
                XtSetArg( arg, XtNlabel, "ON" ) ;
                XtSetValues(ma_onoff_but_1 , &arg, 1 ) ;
                supply[supply_number].on_off = ON;
                XtSetArg( arg, XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_1 , &arg, 1 ) ;
                }
            }

}


void ma_remloc_act(ma_remloc_but_1, tag, ma_remloc_dat)
Widget ma_remloc_but_1;
caddr_t tag ;
caddr_t ma_remloc_dat;
{
Arg args[9];
int supply_number, value_to_send;
int corrente_letta;


supply_number = (int)tag;

switch (supply[supply_number].rem_loc)
        {
        case REMOTE:        /* devo passare local */
        XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
        XtSetValues(ma_remloc_but_1 , args, 1 ) ;
        supply[supply_number].rem_loc = LOCAL;

        /*printf("onoff : alimentatore = %d valore =%d\n",
        supply_number,supply[supply_number].rem_loc);*/

        sprintf ( to_server ,"ma comm : ALIM = %2d !LOC", supply_number);
        /*printf ("messaggi inviato al netm  : %s\n", to_server);*/

        write( server_channel, to_server, BUF_SIZE ) ;
            read( server_channel, from_server, BUF_SIZE ) ;
        /*printf ("messaggio ricevuto dal netm  : %s\n", from_server);*/
	mag_page_refresh();
        break;

        case LOCAL:        /* devo passare remote */
        XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
        XtSetValues(ma_remloc_but_1 , args, 1 ) ;
        supply[supply_number].rem_loc = REMOTE;


        sprintf ( to_server ,"ma comm : ALIM = %2d !REM", supply_number);
        /*printf ("messaggio inviato al netm  : %s\n", to_server);*/

        write( server_channel, to_server, BUF_SIZE ) ;

             read( server_channel, from_server, BUF_SIZE ) ;

        if ( supply_number > 0 && supply_number < 100 )
            {
            sprintf(to_server,"ma comm : ALIM = %2d !S1", supply_number);
            write(server_channel, to_server, BUF_SIZE ) ;
            read(server_channel, from_server, BUF_SIZE ) ;

//            strcpy(ma_suppl_status[supply_number],from_server); /* e' un baco !!! */
            if ( from_server[0] == '.' )
                supply[supply_number].on_off = ON;
            if ( from_server[0] == '!' )
                supply[supply_number].on_off = OFF;
            if ( from_server[0] == 'S' )
                {
                supply[supply_number].on_off = STUB;
                if ( strncmp (from_server, "STUB alim", 9) == 0 )
                    printf ("^[[01;31ml'alimentatore di %s non risponde^[[0m\n",magnet[cnt].n_alim1);
                else
                    printf ("^[[01;35ml'alimentatore di %s e' stato disattivato^[[0m\n",magnet[cnt].n_alim1);
                }

            sprintf(to_server,"ma comm : ALIM = %2d !RA", supply_number);
            write(server_channel, to_server, BUF_SIZE ) ;
            read(server_channel, from_server, BUF_SIZE ) ;
            sscanf(from_server,"%d",&corrente_letta);
            if ( corrente_letta > 999999 )
                corrente_letta = 999999 ;
            supply[supply_number].current =
                ((float)corrente_letta/1000000.0)*
                supply[supply_number].maxcurrent;
                /* STEFANIA: inizializz. di ra_current e wa_current */
            supply[supply_number].ra_current = corrente_letta ;
            supply[supply_number].wa_current = corrente_letta ;

            if ( verbose )
                printf("alim = %d current = %d\n",supply_number, corrente_letta);

            /* STEFANIA il p.s. puo' anche essere LOCK,
               in tal caso va' messo UNLOCK */
            sprintf(to_server,"ma comm : ALIM = %2d !CMDSTATE",
                    supply_number);
            write(server_channel, to_server, BUF_SIZE ) ;
            read(server_channel, from_server, BUF_SIZE ) ;
            if ( strncmp(from_server,"LOCK",4) == 0 )
                {
                sprintf(to_server,"ma comm : ALIM = %2d !UNLOCK",
                        supply_number);
                write(server_channel, to_server, BUF_SIZE ) ;
                read(server_channel, from_server, BUF_SIZE ) ;
                supply[supply_number].rem_loc = LOCAL;
                }
            else
                {
                if ( strncmp(from_server,"LOCAL",5) == 0 )
                    supply[supply_number].rem_loc = LOCAL;
                if ( strncmp(from_server,"REMOTE",6) == 0 )
                    supply[supply_number].rem_loc = REMOTE;
                }
            }


	mag_page_refresh();


        break;

        }
}


/* STEFANIA routine aggiunta per semplificare l'uso
   della grip area: invia il cmd di corrente all'alimentatore */

void ma_current_cmd( supply_number, value_to_send )
int supply_number ;
int value_to_send ;

{
extern int verbose ;

sprintf ( to_server ,"ma comm : ALIM = %2d !WA %06d",
          supply_number, value_to_send);
if ( verbose )
    printf ("ALPI send: %s\n", to_server);
write( server_channel, to_server, BUF_SIZE ) ;
read( server_channel, from_server, BUF_SIZE ) ;
if ( verbose )
    printf ("ALPI receive: %s\n", from_server);
};


/* STEFANIA routine aggiunta per semplificare l'uso
   della grip area: aggiorna la labels di corrente relative
   all'alimentatore */

void ma_current_upd( supply_number )
int supply_number ;

{
char string[30];
Arg arg[1];
int corrente;

if (strncmp("STUB", from_server, 4) == 0 )

        {
        sprintf(string, "STUB : %8.4f %%",
                (float)(supply[supply_number].wa_current)/10000.0) ;
        XtSetArg(arg[0],XtNlabel,string);
        }

else
        {
        sscanf( from_server, "%d",&corrente);
        sprintf(string,"%8.4f %%",((float)corrente)/10000.0);
        XtSetArg(arg[0],XtNlabel,string);
        }

/* aggiornamento label corrente richiesta (in percentuale) */
switch(supply[supply_number].finestra)
        {
        case 1:
        XtSetValues(ma_curimp_perc_lab_a, arg,1);
        break;
        
        case 2:
        XtSetValues(ma_curimp_perc_lab_b, arg,1);
        break;
        
        case 3:
        XtSetValues(ma_curimp_perc_lab_c, arg,1);
        break;
        }

/* aggiornamento label corrente richiesta (in ampere) */
if (strncmp("STUB", from_server, 44) == 0 )

        {
        sprintf(string, "STUB : %8.4f A", supply[supply_number].current);
        XtSetArg(arg[0],XtNlabel,string);
        }

else
        {
        sscanf( from_server, "%d",&corrente);
        sprintf(string,"%8.4f A",
                supply[supply_number].maxcurrent*(float)corrente/1000000.0);
        XtSetArg(arg[0],XtNlabel,string);
        }


switch(supply[supply_number].finestra)
        {
        case 1:
        XtSetValues(ma_curimp_lab_a, arg,1);
        break;
        
        case 2:
        XtSetValues(ma_curimp_lab_b, arg,1);
        break;
        
        case 3:
        XtSetValues(ma_curimp_lab_c, arg,1);
        break;
        }


/* aggiornamento posizione del corrispondente ruler */
switch(supply[supply_number].finestra)
        {
        case 1:
	dimensione_cursore = 0.01;
	posizione_cursore = supply[supply_number].current/supply[supply_number].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_a, arg,3);
        break;
        
        case 2:
	dimensione_cursore = 0.01;
	posizione_cursore = supply[supply_number].current/supply[supply_number].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_b, arg,3);
        break;
        
        case 3:
	dimensione_cursore = 0.01;
	posizione_cursore = supply[supply_number].current/supply[supply_number].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_c, arg,3);
        break;
        }

};


/* STEFANIA: routine modificata in modo da far uso delle routines
   scritte per la grip area */

void ma_curup_act( ma_curup_but, tag, ma_curup_dat )
    Widget ma_curup_but ;
    caddr_t tag ; 
    caddr_t ma_curup_dat ; 

{
extern void ma2q3q_refresh();
int supply_number, value_to_send;

supply_number = (int)tag;

if ( supply[supply_number].wa_current < MA_MAX_WA )
    {
    supply[supply_number].wa_current =
        supply[supply_number].wa_current + MA_STEP ;
    supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
    }


value_to_send = supply[supply_number].wa_current ;


ma_current_cmd( supply_number, value_to_send ) ;

if ( button[ numero_bottone ].type == 2 ||
     button[ numero_bottone ].type == 4 ) ma2q3q_refresh() ;

ma_current_upd( supply_number ) ;

};


/* STEFANIA: routine modificata in modo da far uso delle routines
   scritte per la grip area */

void ma_curup_fast_act( ma_curup_fast_but, tag, ma_curup_fast_dat )
    Widget ma_curup_fast_but ;
    caddr_t tag ; 
    caddr_t ma_curup_fast_dat ; 

{
extern void ma2q3q_refresh();
int supply_number, value_to_send;

supply_number = (int)tag;

if ( supply[supply_number].wa_current < MA_MAX_WA_FAST )
    {
    supply[supply_number].wa_current =
        supply[supply_number].wa_current + MA_STEP_FAST ;
    supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
    }
        

value_to_send = supply[supply_number].wa_current ;

ma_current_cmd( supply_number, value_to_send ) ;

if ( button[ numero_bottone ].type == 2 ||
     button[ numero_bottone ].type == 4 ) ma2q3q_refresh() ;

ma_current_upd( supply_number ) ;

};


/* STEFANIA: routine modificata in modo da far uso delle routines
   scritte per la grip area */

void ma_curdwn_act( ma_curdwn_but, tag, ma_curdwn_dat )
    Widget ma_curdwn_but ;
    caddr_t tag ; /* not used */
    caddr_t ma_curdwn_dat ; /* not used */

{
extern void ma2q3q_refresh();
int supply_number, value_to_send;

supply_number = (int)tag;

if ( supply[supply_number].wa_current > MA_MIN_WA )
    {
    supply[supply_number].wa_current =
        supply[supply_number].wa_current - MA_STEP ;
    supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
    }


value_to_send = supply[supply_number].wa_current ;


ma_current_cmd( supply_number, value_to_send ) ;

if ( button[ numero_bottone ].type == 2 ||
     button[ numero_bottone ].type == 4 ) ma2q3q_refresh() ;

ma_current_upd( supply_number ) ;

};


/* STEFANIA: routine modificata in modo da far uso delle routines
   scritte per la grip area */

void ma_curdwn_fast_act( ma_curdwn_fast_but, tag, ma_curdwn_fast_dat )
    Widget ma_curdwn_fast_but ;
    caddr_t tag ; /* not used */
    caddr_t ma_curdwn_fast_dat ; /* not used */

{
extern void ma2q3q_refresh();
int supply_number, value_to_send;

supply_number = (int)tag;

if ( supply[supply_number].wa_current > MA_MIN_WA_FAST )
    {
    supply[supply_number].wa_current =
        supply[supply_number].wa_current - MA_STEP_FAST ;
    supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
    }


value_to_send = supply[supply_number].wa_current ;


ma_current_cmd( supply_number, value_to_send ) ;

if ( button[ numero_bottone ].type == 2 ||
     button[ numero_bottone ].type == 4 ) ma2q3q_refresh() ;

ma_current_upd( supply_number ) ;

};

/* prima ma_ruler_jump inviava solo il dato,
   ora aggiorna anche le etichette
   con il valore di ritorno dell'alimentatore */

void ma_ruler_jump( ma_cur_ruler, tag, percento )
    Widget ma_cur_ruler ;
    caddr_t tag ; 
    caddr_t percento;

{
extern void ma2q3q_refresh();
int corrente;
Arg arg[10];
char string[30];
float percent;
int supply_number, value_to_send;

supply_number = (int)tag;

percent = *(float *)percento; /* 0.0 < percent 1.0 */

supply[supply_number].current = supply[supply_number].maxcurrent * percent ;

/* STEFANIA: +0.5 per arrotondamento */

value_to_send = (int)( percent * 1000000.0 + 0.5 ) ;

/* STEFANIA: limitato il valore a 999999 */

if ( value_to_send > 999999 ) value_to_send = 999999 ;
supply[supply_number].wa_current = value_to_send ;

ma_current_cmd( supply_number, value_to_send ) ;

if ( button[ numero_bottone ].type == 2 ||
     button[ numero_bottone ].type == 4 ) ma2q3q_refresh() ;

ma_current_upd( supply_number ) ;

};


void ma_ruler_scroll( ma_cur_ruler, tag, ma_ruler_dat)
    Widget ma_cur_ruler ;
    caddr_t tag ; /* not used */
    int ma_ruler_dat ;

{
Arg arg[10];
char string[30];
int int_percent;
extern struct supply_data supply[100];
float percent, new_value ;
int supply_number, value_to_send;

supply_number = (int)tag;


/* STEFANIA: per limitare il nuovo valore a 99.9999 */
new_value = (float)ma_ruler_dat/5.0 ;
if ( new_value > 99.9999 ) new_value = 99.9999 ;
sprintf(string, "%8.4f %%", new_value );
XtSetArg(arg[0],XtNlabel,string);

switch(supply[supply_number].finestra)
        {
        case 1:
        XtSetValues(ma_curimp_perc_lab_a, arg,1);
        break;
        
        case 2:
        XtSetValues(ma_curimp_perc_lab_b, arg,1);
        break;
        
        case 3:
        XtSetValues(ma_curimp_perc_lab_c, arg,1);
        break;
        }

sprintf(string, "%8.4f A",
        ((supply[supply_number].maxcurrent*new_value)/100.0));
XtSetArg(arg[0],XtNlabel,string);


switch(supply[supply_number].finestra)
        {
        case 1:
        XtSetValues(ma_curimp_lab_a, arg,1);
        break;
        
        case 2:
        XtSetValues(ma_curimp_lab_b, arg,1);
        break;
        
        case 3:
        XtSetValues(ma_curimp_lab_c, arg,1);
        break;
        }


};


/* STEFANIA2 (il ritorno) : modificato per pag. di tipo 7 e 8 */
ma_combox( flag )
int flag ; /*  flag=0 => for true magnets, flag=1 => for steerer */
{
        extern Pixmap s8000_pixmap ;
        extern ma_refr_onoff_act();
        extern ma_magexit_act(), ma_next_act(), ma_previous_act() ;
        Arg args[2] ;

        XtSetArg( args[0], XtNiconName, magnet[numero_bottone].name ) ;
        XtSetArg( args[1], XtNiconPixmap, s8000_pixmap ) ;


        ma_comm_shell = XtCreateApplicationShell ( "ma_comm_shell",
                        applicationShellWidgetClass, args, 2);

        ma_setup_box = XtCreateWidget( "ma_setup_box", formWidgetClass,
                                       ma_comm_shell, NULL, 0 ) ;
        XtManageChild(ma_setup_box);

        if ( flag == 0 ) /* pagine tipo 1,2,3,4,5,6 */

            {

            ma_refr_onoff_but =  XtCreateWidget("ma_refr_onoff_but",
                                 commandWidgetClass, ma_setup_box,NULL,0);

            XtAddCallback( ma_refr_onoff_but, XtNcallback,
                           ma_refr_onoff_act, 1 ) ;

            XtManageChild(ma_refr_onoff_but);

            XtSetArg( args[0], XtNfromHoriz, ma_refr_onoff_but ) ;

            ma_magexit_but =  XtCreateWidget( "ma_magexit_but",
                              commandWidgetClass, ma_setup_box, args , 1 );
            }

        else /* pagine tipo 7 e 8 */
            ma_magexit_but =  XtCreateWidget( "ma_magexit_but",
                              commandWidgetClass, ma_setup_box, NULL , 0 );

        XtAddCallback( ma_magexit_but, XtNcallback,
                       ma_magexit_act, NULL ) ;

        XtManageChild(ma_magexit_but);

        XtSetArg( args[0], XtNfromHoriz, ma_magexit_but ) ;

        ma_previous_but =  XtCreateWidget( "ma_previous_but",
                       commandWidgetClass, ma_setup_box, args , 1 );

        XtAddCallback( ma_previous_but, XtNcallback,
                       ma_previous_act, NULL ) ;

        XtManageChild(ma_previous_but);

        XtSetArg( args[0], XtNfromHoriz, ma_previous_but ) ;

        ma_next_but =  XtCreateWidget( "ma_next_but",
                       commandWidgetClass, ma_setup_box, args , 1 );

        XtAddCallback( ma_next_but, XtNcallback,
                       ma_next_act, NULL ) ;

        XtManageChild(ma_next_but);


}


/* STEFANIA : routine che crea i
   widget per il controllo unificato di 2Q e 3Q */
ma_2q3qbox()
{

        Arg arg ;

        ma2q3q_box = XtCreateWidget( "ma2q3q_box", formWidgetClass,
                                     ma_setup_box, NULL, 0 ) ;
        XtSetArg( arg, XtNfromVert, ma_box_b );
        XtSetValues( ma2q3q_box , &arg, 1 ) ;
        XtManageChild( ma2q3q_box ) ;

        mafill_2q3qbox( magnet[button[numero_bottone].obj2].alim1, 
                        magnet[button[numero_bottone].obj2].alim2, 
                        magnet[numero_bottone].name,
                        ma2q3q_box,
                        &ma2q3q_label,
                        &ma2q3q_I_lab,
                        &ma2q3q_D_lab,
                        &ma2q3q_I_ruler,
                        &ma2q3q_D_ruler,
                        &ma2q3q_I_grip,
                        &ma2q3q_D_grip,
                        &ma2q3q_I_griplab,
                        &ma2q3q_D_griplab ) ;

}



crea_pag_2()
{

extern ma_refr_onoff_act(), st_page_act();

        Arg args[9] ;
        int counter;

        ma_combox( 0 ) ;

        XtSetArg( args[0], XtNfromHoriz, ma_next_but) ;

        st_page_but =   XtCreateWidget("st_page_but",
                        commandWidgetClass, ma_setup_box,args,1);

        XtAddCallback( st_page_but, XtNcallback,
                       st_page_act, NULL ) ;

        XtManageChild(st_page_but);


        ma_box_a = XtCreateWidget( "ma_box_a", formWidgetClass,
                                   ma_setup_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNfromVert, ma_refr_onoff_but );
        XtSetValues(ma_box_a , args, 1 ) ;
        XtManageChild(ma_box_a);

        ma_box_b = XtCreateWidget( "ma_box_b", formWidgetClass,
                                   ma_setup_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNfromVert, ma_box_a);
        XtSetValues(ma_box_b , args, 1 ) ;
        XtManageChild(ma_box_b);

        supply[magnet[numero_bottone].alim1].finestra = 1;

        fill_box(magnet[button[numero_bottone].obj2].alim1, 
        ma_box_a, 
        &ma_name_label_a,
        &ma_cur_lab_a, 
        &ma_curimp_name_lab_a,
        &ma_curatt_name_lab_a,
        &ma_curimp_lab_a, 
        &ma_curatt_lab_a, 
        &ma_curimp_perc_lab_a,
        &ma_curatt_perc_lab_a, 
        &ma_curup_but_a, 
        &ma_curdwn_but_a, 
        &ma_curup_fast_but_a, 
        &ma_curdwn_fast_but_a, 
        &ma_cur_ruler_a, 
        &ma_volt_text_lab_a,
        &ma_volt_perc_lab_a,  
        &ma_temp_text_lab_a, 
        &ma_trans_text_lab_a,
        &ma_v1_text_lab_a,
        &ma_v2_text_lab_a,
        &ma_v3_text_lab_a,
        &ma_volt_lab_a, 
        &ma_temp_lab_a, 
        &ma_trans_lab_a,
        &ma_v1_lab_a,
        &ma_v2_lab_a,
        &ma_v3_lab_a,
        &ma_onoff_but_a,
        &ma_remloc_but_a,
        &ma_reset_but_a,
        magnet[numero_bottone].n_alim1,        
        &ma_status_lab_1_a,
        &ma_status_lab_2_a,
        &ma_status_lab_3_a,
        &ma_status_lab_4_a,
        &ma_status_lab_5_a,
        &ma_status_lab_6_a,
        &ma_status_lab_7_a,
        &ma_status_lab_8_a,
        &ma_status_lab_9_a,
        &ma_status_lab_10_a,
        &ma_status_lab_11_a,
        &ma_status_lab_12_a,
        &ma_status_lab_13_a,
        &ma_status_lab_14_a,
        &ma_status_lab_15_a,
        &ma_status_lab_16_a,
        &ma_grip_a, &ma_incdec_a
        ); /* STEFANIA : aggiunto il widget grip e incdec */

        supply[magnet[numero_bottone].alim2].finestra = 2;

        fill_box(magnet[button[numero_bottone].obj2].alim2, 
        ma_box_b, 
        &ma_name_label_b, 
        &ma_cur_lab_b, 
        &ma_curimp_name_lab_b,
        &ma_curatt_name_lab_b,
        &ma_curimp_lab_b, 
        &ma_curatt_lab_b, 
        &ma_curimp_perc_lab_b, 
        &ma_curatt_perc_lab_b, 
        &ma_curup_but_b, 
        &ma_curdwn_but_b, 
        &ma_curup_fast_but_b, 
        &ma_curdwn_fast_but_b, 
        &ma_cur_ruler_b, 
        &ma_volt_text_lab_b,
        &ma_volt_perc_lab_b,  
        &ma_temp_text_lab_b, 
        &ma_trans_text_lab_b,
        &ma_v1_text_lab_b,
        &ma_v2_text_lab_b,
        &ma_v3_text_lab_b,
        &ma_volt_lab_b, 
        &ma_temp_lab_b, 
        &ma_trans_lab_b,
        &ma_v1_lab_b,
        &ma_v2_lab_b,
        &ma_v3_lab_b,
        &ma_onoff_but_b,
        &ma_remloc_but_b,
        &ma_reset_but_b,
        magnet[numero_bottone].n_alim2,
        &ma_status_lab_1_b,
        &ma_status_lab_2_b,
        &ma_status_lab_3_b,
        &ma_status_lab_4_b,
        &ma_status_lab_5_b,
        &ma_status_lab_6_b,
        &ma_status_lab_7_b,
        &ma_status_lab_8_b,
        &ma_status_lab_9_b,
        &ma_status_lab_10_b,
        &ma_status_lab_11_b,
        &ma_status_lab_12_b,
        &ma_status_lab_13_b,
        &ma_status_lab_14_b,
        &ma_status_lab_15_b,
        &ma_status_lab_16_b,
        &ma_grip_b, &ma_incdec_b
        ); /* STEFANIA : aggiunto il widget grip e incdec */

        ma_2q3qbox() ; /* STEFANIA : 2q3q widgets */

}



crea_pag_4()
{

        Arg args[9] ;
        int counter;

        ma_combox( 0 ) ;

        ma_box_a = XtCreateWidget( "ma_box_a", formWidgetClass,
                                   ma_setup_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNfromVert, ma_refr_onoff_but );
        XtSetValues(ma_box_a , args, 1 ) ;
        XtManageChild(ma_box_a);

        ma_box_b = XtCreateWidget( "ma_box_b", formWidgetClass,
                                   ma_setup_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNfromVert, ma_box_a);
        XtSetValues(ma_box_b , args, 1 ) ;
        XtManageChild(ma_box_b);

        supply[magnet[button[numero_bottone].obj2].alim1].finestra = 1;
        fill_box(magnet[button[numero_bottone].obj2].alim1, 
        ma_box_a, 
        &ma_name_label_a, 
        &ma_cur_lab_a, 
        &ma_curimp_name_lab_a,
        &ma_curatt_name_lab_a,
        &ma_curimp_lab_a, 
        &ma_curatt_lab_a, 
        &ma_curimp_perc_lab_a, 
        &ma_curatt_perc_lab_a, 
        &ma_curup_but_a, 
        &ma_curdwn_but_a, 
        &ma_curup_fast_but_a, 
        &ma_curdwn_fast_but_a, 
        &ma_cur_ruler_a, 
        &ma_volt_text_lab_a,
        &ma_volt_perc_lab_a,  
        &ma_temp_text_lab_a, 
        &ma_trans_text_lab_a,
        &ma_v1_text_lab_a,
        &ma_v2_text_lab_a,
        &ma_v3_text_lab_a,
        &ma_volt_lab_a, 
        &ma_temp_lab_a, 
        &ma_trans_lab_a,
        &ma_v1_lab_a,
        &ma_v2_lab_a,
        &ma_v3_lab_a,
        &ma_onoff_but_a,
        &ma_remloc_but_a,
        &ma_reset_but_a,
        magnet[numero_bottone].n_alim1,
        &ma_status_lab_1_a,
        &ma_status_lab_2_a,
        &ma_status_lab_3_a,
        &ma_status_lab_4_a,
        &ma_status_lab_5_a,
        &ma_status_lab_6_a,
        &ma_status_lab_7_a,
        &ma_status_lab_8_a,
        &ma_status_lab_9_a,
        &ma_status_lab_10_a,
        &ma_status_lab_11_a,
        &ma_status_lab_12_a,
        &ma_status_lab_13_a,
        &ma_status_lab_14_a,
        &ma_status_lab_15_a,
        &ma_status_lab_16_a,
        &ma_grip_a, &ma_incdec_a
        ); /* STEFANIA : aggiunto il widget grip e incdec */

        supply[magnet[numero_bottone].alim2].finestra = 2;

        fill_box(magnet[button[numero_bottone].obj2].alim2, 
        ma_box_b, 
        &ma_name_label_b, 
        &ma_cur_lab_b, 
        &ma_curimp_name_lab_b,
        &ma_curatt_name_lab_b,
        &ma_curimp_lab_b, 
        &ma_curatt_lab_b, 
        &ma_curimp_perc_lab_b, 
        &ma_curatt_perc_lab_b, 
        &ma_curup_but_b, 
        &ma_curdwn_but_b, 
        &ma_curup_fast_but_b, 
        &ma_curdwn_fast_but_b, 
        &ma_cur_ruler_b, 
        &ma_volt_text_lab_b, 
        &ma_volt_perc_lab_b, 
        &ma_temp_text_lab_b, 
        &ma_trans_text_lab_b,
        &ma_v1_text_lab_b,
        &ma_v2_text_lab_b,
        &ma_v3_text_lab_b,
        &ma_volt_lab_b, 
        &ma_temp_lab_b, 
        &ma_trans_lab_b,
        &ma_v1_lab_b,
        &ma_v2_lab_b,
        &ma_v3_lab_b,
        &ma_onoff_but_b,
        &ma_remloc_but_b,
        &ma_reset_but_b,
        magnet[numero_bottone].n_alim2,
        &ma_status_lab_1_b,
        &ma_status_lab_2_b,
        &ma_status_lab_3_b,
        &ma_status_lab_4_b,
        &ma_status_lab_5_b,
        &ma_status_lab_6_b,
        &ma_status_lab_7_b,
        &ma_status_lab_8_b,
        &ma_status_lab_9_b,
        &ma_status_lab_10_b,
        &ma_status_lab_11_b,
        &ma_status_lab_12_b,
        &ma_status_lab_13_b,
        &ma_status_lab_14_b,
        &ma_status_lab_15_b,
        &ma_status_lab_16_b,
        &ma_grip_b, &ma_incdec_b
        ); /* STEFANIA : aggiunto il widget grip e incdec */

        ma_2q3qbox() ; /* STEFANIA : 2q3q widgets */

}


refresh_onoff_act(status)
int status;
{
extern int pixel_orange, pixel_grey ;
extern int timeout_proc();

Arg arg ;

if (status == 0)
        {
        if ( interval_id != NULL )
            XtRemoveTimeOut(interval_id);
        interval_id = NULL ;
        XtSetArg( arg, XtNbackground, pixel_grey );
        XtSetValues( ma_refr_onoff_but, &arg, 1 ) ;
        refresh_onoff = 0;
        }
if ( status == 1)
        {
        interval_id = XtAddTimeOut ( 500, timeout_proc, NULL);
        XtSetArg( arg, XtNbackground, pixel_orange );
        XtSetValues( ma_refr_onoff_but, &arg, 1 ) ;
        refresh_onoff = 1;
        }
}


ma_refr_onoff_act(ma_refr_onoff_but, tag, ma_refr_onoff_dat)
Widget ma_refr_onoff_but;
caddr_t tag ;
caddr_t ma_refr_onoff_dat;
{
    Arg args[9];

    if ( refresh_onoff == 1 )
        {
        /* devo passare off */
        XtSetArg( args[0], XtNlabel, "REFRESH  OFF" ) ;
        XtSetValues(ma_refr_onoff_but , args, 1 ) ;
        refresh_onoff_act(0) ;
        }
    else
        {
        /* devo passare on */
        XtSetArg( args[0], XtNlabel, "REFRESH  ON" ) ;
        XtSetValues(ma_refr_onoff_but , args, 1 ) ;
        refresh_onoff_act(1) ;
        }

}

ma_magexit_act(ma_magexit_but, tag, ma_magexit_dat)
Widget ma_magexit_but;
caddr_t tag ;
caddr_t ma_magexit_dat;
{
extern Widget wid_array[100] ;
extern int button_act() ;

button_act ( wid_array[numero_bottone], numero_bottone, NULL ) ;

}

ma_next_act(ma_next_but, tag, ma_next_dat)
Widget ma_next_but;
caddr_t tag ;
caddr_t ma_next_dat;
{
extern Widget wid_array[100] ;
extern int button_act() ;

if ( numero_bottone < numero_bottoni )
    button_act ( wid_array[numero_bottone], numero_bottone + 1,
                 NULL ) ;
else
    if ( numero_bottone == numero_bottoni )
        button_act ( wid_array[numero_bottone], 1,
                     NULL ) ;

}

ma_previous_act(ma_previous_but, tag, ma_previous_dat)
Widget ma_previous_but;
caddr_t tag ;
caddr_t ma_previous_dat;
{
extern Widget wid_array[100] ;
extern int button_act () ;

if ( numero_bottone > 1 )
    button_act ( wid_array[numero_bottone],
                 numero_bottone - 1, NULL ) ;
else
    button_act ( wid_array[numero_bottone],
                 numero_bottoni, NULL ) ;

}

mag_page_refresh()
{
char string[30];
Arg args[9] ;



    if ( refresh_onoff == 1 )
        {
        XtSetArg( args[0], XtNlabel, "REFRESH  ON" ) ;
        XtSetArg( args[1], XtNbackground, pixel_orange ) ;
        XtSetValues(ma_refr_onoff_but , args, 2 ) ;
        }
    else /* refresh_onoff == 0 */
        {
        XtSetArg( args[0], XtNlabel, "REFRESH  OFF" ) ;
        XtSetArg( args[1], XtNbackground, pixel_grey ) ;
        XtSetValues(ma_refr_onoff_but , args, 2 ) ;
        }

    /* aggiornamento label inc/dec */
    sprintf( string, "%d", ma_step_key ) ;
    XtSetArg( args[0], XtNlabel, string ) ;
    switch ( button[bottone_premuto].type )
        {
        case 1:
        XtSetValues( ma_incdec_a, args, 1 ) ;
        break;

        case 2:
        case 4:
        XtSetValues( ma_incdec_a, args, 1 ) ;
        XtSetValues( ma_incdec_b, args, 1 ) ;
        break;

        case 3:
        case 5:
        case 6:
        XtSetValues( ma_incdec_a, args, 1 ) ;
        XtSetValues( ma_incdec_b, args, 1 ) ;
        XtSetValues( ma_incdec_c, args, 1 ) ;
        break;

        }

XtSetArg( args[0], XtNbackground, pixel_lightsteelblue3 ) ;
    switch ( button[bottone_premuto].type )
        {
        case 1:
	XtSetValues(ma_box_a , args, 1 ) ;
        break;

        case 2:
        case 4:
	XtSetValues(ma_box_a , args, 1 ) ;
	XtSetValues(ma_box_b , args, 1 ) ;
        break;

        case 3:
        case 5:
        case 6:
	XtSetValues(ma_box_a , args, 1 ) ;
	XtSetValues(ma_box_b , args, 1 ) ;
	XtSetValues(ma_box_c , args, 1 ) ;
        break;

        }


    switch ( button[bottone_premuto].type )
        {
        case 1:

	dimensione_cursore = 0.01;
	posizione_cursore = supply[magnet[button[numero_bottone].obj2].alim1].current/supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_a, args,3);

        XtSetArg( args[0], XtNlabel, magnet[numero_bottone].n_alim1 ) ;
        XtSetValues(ma_name_label_a , args, 1 ) ;

        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj2].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_a, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj2].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_a, args,1);


        if ( supply[magnet[button[numero_bottone].obj2].alim1].on_off == ON )
            {
            XtSetArg( args[0], XtNlabel, "ON" ) ;
            XtSetArg( args[1], XtNbackground, pixel_orange ) ;
            XtSetValues(ma_onoff_but_a , args, 2 ) ;
            }
        if ( supply[magnet[button[numero_bottone].obj2].alim1].on_off == OFF )
            {
            XtSetArg( args[0], XtNlabel, "OFF" ) ;
            XtSetArg( args[1], XtNbackground, pixel_grey ) ;
            XtSetValues(ma_onoff_but_a , args, 2 ) ;
            }

        if ( supply[magnet[button[numero_bottone].obj2].alim1].on_off == STUB )
            {
            XtSetArg( args[0], XtNlabel, "STUB" ) ;
            XtSetArg( args[1], XtNbackground, pixel_white ) ;
            XtSetArg( args[2], XtNforeground, pixel_black ) ;
            XtSetValues(ma_onoff_but_a , args, 3 ) ;
            XtSetArg( args[0], XtNbackground, pixel_black ) ;
            XtSetValues(ma_box_a , args, 1 ) ;
            }



        if ( supply[magnet[button[numero_bottone].obj2].alim1].rem_loc
             == REMOTE)
            {
            XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
            XtSetValues(ma_remloc_but_a , args, 1 ) ;
	    XtSetArg( args[0], XtNbackground, pixel_lightsteelblue3 ) ;
	    XtSetValues(ma_box_a , args, 1 ) ; 
            }
        else /* LOCAL */
            {
            XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
            XtSetValues(ma_remloc_but_a , args, 1 ) ;
	    XtSetArg( args[0], XtNbackground, pixel_yellow ) ;
	    XtSetValues(ma_box_a , args, 1 ) ; 
            }

        XtSetArg( args[0], XtNlabel, "        " ) ;
        XtSetArg( args[1], XtNbackground, pixel_grey);

        XtSetValues(ma_status_lab_1_a, args, 2 ) ;
        XtSetValues(ma_status_lab_2_a, args, 2 ) ;
        XtSetValues(ma_status_lab_3_a, args, 2 ) ;
        XtSetValues(ma_status_lab_4_a, args, 2 ) ;
        XtSetValues(ma_status_lab_5_a, args, 2 ) ;
        XtSetValues(ma_status_lab_6_a, args, 2 ) ;
        XtSetValues(ma_status_lab_7_a, args, 2 ) ;
        XtSetValues(ma_status_lab_8_a, args, 2 ) ;
        XtSetValues(ma_status_lab_9_a, args, 2 ) ;
        XtSetValues(ma_status_lab_10_a, args, 2 ) ;
        XtSetValues(ma_status_lab_11_a, args, 2 ) ;
        XtSetValues(ma_status_lab_12_a, args, 2 ) ;
        XtSetValues(ma_status_lab_13_a, args, 2 ) ;
        XtSetValues(ma_status_lab_14_a, args, 2 ) ;
        XtSetValues(ma_status_lab_15_a, args, 2 ) ;
        XtSetValues(ma_status_lab_16_a, args, 2 ) ;

        break;



        case 2:
        case 4:

	dimensione_cursore = 0.01;
	posizione_cursore = supply[magnet[button[numero_bottone].obj2].alim1].current/supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_a, args,3);

        XtSetArg( args[0], XtNlabel, magnet[numero_bottone].n_alim1 ) ;
        XtSetValues(ma_name_label_a , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj2].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_a, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj2].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_a, args,1);


        switch (supply[magnet[button[numero_bottone].obj2].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_a , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_a , args, 1 ) ;
                break;
                }






        switch (supply[magnet[button[numero_bottone].obj2].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_lightsteelblue3 ) ;
	        XtSetValues(ma_box_a , args, 1 ) ; 
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_yellow ) ;
	        XtSetValues(ma_box_a , args, 1 ) ; 
                break;
                }



                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_a, args, 2 ) ;
                XtSetValues(ma_status_lab_2_a, args, 2 ) ;
                XtSetValues(ma_status_lab_3_a, args, 2 ) ;
                XtSetValues(ma_status_lab_4_a, args, 2 ) ;
                XtSetValues(ma_status_lab_5_a, args, 2 ) ;
                XtSetValues(ma_status_lab_6_a, args, 2 ) ;
                XtSetValues(ma_status_lab_7_a, args, 2 ) ;
                XtSetValues(ma_status_lab_8_a, args, 2 ) ;
                XtSetValues(ma_status_lab_9_a, args, 2 ) ;
                XtSetValues(ma_status_lab_10_a, args, 2 ) ;
                XtSetValues(ma_status_lab_11_a, args, 2 ) ;
                XtSetValues(ma_status_lab_12_a, args, 2 ) ;
                XtSetValues(ma_status_lab_13_a, args, 2 ) ;
                XtSetValues(ma_status_lab_14_a, args, 2 ) ;
                XtSetValues(ma_status_lab_15_a, args, 2 ) ;
                XtSetValues(ma_status_lab_16_a, args, 2 ) ;


	dimensione_cursore = 0.01;
	posizione_cursore = supply[magnet[button[numero_bottone].obj2].alim2].current/supply[magnet[button[numero_bottone].obj2].alim2].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_b, args,3);

        XtSetArg( args[0], XtNlabel, magnet[numero_bottone].n_alim2 ) ;
        XtSetValues(ma_name_label_b , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj2].alim2].current * 100.0 /
        supply[magnet[button[numero_bottone].obj2].alim2].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_b, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj2].alim2].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_b, args,1);


        switch (supply[magnet[button[numero_bottone].obj2].alim2].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_b , args, 3 ) ;
                   XtSetArg( args[0], XtNbackground, pixel_black ) ;
                   XtSetValues(ma_box_b , args, 1 ) ;
                break;
                }





        switch (supply[magnet[button[numero_bottone].obj2].alim2].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_b , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_lightsteelblue3 ) ;
	        XtSetValues(ma_box_b , args, 1 ) ; 
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_b , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_yellow ) ;
	        XtSetValues(ma_box_b , args, 1 ) ; 
                break;
                }


                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_b, args, 2 ) ;
                XtSetValues(ma_status_lab_2_b, args, 2 ) ;
                XtSetValues(ma_status_lab_3_b, args, 2 ) ;
                XtSetValues(ma_status_lab_4_b, args, 2 ) ;
                XtSetValues(ma_status_lab_5_b, args, 2 ) ;
                XtSetValues(ma_status_lab_6_b, args, 2 ) ;
                XtSetValues(ma_status_lab_7_b, args, 2 ) ;
                XtSetValues(ma_status_lab_8_b, args, 2 ) ;
                XtSetValues(ma_status_lab_9_b, args, 2 ) ;
                XtSetValues(ma_status_lab_10_b, args, 2 ) ;
                XtSetValues(ma_status_lab_11_b, args, 2 ) ;
                XtSetValues(ma_status_lab_12_b, args, 2 ) ;
                XtSetValues(ma_status_lab_13_b, args, 2 ) ;
                XtSetValues(ma_status_lab_14_b, args, 2 ) ;
                XtSetValues(ma_status_lab_15_b, args, 2 ) ;
                XtSetValues(ma_status_lab_16_b, args, 2 ) ;


        break;


        case 3:

	dimensione_cursore = 0.01;
	posizione_cursore = supply[magnet[button[numero_bottone].obj1].alim1].current/supply[magnet[button[numero_bottone].obj1].alim1].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_a, args,3);

        XtSetArg( args[0], XtNlabel,
        magnet[button[numero_bottone].obj1].n_alim1 ) ;
        XtSetValues(ma_name_label_a , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj1].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj1].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_a, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj1].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_a, args,1);


        switch (supply[magnet[button[numero_bottone].obj1].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;
                
                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_a , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_a , args, 1 ) ;
                break;
                }



        switch (supply[magnet[button[numero_bottone].obj1].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_lightsteelblue3 ) ;
	        XtSetValues(ma_box_a , args, 1 ) ; 
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_yellow ) ;
	        XtSetValues(ma_box_a , args, 1 ) ; 
                break;
                }


                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_a, args, 2 ) ;
                XtSetValues(ma_status_lab_2_a, args, 2 ) ;
                XtSetValues(ma_status_lab_3_a, args, 2 ) ;
                XtSetValues(ma_status_lab_4_a, args, 2 ) ;
                XtSetValues(ma_status_lab_5_a, args, 2 ) ;
                XtSetValues(ma_status_lab_6_a, args, 2 ) ;
                XtSetValues(ma_status_lab_7_a, args, 2 ) ;
                XtSetValues(ma_status_lab_8_a, args, 2 ) ;
                XtSetValues(ma_status_lab_9_a, args, 2 ) ;
                XtSetValues(ma_status_lab_10_a, args, 2 ) ;
                XtSetValues(ma_status_lab_11_a, args, 2 ) ;
                XtSetValues(ma_status_lab_12_a, args, 2 ) ;
                XtSetValues(ma_status_lab_13_a, args, 2 ) ;
                XtSetValues(ma_status_lab_14_a, args, 2 ) ;
                XtSetValues(ma_status_lab_15_a, args, 2 ) ;
                XtSetValues(ma_status_lab_16_a, args, 2 ) ;



	dimensione_cursore = 0.01;
	posizione_cursore = supply[magnet[button[numero_bottone].obj2].alim1].current/supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_b, args,3);

        XtSetArg( args[0], XtNlabel,
        magnet[button[numero_bottone].obj2].n_alim1 ) ;
        XtSetValues(ma_name_label_b , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj2].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_b, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj2].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_b, args,1);


        switch (supply[magnet[button[numero_bottone].obj2].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_b , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_b , args, 1 ) ;
                break;
                }


        switch (supply[magnet[button[numero_bottone].obj2].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_lightsteelblue3 ) ;
	        XtSetValues(ma_box_a , args, 1 ) ; 
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_b , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_yellow ) ;
	        XtSetValues(ma_box_a , args, 1 ) ; 
                break;
                }


                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_b, args, 2 ) ;
                XtSetValues(ma_status_lab_2_b, args, 2 ) ;
                XtSetValues(ma_status_lab_3_b, args, 2 ) ;
                XtSetValues(ma_status_lab_4_b, args, 2 ) ;
                XtSetValues(ma_status_lab_5_b, args, 2 ) ;
                XtSetValues(ma_status_lab_6_b, args, 2 ) ;
                XtSetValues(ma_status_lab_7_b, args, 2 ) ;
                XtSetValues(ma_status_lab_8_b, args, 2 ) ;
                XtSetValues(ma_status_lab_9_b, args, 2 ) ;
                XtSetValues(ma_status_lab_10_b, args, 2 ) ;
                XtSetValues(ma_status_lab_11_b, args, 2 ) ;
                XtSetValues(ma_status_lab_12_b, args, 2 ) ;
                XtSetValues(ma_status_lab_13_b, args, 2 ) ;
                XtSetValues(ma_status_lab_14_b, args, 2 ) ;
                XtSetValues(ma_status_lab_15_b, args, 2 ) ;
                XtSetValues(ma_status_lab_16_b, args, 2 ) ;



	dimensione_cursore = 0.01;
	posizione_cursore = supply[magnet[button[numero_bottone].obj3].alim1].current/supply[magnet[button[numero_bottone].obj3].alim1].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_c, args,3);

        XtSetArg( args[0], XtNlabel,
        magnet[button[numero_bottone].obj3].n_alim1 ) ;
        XtSetValues(ma_name_label_c , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj3].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj3].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_c, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj3].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_c, args,1);


        switch (supply[magnet[button[numero_bottone].obj3].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_c , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_c , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_c , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_c , args, 1 ) ;
                break;
                }





        switch (supply[magnet[button[numero_bottone].obj3].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_c , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_lightsteelblue3 ) ;
	        XtSetValues(ma_box_c , args, 1 ) ; 
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_c , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_yellow ) ;
	        XtSetValues(ma_box_c , args, 1 ) ; 
                break;
                }


                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_c, args, 2 ) ;
                XtSetValues(ma_status_lab_2_c, args, 2 ) ;
                XtSetValues(ma_status_lab_3_c, args, 2 ) ;
                XtSetValues(ma_status_lab_4_c, args, 2 ) ;
                XtSetValues(ma_status_lab_5_c, args, 2 ) ;
                XtSetValues(ma_status_lab_6_c, args, 2 ) ;
                XtSetValues(ma_status_lab_7_c, args, 2 ) ;
                XtSetValues(ma_status_lab_8_c, args, 2 ) ;
                XtSetValues(ma_status_lab_9_c, args, 2 ) ;
                XtSetValues(ma_status_lab_10_c, args, 2 ) ;
                XtSetValues(ma_status_lab_11_c, args, 2 ) ;
                XtSetValues(ma_status_lab_12_c, args, 2 ) ;
                XtSetValues(ma_status_lab_13_c, args, 2 ) ;
                XtSetValues(ma_status_lab_14_c, args, 2 ) ;
                XtSetValues(ma_status_lab_15_c, args, 2 ) ;
                XtSetValues(ma_status_lab_16_c, args, 2 ) ;



        break;


        case 5:


	dimensione_cursore = 0.01;
	posizione_cursore = supply[magnet[button[numero_bottone].obj1].alim1].current/supply[magnet[button[numero_bottone].obj1].alim1].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_a, args,3);

        XtSetArg( args[0], XtNlabel,
                  magnet[button[numero_bottone].obj1].n_alim1 ) ;
        XtSetValues(ma_name_label_a , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj1].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj1].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_a, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj1].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_a, args,1);


        switch (supply[magnet[button[numero_bottone].obj1].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_a , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_a , args, 1 ) ;
                break;
                }




        switch (supply[magnet[button[numero_bottone].obj1].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_lightsteelblue3 ) ;
	        XtSetValues(ma_box_a , args, 1 ) ; 
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_yellow ) ;
	        XtSetValues(ma_box_a , args, 1 ) ; 
                break;
                }



                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_a, args, 2 ) ;
                XtSetValues(ma_status_lab_2_a, args, 2 ) ;
                XtSetValues(ma_status_lab_3_a, args, 2 ) ;
                XtSetValues(ma_status_lab_4_a, args, 2 ) ;
                XtSetValues(ma_status_lab_5_a, args, 2 ) ;
                XtSetValues(ma_status_lab_6_a, args, 2 ) ;
                XtSetValues(ma_status_lab_7_a, args, 2 ) ;
                XtSetValues(ma_status_lab_8_a, args, 2 ) ;
                XtSetValues(ma_status_lab_9_a, args, 2 ) ;
                XtSetValues(ma_status_lab_10_a, args, 2 ) ;
                XtSetValues(ma_status_lab_11_a, args, 2 ) ;
                XtSetValues(ma_status_lab_12_a, args, 2 ) ;
                XtSetValues(ma_status_lab_13_a, args, 2 ) ;
                XtSetValues(ma_status_lab_14_a, args, 2 ) ;
                XtSetValues(ma_status_lab_15_a, args, 2 ) ;
                XtSetValues(ma_status_lab_16_a, args, 2 ) ;


	dimensione_cursore = 0.01;
	posizione_cursore = supply[magnet[button[numero_bottone].obj1].alim2].current/supply[magnet[button[numero_bottone].obj1].alim2].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_b, args,3);
        XtSetArg( args[0], XtNlabel,
        magnet[button[numero_bottone].obj1].n_alim2 ) ;
        XtSetValues(ma_name_label_b , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj1].alim2].current * 100.0 /
        supply[magnet[button[numero_bottone].obj1].alim2].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_b, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj1].alim2].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_b, args,1);


        switch (supply[magnet[button[numero_bottone].obj1].alim2].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_b , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_b , args, 1 ) ;
                break;
                }



        switch (supply[magnet[button[numero_bottone].obj1].alim2].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_b , args, 1 ) ; // 13/12/06 era ma_remloc_but_a baco ??????
	        XtSetArg( args[0], XtNbackground, pixel_lightsteelblue3 ) ;
	        XtSetValues(ma_box_b , args, 1 ) ; 
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_b , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_yellow ) ;
	        XtSetValues(ma_box_b , args, 1 ) ; 
                break;
                }

                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_b, args, 2 ) ;
                XtSetValues(ma_status_lab_2_b, args, 2 ) ;
                XtSetValues(ma_status_lab_3_b, args, 2 ) ;
                XtSetValues(ma_status_lab_4_b, args, 2 ) ;
                XtSetValues(ma_status_lab_5_b, args, 2 ) ;
                XtSetValues(ma_status_lab_6_b, args, 2 ) ;
                XtSetValues(ma_status_lab_7_b, args, 2 ) ;
                XtSetValues(ma_status_lab_8_b, args, 2 ) ;
                XtSetValues(ma_status_lab_9_b, args, 2 ) ;
                XtSetValues(ma_status_lab_10_b, args, 2 ) ;
                XtSetValues(ma_status_lab_11_b, args, 2 ) ;
                XtSetValues(ma_status_lab_12_b, args, 2 ) ;
                XtSetValues(ma_status_lab_13_b, args, 2 ) ;
                XtSetValues(ma_status_lab_14_b, args, 2 ) ;
                XtSetValues(ma_status_lab_15_b, args, 2 ) ;
                XtSetValues(ma_status_lab_16_b, args, 2 ) ;
	dimensione_cursore = 0.01;
	posizione_cursore = supply[magnet[button[numero_bottone].obj2].alim1].current/supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_c, args,3);

        XtSetArg( args[0], XtNlabel,
        magnet[button[numero_bottone].obj2].n_alim1 ) ;
        XtSetValues(ma_name_label_c , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj2].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_c, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj2].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_c, args,1);


        switch (supply[magnet[button[numero_bottone].obj2].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_c , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_c , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_c , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_c , args, 1 ) ;
                break;
                }




        switch (supply[magnet[button[numero_bottone].obj2].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_c , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_lightsteelblue3 ) ;
	        XtSetValues(ma_box_c , args, 1 ) ; 
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_c , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_yellow ) ;
	        XtSetValues(ma_box_c , args, 1 ) ; 
                break;
                }


                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_c, args, 2 ) ;
                XtSetValues(ma_status_lab_2_c, args, 2 ) ;
                XtSetValues(ma_status_lab_3_c, args, 2 ) ;
                XtSetValues(ma_status_lab_4_c, args, 2 ) ;
                XtSetValues(ma_status_lab_5_c, args, 2 ) ;
                XtSetValues(ma_status_lab_6_c, args, 2 ) ;
                XtSetValues(ma_status_lab_7_c, args, 2 ) ;
                XtSetValues(ma_status_lab_8_c, args, 2 ) ;
                XtSetValues(ma_status_lab_9_c, args, 2 ) ;
                XtSetValues(ma_status_lab_10_c, args, 2 ) ;
                XtSetValues(ma_status_lab_11_c, args, 2 ) ;
                XtSetValues(ma_status_lab_12_c, args, 2 ) ;
                XtSetValues(ma_status_lab_13_c, args, 2 ) ;
                XtSetValues(ma_status_lab_14_c, args, 2 ) ;
                XtSetValues(ma_status_lab_15_c, args, 2 ) ;
                XtSetValues(ma_status_lab_16_c, args, 2 ) ;


        break;



        case 6:
	dimensione_cursore = 0.01;
	posizione_cursore = supply[magnet[button[numero_bottone].obj1].alim1].current/supply[magnet[button[numero_bottone].obj1].alim1].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_a, args,3);

        XtSetArg( args[0], XtNlabel,
                  magnet[button[numero_bottone].obj1].n_alim1 ) ;
        XtSetValues(ma_name_label_a , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj1].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj1].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_a, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj1].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_a, args,1);


        switch (supply[magnet[button[numero_bottone].obj1].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_a , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_a , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_a , args, 1 ) ;
                break;
                }





        switch (supply[magnet[button[numero_bottone].obj1].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_lightsteelblue3 ) ;
	        XtSetValues(ma_box_a , args, 1 ) ; 
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_yellow ) ;
	        XtSetValues(ma_box_a , args, 1 ) ; 
                break;
                }



                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_a, args, 2 ) ;
                XtSetValues(ma_status_lab_2_a, args, 2 ) ;
                XtSetValues(ma_status_lab_3_a, args, 2 ) ;
                XtSetValues(ma_status_lab_4_a, args, 2 ) ;
                XtSetValues(ma_status_lab_5_a, args, 2 ) ;
                XtSetValues(ma_status_lab_6_a, args, 2 ) ;
                XtSetValues(ma_status_lab_7_a, args, 2 ) ;
                XtSetValues(ma_status_lab_8_a, args, 2 ) ;
                XtSetValues(ma_status_lab_9_a, args, 2 ) ;
                XtSetValues(ma_status_lab_10_a, args, 2 ) ;
                XtSetValues(ma_status_lab_11_a, args, 2 ) ;
                XtSetValues(ma_status_lab_12_a, args, 2 ) ;
                XtSetValues(ma_status_lab_13_a, args, 2 ) ;
                XtSetValues(ma_status_lab_14_a, args, 2 ) ;
                XtSetValues(ma_status_lab_15_a, args, 2 ) ;
                XtSetValues(ma_status_lab_16_a, args, 2 ) ;


	dimensione_cursore = 0.01;
	posizione_cursore = supply[magnet[button[numero_bottone].obj2].alim1].current/supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_b, args,3);

        XtSetArg( args[0], XtNlabel,
                  magnet[button[numero_bottone].obj2].n_alim1 ) ;
        XtSetValues(ma_name_label_b , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj2].alim1].current * 100.0 /
        supply[magnet[button[numero_bottone].obj1].alim1].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_b, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj2].alim1].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_b, args,1);


        switch (supply[magnet[button[numero_bottone].obj2].alim1].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_b , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_b , args, 3 ) ;
                    XtSetArg( args[0], XtNbackground, pixel_black ) ;
                    XtSetValues(ma_box_b , args, 1 ) ;
                break;
                }




        switch (supply[magnet[button[numero_bottone].obj2].alim1].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_lightsteelblue3 ) ;
	        XtSetValues(ma_box_a , args, 1 ) ; 
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_a , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_yellow ) ;
	        XtSetValues(ma_box_a , args, 1 ) ; 
                break;
                }



                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_b, args, 2 ) ;
                XtSetValues(ma_status_lab_2_b, args, 2 ) ;
                XtSetValues(ma_status_lab_3_b, args, 2 ) ;
                XtSetValues(ma_status_lab_4_b, args, 2 ) ;
                XtSetValues(ma_status_lab_5_b, args, 2 ) ;
                XtSetValues(ma_status_lab_6_b, args, 2 ) ;
                XtSetValues(ma_status_lab_7_b, args, 2 ) ;
                XtSetValues(ma_status_lab_8_b, args, 2 ) ;
                XtSetValues(ma_status_lab_9_b, args, 2 ) ;
                XtSetValues(ma_status_lab_10_b, args, 2 ) ;
                XtSetValues(ma_status_lab_11_b, args, 2 ) ;
                XtSetValues(ma_status_lab_12_b, args, 2 ) ;
                XtSetValues(ma_status_lab_13_b, args, 2 ) ;
                XtSetValues(ma_status_lab_14_b, args, 2 ) ;
                XtSetValues(ma_status_lab_15_b, args, 2 ) ;
                XtSetValues(ma_status_lab_16_b, args, 2 ) ;


	dimensione_cursore = 0.01;
	posizione_cursore = supply[magnet[button[numero_bottone].obj2].alim2].current/supply[magnet[button[numero_bottone].obj2].alim2].maxcurrent;
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
	     XtSetValues(ma_cur_ruler_c, args,3);

        XtSetArg( args[0], XtNlabel,
                  magnet[button[numero_bottone].obj2].n_alim2 ) ;
        XtSetValues(ma_name_label_c , args, 1 ) ;


        sprintf(string, "%8.4f %%",
        supply[magnet[button[numero_bottone].obj2].alim2].current * 100.0 /
        supply[magnet[button[numero_bottone].obj2].alim2].maxcurrent );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_perc_lab_c, args,1);
        sprintf(string, "%8.4f A",
        supply[magnet[button[numero_bottone].obj2].alim2].current );
        XtSetArg(args[0],XtNlabel,string);
        XtSetValues(ma_curimp_lab_c, args,1);


        switch (supply[magnet[button[numero_bottone].obj2].alim2].on_off)
                {
                case ON:        /* devo scrivere ON */
                XtSetArg( args[0], XtNlabel, "ON" ) ;
                XtSetArg( args[1], XtNbackground, pixel_orange ) ;
                XtSetValues(ma_onoff_but_c , args, 2 ) ;
                break;

                case OFF:        /* devo scrivere OFF */
                XtSetArg( args[0], XtNlabel, "OFF" ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey ) ;
                XtSetValues(ma_onoff_but_c , args, 2 ) ;
                break;
                

                case STUB:        /* devo scrivere STUB */
                XtSetArg( args[0], XtNlabel, "STUB" ) ;
                XtSetArg( args[1], XtNbackground, pixel_white ) ;
                XtSetArg( args[2], XtNforeground, pixel_black ) ;
                XtSetValues(ma_onoff_but_c , args, 3 ) ;
                XtSetArg( args[0], XtNbackground, pixel_black ) ;
                XtSetValues(ma_box_c , args, 1 ) ;
                break;
                }




        switch (supply[magnet[button[numero_bottone].obj2].alim2].rem_loc)
                {
                case REMOTE:        /* devo scrivere REMOTE */
                XtSetArg( args[0], XtNlabel, "REMOTE" ) ;
                XtSetValues(ma_remloc_but_c , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_lightsteelblue3 ) ;
	        XtSetValues(ma_box_c , args, 1 ) ; 
                break;

                case LOCAL:        /* devo scrivere LOCAL */
                XtSetArg( args[0], XtNlabel, "LOCAL" ) ;
                XtSetValues(ma_remloc_but_c , args, 1 ) ;
	        XtSetArg( args[0], XtNbackground, pixel_yellow ) ;
	        XtSetValues(ma_box_c , args, 1 ) ; 
                break;
                }


                XtSetArg( args[0], XtNlabel, "        " ) ;
                XtSetArg( args[1], XtNbackground, pixel_grey);

                XtSetValues(ma_status_lab_1_c, args, 2 ) ;
                XtSetValues(ma_status_lab_2_c, args, 2 ) ;
                XtSetValues(ma_status_lab_3_c, args, 2 ) ;
                XtSetValues(ma_status_lab_4_c, args, 2 ) ;
                XtSetValues(ma_status_lab_5_c, args, 2 ) ;
                XtSetValues(ma_status_lab_6_c, args, 2 ) ;
                XtSetValues(ma_status_lab_7_c, args, 2 ) ;
                XtSetValues(ma_status_lab_8_c, args, 2 ) ;
                XtSetValues(ma_status_lab_9_c, args, 2 ) ;
                XtSetValues(ma_status_lab_10_c, args, 2 ) ;
                XtSetValues(ma_status_lab_11_c, args, 2 ) ;
                XtSetValues(ma_status_lab_12_c, args, 2 ) ;
                XtSetValues(ma_status_lab_13_c, args, 2 ) ;
                XtSetValues(ma_status_lab_14_c, args, 2 ) ;
                XtSetValues(ma_status_lab_15_c, args, 2 ) ;
                XtSetValues(ma_status_lab_16_c, args, 2 ) ;

        break;

        }

}



crea_pag_1()
{

        Arg args[9] ;
        int counter;

        ma_combox( 0 ) ;

        ma_box_a = XtCreateWidget( "ma_box_a", formWidgetClass,
                                   ma_setup_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNfromVert, ma_refr_onoff_but );
        XtSetValues(ma_box_a , args, 1 ) ;
        XtManageChild(ma_box_a);

        supply[magnet[numero_bottone].alim1].finestra = 1;

        fill_box((int)magnet[button[numero_bottone].obj2].alim1, 
        ma_box_a, 
        &ma_name_label_a, 
        &ma_cur_lab_a, 
        &ma_curimp_name_lab_a,
        &ma_curatt_name_lab_a,
        &ma_curimp_lab_a, 
        &ma_curatt_lab_a, 
        &ma_curimp_perc_lab_a, 
        &ma_curatt_perc_lab_a, 
        &ma_curup_but_a, 
        &ma_curdwn_but_a, 
        &ma_curup_fast_but_a, 
        &ma_curdwn_fast_but_a, 
        &ma_cur_ruler_a,
        &ma_volt_text_lab_a,
        &ma_volt_perc_lab_a,  
        &ma_temp_text_lab_a, 
        &ma_trans_text_lab_a,
        &ma_v1_text_lab_a,
        &ma_v2_text_lab_a,
        &ma_v3_text_lab_a,
        &ma_volt_lab_a, 
        &ma_temp_lab_a, 
        &ma_trans_lab_a,
        &ma_v1_lab_a,
        &ma_v2_lab_a,
        &ma_v3_lab_a,
        &ma_onoff_but_a,
        &ma_remloc_but_a,
        &ma_reset_but_a,
        magnet[numero_bottone].n_alim1,
        &ma_status_lab_1_a,
        &ma_status_lab_2_a,
        &ma_status_lab_3_a,
        &ma_status_lab_4_a,
        &ma_status_lab_5_a,
        &ma_status_lab_6_a,
        &ma_status_lab_7_a,
        &ma_status_lab_8_a,
        &ma_status_lab_9_a,
        &ma_status_lab_10_a,
        &ma_status_lab_11_a,
        &ma_status_lab_12_a,
        &ma_status_lab_13_a,
        &ma_status_lab_14_a,
        &ma_status_lab_15_a,
        &ma_status_lab_16_a,
        &ma_grip_a, &ma_incdec_a
        ); /* STEFANIA : aggiunto il widget grip e incdec */

}



crea_pag_3()
{

char *nome1, *nome2, *nome3;
int alim_1, alim_2, alim_3;

        Arg args[9] ;
        int counter;

        ma_combox( 0 ) ;

        ma_box_a = XtCreateWidget( "ma_box_a", formWidgetClass,
                                   ma_setup_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNfromVert, ma_refr_onoff_but );
        XtSetValues(ma_box_a , args, 1 ) ;
        XtManageChild(ma_box_a);

        ma_box_b = XtCreateWidget( "ma_box_b", formWidgetClass,
                                   ma_setup_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNfromVert, ma_box_a);
        XtSetValues(ma_box_b , args, 1 ) ;
        XtManageChild(ma_box_b);

        ma_box_c = XtCreateWidget( "ma_box_c", formWidgetClass,
                                   ma_setup_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNfromVert, ma_box_b);
        XtSetValues(ma_box_c , args, 1 ) ;
        XtManageChild(ma_box_c);

        switch ( button[numero_bottone].type )
                
                {

                case 3:

         supply[magnet[button[numero_bottone].obj1].alim1].finestra = 1;

                fill_box(magnet[button[numero_bottone].obj1].alim1, 
                ma_box_a, 
                &ma_name_label_a, 
                &ma_cur_lab_a, 
                &ma_curimp_name_lab_a,
                &ma_curatt_name_lab_a,
                &ma_curimp_lab_a, 
                &ma_curatt_lab_a, 
                &ma_curimp_perc_lab_a, 
                &ma_curatt_perc_lab_a, 
                &ma_curup_but_a, 
                &ma_curdwn_but_a, 
                &ma_curup_fast_but_a, 
                &ma_curdwn_fast_but_a, 
                &ma_cur_ruler_a, 
                &ma_volt_text_lab_a,
                &ma_volt_perc_lab_a,  
                &ma_temp_text_lab_a, 
                &ma_trans_text_lab_a,
                &ma_v1_text_lab_a,
                &ma_v2_text_lab_a,
                &ma_v3_text_lab_a,
                &ma_volt_lab_a, 
                &ma_temp_lab_a, 
                &ma_trans_lab_a,
                &ma_v1_lab_a,
                &ma_v2_lab_a,
                &ma_v3_lab_a,
                &ma_onoff_but_a,
                &ma_remloc_but_a,
                &ma_reset_but_a,
                magnet[button[numero_bottone].obj1].n_alim1,
                &ma_status_lab_1_a,
                &ma_status_lab_2_a,
                &ma_status_lab_3_a,
                &ma_status_lab_4_a,
                &ma_status_lab_5_a,
                &ma_status_lab_6_a,
                &ma_status_lab_7_a,
                &ma_status_lab_8_a,
                &ma_status_lab_9_a,
                &ma_status_lab_10_a,
                &ma_status_lab_11_a,
                &ma_status_lab_12_a,
                &ma_status_lab_13_a,
                &ma_status_lab_14_a,
                &ma_status_lab_15_a,
                &ma_status_lab_16_a,
                &ma_grip_a, &ma_incdec_a
                ); /* STEFANIA : aggiunto il widget grip e incdec */

        supply[magnet[button[numero_bottone].obj2].alim1].finestra = 2;

                fill_box( magnet[button[numero_bottone].obj2].alim1,
                ma_box_b, 
                &ma_name_label_b, 
                &ma_cur_lab_b, 
                &ma_curimp_name_lab_b,
                &ma_curatt_name_lab_b,
                &ma_curimp_lab_b, 
                &ma_curatt_lab_b, 
                &ma_curimp_perc_lab_b, 
                &ma_curatt_perc_lab_b, 
                &ma_curup_but_b, 
                &ma_curdwn_but_b, 
                &ma_curup_fast_but_b, 
                &ma_curdwn_fast_but_b, 
                &ma_cur_ruler_b, 
                &ma_volt_text_lab_b,
                &ma_volt_perc_lab_b,  
                &ma_temp_text_lab_b, 
                &ma_trans_text_lab_b,
                &ma_v1_text_lab_b,
                &ma_v2_text_lab_b,
                &ma_v3_text_lab_b,
                &ma_volt_lab_b, 
                &ma_temp_lab_b, 
                &ma_trans_lab_b,
                &ma_v1_lab_b,
                &ma_v2_lab_b,
                &ma_v3_lab_b,
                &ma_onoff_but_b,
                &ma_remloc_but_b,
                &ma_reset_but_b,
                magnet[button[numero_bottone].obj2].n_alim1,
                &ma_status_lab_1_b,
                &ma_status_lab_2_b,
                &ma_status_lab_3_b,
                &ma_status_lab_4_b,
                &ma_status_lab_5_b,
                &ma_status_lab_6_b,
                &ma_status_lab_7_b,
                &ma_status_lab_8_b,
                &ma_status_lab_9_b,
                &ma_status_lab_10_b,
                &ma_status_lab_11_b,
                &ma_status_lab_12_b,
                &ma_status_lab_13_b,
                &ma_status_lab_14_b,
                &ma_status_lab_15_b,
                &ma_status_lab_16_b,
                &ma_grip_b, &ma_incdec_b
                ); /* STEFANIA : aggiunto il widget grip e incdec */

        supply[magnet[button[numero_bottone].obj3].alim1].finestra = 3;

                fill_box(magnet[button[numero_bottone].obj3].alim1,
                ma_box_c, 
                &ma_name_label_c, 
                &ma_cur_lab_c, 
                &ma_curimp_name_lab_c,
                &ma_curatt_name_lab_c,
                &ma_curimp_lab_c, 
                &ma_curatt_lab_c, 
                &ma_curimp_perc_lab_c, 
                &ma_curatt_perc_lab_c, 
                &ma_curup_but_c, 
                &ma_curdwn_but_c, 
                &ma_curup_fast_but_c, 
                &ma_curdwn_fast_but_c, 
                &ma_cur_ruler_c, 
                &ma_volt_text_lab_c,
                &ma_volt_perc_lab_c,  
                &ma_temp_text_lab_c, 
                &ma_trans_text_lab_c,
                &ma_v1_text_lab_c,
                &ma_v2_text_lab_c,
                &ma_v3_text_lab_c,
                &ma_volt_lab_c, 
                &ma_temp_lab_c, 
                &ma_trans_lab_c,
                &ma_v1_lab_c,
                &ma_v2_lab_c,
                &ma_v3_lab_c,
                &ma_onoff_but_c,
                &ma_remloc_but_c,
                &ma_reset_but_c,
                magnet[button[numero_bottone].obj3].n_alim1,
                &ma_status_lab_1_c,
                &ma_status_lab_2_c,
                &ma_status_lab_3_c,
                &ma_status_lab_4_c,
                &ma_status_lab_5_c,
                &ma_status_lab_6_c,
                &ma_status_lab_7_c,
                &ma_status_lab_8_c,
                &ma_status_lab_9_c,
                &ma_status_lab_10_c,
                &ma_status_lab_11_c,
                &ma_status_lab_12_c,
                &ma_status_lab_13_c,
                &ma_status_lab_14_c,
                &ma_status_lab_15_c,
                &ma_status_lab_16_c,
                &ma_grip_c, &ma_incdec_c
                ); /* STEFANIA : aggiunto il widget grip e incdec */

                break;

                case 5:
                
        supply[magnet[button[numero_bottone].obj1].alim1].finestra = 1;

                fill_box(magnet[button[numero_bottone].obj1].alim1, 
                ma_box_a, 
                &ma_name_label_a, 
                &ma_cur_lab_a, 
                &ma_curimp_name_lab_a,
                &ma_curatt_name_lab_a,
                &ma_curimp_lab_a, 
                &ma_curatt_lab_a, 
                &ma_curimp_perc_lab_a, 
                &ma_curatt_perc_lab_a, 
                &ma_curup_but_a, 
                &ma_curdwn_but_a, 
                &ma_curup_fast_but_a, 
                &ma_curdwn_fast_but_a, 
                &ma_cur_ruler_a, 
                &ma_volt_text_lab_a,
                &ma_volt_perc_lab_a,  
                &ma_temp_text_lab_a, 
                &ma_trans_text_lab_a,
                &ma_v1_text_lab_a,
                &ma_v2_text_lab_a,
                &ma_v3_text_lab_a,
                &ma_volt_lab_a, 
                &ma_temp_lab_a, 
                &ma_trans_lab_a,
                &ma_v1_lab_a,
                &ma_v2_lab_a,
                &ma_v3_lab_a,
                &ma_onoff_but_a,
                &ma_remloc_but_a,
                &ma_reset_but_a,
                magnet[button[numero_bottone].obj1].n_alim1,
                &ma_status_lab_1_a,
                &ma_status_lab_2_a,
                &ma_status_lab_3_a,
                &ma_status_lab_4_a,
                &ma_status_lab_5_a,
                &ma_status_lab_6_a,
                &ma_status_lab_7_a,
                &ma_status_lab_8_a,
                &ma_status_lab_9_a,
                &ma_status_lab_10_a,
                &ma_status_lab_11_a,
                &ma_status_lab_12_a,
                &ma_status_lab_13_a,
                &ma_status_lab_14_a,
                &ma_status_lab_15_a,
                &ma_status_lab_16_a,
                &ma_grip_a, &ma_incdec_a
                ); /* STEFANIA : aggiunto il widget grip e incdec */

        supply[magnet[button[numero_bottone].obj1].alim2].finestra = 2;

                fill_box( magnet[button[numero_bottone].obj1].alim2,
                ma_box_b, 
                &ma_name_label_b, 
                &ma_cur_lab_b, 
                &ma_curimp_name_lab_b,
                &ma_curatt_name_lab_b,
                &ma_curimp_lab_b, 
                &ma_curatt_lab_b, 
                &ma_curimp_perc_lab_b, 
                &ma_curatt_perc_lab_b, 
                &ma_curup_but_b, 
                &ma_curdwn_but_b, 
                &ma_curup_fast_but_b, 
                &ma_curdwn_fast_but_b, 
                &ma_cur_ruler_b, 
                &ma_volt_text_lab_b, 
                &ma_volt_perc_lab_b, 
                &ma_temp_text_lab_b, 
                &ma_trans_text_lab_b,
                &ma_v1_text_lab_b,
                &ma_v2_text_lab_b,
                &ma_v3_text_lab_b,
                &ma_volt_lab_b, 
                &ma_temp_lab_b, 
                &ma_trans_lab_b,
                &ma_v1_lab_b,
                &ma_v2_lab_b,
                &ma_v3_lab_b,
                &ma_onoff_but_b,
                &ma_remloc_but_b,
                &ma_reset_but_b,
                magnet[button[numero_bottone].obj1].n_alim2,
                &ma_status_lab_1_b,
                &ma_status_lab_2_b,
                &ma_status_lab_3_b,
                &ma_status_lab_4_b,
                &ma_status_lab_5_b,
                &ma_status_lab_6_b,
                &ma_status_lab_7_b,
                &ma_status_lab_8_b,
                &ma_status_lab_9_b,
                &ma_status_lab_10_b,
                &ma_status_lab_11_b,
                &ma_status_lab_12_b,
                &ma_status_lab_13_b,
                &ma_status_lab_14_b,
                &ma_status_lab_15_b,
                &ma_status_lab_16_b,
                &ma_grip_b, &ma_incdec_b
                ); /* STEFANIA : aggiunto il widget grip e incdec */

        supply[magnet[button[numero_bottone].obj2].alim1].finestra = 3;

                fill_box(magnet[button[numero_bottone].obj2].alim1,
                ma_box_c, 
                &ma_name_label_c, 
                &ma_cur_lab_c, 
                &ma_curimp_name_lab_c,
                &ma_curatt_name_lab_c,
                &ma_curimp_lab_c, 
                &ma_curatt_lab_c, 
                &ma_curimp_perc_lab_c, 
                &ma_curatt_perc_lab_c, 
                &ma_curup_but_c, 
                &ma_curdwn_but_c, 
                &ma_curup_fast_but_c, 
                &ma_curdwn_fast_but_c, 
                &ma_cur_ruler_c, 
                &ma_volt_text_lab_c,
                &ma_volt_perc_lab_c,  
                &ma_temp_text_lab_c, 
                &ma_trans_text_lab_c,
                &ma_v1_text_lab_c,
                &ma_v2_text_lab_c,
                &ma_v3_text_lab_c,
                &ma_volt_lab_c, 
                &ma_temp_lab_c, 
                &ma_trans_lab_c,
                &ma_v1_lab_c,
                &ma_v2_lab_c,
                &ma_v3_lab_c,
                &ma_onoff_but_c,
                &ma_remloc_but_c,
                &ma_reset_but_c,
                magnet[button[numero_bottone].obj2].n_alim1,
                &ma_status_lab_1_c,
                &ma_status_lab_2_c,
                &ma_status_lab_3_c,
                &ma_status_lab_4_c,
                &ma_status_lab_5_c,
                &ma_status_lab_6_c,
                &ma_status_lab_7_c,
                &ma_status_lab_8_c,
                &ma_status_lab_9_c,
                &ma_status_lab_10_c,
                &ma_status_lab_11_c,
                &ma_status_lab_12_c,
                &ma_status_lab_13_c,
                &ma_status_lab_14_c,
                &ma_status_lab_15_c,
                &ma_status_lab_16_c,
                &ma_grip_c, &ma_incdec_c
                ); /* STEFANIA : aggiunto il widget grip e incdec */

                break;

                case 6:

        supply[magnet[button[numero_bottone].obj1].alim1].finestra = 1;

                fill_box(magnet[button[numero_bottone].obj1].alim1, 
                ma_box_a, 
                &ma_name_label_a, 
                &ma_cur_lab_a, 
                &ma_curimp_name_lab_a,
                &ma_curatt_name_lab_a,
                &ma_curimp_lab_a, 
                &ma_curatt_lab_a, 
                &ma_curimp_perc_lab_a, 
                &ma_curatt_perc_lab_a, 
                &ma_curup_but_a, 
                &ma_curdwn_but_a, 
                &ma_curup_fast_but_a, 
                &ma_curdwn_fast_but_a, 
                &ma_cur_ruler_a, 
                &ma_volt_text_lab_a,
                &ma_volt_perc_lab_a,  
                &ma_temp_text_lab_a, 
                &ma_trans_text_lab_a,
                &ma_v1_text_lab_a,
                &ma_v2_text_lab_a,
                &ma_v3_text_lab_a,
                &ma_volt_lab_a, 
                &ma_temp_lab_a, 
                &ma_trans_lab_a,
                &ma_v1_lab_a,
                &ma_v2_lab_a,
                &ma_v3_lab_a,
                &ma_onoff_but_a,
                &ma_remloc_but_a,
                &ma_reset_but_a,
                magnet[button[numero_bottone].obj1].n_alim1,
                &ma_status_lab_1_a,
                &ma_status_lab_2_a,
                &ma_status_lab_3_a,
                &ma_status_lab_4_a,
                &ma_status_lab_5_a,
                &ma_status_lab_6_a,
                &ma_status_lab_7_a,
                &ma_status_lab_8_a,
                &ma_status_lab_9_a,
                &ma_status_lab_10_a,
                &ma_status_lab_11_a,
                &ma_status_lab_12_a,
                &ma_status_lab_13_a,
                &ma_status_lab_14_a,
                &ma_status_lab_15_a,
                &ma_status_lab_16_a,
                &ma_grip_a, &ma_incdec_a
                ); /* STEFANIA : aggiunto il widget grip e incdec */

        supply[magnet[button[numero_bottone].obj2].alim1].finestra = 2;

                fill_box( magnet[button[numero_bottone].obj2].alim1,
                ma_box_b, 
                &ma_name_label_b, 
                &ma_cur_lab_b, 
                &ma_curimp_name_lab_b,
                &ma_curatt_name_lab_b,
                &ma_curimp_lab_b, 
                &ma_curatt_lab_b, 
                &ma_curimp_perc_lab_b, 
                &ma_curatt_perc_lab_b, 
                &ma_curup_but_b, 
                &ma_curdwn_but_b, 
                &ma_curup_fast_but_b, 
                &ma_curdwn_fast_but_b, 
                &ma_cur_ruler_b, 
                &ma_volt_text_lab_b, 
                &ma_volt_perc_lab_b, 
                &ma_temp_text_lab_b, 
                &ma_trans_text_lab_b,
                &ma_v1_text_lab_b,
                &ma_v2_text_lab_b,
                &ma_v3_text_lab_b,
                &ma_volt_lab_b, 
                &ma_temp_lab_b, 
                &ma_trans_lab_b,
                &ma_v1_lab_b,
                &ma_v2_lab_b,
                &ma_v3_lab_b,
                &ma_onoff_but_b,
                &ma_remloc_but_b,
                &ma_reset_but_b,
                magnet[button[numero_bottone].obj2].n_alim1,
                &ma_status_lab_1_b,
                &ma_status_lab_2_b,
                &ma_status_lab_3_b,
                &ma_status_lab_4_b,
                &ma_status_lab_5_b,
                &ma_status_lab_6_b,
                &ma_status_lab_7_b,
                &ma_status_lab_8_b,
                &ma_status_lab_9_b,
                &ma_status_lab_10_b,
                &ma_status_lab_11_b,
                &ma_status_lab_12_b,
                &ma_status_lab_13_b,
                &ma_status_lab_14_b,
                &ma_status_lab_15_b,
                &ma_status_lab_16_b,
                &ma_grip_b, &ma_incdec_b
                ); /* STEFANIA : aggiunto il widget grip e incdec */

        supply[magnet[button[numero_bottone].obj2].alim2].finestra = 3;

                fill_box(magnet[button[numero_bottone].obj2].alim2,
                ma_box_c, 
                &ma_name_label_c, 
                &ma_cur_lab_c, 
                &ma_curimp_name_lab_c,
                &ma_curatt_name_lab_c,
                &ma_curimp_lab_c, 
                &ma_curatt_lab_c, 
                &ma_curimp_perc_lab_c, 
                &ma_curatt_perc_lab_c, 
                &ma_curup_but_c, 
                &ma_curdwn_but_c, 
                &ma_curup_fast_but_c, 
                &ma_curdwn_fast_but_c, 
                &ma_cur_ruler_c, 
                &ma_volt_text_lab_c,
                &ma_volt_perc_lab_c, 
                &ma_temp_text_lab_c, 
                &ma_trans_text_lab_c,
                &ma_v1_text_lab_c,
                &ma_v2_text_lab_c,
                &ma_v3_text_lab_c,
                &ma_volt_lab_c, 
                &ma_temp_lab_c, 
                &ma_trans_lab_c,
                &ma_v1_lab_c,
                &ma_v2_lab_c,
                &ma_v3_lab_c,
                &ma_onoff_but_c,
                &ma_remloc_but_c,
                &ma_reset_but_c,
                magnet[button[numero_bottone].obj2].n_alim2,
                &ma_status_lab_1_c,
                &ma_status_lab_2_c,
                &ma_status_lab_3_c,
                &ma_status_lab_4_c,
                &ma_status_lab_5_c,
                &ma_status_lab_6_c,
                &ma_status_lab_7_c,
                &ma_status_lab_8_c,
                &ma_status_lab_9_c,
                &ma_status_lab_10_c,
                &ma_status_lab_11_c,
                &ma_status_lab_12_c,
                &ma_status_lab_13_c,
                &ma_status_lab_14_c,
                &ma_status_lab_15_c,
                &ma_status_lab_16_c,
                &ma_grip_c, &ma_incdec_c
                ); /* STEFANIA : aggiunto il widget grip e incdec */

                break;

                }

}



/* STEFANIA2 (il ritorno) : pagina di tipo 7, solo 2 steerer */
crea_pag_7()
{
    /* crea la bottoniera generale */
    ma_combox( 1 ) ;
    crea_steerer_page( 1 );
    st_refresh_a();
    st_refresh_b();
//    disegna(steerer[button[numero_bottone].obj3].alim1);
//    disegna(steerer[button[numero_bottone].obj1].alim1);
}

/* STEFANIA2 (il ritorno) : pagina di tipo 8, solo 1 steerer */
crea_pag_8()
{
    /* crea la bottoniera generale */
    ma_combox( 1 ) ;
    crea_steerer_page( 2 );
    st_refresh_a();
}

/* STEFANIA : call back della grip area  */
void ma_grip_act(w, index, call_data)
    Widget w;
    int index;
    GripCallDataRec *call_data;
{
    extern int verbose ;
    void ma_cur_right(), ma_cur_left() ;
    void ma_cur_up(), ma_cur_down() ;
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
    if (!strncmp(*(call_data->params), "right", 5))  {
       XSync( XtDisplay(w), TRUE);
       /*if (verbose)
           printf( "up alim %d\n", index ) ;*/
       ma_cur_right( index ) ;
       }
    if (!strncmp(*(call_data->params), "left", 4))  {
       XSync( XtDisplay(w), TRUE);
       /*if (verbose)
           printf( "down alim %d\n", index ) ;*/
       ma_cur_left( index ) ;
       }
    if (!strncmp(*(call_data->params), "up", 2))  {
       XSync( XtDisplay(w), TRUE);
       if (verbose)
           printf( "up inc\n" ) ;
       ma_cur_up( index ) ;
       }
    if (!strncmp(*(call_data->params), "down", 4))  {
       XSync( XtDisplay(w), TRUE);
       if (verbose)
           printf( "down inc\n" ) ;
       ma_cur_down( index ) ;
       }
}

/* STEFANIA: end call back della grip area */


/* STEFANIA: routine associata al tasto "arrow =>" (right) */

void ma_cur_right( supply_number )
    int supply_number ;

{
extern void ma2q3q_refresh();
int value_to_send;

if ( supply[supply_number].wa_current < ma_max_wa_key )
    {
    supply[supply_number].wa_current =
        supply[supply_number].wa_current + ma_step_key ;
    supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
    }


value_to_send = supply[supply_number].wa_current ;


ma_current_cmd( supply_number, value_to_send ) ;

if ( button[ numero_bottone ].type == 2 ||
     button[ numero_bottone ].type == 4 ) ma2q3q_refresh() ;

ma_current_upd( supply_number ) ;

};


/* STEFANIA: routine associata al tasto "arrow <=" (left) */

void ma_cur_left( supply_number )
    int supply_number ;

{
extern void ma2q3q_refresh();
int value_to_send;

if ( supply[supply_number].wa_current > ma_min_wa_key )
    {
    supply[supply_number].wa_current =
        supply[supply_number].wa_current - ma_step_key ;
    supply[supply_number].current = supply[supply_number].wa_current*
        (supply[supply_number].maxcurrent/1000000.0);
    }


value_to_send = supply[supply_number].wa_current ;


ma_current_cmd( supply_number, value_to_send ) ;

if ( button[ numero_bottone ].type == 2 ||
     button[ numero_bottone ].type == 4 ) ma2q3q_refresh() ;

ma_current_upd( supply_number ) ;

};

/* STEFANIA: routine associata al tasto "arrow ^" (up) */

void ma_cur_up( supply_number )
    int supply_number ;

{
    Arg arg;
    char string[8] ;

    if ( ma_step_key < 25 ) {
        switch ( ma_step_key ) {
            case 10:
            ma_step_key = 25 ;
            break ;
            case 5:
            ma_step_key = 10 ;
            break ;
            case 2:
            ma_step_key = 5 ;
            break ;
            default:
            ma_step_key = 2 ;
            break ;
            }
        ma_min_wa_key =           ma_step_key - 1 ;
        ma_max_wa_key = ( 1000000 - ma_step_key ) ;
        }
    else
        if ( ma_step_key < 2000 ) {
            ma_step_key   =           ma_step_key * 2 ;
            ma_min_wa_key =           ma_step_key - 1 ;
            ma_max_wa_key = ( 1000000 - ma_step_key ) ;
            }

    /* aggiornamento label inc/dec */
    sprintf( string, "%d", ma_step_key ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    switch ( button[bottone_premuto].type )
        {
        case 1:
        XtSetValues( ma_incdec_a, &arg, 1 ) ;
        break;

        case 2:
        case 4:
        XtSetValues( ma_incdec_a, &arg, 1 ) ;
        XtSetValues( ma_incdec_b, &arg, 1 ) ;
        break;

        case 3:
        case 5:
        case 6:
        XtSetValues( ma_incdec_a, &arg, 1 ) ;
        XtSetValues( ma_incdec_b, &arg, 1 ) ;
        XtSetValues( ma_incdec_c, &arg, 1 ) ;
        break;

        }

};


/* STEFANIA: routine associata al tasto "arrow v" (down) */

void ma_cur_down( supply_number )
    int supply_number ;

{
    Arg arg;
    char string[8] ;

    if ( ma_step_key < 50 ) {
        switch ( ma_step_key ) {
            case 25:
            ma_step_key = 10 ;
            break ;
            case 10:
            ma_step_key = 5 ;
            break ;
            case 5:
            ma_step_key = 2 ;
            break ;
            default:
            ma_step_key = 1 ;
            break ;
            }
        ma_min_wa_key =           ma_step_key - 1 ;
        ma_max_wa_key = ( 1000000 - ma_step_key ) ;
        }
    else {
        ma_step_key   =           ma_step_key / 2 ;
        ma_min_wa_key =           ma_step_key - 1 ;
        ma_max_wa_key = ( 1000000 - ma_step_key ) ;
        }

    /* aggiornamento label inc/dec */
    sprintf( string, "%d", ma_step_key ) ;
    XtSetArg( arg, XtNlabel, string ) ;
    switch ( button[bottone_premuto].type )
        {
        case 1:
        XtSetValues( ma_incdec_a, &arg, 1 ) ;
        break;

        case 2:
        case 4:
        XtSetValues( ma_incdec_a, &arg, 1 ) ;
        XtSetValues( ma_incdec_b, &arg, 1 ) ;
        break;

        case 3:
        case 5:
        case 6:
        XtSetValues( ma_incdec_a, &arg, 1 ) ;
        XtSetValues( ma_incdec_b, &arg, 1 ) ;
        XtSetValues( ma_incdec_c, &arg, 1 ) ;
        break;

        }

};

