#include <stdio.h>
#include <stdlib.h>

#ifdef vms
#include <string.h>
#include <pthread.h>
#include <ssdef.h>
#include <stsdef.h>
#include <starlet.h>
#include <libdef.h>
#include <lib$routines.h>
#include <iodef.h>
#include <ttdef.h>
#include <tt2def.h>
#include <descrip.h>
#else
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/dir.h>
#include <dirent.h>
#include <poll.h>
#include <errno.h>
#endif

#define KNOB_A 1
#define KNOB_B 2
#define LEDS_A 1
#define LEDS_B 2
#define SWITCH_A 4
#define SWITCH_B 8

#define True 1
#define False 0

#define CONF2		0x41
#define CONF1		0xC1
#define CLR_LEFT	0x42
#define CLR_RIGHT	0x82
#define CLR_WHOLE	0xC2
#define CLRL_LEFT	0x43
#define CLRL_RIGHT	0x83
#define CLRL_WHOLE	0xC3
#define POS_LEFT	0x44
#define POS_RIGHT	0x84
#define POS_WHOLE	0xC4
#define CLRC_LEFT	0x45
#define CLRC_RIGHT	0x85
#define CLRC_WHOLE	0xC5
#define WRT_LEFT	0x46
#define WRT_RIGHT	0x86
#define WRT_WHOLE	0xc6
#define LOAD_KNOB_A	0x48
#define LOAD_KNOB_B	0x88
#define LOAD_LEDS_A	0x49
#define LOAD_LEDS_B	0x89
#define READ_KNOB_A	0x47
#define READ_KNOB_B	0x87
#define READ_KNOB_AB	0xC7
#define ENABLE		0xc0
#define BELL		0x07


int serial_line;


typedef struct
{
	unsigned char	header;
    	union
    	{
	    unsigned char	data_byte[4];
	    unsigned int	data_int;
    	} data;
	unsigned char	chksum;
}  rcv_msg_struct;

typedef struct 
{
    int		    flag;
    pthread_mutex_t flag_mutex;
    pthread_cond_t  flag_condition;
} eventF;

typedef struct ku_struct_self {
	struct ku_struct_self *flink,*blink;
	void (*user_callback_ka)();
	int user_argument_ka;
	void (*user_callback_kb)();
	int user_argument_kb;
	void (*user_callback_sa)();
	int user_argument_sa;
	void (*user_callback_sb)();
	int user_argument_sb;
#ifdef vms
	short channel;
#else
	int channel;
#endif
	char  device_name[64];
	short baud;
        rcv_msg_struct 	rcv_msg;
	pthread_t	thread_id;
	eventF send_ef;
} hytec_ku_struct;


typedef struct
{
	unsigned char	control;
	unsigned char	data[50];
	unsigned char   size;
} hytec_snd_msg_struct;

int setup_port(hytec_ku_struct *);
int create_exit_handler(hytec_ku_struct *);
int create_read_thread(hytec_ku_struct *);
int initflag(eventF *);
void lock();
void unlock();

#ifdef vms
hytec_ku_struct
 *ku_init(struct dsc$descriptor *,short,void (*)(),int,int,int *);
hytec_ku_struct
 *ku_init_lat(struct dsc$descriptor *,short,void (*)(),int,int,int *);
#else
hytec_ku_struct *ku_init(char *,short,void (*)(),int,int,int *);
#endif
unsigned int ku_conf2(hytec_ku_struct *);
unsigned int ku_conf1(hytec_ku_struct *);
unsigned int ku_clr_half(hytec_ku_struct *,int);
unsigned int ku_clr_whole(hytec_ku_struct *);
unsigned int ku_clrl_half(hytec_ku_struct *,int,short);
unsigned int ku_clrl_whole(hytec_ku_struct *,short);
unsigned int ku_pos_half(hytec_ku_struct *,int,short,short);
unsigned int ku_pos_whole(hytec_ku_struct *,short,short);
unsigned int ku_clrc_half(hytec_ku_struct *,int,short,short);
unsigned int ku_clrc_whole(hytec_ku_struct *,short,short);
#ifdef vms
unsigned int ku_wrt_half(hytec_ku_struct *,int,short,short,struct dsc$descriptor *);
unsigned int ku_wrt_whole(hytec_ku_struct *,short,short,struct dsc$descriptor *);
#else
unsigned int ku_wrt_half(hytec_ku_struct *,int,short,short,char *);
unsigned int ku_wrt_whole(hytec_ku_struct *,short,short,char *);
#endif

