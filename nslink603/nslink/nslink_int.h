/*
 *  Linux driver for Comtrol SI/IA & DeviceMaster products.
 *
 *  nslink_int.h --- internal header file for the NSLink driver
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

#ifndef _NSLINK_INT_H
#define _NSLINK_INT_H

/*
 * This is the Ethernet protocol ID used by the RPSH-Si.
 * Protocol ID's are supposed to be assigned by the IEEE.
 * Looks like Comtrol picked the PCI id arbitrarily, possibly without
 * getting an official IEEE assignment.  Tsk, tsk....
 */
#define ETH_P_COMTROL	0x11FE

/*
 * This is the low-level "network layer" header.
 *
 * product_id:   Comtrol Product ID Header
 * 		 If set to FFH then all Comtrol Network products will respond.
 * 		 A AAH will cause only the RPSH-Si product to respond
 * 		 A 55H will cause only the Async (VS1000/RPSH) product to respond
 * 		 A 15H will cause only the ISDN (VS3XXX)  product to respond
 *
 * conc_id:	Concentrator Index Field(Server Assigned id).
 *
 * packet_class:	Packet Class(Product Specific, some Common-Admin)
 * 			Value of 1= Administrative.
 * 			Value of 0x55 = Async Concentrator Packet.
 *
 */
struct comtrolhdr {
        __u8 product_id;
        __u8 conc_id;
        __u8 packet_class;
};

/* Product ID's */
#define SI_ID	0xAA

/* Packet classes */
#define ADMIN_CLASS	0x01
#define SI_CLASS	0x55

/*
 * This is the "transport layer" headers.
 *
 * There are two that can be used here: one for the admin packet, and
 * the other for SI-specific packets, which use an HDLC format.
 *
 * Note that the transport layer is not properly aligned, so we have
 * to use byte-width variables.  (This is the reason for the len_low
 * and len_high in the admin header).
 *
 * Note that in the admin header, the length includes the one byte
 * admin_type, in addition to the length of the actual admin packet
 * itself.
 */
struct adminhdr {
        __u8 len_low;
        __u8 len_high;
        __u8 admin_type;
};

/*
 * These are the various ADMIN packet types
 */
#define ADMIN_SEC_LOAD_TYPE	1
#define ADMIN_PRD_ID_REQ_TYPE	2
#define ADMIN_PRD_ID_REP_TYPE	3
#define ADMIN_LOOPBACK_TYPE	4
#define ADMIN_RESET_TYPE	5

/* The data payload for ADMIN_PRD_ID_REQ and ADMIN_PRD_ID_REP */
struct admin_product_id_request {
        __u8 mac_addr[6];
        __u8 flags;
};

/* The data payload for ADMIN_RESET_TYPE */
struct admin_reset {
        __u16 magic;			/* Must be 0x55 to do a reset */
};

/*
 * Max number of data bytes per TCP sendmsg() when downloading
 * boot file.
 *
 * If these get fragmented en route, things won't work. If
 * this happens, reduce this number until packets don't get
 * fragmented.
 */
#define TCP_BOOT_PKT_SIZE 1452

/*
 * The secondary loader commands
 */
#define IOCTL_COMMAND    0x5
#define UPLOAD_COMMAND   0x8
#define DOWNLOAD_COMMAND 0x9

/*
 * IOCTL sub-commands
 */
#define GO_CMD	12

/*
 * This is the HDLC header
 *
 * BYTE 17:  HDLC Control field, used to mark the packet as a
 *  sequenced packet or a non-sequenced packet.  Non sequenced
 *  packets will never be resent and the sequence fields are
 *  ignored.  Sequenced packets are marked with sequence numbers
 *  and can be resent.  1H - Set if Non-sequenced.
 *
 * BYTE 18:  out_snd_index;  // V(s)
 *   Used to mark each outgoing packet with a incrementing sequence
 *   number for packet missing or out of place detection.
 *   Index of outgoing packet, for packet sequence checking
 *   every new packet we send over, we increment this field so that the
 *   receiver can do a packet sequence check.
 *
 * BYTE 19:  next_in_index;  // V(r)
 *   Used to acknowledge reception of all packets up to this value.
 *   We send the expected sequence number of the next packet,
 *     	(NOT the last good received packet index, as BOOT.TXT claims)
 */
