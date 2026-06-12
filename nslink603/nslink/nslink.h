/*
 *  Linux driver for Comtrol SI/IA & DeviceMaster products.
 *
 *  nslink.h --- internal header file of the NSLink driver
 *
 *  Written by Grant Edwards, Comtrol Corp.  1995 - 2003.
 *
 *  Copyright (C) 1995 - 2003 by Comtrol, Inc.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _NSLINK_H
#define _NSLINK_H

// Maximum number of boxes
#define MAX_NRP_SIS	256

// Maximum number of ports for a box
#define MAX_NRP_PORTS	32

// Maximum totoal ports
#define MAX_NRP_TOTAL_PORTS 256

struct si_box_config {
        int		boxid;
        int           	base_port;
        int		num_ports;
        int		state;
        unsigned char	ether_addr[6];
        char		interface[16];
        uint32_t	ip_addr;
        unsigned char	rsmode[MAX_NRP_PORTS];
        int           	link_timeout;
        int           	arch_id;
        unsigned char 	scan_period;
        unsigned char 	rexmit_timer;
        int           	low_latency[MAX_NRP_PORTS];
        int           	rx_fifo_disable[MAX_NRP_PORTS];
        int           	inter_char_timeout[MAX_NRP_PORTS];
        uint32_t	ModelId;
};

struct si_box_report {
        unsigned char	ether_addr[6];
        int		product_id;
        int		flags;
};

/*
 * MAC connected device structure
 */
struct mac_mode_params {
        int           boxid;
};

/*
 * TCP connected device structures.
 */
struct tcp_mode_params {
        int           boxid;
        int           sk;
        int           rbuf_size;
        unsigned char	*rbuf;
        int           wbuf_size;
        unsigned char	*wbuf;
};

// In gcc you can't "pack" a struct unless you declare the
// struct and typedef separately

struct toolRequest {
        unsigned char	cmd;
        unsigned char	index;
        unsigned short	length;
        uint32_t	addr;
}
__attribute__((packed));

typedef struct toolRequest tToolRequest;

#define LOAD_TOOLREQ	1
#define GET_TOOLREQ	2
#define GOTO_TOOLREQ	3
#define MOVE_TOOLREQ	4
#define ID_TOOLREQ	5
#define SET_TOOLREQ	6
#define ENV_TOOLREQ	7
#define PING_TOOLREQ	8
#define DATA_TOOLREQ	9
#define SET_IP_TOOLREQ  10
#define ERASE_IP_TOOLREQ 11
#define DISABLE_IP_TOOLREQ 12
#define RESET_TOOLREQ   13

struct appRecord {
        unsigned short AppId;
        unsigned char  AppRevMajor;
        unsigned char  AppRevMinor;
}
__attribute__((packed));

struct mac_id_response {
        unsigned char	MacAddr[6];
        unsigned char flags;
        unsigned char unused[3];
        uint32_t	ModelId;
        unsigned char   ArchId;
        unsigned char   NumPorts;
        unsigned short  ProgramId;
        unsigned char   BoardRev;
        unsigned char   BootRevMajor;
        unsigned char   BootRevMinor;
        unsigned char   NumApps;
        struct appRecord AppRecord[0];
        unsigned char   UserDataCount;
        unsigned char   UserData[0];
}
__attribute__((packed));

struct toolResponse {
        uint32_t	IpAddr;
        uint32_t	DefaultRoute;
        uint32_t	ServerIpAddr;
        uint32_t	SubnetMask;
        uint32_t	AvailMemAddr;
        uint32_t	AvailMemLength;
        unsigned char	Version[64];
        unsigned char	MacAddr[6];
        uint32_t	  ModelId;
        unsigned char   ArchId;
        unsigned char   NumPorts;
        unsigned short  ProgramId;
        unsigned char   BoardRev;
        unsigned char   BootRevMajor;
        unsigned char   BootRevMinor;
        unsigned char   NumApps;
        struct appRecord AppRecord[0];
        unsigned char   UserDataCount;
        unsigned char   UserData[0];
}
__attribute__((packed));

typedef struct toolResponse tToolResponse;


struct oldToolResponse {
        uint32_t	IpAddr;
        uint32_t	DefaultRoute;
        uint32_t	ServerIpAddr;
        uint32_t	SubnetMask;
        uint32_t	AvailMemAddr;
        uint32_t	AvailMemLength;
        unsigned char	Version[64];
        unsigned char	MacAddr[6];
}
__attribute__((packed));

typedef struct oldToolResponse tOldToolResponse;

/*
 *  SI flags are same as those in <linux/serial.h>
 */


/*
 *  SI state machine defines
 */
#define SI_STATE_RESET		 0
#define SI_STATE_BOOTING	 1
#define SI_STATE_BOOTED		 2
#define SI_STATE_CONNECTING	 3
#define SI_STATE_CONNECTING_WAIT 4
#define SI_STATE_CONNECTED	 5
#define SI_STATE_DEAD		 6

typedef struct {
        int sock;
        int msTimeout;
}
tTcpTxQueueWait;

typedef struct {
        int boxCount;
        int portCount;
}
tBoxPortCount;

/*
 *   NSLinkctl device ioctls supported
 */
#define SI_LOAD_CODE4_8 	0x00565301
#define SI_GET_CONFIG	  	0x00565302
#define SI_SET_CONFIG		0x00565303
#define SI_GET_CTL_MAJOR	0x00565304
#define SI_SET_BOX_CONFIG  	0x00565305
#define SI_GET_BOX_CONFIG	0x00565306
#define SI_GET_BOX_REPORT	0x00565307
#define SI_GET_PROBLEM_BOXES	0x00565308
#define SI_DEBUG_MISC		0x00565309
#define SI_SET_TCP	        0x0056530A
#define SI_LOAD_CODE2	        0x0056530B
#define SI_SET_MAC              0x0056530C
#define SI_TCP_TXQUEUE_WAIT     0x0056530D
#define SI_FORCE_PORT_RELEASE   0x0056530E
#define SI_RELEASE_MODULE       0x0056530F
#define SI_REGISTER_PORTS       0x00565310

/*
 *   NSLinkctl device ioctls supported
 */
#define SI_RSMODE_232    0x32
#define SI_RSMODE_422    0x22
#define SI_RSMODE_485    0x85
#define SI_RSMODE_485FDM 0x87
#define SI_RSMODE_485FDS 0x86
#define SI_RSMODE_OFF    0xde

#endif // _NSLINK_H