unsigned int ku_load_knob(hytec_ku_struct *,int,unsigned int);
unsigned int ku_load_leds(hytec_ku_struct *,int,unsigned int);
unsigned int ku_read_knob(hytec_ku_struct *,int);
unsigned int ku_enable(hytec_ku_struct *,unsigned int);
unsigned int ku_bell(hytec_ku_struct *,unsigned int);
void *hytec_readcomp(void *);
#ifdef vms
int hytec_send_to_line(hytec_ku_struct *, hytec_snd_msg_struct *);
void knob_exit_handler(unsigned int,unsigned int);
void hytec_ctrly_handler();
unsigned int hytec_trap_y();
#else
int hytec_send_to_line(hytec_ku_struct *, hytec_snd_msg_struct *);
void knob_exit_handler(int);
#endif

hytec_ku_struct *hytec_ku_root = 0;

#ifdef vms
#define LINE_POS_CHECK(m_line,m_pos,pos_len)\
    {\
    if (m_line < 0 || m_line > 7)\
      return(SS$_VALNOTVALID);\
    if (m_pos < 0  || m_pos > pos_len)\
      return(SS$_VALNOTVALID);\
    }
#else
#define LINE_POS_CHECK(m_line,m_pos,pos_len){\
      if (m_line < 0 || m_line > 7)\
        return(1);\
      if (m_pos < 0  || m_pos > pos_len)\
        return(1);\
    }
#endif

char hytec_RTS_flag;
unsigned int knob_exit_status;
int	hytec_rcv6_size = 6;


/********************************************************************/
#ifdef vms
hytec_ku_struct *ku_init(struct dsc$descriptor *_device,
		short baud,
		void (*callback)(),
		int user_argument,
		int object_mask,
		int *status)
#else
hytec_ku_struct *ku_init(char *_device,
		short baud,
		void (*callback)(),
		int user_argument,
		int object_mask,
		int *status)
