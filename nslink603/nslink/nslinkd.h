/*
 *  Header file for the NSLink daemon
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

/*
 * This is the Ethernet protocol ID used by the RPSH-Si.
 * Protocol ID's are supposed to be assigned by the IEEE.
 * Looks like Comtrol picked the PCI id arbitrarily, possibly without
 * getting an official IEEE assignment.  Tsk, tsk....
 */
#define ETH_P_COMTROL	0x11FE

struct daemon_si_box_config
{
        unsigned char	ether_addr[6];
        char		interface[16];
        uint32_t	ip_addr;
        int   		base_port;
        int		num_lines;
        unsigned char	rsmode[MAX_NRP_PORTS];
        int 		link_timeout;
        unsigned char 	scan_period;
        unsigned char 	rexmit_timer;
        int 		low_latency[MAX_NRP_PORTS];
        int 		rx_fifo_disable[MAX_NRP_PORTS];
        int 		inter_char_timeout[MAX_NRP_PORTS];
        int 		arch_id;
        uint32_t 	ModelId;
};

extern struct daemon_si_box_config box_config[MAX_NRP_SIS+1];
extern int num_box_config;


extern char *device_name;
extern char *boot_file4_8;
extern char *boot_file2;
extern char *boot_fileDM;

extern char *sprint_ether_addr(unsigned char addr[6]);
extern void parse_config(const char *cfg_filename);