struct HDLChdr {
        __u8 hdlc_control;
        __u8 out_snd_index;
        __u8 next_in_index;
};

/* control fields for hdlc header control field */
#define CONTROL_IFRAME        0
#define CONTROL_UFRAME        1
#define CONTROL_CONNECT_ASK   3
#define CONTROL_CONNECT_REPLY 5

/*
 * How long the driver waits before deciding it wants to hear from the
 * box
 */
#define SI_KEEP_ALIVE_TIME     (30*HZ)
#define SI_TCP_KEEP_ALIVE_TIME (30*HZ)
#define SI_TCP_TIMEOUT	    2


#define HDLC_MAX_UNACKED_COUNT 8

/*
 * HDLC sub-packet headers
 */
#define RK_QIN_STATUS       0x60	/* qin status report */
#define RK_DATA_BLK         0x61	/* data block */
#define RK_PORT_SET         0x62	/* set the port num to work with */
#define RK_CONTROL_SET      0x63	/* config the hardware */
#define RK_MCR_SET          0x64	/* change on modem control reg. */
#define RK_MSR_SET          0x65	/* change on modem status reg. */
#define RK_ACTION_SET       0x66	/* new actions, such as flush. */
#define RK_ACTION_ACK       0x67	/* response to actions, such as flush. */
#define RK_BAUD_SET         0x70	/* set the baud rate */
#define RK_SPECIAL_CHAR_SET 0x71	/* xon,xoff, err-replace, */
/* event-match */
#define RK_ESR_SET          0x72	/* set error status register. */
#define RK_ASSIGN_REQ       0x73	/* assign ports request. */
#define RK_ASSIGN_REPLY     0x74	/* assign ports reply. */
#define RK_RSMODE_SET       0x75	/* RS232/422/485 */
#define RK_REMOTE_STATUS    0x76	/* check/assign remote port status */
#define RK_TIMEOUT_SET      0x77	/* set port activity timout value */
#define RK_RELEASE_REQ      0x78        /* third party port release request */
#define RK_IC_TIMEOUT_SET   0x79        /* set inter-character timeout on port */
#define RK_IC_TIMEOUT       0x7a        /* inter-character timeout happened */
#define RK_SCANPERIOD_SET   0x7b        /* set hub scan period */
#define RK_RETRANS_TIMER_SET   0x7c     /* set hub hdlc retransmit timer */

#define RK_CONNECT_CHECK    0xfd	/* server packet to check link is */
/* working */
#define RK_CONNECT_REPLY    0xfe	/* reply from RK_CONNECT_ASK */
#define RK_CONNECT_ASK      0xff	/* broadcast from server to get */
/* report from boxes */

/*
 * control_settings
 */
#define SC_STOPBITS_MASK  0x0001
#define SC_STOPBITS_1     0x0000
#define SC_STOPBITS_2     0x0001

#define SC_DATABITS_MASK  0x0002
#define SC_DATABITS_7     0x0002
#define SC_DATABITS_8     0x0000

#define SC_PARITY_MASK    0x000c
#define SC_PARITY_NONE    0x0000
#define SC_PARITY_EVEN    0x0004
#define SC_PARITY_ODD     0x0008

#define SC_FLOW_RTS_MASK   0x0070
#define SC_FLOW_RTS_NONE   0x0000
#define SC_FLOW_RTS_AUTO   0x0010
#define SC_FLOW_RTS_RS485  0x0020	// rts turn on to transmit
#define SC_FLOW_RTS_ARS485 0x0040	// rts turn off to transmit(auto-rocketport)

#define SC_FLOW_CTS_MASK  0x0080
#define SC_FLOW_CTS_NONE  0x0000
#define SC_FLOW_CTS_AUTO  0x0080