#endif
{
	short	not_assigned;
	hytec_ku_struct *ku;
	hytec_ku_struct *temp;
	char	device[64];

#ifdef vms
	_device->dsc$a_pointer[_device->dsc$w_length] = 0;
        strcpy(device,_device->dsc$a_pointer);
#else
	sprintf(device, "/dev/%s", _device);
#endif

/* Has this serial port been assigned to a unit yet ? */

	ku = hytec_ku_root;
	not_assigned = 1;
	while(ku != 0)
	{
		if (strcmp(ku->device_name,device) != 0)
		{
			if(ku->flink != 0)
				ku = ku->flink;
			else
				break;
                }
		else
		{
			not_assigned = 0;
			break;
		}
	}


/* if no channel assigned to this unit then assign channel to terminal line */

	if(not_assigned)
	{
		if(hytec_ku_root == (hytec_ku_struct *)0)
		{
			ku = calloc(1,sizeof(hytec_ku_struct));
			if(ku == 0)
			{
				*status = 0;
				return(0);
			}
                        else
				hytec_ku_root = ku;
		}
		else
		{
			ku->flink = calloc(1,sizeof(hytec_ku_struct));
			if(ku->flink == 0)
			{
#ifdef vms
				*status = SS$_INSFMEM;
#else
				*status = 0;
#endif
				return(0);
			}
                        else
			{
				temp = ku;
				ku = ku->flink;
				ku->blink = temp;
			}
		}

#ifdef vms
		*status = sys$assign (_device,&ku->channel,0,0);
#else
	    	ku->channel = open(device, O_RDWR, 0666);
		serial_line = ku->channel;
	    	
	    	*status = (ku->channel ? 1:0);
#endif
		if(!(*status & 0x01))
		{
			if(temp = ku->blink)
				temp->flink = 0;
			cfree(ku);
			return(0);
		}


		/* COPY DEVICE NAME SO IT CAN BE USED NEXT TIME IN. */
		strcpy (ku->device_name,device);
                ku->baud = baud;

		setup_port(ku);

/* initialize a flag mechanism */
		*status = initflag(&ku->send_ef);

/* Spawn a thread at this point to wait for messages from the Knob box */
		*status = create_read_thread(ku);

/* declare an exit handler */
		*status = create_exit_handler(ku);

        }

	if ((object_mask & KNOB_A) == KNOB_A)
	{
		ku->user_callback_ka = callback;
		ku->user_argument_ka = user_argument;
	}
	if ((object_mask & KNOB_B) == KNOB_B)
	{
		ku->user_callback_kb = callback;
		ku->user_argument_kb = user_argument;
	}
	if ((object_mask & SWITCH_A) == SWITCH_A)
	{
		ku->user_callback_sa = callback;
		ku->user_argument_sa = user_argument;
	}
	if ((object_mask & SWITCH_B) == SWITCH_B)
	{
		ku->user_callback_sb = callback;
		ku->user_argument_sb = user_argument;
	}

	return(ku);

}
/*******************************************************************/
unsigned int ku_conf1(hytec_ku_struct *ku)
{
	hytec_snd_msg_struct hytec_snd_msg;

	hytec_snd_msg.size = 3;
	hytec_snd_msg.control = CONF1;
	hytec_snd_msg.data[0] = 0;
	hytec_snd_msg.data[1] = 0;

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
unsigned int ku_conf2(hytec_ku_struct *ku)
{
	hytec_snd_msg_struct hytec_snd_msg;

	hytec_snd_msg.size = 3;
	hytec_snd_msg.control = CONF2;
	hytec_snd_msg.data[0] = 0;
	hytec_snd_msg.data[1] = 0;

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
unsigned int ku_clr_half(hytec_ku_struct *ku,int side)
{
	hytec_snd_msg_struct hytec_snd_msg;

	hytec_snd_msg.size = 3;
	if (side == KNOB_A)
		hytec_snd_msg.control = CLR_LEFT;
	else
		hytec_snd_msg.control = CLR_RIGHT;
	hytec_snd_msg.data[0] = 0;
	hytec_snd_msg.data[1] = 0;

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
unsigned int ku_clr_whole(hytec_ku_struct *ku)
{
	hytec_snd_msg_struct hytec_snd_msg;

	hytec_snd_msg.size = 3;
	hytec_snd_msg.control = CLR_WHOLE;
	hytec_snd_msg.data[0] = 0;
	hytec_snd_msg.data[1] = 0;

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
unsigned int ku_clrl_half(hytec_ku_struct *ku,int side,short line)
{
	hytec_snd_msg_struct hytec_snd_msg;

        LINE_POS_CHECK(line,1,19);

	hytec_snd_msg.size = 3;
	if (side == KNOB_A)
		hytec_snd_msg.control = CLRL_LEFT;
	else
		hytec_snd_msg.control = CLRL_RIGHT;
	hytec_snd_msg.data[0] = line;
	hytec_snd_msg.data[1] = 0;

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
unsigned int ku_clrl_whole(hytec_ku_struct *ku,short line)
{
	hytec_snd_msg_struct hytec_snd_msg;

        LINE_POS_CHECK(line,1,19);

	hytec_snd_msg.size = 3;
	hytec_snd_msg.control = CLRL_WHOLE;
	hytec_snd_msg.data[0] = line;
	hytec_snd_msg.data[1] = 0;

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
unsigned int ku_pos_half(hytec_ku_struct *ku,int side,short line,short position)
{
	hytec_snd_msg_struct hytec_snd_msg;

        LINE_POS_CHECK(line,position,19);

	hytec_snd_msg.size = 3;
	if (side == KNOB_A)
		hytec_snd_msg.control = POS_LEFT;
	else
		hytec_snd_msg.control = POS_RIGHT;
	hytec_snd_msg.data[0] = line;
	hytec_snd_msg.data[1] = position;

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
unsigned int ku_pos_whole(hytec_ku_struct *ku,short line,short position)
{
	hytec_snd_msg_struct hytec_snd_msg;

        LINE_POS_CHECK(line,position,41);

	hytec_snd_msg.size = 3;
	hytec_snd_msg.control = POS_WHOLE;
	hytec_snd_msg.data[0] = line;
	hytec_snd_msg.data[1] = position;

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
unsigned int ku_clrc_half(hytec_ku_struct *ku,int side,short line,short position)
{
	hytec_snd_msg_struct hytec_snd_msg;

        LINE_POS_CHECK(line,position,19);

	hytec_snd_msg.size = 3;
	if (side == KNOB_A) 
		hytec_snd_msg.control = CLRC_LEFT;
	else
		hytec_snd_msg.control = CLRC_RIGHT;
	hytec_snd_msg.data[0] = line;
	hytec_snd_msg.data[1] = position;

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
unsigned int ku_clrc_whole(hytec_ku_struct *ku,short line,short position)
{
	hytec_snd_msg_struct hytec_snd_msg;

        LINE_POS_CHECK(line,position,41);

	hytec_snd_msg.size = 3;
	hytec_snd_msg.control = CLRC_WHOLE;
	hytec_snd_msg.data[0] = line;
	hytec_snd_msg.data[1] = position;

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
#ifdef vms
unsigned int ku_wrt_half(hytec_ku_struct *ku,int side,short line,short position,
			 struct dsc$descriptor *string)
#else
unsigned int ku_wrt_half(hytec_ku_struct *ku,int side,short line,short position,
			 char *string)
#endif
{
	hytec_snd_msg_struct hytec_snd_msg;
	int len;
	unsigned char *text;

        LINE_POS_CHECK(line,position,19);

	if (side == KNOB_A)
		hytec_snd_msg.control = WRT_LEFT;
	else
		hytec_snd_msg.control = WRT_RIGHT;
	hytec_snd_msg.data[0] = line;
	hytec_snd_msg.data[1] = position;

	/* AUTO TRUNCATE */
#ifdef vms
	if (string->dsc$w_length > (20-position)) {
	  string->dsc$w_length = 20-position;
	}
	string->dsc$a_pointer[string->dsc$w_length] = 0;
	strcpy(&hytec_snd_msg.data[2],string->dsc$a_pointer);
	hytec_snd_msg.size = string->dsc$w_length+4;
#else
	text = &hytec_snd_msg.data[2];
	strncpy((char *)text,string,30);
	len = strlen((const char *)text);
	if (len > 20-position) {
	  len = 20-position;
	}
	text[len] = 0;
	hytec_snd_msg.size = len + 4;
#endif


	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
#ifdef vms
unsigned int ku_wrt_whole(hytec_ku_struct *ku,short line,short position,
			 struct dsc$descriptor *string)
#else
unsigned int ku_wrt_whole(hytec_ku_struct *ku,short line,short position,
			 char *string)
#endif
{
	hytec_snd_msg_struct hytec_snd_msg;
	int len;
	unsigned char *text;

        LINE_POS_CHECK(line,position,41);

	hytec_snd_msg.control = WRT_WHOLE;
	hytec_snd_msg.data[0] = line;
	hytec_snd_msg.data[1] = position;

	/* AUTO TRUNCATE */
#ifdef vms
	if (string->dsc$w_length > (42-position)) {
	  string->dsc$w_length = 42-position;
	}
	string->dsc$a_pointer[string->dsc$w_length] = 0;
	strcpy(&hytec_snd_msg.data[2],string->dsc$a_pointer);
	hytec_snd_msg.size = string->dsc$w_length+4;
#else
	text = &hytec_snd_msg.data[2];
	strncpy((char *)text,string,48);
	len = strlen((const char *)text);
	if (len > 42-position) {
	  len = 42-position;
	}
	text[len] = 0;
	hytec_snd_msg.size = len + 4;

#endif

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
unsigned int ku_load_knob(hytec_ku_struct *ku,int side,unsigned int value)
{
	hytec_snd_msg_struct hytec_snd_msg;

	hytec_snd_msg.size = 5;
	if (side == KNOB_A)
		hytec_snd_msg.control = LOAD_KNOB_A;
	else
		hytec_snd_msg.control = LOAD_KNOB_B;
	memcpy(&hytec_snd_msg.data,&value,sizeof(value));    

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
unsigned int ku_load_leds(hytec_ku_struct *ku,int side,unsigned int value)
{
	hytec_snd_msg_struct hytec_snd_msg;

	hytec_snd_msg.size = 3;
	if (side == KNOB_A)
		hytec_snd_msg.control = LOAD_LEDS_A;
	else
		hytec_snd_msg.control = LOAD_LEDS_B;
	hytec_snd_msg.data[0] = (char)value;
	hytec_snd_msg.data[1] = 0;

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
unsigned int ku_read_knob(hytec_ku_struct *ku,int side)
{
	hytec_snd_msg_struct hytec_snd_msg;

	hytec_snd_msg.size = 3;
	if (side == KNOB_A)
		hytec_snd_msg.control = READ_KNOB_A;
	else if(side == KNOB_B)
		hytec_snd_msg.control = READ_KNOB_B;
	else if(side == (KNOB_A | KNOB_B))
		hytec_snd_msg.control = READ_KNOB_AB;
	hytec_snd_msg.data[0] = 0;
	hytec_snd_msg.data[1] = 0;

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
unsigned int ku_enable(hytec_ku_struct *ku,unsigned int data)
{
	hytec_snd_msg_struct hytec_snd_msg;

	hytec_snd_msg.size = 3;
	hytec_snd_msg.control = ENABLE;
	hytec_snd_msg.data[0] = (char)(data);
	hytec_snd_msg.data[1] = 0;

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
unsigned int ku_bell(hytec_ku_struct *ku,unsigned int data)
{
	hytec_snd_msg_struct hytec_snd_msg;

	hytec_snd_msg.size = 3;
	hytec_snd_msg.control = BELL;
	hytec_snd_msg.data[0] = (char)(data);
	hytec_snd_msg.data[1] = 0;

	return(hytec_send_to_line(ku,&hytec_snd_msg));
}
/********************************************************************/
int hytec_send_to_line( hytec_ku_struct *ku, hytec_snd_msg_struct *hytec_snd_msg)
{
	
	int status;
	short iosb[4];

	if(!hytec_RTS_flag)
	{
		waitflag(&ku->send_ef);
	}
	
#ifdef vms
	status = sys$qiow(0,ku->channel,IO$_WRITEVBLK | IO$M_NOFORMAT,iosb,0,0,
			   hytec_snd_msg,hytec_snd_msg->size,0,0,0,0);

#else
	lock();
	status = write(ku->channel, (const char *)hytec_snd_msg, hytec_snd_msg->size);
	if(status == hytec_snd_msg->size)
	    status = 1;
	else
	    status = 0;
	    
	unlock();

#endif

#ifdef vms
	if(!(status & 0x01))
		return(status);
	else
		return((int)(iosb[0]));
#else
	return(status);
#endif
}
/********************************************************************/
void *hytec_readcomp(void *arg)
{
	unsigned char	check;
	int status;
	char valid_header;
	hytec_ku_struct *temp;
	hytec_ku_struct *ku;

	ku = (hytec_ku_struct *)arg;

	while((status = read_from_line(ku)) == 0)
	{
/* is it a valid header or XON/XOFF or is it rubbish ? */

		switch(ku->rcv_msg.header)
		{
			case 0x40:	/* KNOB A */
			case 0x80:	/* KNOB B */
			case 0x41:	/* SWITCH A */
			case 0x81:	/* SWITCH B */
				valid_header = 1;
				break;
        
			case 0x11:	/* XON */
				hytec_RTS_flag = 1;
				setflag(&ku->send_ef);
				valid_header = 2;
				break;

			case 0x13:	/* XOFF */
				hytec_RTS_flag = 0;
				clearflag(&ku->send_ef);
				valid_header = 2;
				break;

			default:	/* must be rubbish */
				valid_header = 0;
		}

		if(valid_header == 1)	
		{
			check = ku->rcv_msg.header ^ ku->rcv_msg.data.data_byte[0];
			check ^= ku->rcv_msg.data.data_byte[1];
			check ^= ku->rcv_msg.data.data_byte[2];
			check ^= ku->rcv_msg.data.data_byte[3];

			if(check == ku->rcv_msg.chksum)
			{
				switch(ku->rcv_msg.header)
				{
					case 0x40:	/* KNOB A */
						if (ku->user_callback_ka)
							ku->user_callback_ka(ku->user_argument_ka,KNOB_A,
							ku->rcv_msg.data.data_int);
						break;

					case 0x80:	/* KNOB B */
						if (ku->user_callback_kb)
							ku->user_callback_kb(ku->user_argument_kb,KNOB_B,
							ku->rcv_msg.data.data_int);
						break;

					case 0x41:	/* SWITCH A */
						if (ku->user_callback_sa)
							ku->user_callback_sa(ku->user_argument_sa,SWITCH_A,
							ku->rcv_msg.data.data_int);
						break;

					case 0x81:	/* SWITCH B */
						if (ku->user_callback_kb)
							ku->user_callback_kb(ku->user_argument_kb,SWITCH_B,
							ku->rcv_msg.data.data_int);
						break;

					default:
printf("Bad Packet  -- header = %x\ndata1 = %x\ndata2 = %x\ndata3 = %x\ndata4 = %x\nchecksum = %x\n",
ku->rcv_msg.header,ku->rcv_msg.data.data_byte[0],ku->rcv_msg.data.data_byte[1],
ku->rcv_msg.data.data_byte[2],ku->rcv_msg.data.data_byte[3],
ku->rcv_msg.chksum);
				}
			}
			else
			{
					/* checksum error */
printf("Checksum error  -- header = %x\ndata1 = %x\ndata2 = %x\ndata3 = %x\ndata4 = %x\nchecksum = %x\n",
ku->rcv_msg.header,ku->rcv_msg.data.data_byte[0],ku->rcv_msg.data.data_byte[1],
ku->rcv_msg.data.data_byte[2],ku->rcv_msg.data.data_byte[3],
ku->rcv_msg.chksum);
			}
               	}
		else if(valid_header == 0)	
		{
			printf("Invalid byte recieved %x\n",ku->rcv_msg.header);
			;/* ignore it */
		}
	}

	/* IO Problem */
	printf("IO error return from read = %x\n",status);

	return(0);

}
/*****************************************************/
#ifdef vms
void knob_exit_handler(unsigned int arg1,unsigned int arg)
#else
void knob_exit_handler(int arg)
#endif
{
	int data;
#ifdef vms
	$DESCRIPTOR (disconected, "DISCONNECTED");
#endif

	data = 0;
	ku_enable(hytec_ku_root,data);
	ku_clr_whole(hytec_ku_root);
	ku_load_leds(hytec_ku_root,LEDS_A,0);
	ku_load_leds(hytec_ku_root,LEDS_B,0);
#ifdef vms
	ku_wrt_half(hytec_ku_root,WRT_LEFT,5,3,&disconected);
#else
	ku_wrt_half(hytec_ku_root,WRT_LEFT,5,3,"Disconnected");
#endif
	ku_bell(hytec_ku_root,4);
	threadDelete(&hytec_ku_root->thread_id);
}
/*****************************************************/
#ifdef vms
unsigned int hytec_trap_y()
{
/*
*	CTRL_Y Trapping
*
*	Disables CTRL-Y attention, assigns SYS$INPUT here to trap CTRL-Y
*	and execute the AST routine ctrly_trap()
*/
	$DESCRIPTOR(devnam, "SYS$INPUT");

	char iosb[4];
	unsigned long mask;		/* bit mask to disable CTRL-Y */
	unsigned long status;		/* return status */
	unsigned short ctrly_chan;	/* channel for QIO */

	if (isatty(fileno(stdin)))
	{
		mask = 0x02000000;
		status = lib$disable_ctrl(&mask, 0);
		if (status)
		{
			status = sys$assign(&devnam, &ctrly_chan, 0, 0);
			if (status)
			{
				status = sys$qiow(0, ctrly_chan, IO$_SETMODE | IO$M_CTRLYAST, &iosb, 0, 0, &hytec_ctrly_handler, 0, 0, 0, 0, 0);
			}
		}
	}
	return(status);
}
/******************************************************************************/
void hytec_ctrly_handler()
{
/*
*	CTRL-Y TRAP
*
*	AST Routine called as a result of typing CTRL_Y
*	Re-enable ctrl_y trapping
*	Get buffer and pass disaster buffer to manager
*/
	long mask;
	int data;

	mask = 0x02000000;
	lib$enable_ctrl(&mask, 0);

	exit((SS$_CONTROLC & ~STS$M_SEVERITY) | STS$K_SEVERE);
}
#endif
/******************************************************************************/
int setup_port(hytec_ku_struct *ku)
{
	int status=0;
	hytec_ku_struct *temp;

#ifdef vms
	short	iosb[4];
	short baud;

	struct hytec_characteristics
	{
		char	class;
		char	type;
		short	page_width;
		unsigned int 	b_char;
		unsigned int	e_char;
	} hytec_term_char;

	int hytec_char_len = 12;

/* read existing line characteristics */
	status = sys$qiow(0,ku->channel,IO$_SENSECHAR,&iosb,0,0,
		   &hytec_term_char,hytec_char_len,0,0,0,0);
	if(!(status & 0x01))
	{
		if(temp = ku->blink)
			temp->flink = 0;
		cfree(ku);
		return(status);
	}

	if(!(iosb[0] & 0x01))
	{
		if(temp = ku->blink)
			(hytec_ku_struct *)(temp)->flink = 0;
		cfree(ku);
		return(iosb[0]);
	}

/* set line characteristics */

	hytec_term_char.type = TT$_FT1;
	hytec_term_char.b_char |= TT$M_EIGHTBIT;
	hytec_term_char.b_char |= TT$M_NOECHO;
	hytec_term_char.b_char |= TT$M_NOBRDCST;
	hytec_term_char.b_char |= TT$M_HOSTSYNC;
	hytec_term_char.b_char &= (TT$M_TTSYNC ^ 0xffffffff);
	hytec_term_char.e_char |= TT2$M_SETSPEED;
/*	hytec_term_char.e_char &= (TT2$M_SETSPEED ^ 0xffffffff);*/
	hytec_term_char.e_char |= TT2$M_PASTHRU;
	hytec_term_char.b_char &= (TT$M_NOTYPEAHD ^ 0xffffffff);
	hytec_term_char.b_char &= (TT$M_WRAP ^ 0xffffffff);
	hytec_term_char.b_char &= (TT$M_READSYNC ^ 0xffffffff);
	hytec_term_char.b_char &= (TT$M_MODEM ^ 0xffffffff);
	hytec_term_char.e_char &= (TT2$M_AUTOBAUD ^ 0xffffffff);

	switch(ku->baud)
	{
		case 600:
			baud = TT$C_BAUD_600;
			break;

		case 1200:
			baud = TT$C_BAUD_1200;
			break;

		case 2400:
			baud = TT$C_BAUD_2400;
			break;

		case 3600:
			baud = TT$C_BAUD_3600;
			break;

		case 4800:
			baud = TT$C_BAUD_4800;
			break;

		case 7200:
			baud = TT$C_BAUD_7200;
			break;

		case 9600:
			baud = TT$C_BAUD_9600;
			break;

		case 19200:
			baud = TT$C_BAUD_19200;
			break;

		default:
			if(temp = ku->blink)
				(hytec_ku_struct *)(temp)->flink = 0;
			cfree(ku);
			status = SS$_VALNOTVALID;
			return(status);
	}

	status = sys$qiow(0,ku->channel,IO$_SETCHAR,&iosb,0,0,
		   	&hytec_term_char,hytec_char_len,baud,
		   	0,TT$M_ALTRPAR/* | TT$M_PARITY*/ ,0);
	if(!(status & 0x01))
	{
		if(temp = ku->blink)
			(hytec_ku_struct *)(temp)->flink = 0;
		cfree(ku);
		return(status);
	}

	if(!(iosb[0] & 0x01))
	{
		if(temp = ku->blink)
			(hytec_ku_struct *)(temp)->flink = 0;
		cfree(ku);
		return(iosb[0]);
	}

/* Next lines are only for DEC LAT protocol connections */

/*	status = sys$qiow(0,ku->channel,IO$_TTY_PORT|IO$M_LT_CONNECT,
			&iosb,0,0,
		   	0,0,
		   	0,0,0,0);

	if(!(status & 0x01))
	{
		if(temp = ku->blink)
			(hytec_ku_struct *)(temp)->flink = 0;
		cfree(ku);
		return(status);
	}

	if(!(iosb[0] & 0x01))
	{
		if(temp = ku->blink)
			(hytec_ku_struct *)(temp)->flink = 0;
		cfree(ku);
		return(iosb[0]);
	}

*/

#else
	struct termios tty;
	speed_t baud;
	
	tcgetattr(ku->channel, &tty);

	/* Set speed */

	switch(ku->baud)
	{
		case 600:
			baud = B600;
			break;

		case 1200:
			baud = B1200;
			break;

		case 2400:
			baud = B2400;
			break;

		case 4800:
			baud = B4800;
			break;

		case 9600:
			baud = B9600;
			break;

		case 19200:
			baud = B19200;
			break;

		default:
			//if(temp = ku->blink)
			//	(hytec_ku_struct *)(temp)->flink = 0;
			cfree(ku);
			status = -1;
			return(status);
	}
	if (ku->baud != -1) {
	    cfsetospeed(&tty, baud);
	    cfsetispeed(&tty, baud);
	}

	/* Set to eight bits */

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_cflag |= CLOCAL | CREAD;
	tty.c_cflag &= ~(PARENB | PARODD);
	tty.c_iflag |= IXON|IXOFF;

	tty.c_iflag =  IGNBRK;

	tty.c_lflag = 0;

	tty.c_oflag = 0;

	tcsetattr(ku->channel, TCSANOW, &tty);

	return(status);
#endif

}
/******************************************************************************/
int create_read_thread(hytec_ku_struct *ku)
{
	int status;
    	pthread_attr_t attr;
	hytec_ku_struct *temp;
    	int detach_state;
    	int stacksize=0;

	if (status = pthread_attr_init(&attr))
	{
		printf("Error initializing thread attributes %x\n",status);
		return(status);
	}
        else
    	{
	    	if (stacksize!= 0)
		    pthread_attr_setstacksize (&attr, stacksize);

    		detach_state= PTHREAD_CREATE_JOINABLE; 
    		pthread_attr_setdetachstate (&attr, detach_state);

		status = pthread_create( &ku->thread_id, &attr, &hytec_readcomp, (void *)ku );
		if ( status )
		{
			//if(temp = ku->blink)
			//	(hytec_ku_struct *)(temp)->flink = 0;
			cfree(ku);
			return(status);
		}
	}

	return(1);
}

/******************************************************************************/
int create_exit_handler(hytec_ku_struct *ku)
{
	int status;

#ifdef vms
	typedef struct
	{
		unsigned int	VMS_USE;
		unsigned int	address;
		unsigned char	count;
		unsigned char	must_be_zero[3];
		unsigned int	condition_value;
		unsigned int	user_arg;
	}   exit_handler_struct;

	exit_handler_struct *exit_handler_block;

	hytec_ku_struct *temp;

	exit_handler_block = calloc(1,sizeof(exit_handler_struct));
	exit_handler_block->VMS_USE = 0;
	exit_handler_block->address = knob_exit_handler;
	exit_handler_block->count = 1;
	exit_handler_block->must_be_zero[0] = 0;
	exit_handler_block->must_be_zero[1] = 0;
	exit_handler_block->must_be_zero[2] = 0;
	exit_handler_block->condition_value = &knob_exit_status;
	exit_handler_block->user_arg = ku;

	status = sys$dclexh(exit_handler_block);
	if(!(status & 0x01))
	{
		if(temp = ku->blink)
			(hytec_ku_struct *)(temp)->flink = 0;
		cfree(ku);
		return(status);
	}

	status = hytec_trap_y();
	if(!(status & 0x01))
	{
		if(temp = ku->blink)
			(hytec_ku_struct *)(temp)->flink = 0;
		cfree(ku);
		return(status);
	}

#else

//	signal(SIGTERM, knob_exit_handler);

#endif

	return(1);
}
/******************************************************************************/
int initflag (eventF *_flag)
{
	hytec_RTS_flag = 1;
	if (pthread_mutex_init (&_flag->flag_mutex, NULL))
	    return 1;

    	if (pthread_cond_init (&_flag->flag_condition, NULL))
	    return 1;

    	_flag->flag = 0;

    	return 0;

}
/****************************************************************************/
int setflag (eventF *_flag)
{
    pthread_mutex_lock (&_flag->flag_mutex);
    _flag->flag= 1;
    pthread_cond_broadcast (&_flag->flag_condition);
    pthread_mutex_unlock (&_flag->flag_mutex);
    return 0;
}

/****************************************************************************/
int clearflag (eventF *_flag)
{
    pthread_mutex_lock (&_flag->flag_mutex);
    _flag->flag= 0;
    pthread_cond_broadcast (&_flag->flag_condition);
    pthread_mutex_unlock (&_flag->flag_mutex);
    return 0;
}
/****************************************************************************/
int waitflag (eventF *_flag)
{

    struct timespec priv_timerec;
    struct timespec wait_time;
    int requested_delay;
    int status;

    requested_delay = 500;

    if (requested_delay == 0) 
    {
        pthread_mutex_lock (&_flag->flag_mutex);
        while (_flag->flag== 0) 
	{
            status = pthread_cond_wait (&_flag->flag_condition,
				&_flag->flag_mutex);
            if (status != 0) 
		break;
        }

        _flag->flag = 0;
        pthread_mutex_unlock (&_flag->flag_mutex);

        if (status== 0)
            return 0;
        else
            return 1;
    }
    else
    {
	if (requested_delay >=  1000) 
	    priv_timerec.tv_sec= requested_delay / 1000;
	else 
	    priv_timerec.tv_sec= 0;
    
	priv_timerec.tv_nsec = (requested_delay % 1000) * 1000000;

	pthread_mutex_lock (&_flag->flag_mutex);
	while (_flag->flag == 0) 
	{
	    status= pthread_cond_timedwait (&_flag->flag_condition,
			    &_flag->flag_mutex,  &priv_timerec);
	    if (status != 0) 
		break;
	}

	_flag->flag= 0;
	pthread_mutex_unlock (&_flag->flag_mutex);

	if (status== 0) 
	    return 0;
	else if (status== ETIMEDOUT)
	    return 2;
	else 
	    return 1;
    }

    return 0;
}

/****************************************************************************/
int destroyFlag (eventF *_flag)
{
    pthread_mutex_destroy (&_flag->flag_mutex);
    pthread_cond_destroy (&_flag->flag_condition);

    return 0;
}

/****************************************************************************/
int threadDelete (pthread_t _thread)
{
    int	    stat;

    stat= pthread_cancel (_thread);
    if (stat< 0)
	return 1;
    else
	return 0;
}


/****************************************************************************/
void lock(void)
{
//    pthread_lock_global_np ();
}


/****************************************************************************/
/*									    */
/****************************************************************************/
void unlock (void)
{
//    pthread_unlock_global_np ();
}
/****************************************************************************/
int read_from_line (hytec_ku_struct *ku)
{
	int status;
	hytec_ku_struct *temp;

#ifdef vms
	short iosb[4];
	int hytec_termination_mask[2];
#else
	struct pollfd	pollDevice[1];
	char		buffer[BUFSIZ];
#endif

#ifdef vms
	hytec_termination_mask[0] = 0;
	hytec_termination_mask[1] = 0;

	status = sys$qiow(0,ku->channel,
	IO$_READVBLK | IO$M_NOECHO | IO$M_NOFILTR | IO$M_TRMNOECHO,
	&iosb,0,0,
	&ku->rcv_msg,hytec_rcv6_size,0,&hytec_termination_mask,0,0);

	if(!(status & 0x01))
	{
		if(temp = ku->blink)
			(hytec_ku_struct *)(temp)->flink = 0;
		cfree(ku);
		return (status);
	}

	if(!(iosb[0] & 0x01))
	{
		if(temp = ku->blink)
			(hytec_ku_struct *)(temp)->flink = 0;
		cfree(ku);
		return (iosb[0]);
	}

	status = 0;

#else
	pollDevice[0].fd = ku->channel;
//	pollDevice[0].events = POLLIN | POLLRDBAND;
	pollDevice[0].events = POLLIN;
	status = -1;

	while (poll(pollDevice, 1, -1) != -1)
	{
	    if(pollDevice[0].revents == POLLIN)
	    {
		lock();
		status = read(ku->channel, &ku->rcv_msg.header, 1);
		status = read(ku->channel, &ku->rcv_msg.data.data_byte[0], 1);
		status = read(ku->channel, &ku->rcv_msg.data.data_byte[1], 1);
		status = read(ku->channel, &ku->rcv_msg.data.data_byte[2], 1);
		status = read(ku->channel, &ku->rcv_msg.data.data_byte[3], 1);
		status = read(ku->channel, &ku->rcv_msg.chksum, 1);
		unlock();
		status = 0;
		break;
	    }
	    else
	    {
/* read out of band message */
		read(ku->channel, buffer, BUFSIZ);
	    }

	}

	if(status != 0)
	switch(status = errno) {
	case EAGAIN:
	    fprintf(stderr, "Error polling: Internal allocation error\n");
	    break;
	case EINTR:
	    fprintf(stderr, "Error polling: Interupt signal\n");
	    break;
	case EINVAL:
	    fprintf(stderr, "Error polling: Invalid parameter\n");
	    break;
	case EFAULT:
	    fprintf(stderr, "Error polling: Memory Fault\n");
	    break;
	default:
	    fprintf(stderr, "Error polling: %d\n", errno);
	}

#endif
	return (status);
}
