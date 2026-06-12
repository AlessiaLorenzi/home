/* test program for knob units */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/dir.h>
#include "alpi.h"
#include "ma_const.h"
#include "ma_ext_vars.h"
#include "ma.bit"
#include "bottsu"
#include "bottgiu"
#include <Shell.h>
#include <Grip.h>



#include <unistd.h>
/***** the following device name may need to be changed ******/

#define device_d "ttyS0"
/*****                                                  ******/

#define name_d "HYTEC ELECTRONICS"
#define knoba_d "KNOB A"
#define knobb_d "KNOB B"
#define data_d "00000000"
#define count_d1 "X"
#define count_d2 " "
#define disable_d "- disabled"
#define enable_d "- enabled "

#define KNOB_A 1
#define KNOB_B 2
#define SWITCH_A 4
#define SWITCH_B 8

void readast();
void hytec_wait (int);

unsigned int channel;
unsigned static int knob_a_old = 0 ;
unsigned static int knob_b_old = 0 ;
static int knob_a_scale = 1 ;
static int knob_b_scale = 1 ;

knobbo()
{
	int	i,status;
	int	state;
	unsigned int knob_a = 0x1000;
	unsigned int knob_b = 0x1000;
	unsigned char switch_a;
	unsigned char switch_b;
	unsigned char data;
	unsigned char enable_data;
	unsigned int code,read_data;
	unsigned int userarg = 0x22;
	unsigned int loop_count = 0;
	int count1,count2;
	int   wait = 500000;
	char temp_d[2];
	int	toggle;

	channel = ku_init(
		&device_d,19200,readast,userarg,
		KNOB_A | KNOB_B | SWITCH_A | SWITCH_B,
		&status);
	if(!channel)printf("Error from init  %x\n",status);

	status = ku_conf2(channel);
	if(!(status & 0x01))printf("Error from conf2  %x\n",status);

	status = ku_load_knob(channel,KNOB_A,knob_a);
	if(!(status & 0x01))printf("Error from load knob  %x\n",status);

	status = ku_load_knob(channel,KNOB_B,knob_b);
	if(!(status & 0x01))printf("Error from load knob  %x\n",status);

	for(i=0;i<8;i++)
	{
		status = ku_wrt_half(channel,KNOB_A,i,1,name_d);
		if(!(status & 0x01))printf("Error from wrt half  %x\n",status);
        }

	for(i=0;i<8;i++)
	{
		status = ku_wrt_half(channel,KNOB_B,i,1,name_d);
		if(!(status & 0x01))printf("Error from wrt half  %x\n",status);

       	}

	for(i=2;i<6;i++)
	{
		status = ku_clrl_half(channel,KNOB_A,i);
		if(!(status & 0x01))printf("Error from clrl half  %x\n",status);

		status = ku_clrl_half(channel,KNOB_B,i);
		if(!(status & 0x01))printf("Error from clrl half  %x\n",status);
        }

	status = ku_wrt_half(channel,KNOB_A,3,1,knoba_d);
	if(!(status & 0x01))printf("Error from wrt half  %x\n",status);

	status = ku_wrt_half(channel,KNOB_B,3,1,knobb_d);
	if(!(status & 0x01))printf("Error from wrt half  %x\n",status);

       	//status = ku_read_knob(channel,KNOB_A | KNOB_B);
	//if(!(status & 0x01))printf("Error from read knob  %x\n",status);


	for(i=0;i<8;i++)
	{
		data = 1<<i;
		status = ku_load_leds(channel,KNOB_A,data);
		if(!(status & 0x01))printf("Error from load leds  %x\n",status);
        }
	data = 0;
	status = ku_load_leds(channel,KNOB_A,data);

	for(i=0;i<8;i++)
	{
		data = 1<<i;
		status = ku_load_leds(channel,KNOB_B,data);
		if(!(status & 0x01))printf("Error from load leds  %x\n",status);
        }
	data = 0;
	status = ku_load_leds(channel,KNOB_B,data);

	enable_data = KNOB_A | KNOB_B | SWITCH_A | SWITCH_B;
	status = ku_enable(channel,enable_data);

	status = ku_wrt_half(channel,KNOB_A,3,8,enable_d);
	if(!(status & 0x01))printf("Error from wrt half  %x\n",status);

	status = ku_wrt_half(channel,KNOB_B,3,8,enable_d);
	if(!(status & 0x01))printf("Error from wrt half  %x\n",status);

 	count1 = 0;
	count2 = 1;
	strncpy(temp_d,count_d1,1);
	temp_d[1]=0;
	toggle = 1;
//	for(;;)
	//{
	//	printf("before wait\n");
	//	hytec_wait(wait);
	//	printf("after wait\n");
//
//		data = 1<<count1;
//		status = ku_load_leds(channel,KNOB_A,data);
//		if(!(status & 0x01))printf("Error from load leds  %x\n",status);
//		status = ku_load_leds(channel,KNOB_B,data);
//		if(!(status & 0x01))printf("Error from load leds  %x\n",status);
//		count1++;
//		if(count1 == 8)count1=0;
//
//		status = ku_wrt_half(channel,KNOB_A,6,count2,temp_d);
//		if(!(status & 0x01))printf("Error 1 from wrt half  %x\n",status);
//		status = ku_wrt_half(channel,KNOB_A,7,count2,temp_d);
//		if(!(status & 0x01))printf("Error 2 from wrt half  %x\n",status);
//		status = ku_wrt_half(channel,KNOB_B,6,count2,temp_d);
//		if(!(status & 0x01))printf("Error 3 from wrt half  %x\n",status);
//		status = ku_wrt_half(channel,KNOB_B,7,count2,temp_d);
//		if(!(status & 0x01))printf("Error 4 from wrt half  %x\n",status);
//		count2++;
//		if(count2 == 19)
//		{
//			if(toggle)
//			{
//		       		enable_data = KNOB_B | SWITCH_A | SWITCH_B;
//				status = ku_enable(channel,enable_data);
//				strncpy(temp_d,count_d2,1);
//				toggle = 0;
//
//				status = ku_wrt_half(channel,KNOB_A,3,8,disable_d);
//				if(!(status & 0x01))printf("Error 5 from wrt half  %x\n",status);
//
//				status = ku_wrt_half(channel,KNOB_B,3,8,enable_d);
//				if(!(status & 0x01))printf("Error 6 from wrt half  %x\n",status);
//			}
//			else
//			{
//		       		enable_data = KNOB_A | SWITCH_A | SWITCH_B;
//				status = ku_enable(channel,enable_data);
//				strncpy(temp_d,count_d1,1);
//				temp_d[1]=0;
//				toggle = 1;
//
//				status = ku_wrt_half(channel,KNOB_A,3,8,enable_d);
//				if(!(status & 0x01))printf("Error 7 from wrt half  %x\n",status);
//
//				status = ku_wrt_half(channel,KNOB_B,3,8,disable_d);
//				if(!(status & 0x01))printf("Error 8 from wrt half  %x\n",status);
//			}
//
//			count2=1;
//		}
//	}
}
void readast(userval,code,data)
unsigned int code;
unsigned int data;
unsigned int userval;
{
	int	i,status;
	int value_to_send;
	char	buf[32];
	extern void ma2q3q_refresh();
	int semop_retval;
struct sembuf semaphore_struct ;



/* structure used to clear the semaphore (red light) */
semaphore_struct.sem_num = 0 ;
semaphore_struct.sem_op = -1 ;
semaphore_struct.sem_flg = 0 ;



printf("  **  scale = %d\n", knob_a_scale);

//printf("page type = %d, magnet[button[numero_bottone].obj2].alim1 = %d magnet[button[numero_bottone].obj2].alim2 = %d\n", page_type, magnet[button[numero_bottone].obj2].alim1, magnet[button[numero_bottone].obj2].alim2 );

	switch(code)
	{
		case KNOB_A:
			printf("KNOB A  %8d\n", data - knob_a_old);
     			sprintf(buf,"%8d",data);
     			//sprintf(buf,"%8x",data);
			status = ku_wrt_half(channel,KNOB_A,4,3,buf);
			if(!(status & 0x01))printf("Error 9 from wrt half  %x\n",status);

			if (page_type > 0 )
				{
                		semop_retval = semop( semaphore_id, &semaphore_struct, 1 ) ;
                		if( semop_retval == -1 )
                        	printf( "*1* error in semaphore operation\n" ) ;
                		else
                        		{
					supply[magnet[button[numero_bottone].obj2].alim1].wa_current = 
					supply[magnet[button[numero_bottone].obj2].alim1].wa_current + ( data - knob_a_old ) * knob_a_scale ;
					if (supply[magnet[button[numero_bottone].obj2].alim1].wa_current > 999999 ) 
					supply[magnet[button[numero_bottone].obj2].alim1].wa_current = 999999; 
					if (supply[magnet[button[numero_bottone].obj2].alim1].wa_current < 0 ) 
					supply[magnet[button[numero_bottone].obj2].alim1].wa_current = 0; 
					supply[magnet[button[numero_bottone].obj2].alim1].current = 
					supply[magnet[button[numero_bottone].obj2].alim1].wa_current * 
					(supply[magnet[button[numero_bottone].obj2].alim1].maxcurrent/1000000.0);
					value_to_send = supply[magnet[button[numero_bottone].obj2].alim1].wa_current ;
					ma_current_cmd( magnet[button[numero_bottone].obj2].alim1, value_to_send ) ;

					if ( button[ numero_bottone ].type == 2 ||
     					button[ numero_bottone ].type == 4 ) ma2q3q_refresh() ;

					ma_current_upd( magnet[button[numero_bottone].obj2].alim1 ) ;

                			/* turn on the green light */
                			semop_retval = semctl( semaphore_id,  0, SETVAL, 1 ) ;
					}
				}
			knob_a_old = data ;
			break;

		case KNOB_B:
			printf("KNOB B           %8d\n", data - knob_b_old);
     			sprintf(buf,"%8d",data);
			status = ku_wrt_half(channel,KNOB_B,4,3,buf);
			if(!(status & 0x01))printf("Error 10 from wrt half  %x\n",status);

			if (page_type > 0 )
				{
                		semop_retval = semop( semaphore_id, &semaphore_struct, 1 ) ;
                		if( semop_retval == -1 )
                        	printf( "*1* error in semaphore operation\n" ) ;
                		else
                        		{
					supply[magnet[button[numero_bottone].obj2].alim2].wa_current = 
					supply[magnet[button[numero_bottone].obj2].alim2].wa_current + ( data - knob_b_old ) * knob_b_scale ;
					if (supply[magnet[button[numero_bottone].obj2].alim2].wa_current > 999999 ) 
					supply[magnet[button[numero_bottone].obj2].alim2].wa_current = 999999; 
					if (supply[magnet[button[numero_bottone].obj2].alim2].wa_current < 0 ) 
					supply[magnet[button[numero_bottone].obj2].alim2].wa_current = 0; 
					supply[magnet[button[numero_bottone].obj2].alim2].current = 
					supply[magnet[button[numero_bottone].obj2].alim2].wa_current * 
					(supply[magnet[button[numero_bottone].obj2].alim2].maxcurrent/1000000.0);
					value_to_send = supply[magnet[button[numero_bottone].obj2].alim2].wa_current ;
					ma_current_cmd( magnet[button[numero_bottone].obj2].alim2, value_to_send ) ;

					if ( button[ numero_bottone ].type == 2 ||
     					button[ numero_bottone ].type == 4 ) ma2q3q_refresh() ;

					ma_current_upd( magnet[button[numero_bottone].obj2].alim2 ) ;

                			/* turn on the green light */
                			semop_retval = semctl( semaphore_id,  0, SETVAL, 1 ) ;
					}
				}
			knob_b_old = data ;
			break;

		case SWITCH_A:
			printf("SWITCH A\n");
			printf("switch A  %d\n",data);
			if ( data == 1 ) knob_a_scale = 1 ;
			if ( data == 2 ) knob_a_scale = 10 ;
			if ( data == 4 ) knob_a_scale = 100 ;
			if ( data == 8 ) knob_a_scale = 1000 ;
			status = ku_load_leds(channel,KNOB_A,data);
			if(!(status & 0x01))printf("Error from load leds  %x\n",status);
printf("scale = %d\n", knob_a_scale);
			break;

		case SWITCH_B:
			printf("SWITCH B\n");
			printf("switch B  %d\n",data);
			if ( data == 1 ) knob_b_scale = 1 ;
			if ( data == 2 ) knob_b_scale = 10 ;
			if ( data == 4 ) knob_b_scale = 100 ;
			if ( data == 8 ) knob_b_scale = 1000 ;
			status = ku_load_leds(channel,KNOB_B,data);
			if(!(status & 0x01))printf("Error from load leds  %x\n",status);

/*			status = ku_bell(channel,data);*/


			break;
	}
}
/****************************************************************************/

void hytec_wait (int mseconds)
{
#ifdef linux
    struct timeval priv_timerec;
#else
    struct timespec priv_timerec;
#endif
    int requested_delay;

    requested_delay = mseconds* 1000;

    if (requested_delay >=  1000000) 
	priv_timerec.tv_sec = requested_delay / 1000000;
    else   
	priv_timerec.tv_sec = 0;


#ifdef linux
    priv_timerec.tv_usec = (requested_delay % 1000000);
    select (0, NULL, NULL, NULL, &priv_timerec);
#else
    priv_timerec.tv_nsec = (requested_delay % 1000000) * 1000;
    pthread_delay_np (&priv_timerec);
#endif

    
    
    
}