#define SC_FLOW_DTR_MASK  0x0100
#define SC_FLOW_DTR_NONE  0x0000
#define SC_FLOW_DTR_AUTO  0x0100

#define SC_FLOW_DSR_MASK  0x0200
#define SC_FLOW_DSR_NONE  0x0000
#define SC_FLOW_DSR_AUTO  0x0200

#define SC_FLOW_CD_MASK   0x0400
#define SC_FLOW_CD_NONE   0x0000
#define SC_FLOW_CD_AUTO   0x0400

#define SC_FLOW_XON_TX_AUTO 0x0800
#define SC_FLOW_XON_RX_AUTO 0x1000

#define SC_NULL_STRIP        0x2000
#define SC_FLOW_RTS_OVERRIDE 0x4000
#define SC_FLOW_IXANY        0x8000

/*
 * MCR settings
 */
#define MCR_RTS_SET_MASK   0x0001
#define MCR_RTS_SET_ON     0x0001
#define MCR_RTS_SET_OFF    0x0000

#define MCR_DTR_SET_MASK   0x0002
#define MCR_DTR_SET_ON     0x0002
#define MCR_DTR_SET_OFF    0x0000

// loop in rocketport asic chip
#define MCR_LOOP_SET_MASK  0x0004
#define MCR_LOOP_SET_ON    0x0004
#define MCR_LOOP_SET_OFF   0x0000

#define MCR_BREAK_SET_MASK  0x0008
#define MCR_BREAK_SET_ON    0x0008
#define MCR_BREAK_SET_OFF   0x0000

/*
 * MSR definitions
 */
#define MSR_TX_FLOWED_OFF     0x0001
#define MSR_CD_ON             0x0008
#define MSR_DSR_ON            0x0010
#define MSR_CTS_ON            0x0020
#define MSR_RING_ON           0x0040
#define MSR_BREAK_ON          0x0080
#define MSR_ICTO_OCCURRED     0x0100

/*
 * action request definitions
 */
#define ACT_FLUSH_INPUT    0x0001
#define ACT_FLUSH_OUTPUT   0x0002
#define ACT_SET_TX_XOFF    0x0004
#define ACT_CLEAR_TX_XOFF  0x0008
#define ACT_SEND_XON       0x0010
#define ACT_SEND_XOFF      0x0020
#define ACT_MODEM_RESET    0x0040
#define ACT_RXFIFO_DISABLE 0x0200

/*
 * ESR definitions
 */
#define ESR_FRAME_ERROR    0x0001
#define ESR_PARITY_ERROR   0x0002
#define ESR_OVERFLOW_ERROR 0x0004
#define ESR_BREAK_ERROR    0x0008

/*
 * This data structure contains of all the state specific to a
 * particular SI port which we have to keep all the time.
 */
struct si_port_state {
        /*
         * Transmit/receive flow control
         *
         * The remote sends us nGetRemote to tell us how much data it
         * was able to clear out.  We send to the nGetLocal so the
         * remote knows how much data to send it.
         */
        __u16 nPutRemote;		/* tx data we sent to remote, Modulo 2**16 */
        __u16 nGetRemote;		/* tx data remote cleared out, Modulo 2**16 */
        __u16 nGetLocal;		/* tx data we cleared out, Modulo 2**16 */
        spinlock_t nGetLocalLock;

        __u16 modemStatusRegister;
        __u16 errorStatusRegister;
        __u16 actionResponse;
int update_queue:
        1;		/* 1 means we need to send nGetLocal */
};

/*
 * Structure used to hold boot file data
 */


/*
 * This data structure contains all of the state specific to a hub.
 */
struct si_state {
        unsigned char ether_addr[6];
        uint32_t ip_addr;
        struct NET_DEVICE *dev;
        struct si_port_state *port_state;
        int num_ports;
        unsigned char rsmode[MAX_NRP_PORTS];
        int arch_id;
        int base_port;		/* The first port of this box in nrp_table */
        __u8 conc_id;			/* Concentrator ID */
        int state;                    /* The state variable for the hub. */
        int need_to_send;             /* Indicates that there is a need to send a packet to the hub */
        int modelID;                  /* Model ID, retrieved during from the ID query response */

        /*
         * The linux timer structure.  We use a separate timer for
         * each RPSH-Si box.
         */
        struct timer_list timer;
        unsigned long send_connect_time;

        int resend_counter;

        /*
         * hdlc state variables
         */
        int hdlc_state_flags;

        /*
         * out_snd_index is sent out on each packet, increment by one
         * each time a new packet is sent out.  The receiver uses this
         * to check for packet sequence order.  This value is copied
         * into the snd_index field when we are ready to send a
         * packet.  A sync-message will set this to an initial working
         * value of 0.
         */
        __u8 out_snd_index;

        /*
         * in_ack_index is the last good rx ack_index received.  The
         * receiver will send us an acknowledge index(ack_index field)
         * indicating the last good received packet index it received.
         * This allows us to remove all packets up to this index
         * number from our transmit buffer since they have been
         * acknowledged.  Until this point we must retain the packet
         * for retransmition in case the receiver does not acknowledge
         * reception after a timeout period.
         */
        __u8 in_ack_index;

        /*
         * next_in_index indicates the sequence number of the next
         * packet expected from the communication partner.  Any out of
         * order packets are discarded.  After a HDLC re-sync, this
         * value is set to 0x00.
         */
        __u8 next_in_index;

        /*
         * unacked_pkts is used to measure how many incoming pkts
         * received which are unacknowledged so we can trip a
         * acknowledgement at 80% full
         */
        atomic_t unacked_pkts;

        /*
         * The sender_ack_timer is used to timeout sent packets and
         * the expected acknowledgement.
         */
        unsigned long sender_ack_timer;

        /*
         * The rcv_ack_timer is used to timeout rec. packets and our
         * responsibility to send an ack on them
         */
        unsigned long receiver_ack_timer;

        /*
         * link_timeout is how long we'll wait between receiving RK_CONNECT_CHECK
         * replies before we declare the box dead.
         */
        unsigned long linkTimeoutJiffies;        //  In jiffies
        int link_timeout;                        //  In seconds

        unsigned char scan_period; // ms, not really used in linux

        unsigned char rexmit_timer; // ms

        /*
         * timer set for the next time we want to send RK_CONNECT_CHECK
         */
        unsigned long keep_alive_tx_timer;

        /*
         * timer set for when the link will time out
         */
        unsigned long keep_alive_timeout;

        /*
         * flag that tells us we need to send an RK_CONNECT_CHECK
         */
        int need_to_send_connect_check;

        /* need to send link timeout, scan period and retransmit timer values */
        int update_timeout;

        /*
         * This is the HDLC retransmit queue, where we place packets
         * that need to be retransmited.
         */
        struct sk_buff_head rtq;
        int rtqCount;               /* How many packets we think is in the re-tx queue, for stats only */

        int retransCount;           /*  Retransmit stat */
        int rxBadOrderCount;        /*  Bad order stat */

        atomic_t timer_active;      /*  Flag that indicates si_timer() function is already running */

        /*
         * TCP stuff...
         */
        int use_tcp;
        unsigned long tcp_timer;
        unsigned long tcp_to_rcvp;
        unsigned long tcp_to_xmtp;
        unsigned long saddr;
        unsigned long daddr;
        unsigned short sport;
        struct socket *tcp_sock;
        struct sock *tcp_sk;
        unsigned char *tcp_k_rbuf;
        unsigned char *tcp_k_wbuf;
        int tcp_rbuf_size;
        unsigned char *tcp_u_rbuf;
        int tcp_wbuf_size;
        unsigned char *tcp_k_rfrag;
        int tcp_frag_len;
        int tcp_last_port;

        struct task_struct *taskPtr;

        /*
         * wait queue for set_mac ioctls to wait on
         */
        wait_queue_head_t waitqueue;

        // mutex for serializing functions that send data/commands to the hub
        struct semaphore write_sem;
};

#define HDLC_STATE_RESYNC 0x0001	/* we need to re-sync */
#define HDLC_RECLAIM	  0x0004	/* we should to reclaim ack'ed packets */
#define HDLC_SEND_ACK     0x0008	/* we should send an immediate ACK */

#define HDLC_RCV_TIMER_ACTIVE	0x0100	/* Receiver ack timer active */
#define HDLC_SND_TIMER_ACTIVE	0x0200	/* Sender ack timer active */

#define T_INFINITE	(((unsigned long)~0)>>1)


/* The size of the xmit buffer is 1 page, or 4096 bytes */
#define XMIT_BUF_SIZE 4096
#define RECV_BUF_SIZE 4096

/* number of characters left in xmit buffer before we ask for more */
#define WAKEUP_CHARS 256

/*
 * Here's the data structures for specific ports on the RPSH-Si.
 */

// Struct containing config/info about a port.  Note that
// qin pointer info is not kept here, but in si->pstate.  Perhaps
// these should be combined.

struct nr_port {
        int magic;
        int line;
        int flags;
        int baud_base;
        int custom_divisor;
        int count;
        int blocked_open;
        struct tty_struct *tty;
        struct si_state *si;
        int portnum;
        int closing_wait;
        int close_delay;
        int remote_status;
        unsigned char *xmit_buf;
        unsigned char *recv_buf;
        int xmit_head;
        int xmit_tail;
        int xmit_cnt;
        int recv_head;
        int recv_tail;
        int recv_cnt;
        int cd_status;
        int cps;
        int baud_rate;
        char rx_xon;
        char rx_xoff;
        char tx_xon;
        char tx_xoff;
        int control_settings;
        unsigned char rsmode;
        int modem_control_register;
        int action_register;
        int x_char;			/* xon/xoff character */
        /*
         * Status flags
         */
int update_baud_rate:
        1;
int update_control_settings:
        1;
int update_mcr:
        1;
int update_action:
        1;
int update_special_charset:
        1;
        int update_ic_timeout;
int not_configured:
        1;
int send_force_release:
        1;
int conn_refused:
        1;
int force_release_acked:
        1;

#ifdef KTERMIOS_NEW

        struct ktermios normal_termios;
        struct ktermios callout_termios;
#else

        struct termios normal_termios;
        struct termios callout_termios;
#endif
        //  struct termios normal_termios;
        //  struct termios callout_termios;

        wait_queue_head_t open_wait;
        wait_queue_head_t close_wait;
        wait_queue_head_t release_wait;

        int rx_fifo_disable;
        int inter_char_timeout;

        spinlock_t slock;
        struct semaphore write_sem;

        /*  Stats */
        unsigned int rxBytes;
        unsigned int txBytes;
        unsigned int parityErrors;
        unsigned int framingErrors;
        unsigned int overflowErrors;

};

/*
 * remote status
 */
#define REM_PORT_AVAIL		0x0000
#define REM_OPEN_PORT		0x0001
#define REM_RELEASE_PORT	0x0002
#define REM_PORT_ASSIGNED	0x0010
#define REM_PORT_BUSY		0x0020
#define REM_WAITING_OPEN	0x0100
#define REM_WAITING_CLOSE	0x0200

#define RPORT_MAGIC 0x525001

#ifndef SERIAL_TYPE_NORMAL
#define SERIAL_TYPE_NORMAL 1
#define SERIAL_TYPE_CALLOUT 2
#endif

/*
 * The size of the serial xmit buffer is 1 page, or 4096 bytes
 */
#ifndef SERIAL_XMIT_SIZE
#define SERIAL_XMIT_SIZE 4096
#endif

/*  Taking care of some kernel incompatibilities... */
#if LINUX_VERSION_CODE > VERSION_CODE(2,5,68)
#define TTY_GET_LINE(t) t->index
#define TTY_DRIVER_MINOR_START(t) t->driver->minor_start
#define TTY_DRIVER_SUBTYPE(t) t->driver->subtype
#define TTY_DRIVER_NAME(t) t->driver->name
#define TTY_DRIVER_NAME_BASE(t) t->driver->name_base

//the new api calls check for us but until we drop backwards support
//we will have to continue to check for the pointer.
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,26)
#define TTY_DRIVER_FLUSH_BUFFER_EXISTS(t) t->driver->flush_buffer
#define TTY_DRIVER_FLUSH_BUFFER(t) t->driver->flush_buffer(t)
#define TTY_LDISC_FLUSH_BUFFER_EXISTS(t) t->ldisc.flush_buffer
#define TTY_LDISC_FLUSH_BUFFER(t) t->ldisc.flush_buffer(t)
#define TTY_LDISC_WRITE_WAKEUP(t) ((tty->ldisc.write_wakeup != NULL) ? \
					((tty->ldisc.write_wakeup) (t)) : 0)
#define TTY_LDISC_RECV_BUF(t, b, p, c) t->ldisc.receive_buf(t, b, p, c);

#else		//implies kernel >= 2,6,26
#define TTY_DRIVER_FLUSH_BUFFER_EXISTS(t) t->driver->ops->flush_buffer
#define TTY_DRIVER_FLUSH_BUFFER(t) tty_driver_flush_buffer(t)
#define TTY_LDISC_FLUSH_BUFFER_EXISTS(t) 1
#define TTY_LDISC_FLUSH_BUFFER(t) tty_ldisc_flush(t)

//have to do this bake since .26 didn't quite get updated enough
#if LINUX_VERSION_CODE > VERSION_CODE(2,6,30)
#define TTY_LDISC_WRITE_WAKEUP(t) t->ldisc->ops->write_wakeup(t)
#define TTY_LDISC_RECV_BUF(t, b, p, c) t->ldisc->ops->receive_buf(t, b, p, c);
#elif LINUX_VERSION_CODE > VERSION_CODE(2,6,26)
#define TTY_LDISC_WRITE_WAKEUP(t) t->ldisc.ops->write_wakeup(t)
#define TTY_LDISC_RECV_BUF(t, b, p, c) t->ldisc.ops->receive_buf(t, b, p, c);
#else		//implies kernel = 2,6,26
#define TTY_LDISC_WRITE_WAKEUP(t) t->ldisc.write_wakeup(t)
#define TTY_LDISC_RECV_BUF(t, b, p, c) t->ldisc.receive_buf(t, b, p, c);
#endif		//version_code >2,6,26

#endif		//version_code <2,6,26

#define SOCK_STATE(s)     s->sk_state
#define SOCK_PROT(s)      s->sk_prot
#define SOCK_DATA(s)      s->sk_user_data
#define SOCK_DATA_RDY(s)  s->sk_data_ready
#define SOCK_SHUTDOWN(s)  s->sk_shutdown

#else //  implies Kernel versions < 2.5.68
#define TTY_GET_LINE(t) MINOR(t->device) - TTY_DRIVER_MINOR_START(t)
#define TTY_DRIVER_MINOR_START(t) t->driver.minor_start
#define TTY_DRIVER_SUBTYPE(t) t->driver.subtype
#define TTY_DRIVER_NAME(t) t->driver.name
#define TTY_DRIVER_NAME_BASE(t) t->driver.name_base
#define TTY_DRIVER_FLUSH_BUFFER_EXISTS(t) t->driver.flush_buffer
#define TTY_DRIVER_FLUSH_BUFFER(t) t->driver.flush_buffer(t)

#define SOCK_STATE(s)     s->state
#define SOCK_PROT(s)      s->prot
#define SOCK_DATA(s)      s->user_data
#define SOCK_DATA_RDY(s)  s->data_ready
#define SOCK_SHUTDOWN(s)  s->shutdown
#endif //  Kernel version > 2.5.68

#endif  // _NSLINK_INT_H
