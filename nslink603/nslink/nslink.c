/*
 *  Linux driver for Comtrol SI/IA & DeviceMaster products.
 *
 *  Written by Grant Edwards, Comtrol Corp.  1995 - 2003.
 *
 *  Copyright (C) 1995 - 2005 by Comtrol, Inc.
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

/*  This is the Linux driver for the Comtrol SI/IA & DeviceMaster products.
 *  These devices are ethernet attached serial port hubs.  For details see
 *  www.comtrol.com.
 *
 *  There are 2 "devices" in this driver.
 *  1)  NSLinkctl is a char device used by the daemon nslinkd to control the hub.
 *  2)  A tty device /dev/ttySIxx used by applications to send/receive data.
 *
 *  The daemon nslinkd is a user space program that loads the serial hub, monitors
 *  the hub, reloads it if needed and configures it.  The daemon communicates with
 *  the hub through the NSLinkctl device created in this driver.
 *
 *  User applications send/receive serial data through the tty devices created in this
 *  driver using the device name /dev/ttySIxx.
 */

/*
 *  Kernel Synchronization:
 *
 *  This driver has multiple kernel control paths:
 *  - TTY exception handlers (calls into the driver from user mode through tty device)
 *  - NSLinkctl exception handlers (not a factor in synchronization, except TCP functions)
 *  - Timer bottom half (tasklet), used when connected via HDLC.
 *  - TCP functions - Each hub has a corresponding user mode process, started by the daemon,
 *    which makes an IOCTL call into the NSLinkctl device.  This call causes a polling
 *    loop to run.
 *  - HDLC network callback bottom half - In HDLC mode, received packets with protocol
 *    0x11FE are sent to the driver.
 *
 * Critical data:
 * -  nrp_table[], accessed through passed "info" pointers, is a global (static) array of
 *    serial port state information and the xmit & rect circular buffers.  xmit/recv head, tail
 *    and cnt are protected with a per port spinlock, as is "count".
 * -  nrp_num_ports_open, int indicating number of open ports, protected by atomic operations.
 * -  si->rtq is a sk_buff_head list holding the HDLC retransmit queue.  The list lock is
 *    used where needed, such as when deleting multiple entries from the list.
 * -  rp_write() and rp_write_char() functions use a per port semaphore to protect against
 *    simultaneous access to the same port by more than one process.
 * -  pstate->nPutRemote and nGetRemote only written by single context.  nGetLocal protected
 *    by a spinlock.
 * -  State variable have been left unprotected, assuming that the states are self-correcting
 *    or mutually exclusive - si->state, si->hdlc_state_flags, info->remote_status.
 */

#ifndef LINUX_VERSION_CODE
#  include <linux/version.h>
#endif

#ifdef MODVERSIONS
#include <config/modversions.h>
#endif

#ifndef VERSION_CODE
#  define VERSION_CODE(vers,rel,seq) ( ((vers)<<16) | ((rel)<<8) | (seq) )
#endif

#if LINUX_VERSION_CODE < VERSION_CODE(2,2,9)
#  error "This kernel is too old: not supported by this file"
#endif


#include <linux/compiler.h>
#include <linux/module.h>
#if LINUX_VERSION_CODE < VERSION_CODE(3,4,0)
#include <asm/system.h>
#endif
#include <asm/bitops.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/in.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/file.h>
#include <linux/inetdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/stat.h>
#include <net/tcp.h>
#include <linux/tcp.h>
#include <linux/net.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/delay.h>
#if LINUX_VERSION_CODE >= VERSION_CODE(2,6,36) && LINUX_VERSION_CODE < VERSION_CODE(2,6,39)
#include <linux/smp_lock.h>
#endif


#define IsPast(j)   (((int)(jiffies-(j))) >= 0)

// RedHat 5.x has mucked with 2.6.18 kernel sources so much that their
// kernel is no longer compatible with the skb interface used by
// vanilla 2.6.18 kernels.  Try to detect the messed up RH 2.6.18
// kernel sources
#if (LINUX_VERSION_CODE == VERSION_CODE(2,6,18)) && defined(RHEL_MAJOR) &&  (RHEL_MAJOR == 5) && defined(dev_consume_skb)
# define RHEL_54_BREAKAGE
#endif

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,22)) && !defined(RHEL_54_BREAKAGE)
static inline unsigned char *skb_tail_pointer(const struct sk_buff *skb)
{
        return skb->tail;
}
#endif

#if LINUX_VERSION_CODE < VERSION_CODE(2,6,16)
int tty_insert_flip_string(struct tty_struct *tty, unsigned char *chars, size_t size)
{
        int toRecv, space;
        struct tty_ldisc *ld = tty_ldisc_ref(tty);

        if (!ld)
                return 0;

        space = ld->receive_room(tty);

        if (space > TTY_FLIPBUF_SIZE)
                space = TTY_FLIPBUF_SIZE;

        toRecv = space < size ? space : size;

        if (toRecv <= 0)
                return 0;

        memcpy(tty->flip.char_buf_ptr,chars,toRecv);
        memset(tty->flip.flag_buf_ptr,0,toRecv);
        tty->flip.count = toRecv;

        return toRecv;
}
#endif


#define NET_DEVICE net_device

//needs to be here so nslink_int.h knows about it. krw
#ifndef	KTERMIOS_NEW
#if LINUX_VERSION_CODE >= VERSION_CODE(2,6,20)
#define KTERMIOS_NEW
#endif
#endif

#include "nslink.h"
#include "nslink_int.h"
#include "version.h"

#if !defined(set_current_state)
#define set_current_state(s) current->state = (s)
#endif

#define xmit_packet(SKBTD) dev_queue_xmit(SKBTD)

//absolute max for this value is 1344.
//keep at 1312 or lower for safety.
#define MAX_CMD_PKT 1312

#undef NSLINK_DEBUG_HDLC
#undef NSLINK_DEBUG_RETRANS
#undef NSLINK_DEBUG_TIMER
#undef NSLINK_DEBUG_KEEPALIVE
#undef NSLINK_DEBUG_IN_CMDS
#undef NSLINK_DEBUG_OUT_CMDS
#undef NSLINK_DEBUG_OPEN
#undef NSLINK_DEBUG_INTR
#undef NSLINK_DEBUG_WRITE
#undef NSLINK_DEBUG_FLOW
#undef NSLINK_DEBUG_THROTTLE
#undef NSLINK_DEBUG_WAIT_UNTIL_SENT
#undef NSLINK_DEBUG_RECEIVE
#undef NSLINK_DEBUG_HANGUP
#undef NSLINK_DEBUG_NETMEM
#undef NSLINK_DEBUG_DEVICE_ID
#undef NSLINK_DEBUG_QUEUE_COUNTS

#define NSLINK_DEBUG_TRACE_HDLC 0
#define NSLINK_DEBUG_TRACE_QUEUE 0

// Macros used to trace MAC-mode HDCL pack handling
#if NSLINK_DEBUG_TRACE_QUEUE
#define NumQueueTrace 4096 /* must be power of 2 */
#define NumQueueMask (NumQueueTrace-1)
struct queueTraceStruct {
        unsigned long timestamp;
        int port;
        unsigned char event;
        unsigned data;
        struct si_port_state pstate;

}
queueTraceBuffer[NumQueueTrace];
unsigned queueTraceIndex;

#define TraceQueue(info,ev,d) \
  do {\
    queueTraceBuffer[queueTraceIndex].timestamp = jiffies; \
    queueTraceBuffer[queueTraceIndex].port = info->line; \
    queueTraceBuffer[queueTraceIndex].event = ev; \
    queueTraceBuffer[queueTraceIndex].data = d; \
    queueTraceBuffer[queueTraceIndex].pstate = info->si->port_state[info->portnum]; \
    ++queueTraceIndex; \
    queueTraceIndex &= (NumQueueTrace-1); \
  } while (0)


#else
#define TraceQueue(info,ev,d) /* nooop */
#endif


#if NSLINK_DEBUG_TRACE_HDLC
#define NumAckTrace 4096 /* must be power of 2 */
#define NumAckMask (NumAckTrace-1)
struct ackTraceStruct {
        unsigned long timestamp;
        int boxid;
        unsigned char out_snd_index;
        unsigned char in_ack_index;
        unsigned char next_in_index;
        unsigned char rtqCount;
        unsigned int head;
        unsigned int tail;
        unsigned event;
        unsigned char data[8];
}
ackTraceBuffer[NumAckTrace];
unsigned ackTraceIndex;
unsigned ackTraceDisabled;

#define TraceHdlc(si,ev,d) \
  do {\
    struct sk_buff *h,*t; \
    if (ackTraceDisabled)  break; \
    h = skb_peek(&si->rtq); \
    t = skb_peek_tail(&si->rtq); \
    ackTraceBuffer[ackTraceIndex].timestamp = jiffies; \
    ackTraceBuffer[ackTraceIndex].boxid = si->conc_id; \
    ackTraceBuffer[ackTraceIndex].out_snd_index = si->out_snd_index; \
    ackTraceBuffer[ackTraceIndex].in_ack_index = si->in_ack_index; \
    ackTraceBuffer[ackTraceIndex].next_in_index = si->next_in_index; \
    ackTraceBuffer[ackTraceIndex].rtqCount = si->rtqCount; \
    ackTraceBuffer[ackTraceIndex].head = h ? ((struct HDLChdr *)(h->head + 19))->out_snd_index : -1; \
    ackTraceBuffer[ackTraceIndex].tail = t ? ((struct HDLChdr *)(t->head + 19))->out_snd_index : -1; \
    ackTraceBuffer[ackTraceIndex].event = ev; \
    ackTraceBuffer[ackTraceIndex].data[0] = d; \
    ++ackTraceIndex; \
    ackTraceIndex &= (NumAckTrace-1); \
  } while (0)

#define TraceHdlcStop(si,ev,d) \
  do {TraceHdlc(si,ev,d); ackTraceDisabled=1;} while (0)
#else
#define TraceHdlcStop(si,ev,d)  /* noop */
#define TraceHdlc(si,ev,d)      /* noop */
#endif


#if defined(NSLINK_DEBUG_OPEN)
# define DebugOpen(...)  printk(KERN_INFO "nslink: " __VA_ARGS__)
#else
# define DebugOpen(...)  /* noop */
#endif


/*
 *   Static Variables
 */

// Global array of config/info structs, one per hub
static struct si_state *conc_state[MAX_NRP_SIS];

// Number of boxes we're actually using
static int numBoxes;

// Global array of config/info structs, one per port.  Indexed [0-n] by
// 'xx'  from /dev/ttySIxx.
static struct nr_port *nrp_table[MAX_NRP_TOTAL_PORTS];

// Global TTY structs
static struct tty_driver *nslink_driver;

#if LINUX_VERSION_CODE < VERSION_CODE(2,5,0)
static struct tty_struct *nslink_table[MAX_NRP_TOTAL_PORTS];
static struct termios *nslink_termios[MAX_NRP_TOTAL_PORTS];
static struct termios  *nslink_termios_locked[MAX_NRP_TOTAL_PORTS];
static int nslink_refcount = 0;

struct tty_driver *alloc_tty_driver(int lines) {
        struct tty_driver *driver;

        driver = kmalloc(sizeof(struct tty_driver), GFP_KERNEL);
        if (driver) {
                memset(driver, 0, sizeof(struct tty_driver));
                driver->magic = TTY_DRIVER_MAGIC;
                driver->num = lines;
                /* later we'll move allocation of tables here */
        }
        return driver;
}
#endif		//if kernel < 2.5

static atomic_t nrp_num_ports_open;	 /*  Number of serial ports open  */
static int control_major = 0;

static const char *ctl_name = "NSLinkctl";

static DECLARE_WAIT_QUEUE_HEAD(wake_daemon_wait);

/* Used to report to the user the RPSH-Si boxes found on the network */
static struct si_box_report si_box_rpts[MAX_NRP_SIS];
static int num_box_rpts;	/* number of valid reports */
static int rpt_loading_only;	/* if non-zero only report available boxes */

static int driver_registered = 0;

static long hdlc_retrans_time = 200;		//HZ/5;	//default value is 200ms
static long hdlc_ack_time = 100;		//HZ/10;	//default is 100ms

// baud table used in old method of getting baud rate in configure_nr_port
// use the tty_get_baud_rate() api call, this is only here for legacy support
// and will get removed down the road.
#if (LINUX_VERSION_CODE < VERSION_CODE(2,5,0))
static int baud_table[] = {
        0, 50, 75, 110, 134, 150, 200, 300,
        600, 1200, 1800, 2400, 4800, 9600, 19200,
        38400, 57600, 115200, 230400, 460800, 0
};
#endif

/*
 * Function declarations
 */
static void hdlc_resync(struct si_state *si);
static void hdlc_send_ack(struct si_state *si, __u8 flags);
static void hdlc_send_remote_cmds(struct si_state *si);

static void si_set_timer(struct si_state *si, unsigned long ticks);
static void si_timer(unsigned long arg);
static int init_hub_state(struct si_box_config *in, int from_user);
static int free_si_box(int boxid);
static int set_tcp(struct tcp_mode_params *in);
static int set_mac(struct mac_mode_params *in);
static int get_box_config(struct si_box_config *retinfo);
static int get_box_report(struct si_box_report *retinfo);
static void si_parse_input_pkt(struct si_state *si, unsigned char *buf, unsigned long len);

static int ctl_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);

static void nrp_wait_until_sent(struct tty_struct *tty, int timeout);
static void nrp_flush_buffer(struct tty_struct *tty);
static void nrp_start(struct tty_struct *tty);
static void configure_nr_port(struct nr_port *info);
static void send_recv_buf_to_ldisc(struct nr_port *info);
static void push_chars_to_recv_buf(struct nr_port *info,  unsigned char *cp, int len);
static void wake_all_tty(struct si_state *si);
static void set_otherclose_all_tty(struct si_state *si);
static void clr_otherclose_all_tty(struct si_state *si);
static void init_nr_port(struct si_state *si, int portnum, int ic_timeout, int low_latency, int rx_fifo_disable);
static int register_tty_driver(tBoxPortCount *userbp);
static int isDM500(int modelID);	//hack to tell if 300 baud is going to get remapped by unit.
static void tcp_send_remote_cmds(struct si_state *si);
static int tcp_txqueue_wait(tTcpTxQueueWait *in);
static void tcp_store_fragment(struct si_state *si, unsigned char *buf, int len);

#ifdef NSLINK_DEBUG_NETMEM
extern void show_net_buffers(void);
#endif

static char *macToStr(unsigned char *mac);
static char *siStateStr(int i);
static char *ipToStr(uint32_t ip);
static int UnitHasNo485(int modelID);
static int UnitHas485FD(int modelID);

#if LINUX_VERSION_CODE > VERSION_CODE(2,4,99)
int nrp_init(void);
static void nrp_cleanup_module(void);

module_init(nrp_init);
module_exit(nrp_cleanup_module);
#endif

#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

#if LINUX_VERSION_CODE >= VERSION_CODE(2,6,24)
# define HeaderCreate(netdev) netdev->header_ops->create
# include <net/net_namespace.h>
# define DevGetByName(name) dev_get_by_name(&init_net,name)
#else
# define HeaderCreate(netdev) netdev->hard_header
# define DevGetByName(name) dev_get_by_name(name)
#endif

#ifndef	TTYLAYER_NEW
# if LINUX_VERSION_CODE >= VERSION_CODE(2,6,15)
#  define TTYLAYER_NEW
# endif
#endif

#ifdef TTYLAYER_NEW
#define ReceiveRoom(tty)  tty->receive_room
#else
#define ReceiveRoom(tty)  tty->ldisc.receive_room(tty)
#endif

// macros to handle sk_buff api changes.
#if LINUX_VERSION_CODE > VERSION_CODE(2,6,21)
# define TransportHeader(skb) skb->transport_header
# define NetworkHeader(skb)   skb->network_header
# define MacHeader(skb)       skb->mac_header
#else
# define TransportHeader(skb) skb->h.raw
# define NetworkHeader(skb)   skb->nh.raw
# define MacHeader(skb)       skb->mac.raw
#endif

//more sk_buff stuff to handle api calls needed for x64
#ifndef NET_SKBUFF_DATA_USES_OFFSET
# define skb_transport_header(skb)	TransportHeader(skb)
# define skb_mac_header(skb) 		MacHeader(skb)
#endif

// interruptible_sleep_on is to be removed in 2.7, but we
// might as well do things "right" for all versions.  I don't
// think we support any kernels that don't have
// wait_event_interruptible, but if that is a problem, then
// figure out what the cutoff is and remove the "0 &&" below

#if 0 && (LINUX_VERSION_CODE < VERSION_CODE(2,4,00))
#define wait_event_interruptible(q,ev) interruptible_sleep_on(&q)
#endif

// Tell us how many characters are pending in the remote that
// haven't yet been acknowledged as being transmited.
static inline int get_tx_cnt_remote(struct si_port_state *pstate)
{
        signed short int s = (pstate->nPutRemote - pstate->nGetRemote);
        if (s<0 || s>2000) {
                printk(KERN_INFO "nslink: get>put error - get=%04x put=%04x cnt=%d\n",
                       pstate->nGetRemote,pstate->nPutRemote,s);
                pstate->nPutRemote = pstate->nGetRemote;
                s = 0;
        }
        return s;
}

// Return how much space is left in the remote's buffer.  We
// assume the remote has space for 1999 characters
static inline int get_tx_space_remote(struct si_port_state *pstate)
{
        int c = 1999 - get_tx_cnt_remote(pstate);
        if (c < 0)
                c = 0;
        return c;
}

static void inline wake_tty_write(struct tty_struct *tty)
{
        wake_up_interruptible(&tty->write_wait);
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,27)
        if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) && tty->ldisc.write_wakeup)
                (tty->ldisc.write_wakeup) (tty);
#else		// implies kernel >= 2,6,27
        if (tty->flags & (1 << TTY_DO_WRITE_WAKEUP))
                TTY_LDISC_WRITE_WAKEUP(tty);
#endif			//if kernel < 2.6.27
}

static void inline wake_tty(struct tty_struct *tty)
{
        //printk(KERN_INFO "nslink: wake_tty(%p)\n",tty);
        wake_up_interruptible(&tty->read_wait);
        wake_tty_write(tty);
#if defined(HavePollWait) && HavePollWait

        wake_up_interruptible(&tty->poll_wait);
#endif
}


/********************************************************************************/
/*                                                                              */
/*        HDLC FUNCTIONS                                                        */
/*                                                                              */
/********************************************************************************/

/*
 * This allocates Linux networking buffer for sending a packet.  Note
 * that the Linux layer takes care of creating the link-level
 * (ethernet) header (i.e., the first 14 bytes of the packet).  We
 * allocate 3 bytes of "network layer header" (struct comtrolhdr),
 * followed by 3 bytes of "transport layer header" which is either the
 * 3 byte header used by the Admin packet, or the 3 byte header used
 * by the HDLC layer.
 */
static struct sk_buff *prepare_packet_hdr(struct NET_DEVICE *dev, int packet_len) {
        struct sk_buff *skb;
        int length;

        if (!dev || !(dev->flags & IFF_UP))
                return NULL;		/* Interface not up! */

        // 6 bytes for the HDLC header, plus the hardware device header,
        // plus the data plus 15 bytes so that we can align the packet
        // properly.

        length = dev->hard_header_len + 6 + packet_len + 15;

        skb = alloc_skb(length, GFP_ATOMIC);
        if (!skb)
                return 0;

        skb->dev = dev;
        skb->protocol = __constant_htons(ETH_P_COMTROL);

        // Sometimes the kernel will allocate more room than
        // requested. There are a _lot_ of places that use skb_tailroom() to
        // determine how much more data will fit in the Ethernet packet.  To
        // make sure that works right, we'll reserve an additional
        // (skb_tailroom(skb)-length) bytes of space in addition to that
        // required for hardware header and packet alignment.

        skb_reserve(skb, ((skb_tailroom(skb)-length) + dev->hard_header_len + 15) & ~15);

        //64bit archs use data offsets - see skbuff.h
#ifdef NET_SKBUFF_DATA_USES_OFFSET
        NetworkHeader(skb) = skb->tail;
        TransportHeader(skb) = skb->tail;
        skb_put(skb, 6);	/* For the headers */
#else
        NetworkHeader(skb) = skb_tail_pointer(skb);
        TransportHeader(skb) = skb_put(skb, 6);	/* For the headers */
#endif

        return skb;
}

/*
 * Send a Product ID query packet
 *
 * If the second argument is NULL, then we're just doing a random
 * query looking for boxes.  If the second argument is non-NULL, then
 * we're querying a specific box with the goal of telling it to listen
 * to *US*.  :-)
 */
static int send_query_packet(struct NET_DEVICE *dev, struct si_state *si)
{
        struct sk_buff *skb;
        struct comtrolhdr *chdr;
        struct adminhdr *ahdr;
        struct admin_product_id_request *areq;

        skb = prepare_packet_hdr(dev, sizeof(struct admin_product_id_request));
        if (!skb) {
                printk(KERN_INFO "nslink: Alloc_skb failed, is %s up?\n",dev->name);
                return -ENOMEM;
        }

        chdr = (struct comtrolhdr *) skb_transport_header(skb);

        chdr->product_id = SI_ID;
        chdr->conc_id = si ? si->conc_id : 0x00;
        chdr->packet_class = ADMIN_CLASS;

        ahdr = (struct adminhdr *) (skb_transport_header(skb) + 3);
        ahdr->len_high = 0;
        ahdr->len_low = sizeof(struct admin_product_id_request) + 1;
        ahdr->admin_type = ADMIN_PRD_ID_REQ_TYPE;

        areq = (struct admin_product_id_request *)skb_put(skb, sizeof(struct admin_product_id_request));

        memcpy(areq->mac_addr, dev->dev_addr, 6);

        if (si)
                areq->flags = 0;		/* You belong to me.... */
        else
                areq->flags = 1;		/* Just testing... */

        HeaderCreate(dev)(skb, dev, ETH_P_COMTROL,
                          (si ? si->ether_addr : dev->broadcast),
                          dev->dev_addr, skb->len);

        xmit_packet(skb);

        return 0;
}

/*
 * hdlc_resync -- reset the hdlc state variables
 *
 * Called by hdlc_assign_ports(), this modifies si variables, but is only called
 * prior to being connected, so can be ignored when protecting critical variables.
 */
static void hdlc_resync(struct si_state *si)
{
        struct sk_buff *skb;
        unsigned long flags;

        si->in_ack_index = 0;
        si->out_snd_index = 0;
        si->next_in_index = 0;
        si->hdlc_state_flags = 0;
        atomic_set(&si->unacked_pkts, 0);

        /* Clear the retransmit queue */
        spin_lock_irqsave(&si->rtq.lock, flags);
        while ((skb = __skb_dequeue(&si->rtq))) {
                --si->rtqCount;
                kfree_skb(skb);
        }
        spin_unlock_irqrestore(&si->rtq.lock, flags);
}

/*
 * Prepare an skbuff with the HDLC headers
 */
static struct sk_buff *prepare_hdlc_packet(struct si_state *si, int size,
                __u8 flags) {
        struct sk_buff *skb;
        struct comtrolhdr *chdr;
        struct HDLChdr *hdlc_hdr;

        skb = prepare_packet_hdr(si->dev, size);
        if (!skb)
                return 0;

        chdr = (struct comtrolhdr *) skb_transport_header(skb);

        chdr->product_id = SI_ID;
        chdr->conc_id = si->conc_id;
        chdr->packet_class = SI_CLASS;

        hdlc_hdr = (struct HDLChdr *) (skb_transport_header(skb)+3);

        hdlc_hdr->hdlc_control = flags;
        if (flags & CONTROL_UFRAME)
                hdlc_hdr->out_snd_index = 0;
        else
                hdlc_hdr->out_snd_index = si->out_snd_index;
        hdlc_hdr->next_in_index = si->next_in_index;
        return skb;
}

/*
 *  This function actually sends the HDLC packet
 *  Inputs: si-> ptr to hub info, skb-> ptr to the skb buf to send
 *  HDLC only.
 */
static void send_hdlc_packet(struct si_state *si, struct sk_buff *skb)
{
        struct HDLChdr *hdlc_hdr;
        struct sk_buff *tmp;

        hdlc_hdr = (struct HDLChdr *) (skb_transport_header(skb) + 3);

        HeaderCreate(si->dev)(skb, si->dev, ETH_P_COMTROL, si->ether_addr,
                              si->dev->dev_addr, skb->len);

        /*
         * Set the receiver sequence number (for acknowledgement purposes)
         * Note that we've acknowledged all of our received packets.
         */
        hdlc_hdr->next_in_index = si->next_in_index;
        atomic_set(&si->unacked_pkts, 0);
        si->hdlc_state_flags &= ~HDLC_RCV_TIMER_ACTIVE;

        if (hdlc_hdr->hdlc_control & CONTROL_UFRAME) {
                /*
                 * This is an unsequenced frame; just send it out.
                 */
                xmit_packet(skb);
                return;
        }

        TraceHdlc(si,3,si->out_snd_index);

        /* Set the outgoing sequence number */
        hdlc_hdr->out_snd_index = si->out_snd_index++;

        /*
         * Enable the retransmission timer, and put the packet on the
         * retransmission queue.  Finally send the packet!
         */
        if (!(si->hdlc_state_flags & HDLC_SND_TIMER_ACTIVE)) {
                si->sender_ack_timer = jiffies + hdlc_retrans_time;
                si->hdlc_state_flags |= HDLC_SND_TIMER_ACTIVE;
                si_set_timer(si, hdlc_retrans_time);
        }

        //  Put the skb on our retransmit queue
        skb_queue_tail(&si->rtq, skb);
        ++si->rtqCount;

        //  Create a copy of the skb, transmit it over the ethernet
        tmp = skb_clone(skb, GFP_ATOMIC);
        if (tmp) {
                xmit_packet(tmp);
                /* Clear the receiver ack timer */
                si->hdlc_state_flags &= ~HDLC_RCV_TIMER_ACTIVE;
        }
        return;
}

/*
 * hdlc_send_ack --- send a bare ack packet
 */
static void hdlc_send_ack(struct si_state *si, __u8 flags)
{
        struct sk_buff *skb;
        unsigned char *cp;

        skb = prepare_hdlc_packet(si, 1, flags);
        if (!skb)
                return;
        cp = skb_put(skb, 1);
        *cp = 0;
        send_hdlc_packet(si, skb);
        si->hdlc_state_flags &= ~HDLC_RCV_TIMER_ACTIVE;
        TraceHdlc(si,4,si->next_in_index);
#ifdef NSLINK_DEBUG_HDLC
        printk(KERN_INFO "nslink: hdlc: [%d] tx ack %02x\n",si->conc_id,si->next_in_index);
#endif
}

#define TimeDelta_ns(start, end) ((((end.tv_sec-start.tv_sec)*1000000000)+end.tv_nsec)-start.tv_nsec)

/*
 * hdlc_retransmit -- retransmit the packets in the retranmission queue.
 */
static void hdlc_retransmit(struct si_state *si)
{
        struct sk_buff *skb, *next, *tail, *head = NULL;
        struct HDLChdr *hdlc_hdr;
        unsigned long flags;

#ifdef NSLINK_DEBUG_RETRANS
        unsigned char h, t;
        struct timespec start;
        struct timespec current_time;
        start.tv_nsec = 0;
        start.tv_sec = 0;
        current_time.tv_nsec = 0;
        current_time.tv_sec = 0;
        getnstimeofday(&start);
        getnstimeofday(&current_time);
        printk(KERN_INFO "nslink: hdlc: [%d] retrans packet(s) started at %ld.%06lds\n",
               si->conc_id, start.tv_sec, (start.tv_nsec/1000));
#endif

        ++si->retransCount;
        TraceHdlc(si,6,si->retransCount);
        if (si->retransCount == 16) {
                printk(KERN_INFO "nslink: hdlc: [%d] 16 retries to %s\n",si->conc_id,macToStr(si->ether_addr));
                TraceHdlcStop(si,-2,0);
        }

        //  Clone the retransmit queue
        //  Must grab the list spinlock so no-one grabs the skb from under us.
        spin_lock_irqsave(&si->rtq.lock, flags);
        skb = skb_peek(&si->rtq);
        if (skb) {
                tail = head = skb_clone(skb, GFP_ATOMIC);
                while (tail && (skb->next != (struct sk_buff *)(&si->rtq))) {
                        tail->next = skb_clone(skb->next, GFP_ATOMIC);
                        if (tail->next == NULL) {
                                printk(KERN_INFO "nslink: skb_clone failed!\n");
                                break;
                        }
                        tail = tail->next;
                        skb = skb->next;
                }
                tail->next = NULL;
        }
        spin_unlock_irqrestore(&si->rtq.lock, flags);

        if (skb == NULL) {
#ifdef NSLINK_DEBUG_RETRANS
                printk(KERN_INFO "nslink: hdlc: [%d] clearing retrans flag.\n",si->conc_id);
                getnstimeofday(&current_time);
                printk(KERN_INFO "nslink: hdlc: [%d] retrans skb == NULL ... Exiting " \
                       "Process time = %ldns\n", si->conc_id, TimeDelta_ns(start,current_time));
#endif
                si->hdlc_state_flags &= ~HDLC_SND_TIMER_ACTIVE;
                return;
        }

        skb = head;

        hdlc_hdr = (struct HDLChdr *) (skb_transport_header(skb) + 3);

#ifdef NSLINK_DEBUG_RETRANS
        h = hdlc_hdr->out_snd_index;
        t = ((struct HDLChdr *)(skb_transport_header(tail)+3))->out_snd_index;
        printk(KERN_INFO "nslink: hdlc: [%d] resend %02x-%02x (LastAck %02x) count=%d\n",
               si->conc_id, h, t,si->in_ack_index,si->retransCount);
#endif

        //  Walk through the cloned queue, retransmitting the packets
        while (skb) {
                hdlc_hdr = (struct HDLChdr *)(skb_transport_header(skb)+3);
                hdlc_hdr->next_in_index = si->next_in_index;
                next = skb->next;
                skb->next = NULL;       // xmit stack pays attention to this field
                TraceHdlc(si,7,hdlc_hdr->out_snd_index);
                xmit_packet(skb);
                skb = next;
        }

#ifdef NSLINK_DEBUG_RETRANS
        getnstimeofday(&current_time);
        printk(KERN_INFO "nslink: hdlc: [%d] retrans packet(s) " \
               "process time = %ldns\n", si->conc_id, TimeDelta_ns(start,current_time));
        printk(KERN_INFO "nslink: hdlc: [%d] retrans packet(s) " \
               "ended at %ld.%06lds\n", si->conc_id,
               current_time.tv_sec, (current_time.tv_nsec/1000) );
#endif
}

/*
 *  Reset (initialize) the port status.  Only called when the port state goes to
 *  CONNECTED, therefore this function does not change critical values.
 */
static void port_resync_all(struct si_state *si)
{
        struct si_port_state *pstate;
        struct nr_port *info;
        int i;

        for (i = 0; i < si->num_ports; i++) {
                pstate = si->port_state + i;

                pstate->nPutRemote = 0;
                pstate->nGetRemote = 0;
                pstate->nGetLocal = 0;
                pstate->modemStatusRegister = 0;
                pstate->errorStatusRegister = 0;
                pstate->update_queue = 0;

                info = nrp_table[i + si->base_port];
                if (info) {
                        TraceQueue(info,1,0);
                        info->update_baud_rate = 1;
                        info->update_control_settings = 1;
                        info->update_special_charset = 1;
                        si->need_to_send = 1;
                        info->rsmode = 0x00;

                        if ((info->remote_status == REM_PORT_ASSIGNED) ||
                            (info->remote_status == REM_OPEN_PORT))
                                info->remote_status = REM_OPEN_PORT;
                        else
                                info->remote_status = REM_PORT_AVAIL;
                }
        }
        if (si->need_to_send) {
                if (si->use_tcp == 0)
                        si_timer((unsigned long) si);
                else
                        tcp_send_remote_cmds(si);
        }
}
/*  Called by si_parse_input_pkt() */
static void port_resync(struct nr_port *info, struct si_port_state *pstate)
{
        pstate->nPutRemote = 0;
        pstate->nGetRemote = 0;
        pstate->nGetLocal = 0;
        pstate->modemStatusRegister = 0;
        pstate->errorStatusRegister = 0;
        pstate->update_queue = 0;

        TraceQueue(info,1,1);

        info->update_baud_rate = 1;
        info->update_control_settings = 1;
        info->update_special_charset = 1;
        info->rsmode = 0x00;

        return;
}

/*
 *  Puts a RK_PORT_SET command in the skb buff, if necessary.
 *  Make sure we have at least 3 bytes in the buff calling this routine!
 *  Inputs:  si-> ptr to the hub, skb-> ptr to put data in, port number to set
 *  Returns: 0 if OK, 1 on error.
 *  HDLC mode only.
 */
static inline int hdlc_set_port(struct si_state *si, struct sk_buff *skb, int port)
{
        unsigned char *cp;

        //check if port already set
        if (skb->cb[0] == (port & 0xFF)) {
#ifdef NSLINK_DEBUG_OUT_CMDS
                printk(KERN_INFO "nslink: hdlc_set_port(%d) skb->pkt_type = %d skb->cb[0] = %x : noop\n", port, skb->pkt_type, skb->cb[0]);
#endif
                return 0;
        }

        if (skb_tailroom(skb) < 3) {
#ifdef NSLINK_DEBUG_OUT_CMDS
                printk(KERN_INFO "nslink: hdlc_set_port(%d) skb_tailroom(skb) < 3 ... returning 1\n", port);
#endif
                return 1;
        }

        cp = skb_put(skb, 2);
        *cp++ = RK_PORT_SET;
        *cp = port;

#ifdef NSLINK_DEBUG_OUT_CMDS
        printk(KERN_INFO "nslink: hdlc_set_port(%d) skb->cb(%x) -> skb->cb(%x) \n", port, skb->cb[0], (port & 0xFF));
#endif

        //set the number of the port_set cmd
        skb->cb[0] = (port & 0xFF);

#ifdef NSLINK_DEBUG_OUT_CMDS
        printk(KERN_INFO "nslink: hdlc_set_port(%d) -> *cp = %x port = %d skb->cb = %x\n", port, *cp, port, skb->cb[0]);
#endif

        return 0;
}

/*
 *  Creates an skb buf, puts the RK_ASSIGN_REQ command in it, and sends it.
 *  Inputs:  si-> ptr to the hub
 *  HDLC mode only.
 */
static void hdlc_assign_ports(struct si_state *si)
{
        struct sk_buff *skb;
        unsigned char *cp;

        skb = prepare_hdlc_packet(si, 2, CONTROL_UFRAME);
        if (!skb)
                return;
        cp = skb_put(skb, 3);
        *cp++ = RK_ASSIGN_REQ;
        *cp = 0;
        send_hdlc_packet(si, skb);
        hdlc_resync(si);
#ifdef NSLINK_DEBUG_HDLC
        printk(KERN_INFO "nslink: hdlc: [%d] sent HDLC assign\n",si->conc_id);
#endif
}


/*
 *  Puts a RK_PORT_SET command in the skb buff, then a command that has 0 bytes
 *  of associated data. Make sure we have at least 4 bytes in the buff calling this routine!
 *  Inputs:  si-> ptr to the hub, skb-> ptr to put data in, port number to set, cmd
 *  Returns: 0 if OK, 1 on error.
 *  HDLC mode only.
 */
static inline int hdlc_0byte_cmd(struct si_state *si,
                                 struct sk_buff *skb, int port,
                                 __u8 cmd)
{
        unsigned char *cp;

        if (hdlc_set_port(si, skb, port))
                return 1;
        if (skb_tailroom(skb) < 2)
                return 1;
        cp = skb_put(skb, 1);
        *cp = cmd;
        return 0;
}

/*
 *  Puts a RK_PORT_SET command in the skb buff, then a command that has 1 byte
 *  of associated data. Make sure we have at least 5 bytes in the buff calling this routine!
 *  Inputs:  si-> ptr to the hub, skb-> ptr to put data in, port number to set, cmd
 *  Returns: 0 if OK, 1 on error.
 *  HDLC mode only.
 */
static inline int hdlc_1byte_cmd(struct si_state *si,
                                 struct sk_buff *skb, int port,
                                 __u8 cmd, __u8 value)
{
        unsigned char *cp;

        if (hdlc_set_port(si, skb, port))
                return 1;
        if (skb_tailroom(skb) < 3)
                return 1;
        cp = skb_put(skb, 2);
        *cp++ = cmd;
        *cp = value;
        return 0;
}

/*
 *  Puts a RK_PORT_SET command in the skb buff, then a command that has 2 bytes
 *  of associated data. Make sure we have at least 6 bytes in the buff calling this routine!
 *  Inputs:  si-> ptr to the hub, skb-> ptr to put data in, port number to set, cmd
 *  Returns: 0 if OK, 1 on error.
 *  HDLC mode only.
 */
static inline int hdlc_2byte_cmd(struct si_state *si,
                                 struct sk_buff *skb, int port,
                                 __u8 cmd, __u16 value)
{
        unsigned char *cp;

        if (hdlc_set_port(si, skb, port))
                return 1;
        if (skb_tailroom(skb) < 4)
                return 1;
        cp = skb_put(skb, 3);
        *cp++ = cmd;
        *cp++ = value & 0xFF;
        *cp = value >> 8;
        return 0;
}

/*
 *  Puts a RK_PORT_SET command in the skb buff, then a command that has 4 bytes
 *  of associated data. Make sure we have at least 8 bytes in the buff calling this routine!
 *  Inputs:  si-> ptr to the hub, skb-> ptr to put data in, port number to set, cmd
 *  Returns: 0 if OK, 1 on error.
 *  HDLC mode only.
 */
static inline int hdlc_4byte_cmd(struct si_state *si,
                                 struct sk_buff *skb, int port,
                                 __u8 cmd, __u32 value)
{
        unsigned char *cp;

        if (hdlc_set_port(si, skb, port))
                return 1;
        if (skb_tailroom(skb) < 6)
                return 1;
        cp = skb_put(skb, 5);
        *cp++ = cmd;
        *cp++ = value;
        *cp++ = value >> 8;
        *cp++ = value >> 16;
        *cp =   value >> 24;

        return 0;
}


/*
 *  Puts a RK_PORT_SET command in the skb buff, then a command that has n bytes
 *  of associated data.
 *  Inputs:  si-> ptr to the hub, skb-> ptr to put data in, port number to set, cmd
 *  Returns: 0 if OK, 1 on error.
 *  HDLC mode only.
 */
static inline int hdlc_Nbyte_cmd(struct si_state *si,
                                 struct sk_buff *skb, int port,
                                 __u8 cmd, __u8 *values, int N)
{
        unsigned char *cp;

        if (hdlc_set_port(si, skb, port))
                return 1;
        if (skb_tailroom(skb) < N+2)
                return 1;
        cp = skb_put(skb, N+1);
        *cp++ = cmd;
        memcpy(cp,values,N);
        return 0;
}

/*
 *  Copies HDLC data from info->xmit_buf into a skb.
 *  Inputs: si, info, pstate -> ptrs to hub&port info, skb-> ptr to the skb buf to be filled in
 *  Returns: 0 if OK, 1 if error
 *  HDLC mode only.
 */
static inline int hdlc_xmit(struct si_state *si, struct sk_buff *skb,
                            struct nr_port *info, struct si_port_state *pstate)
{
        unsigned char *cp;
        int c, space, total_len = 0;
        struct tty_struct *tty;
        unsigned long flags;

        if (!info || (!info->xmit_cnt && !info->x_char) ||
            !get_tx_space_remote(pstate))
                return 1;

        if (hdlc_set_port(si, skb, info->portnum))
                return 1;

        if (skb_tailroom(skb) < 5)
                return 1;

        //  Put the RK_DATA_BLK command into skb
        cp = skb_put(skb, 3);
        *cp++ = RK_DATA_BLK;
        if (info->x_char) {
                *(skb_put(skb, 1)) = info->x_char;
                total_len++;
                pstate->nPutRemote++;
                info->x_char = 0;
                TraceQueue(info,2,1);
        }

        //  Copy the data into skb, spinlock to protect xmit_tail and xmit_cnt
        spin_lock_irqsave(&info->slock, flags);
        while (1) {
                c = skb_tailroom(skb) - 1;
                if (c > info->xmit_cnt)
                        c = info->xmit_cnt;
                if (c > (XMIT_BUF_SIZE - info->xmit_tail))
                        c = XMIT_BUF_SIZE - info->xmit_tail;
                space = get_tx_space_remote(pstate);
                if (c > space)
                        c = space;
                if (c <= 0)
                        break;
                memcpy(skb_put(skb, c), info->xmit_buf + info->xmit_tail, c);
                info->xmit_tail += c;
                info->xmit_tail &= XMIT_BUF_SIZE - 1;
                info->xmit_cnt -= c;
                total_len += c;
                pstate->nPutRemote += (__u16) c;
                TraceQueue(info,2,c);
        }
        spin_unlock_irqrestore(&info->slock, flags);

#ifdef NSLINK_DEBUG_OUT_CMDS
        printk(KERN_INFO "nslink: ttySI%d: sent %d bytes\n", info->line, total_len);
#endif

        //  Copy the length info into skb
        *cp++ = total_len & 0xFF;
        *cp++ = total_len >> 8;

        tty = info->tty;
        if ((info->xmit_cnt < WAKEUP_CHARS) && tty)
                wake_tty_write(tty);
        return 0;
}

/*
 *  Sends required commands to a particular hub.  Creates a skb,
 *  fills it with connect check or update timeout info based on si flags.
 *  Then goes through each port, using it's nr_port and si_port_state
 *  object flags to do qin update, set RSmode, set MCR, IC timeout.
 */
static void hdlc_send_remote_cmds(struct si_state *si)
{
        int send_packet;  //  If set, the skb has HDLC data that needs to be sent to the hub
        int i, aborted, first_port = 0, badval = MAX_NRP_PORTS + 1;
        struct sk_buff *skb = NULL;
        unsigned char *cp;
        struct si_port_state *pstate;
        struct nr_port *info;

retry:
        send_packet = 0;
        si->need_to_send = 0;
        aborted = 0;

        // Get an skb
        skb = prepare_hdlc_packet(si, MAX_CMD_PKT, CONTROL_IFRAME);

        if (!skb) {
                printk(KERN_INFO "nslink: ERROR! hdlc_send_remote_cmds unable to allocate skb\n");
                return;
        }
        //since we aren't using this member for tracking the port number
        //any longer then lets set it and forget it here.
        skb->pkt_type = PACKET_OUTGOING;

        //the cb member is where we are supposed to house our stuff when
        //tossing around these buffers.
        skb->cb[0] = (badval & 0xFF); //set to invalid value

        // Send connect check
        if (si->need_to_send_connect_check) {
                cp = skb_put(skb,1);
                if (cp)
                        *cp = RK_CONNECT_CHECK;
                si->need_to_send_connect_check = 0;
                send_packet = 1;
        }

        // Send update timeout
        if (si->update_timeout) {
                cp = skb_put(skb,5);
                *cp++ = RK_TIMEOUT_SET;
                *cp++ = si->link_timeout;
                *cp++ = si->link_timeout >> 8;
                *cp++ = si->link_timeout >> 16;
                *cp   =   si->link_timeout >> 24;
                if ( si->scan_period && (si->arch_id == 3 || si->arch_id ==2)) {
                        cp = skb_put(skb,2);
                        *cp++ = RK_SCANPERIOD_SET;
                        *cp   = si->scan_period;
                }
                if ( si->rexmit_timer && (si->arch_id == 3 || si->arch_id ==2)) {
                        if( (si->rexmit_timer >= 40 && si->rexmit_timer <= 200) && (si->rexmit_timer % 20 == 0) ) {
                                //printk(KERN_INFO "in hdlc_send_remote_cmds rexmit timer \n");
                                cp = skb_put(skb,2);
                                *cp++ = RK_RETRANS_TIMER_SET;
                                *cp   = si->rexmit_timer;
                        }
                }

                si->update_timeout = 0;
                send_packet = 1;
        }

        // Go through each port in the hub, sending needed commands
        for (i = first_port; i < si->num_ports; i++) {

                // Get port config/info
                info = nrp_table[i + si->base_port];
                if (!info)
                        continue;

                if (info->remote_status == REM_PORT_ASSIGNED) {
                        pstate = si->port_state + i;

                        // Send qin update
                        if (pstate->update_queue) {
                                if (hdlc_2byte_cmd(si, skb, i, RK_QIN_STATUS, pstate->nGetLocal)) {
                                        aborted = 1;
                                        break;
                                }
                                pstate->update_queue = 0;
                                send_packet = 1;
                        }

                        /*
                        *  Update RSmode (RS232, etc).  DeviceMaster Serial hub has only RS232
                        */
                        if (info->rsmode != si->rsmode[i]) {
                                if (si->rsmode[i] != SI_RSMODE_232) {
                                        if (UnitHasNo485(si->modelID)) {
                                                printk(KERN_INFO "nslink: Error - attempting to change interface mode for port ttySI%d on a model that only has RS232 ports \n", info->line);
                                                si->rsmode[i] = SI_RSMODE_232;
                                        }
                                }
                                if ((si->rsmode[i] == SI_RSMODE_485FDM) ||
                                    (si->rsmode[i] == SI_RSMODE_485FDS)) {
                                        if (!UnitHas485FD(si->modelID)) {
                                                printk(KERN_INFO "nslink: Error - attempting to change interface mode for port ttySI%d on a model that does not support RS485 full duplex mode \n", info->line);
                                                si->rsmode[i] = SI_RSMODE_232;
                                        }
                                }
                                info->rsmode = si->rsmode[i];
                                if (hdlc_1byte_cmd(si, skb, i, RK_RSMODE_SET, info->rsmode)) {
                                        info->rsmode = 0;
                                        aborted = 1;
                                        break;
                                }
                                if ((info->rsmode == SI_RSMODE_485) ||
                                    (info->rsmode == SI_RSMODE_485FDS)) {
                                        /* RS485 turn on RTS toggle */
                                        info->update_control_settings = 1;
                                        info->control_settings &= ~SC_FLOW_RTS_MASK;
                                        info->control_settings |= SC_FLOW_RTS_ARS485;
                                }
                                send_packet = 1;
                        }

                        // Update baud rate
                        if (info->update_baud_rate) {
#ifdef NSLINK_DEBUG_OUT_CMDS
                                printk(KERN_INFO "nslink: set baud rate to %d\n", info->baud_rate);
#endif

                                if (hdlc_4byte_cmd(si, skb, i, RK_BAUD_SET, info->baud_rate)) {
                                        aborted = 1;
                                        break;
                                }
                                info->update_baud_rate = 0;
                                send_packet = 1;
                        }

                        //  Set charset
                        if (info->update_special_charset) {
                                unsigned char b[6];
#ifdef NSLINK_DEBUG_OUT_CMDS

                                printk(KERN_INFO "nslink: set specials to %02x,%02x,%02x,%02x,%02x,%02x\n",
                                       info->rx_xon,info->rx_xoff,
                                       info->tx_xon,info->tx_xoff,
                                       0,0);
#endif

                                b[0] = info->rx_xon;
                                b[1] = info->rx_xoff;
                                b[2] = info->tx_xon;
                                b[3] = info->tx_xoff;
                                b[4] = 0;
                                b[5] = 0;
                                if (hdlc_Nbyte_cmd(si, skb, i, RK_SPECIAL_CHAR_SET, b, 6)) {
                                        aborted = 1;
                                        break;
                                }
                                info->update_special_charset = 0;
                                send_packet = 1;
                        }

                        // Control set
                        if (info->update_control_settings) {
#ifdef NSLINK_DEBUG_OUT_CMDS
                                printk(KERN_INFO "nslink: set control byte to %d\n",
                                       info->control_settings);
#endif

                                if (hdlc_2byte_cmd
                                    (si, skb, i, RK_CONTROL_SET, info->control_settings)) {
                                        aborted = 1;
                                        break;
                                }
                                info->update_control_settings = 0;
                                send_packet = 1;
                        }

                        // Set mcr
                        if (info->update_mcr) {
#ifdef NSLINK_DEBUG_OUT_CMDS
                                printk(KERN_INFO "nslink: set MCR to %d\n",
                                       info->modem_control_register);
#endif

                                if (hdlc_2byte_cmd
                                    (si, skb, i, RK_MCR_SET, info->modem_control_register)) {
                                        aborted = 1;
                                        break;
                                }
                                info->update_mcr = 0;
                                send_packet = 1;
                        }

                        // Set IC timeout
                        if (info->update_ic_timeout) {
#ifdef NSLINK_DEBUG_OUT_CMDS
                                printk(KERN_INFO "nslink: set IC timeout %d\n", info->inter_char_timeout);
#endif

                                if (hdlc_4byte_cmd (si, skb, i, RK_IC_TIMEOUT_SET, info->inter_char_timeout)) {
                                        aborted = 1;
                                        break;
                                }
                                info->update_ic_timeout = 0;
                                send_packet = 1;
                        }

                        // make sure action command is last one of the setup
                        // commands we send, since we use waiting for the
                        // action resonse is a synchronization method

                        // Action set
                        if (info->update_action) {
#ifdef NSLINK_DEBUG_OUT_CMDS
                                printk(KERN_INFO "nslink: set action to %d\n", info->action_register);
#endif

                                if (hdlc_2byte_cmd
                                    (si, skb, i, RK_ACTION_SET, info->action_register)) {
                                        aborted = 1;
                                        break;
                                }
                                info->update_action = 0;
                                info->action_register = 0;
                                send_packet = 1;
                        }

                        /*
                         * Transmit characters in our transmit buffer
                         */
                        if (info->x_char ||
                            (info->xmit_cnt && info->tty &&
                             !info->tty->hw_stopped && !info->tty->stopped)) {
                                if (!hdlc_xmit(si, skb, info, pstate))
                                        send_packet = 1;
                        }
                } else if (info->remote_status == REM_OPEN_PORT) {
                        if (hdlc_1byte_cmd(si, skb, i, RK_REMOTE_STATUS, REM_OPEN_PORT)) {
                                aborted = 1;
                                break;
                        }
                        info->remote_status = REM_WAITING_OPEN;
                        info->conn_refused = 0;
                        send_packet = 1;
                } else if (info->remote_status == REM_RELEASE_PORT) {
                        if (hdlc_1byte_cmd(si, skb, i, RK_REMOTE_STATUS, REM_RELEASE_PORT)) {
                                aborted = 1;
                                break;
                        }
                        info->remote_status = REM_WAITING_CLOSE;
                        send_packet = 1;
                }

                if (info->send_force_release) {
                        if (hdlc_0byte_cmd(si,skb,i,RK_RELEASE_REQ)) {
                                aborted = 1;
                                break;
                        }
                        send_packet = 1;
                        info->send_force_release = 0;
                }

                first_port += 1;
        }

        // If data to be send has been placed in the skb, terminate is and send it.
        if (send_packet) {
                cp = skb_put(skb, 1);
                *cp = 0;
                send_hdlc_packet(si, skb);

#ifdef NSLINK_DEBUG_OUT_CMDS

                printk(KERN_INFO "nslink: packet sent (%d, %d)\n", si->out_snd_index,
                       si->in_ack_index);
#endif


        } else {
#ifdef NSLINK_DEBUG_OUT_CMDS
                printk(KERN_INFO "nslink: Freeing unneeded skbuff\n");
#endif

                kfree_skb(skb);
        }
        if (aborted)
                goto retry;
}

/*
 *   Verify a received ACK value in HDLC mode
 */
inline static int verifyAckValue(struct si_state *si, unsigned char rxAck)
{
        struct sk_buff *ph;
        struct sk_buff *pt;
        unsigned long flags;

        //  Get pointers to the first and last skb in the retransmit queue.
        //  Must grab the list spinlock so no-one grabs the skb from under us.
        spin_lock_irqsave(&si->rtq.lock, flags);
        ph = skb_peek(&si->rtq);      // oldest packet
        pt = skb_peek_tail(&si->rtq); // newest packet

        if (ph) {
                unsigned char head = ((struct HDLChdr*)(skb_transport_header(ph)+3))->out_snd_index;
                unsigned char tail = pt ? ((struct HDLChdr*)(skb_transport_header(pt)+3))->out_snd_index : head;
                signed char s1 = rxAck - head;
                signed char s2 = (tail+1) - rxAck;
                if ((s1 < 0) || (s2 < 0)) {
                        printk(KERN_INFO "nslink: hdlc: [%d] bad ACK: head=%02x tail=%02x ack=%02x\n",
                               si->conc_id,head&0xff,tail&0xff,rxAck&0xff);
                        spin_unlock_irqrestore(&si->rtq.lock, flags);
                        return 0;
                }
        }
        spin_unlock_irqrestore(&si->rtq.lock, flags);
        return 1;
}

/*
 * This routine processes incoming HDLC packets, and handles the
 * acknowledging of them.
 */
static void process_hdlc_packet(struct sk_buff *skb, struct si_state *si)
{
        struct HDLChdr *hdlc_hdr, *hdr2;
        unsigned long flags;
        struct sk_buff *pp;
        unsigned char ack_index, ns_save;
        unsigned char *cp;
        int len, ack_count;

        /*
         * We only process HDLC packet when we are connected, waiting
         * for a response from a connect request, or connected but
         * waiting for the response from the keepalive ping.
         */
        switch (si->state) {
        case SI_STATE_CONNECTING:
        case SI_STATE_CONNECTING_WAIT:
        case SI_STATE_CONNECTED:
                break;
        default:
                return;
        }

        hdlc_hdr = (struct HDLChdr *) (skb_transport_header(skb) + 3);

        skb_pull(skb, sizeof(struct HDLChdr));

        // If this is an unindexed packet,
        if (!(hdlc_hdr->hdlc_control & CONTROL_UFRAME)) {
#ifdef NSLINK_DEBUG_HDLC
                printk(KERN_INFO "nslink: hdlc: [%d] rx packet (%d) #%02x\n", si->conc_id, hdlc_hdr->hdlc_control, hdlc_hdr->out_snd_index);
#endif

                // Check for out-of-sequence packet, discard it if so
                if (hdlc_hdr->out_snd_index != si->next_in_index) {
                        ++si->rxBadOrderCount;
#if defined(NSLINK_DEBUG_RETRANS) || defined(NSLINK_DEBUG_HDLC)
                        printk(KERN_INFO "nslink: hdlc: [%d] rx bad seq -- got:%02x, exp:%02x, cnt=%d\n", si->conc_id, hdlc_hdr->out_snd_index, si->next_in_index,si->rxBadOrderCount);
#endif
                        TraceHdlc(si,5,hdlc_hdr->out_snd_index);
                        hdlc_send_ack(si, CONTROL_UFRAME);
                        return;
                }

                // Unindexed packet, in sequence, it is just an ACK.
                // If number of unacked packets exceeds window size,
                // send ack.  Otherwise, make sure rx ack timer is
                // running.
                si->rxBadOrderCount = 0;
                atomic_inc(&si->unacked_pkts);
                if (atomic_read(&si->unacked_pkts) > HDLC_MAX_UNACKED_COUNT/2)
                        hdlc_send_ack(si, CONTROL_UFRAME);
                else if (!(si->hdlc_state_flags & HDLC_RCV_TIMER_ACTIVE)) {
                        si->receiver_ack_timer = jiffies + hdlc_ack_time;
                        si->hdlc_state_flags |= HDLC_RCV_TIMER_ACTIVE;
                        si_set_timer(si, hdlc_ack_time);
                }
                TraceHdlc(si,2,si->next_in_index);
                si->next_in_index++;
        }

        // Prune retrans queue if incoming ack value changed
        if (si->in_ack_index != hdlc_hdr->next_in_index) {
                if (!verifyAckValue(si, hdlc_hdr->next_in_index)) {
                        TraceHdlcStop(si,-3,hdlc_hdr->next_in_index);
                        goto jump;
                }

                TraceHdlc(si,1,hdlc_hdr->next_in_index);

                si->in_ack_index = hdlc_hdr->next_in_index;
                ack_index = si->in_ack_index - 1;
                ack_count = 0;

                /*
                 * Walk through the retransmit queue, pulling off the skb pointers
                 * and freeing the skb's for ack'd packets.  List lock is taken.
                 *
                 * Note that this assumes that we will be removing at
                 * least one item from the queue.  Given that the ack
                 * index has changed, this is a reasonable assumption.
                 */
                spin_lock_irqsave(&si->rtq.lock, flags);
                while ((pp = __skb_dequeue(&si->rtq))) {
                        hdr2 = (struct HDLChdr *) (skb_transport_header(pp) + 3);

                        ns_save = hdr2->out_snd_index;

                        --si->rtqCount;
                        kfree_skb(pp);
                        ack_count++;

                        if (ns_save == ack_index)
                                break;
                }

                si->retransCount = 0;

                // Check for an empty retransmit queue
                if (!skb_peek(&si->rtq)) {
#ifdef NSLINK_DEBUG_HDLC
                        printk(KERN_INFO "nslink: hdlc: [%d] all acked!\n",si->conc_id);
#endif
                        si->hdlc_state_flags &= ~HDLC_SND_TIMER_ACTIVE;
                }

                // Skb queue not empty, start retry timer
                else {
#ifdef NSLINK_DEBUG_HDLC
                        printk(KERN_INFO "nslink: hdlc: [%d] not all acked!\n",si->conc_id);
#endif
                        si->sender_ack_timer = jiffies + hdlc_retrans_time;
                        si->hdlc_state_flags |= HDLC_SND_TIMER_ACTIVE;
                        si_set_timer(si, hdlc_retrans_time);
                }
                spin_unlock_irqrestore(&si->rtq.lock, flags);
        }

jump:

        /*
         * OK, now we actually process the data in the packet.
         *
         * XXX Currently, we assume the packet is null terminated, and
         * we don't do length checking.  We need to fix this.  Right
         * now, a malformed packet can cause the system to crash.  If
         * it's any consolation the NT driver doesn't do this sanity
         * checking either, but Unix drivers should be more robust
         * than NT drivers.  Just because it's easy to make NT machines
         * crash with malformed packets doesn't mean we have to make
         * Unix machines bug-compatible with NT.    :-)
         */
        cp = skb->data;
        len = skb->len;
        si_parse_input_pkt(si, cp, len);

        if (si->need_to_send)
                si_set_timer(si, 0);
}

//  Walk through the global array of hub config/info, searching for
//  the hub that matches the MAC address passed in.
//  Returns:  Ptr to hub config/info, NULL if not found.
static struct si_state *find_si_struct(unsigned char *eth_addr) {
        struct si_state *si;
        int i;

        for (i = 0; i < numBoxes; i++) {
                si = conc_state[i];
                if (!si)
                        continue;
                if (memcmp(si->ether_addr, eth_addr, 6) == 0)
                        return si;
        }
        return 0;
}

// The networking stack calls this function to handle ETH_P_COMTROL (0x11FE) packets received.
static int comtrol_rcv(struct sk_buff *skb,
                       struct NET_DEVICE *dev,
                       struct packet_type *pt
#if LINUX_VERSION_CODE > VERSION_CODE(2,6,13)
                       ,struct net_device *orig_dev
#endif
                      )
{
        struct comtrolhdr *chdr = (struct comtrolhdr *) skb_transport_header(skb);
        struct ethhdr *ehdr = (struct ethhdr *) skb_mac_header(skb);
        unsigned char *hdr;
        struct adminhdr *ahdr;
        unsigned char *admin_data;
        struct si_state *si;
        struct si_box_report *rpt;

        /*
         * Drop packets not meant for us (that may show up if the
         * interface is in promiscuous mode.)
         */
        if (skb->pkt_type != PACKET_HOST) { //was (skb->pkt_type == PACKET_OTHERHOST)
                goto drop;
        }

        //  Get the comtrol header from the skb, check for valid info
        hdr = skb_pull(skb, sizeof(struct comtrolhdr));

        if (hdr == NULL) {
                //printk(KERN_INFO "nslink: comtrol_rcv ... comtrolhdr was NULL - game over! \n");
                goto drop;
        }

        if (chdr->product_id != SI_ID) {
                //printk(KERN_INFO "nslink: comtrol_rcv ... not SI_ID - game over! \n");
                goto drop;
        }

        if (chdr->conc_id >= numBoxes) {
                //printk(KERN_INFO "nslink: comtrol_rcv ... too many boxes %d - game over! \n", chdr->conc_id);
                goto drop;
        }

        // Comtrol hdr OK, get hub config/info
        si = conc_state[chdr->conc_id];

        if (!si) {
                //printk(KERN_INFO "nslink: comtrol_rcv ... no si object - game over! \n");
                goto drop;
        }

        // check to see if MAC address matches the device in our hub table
        if (memcmp(ehdr->h_source, si->ether_addr,6)) {
                //printk(KERN_INFO "nslink: comtrol_rcv ... memcmp failed - game over! \n");
                goto drop;
        }

        //printk(KERN_INFO "nslink: comtrol_rcv ... past checks for goto drop.\n");

        switch (chdr->packet_class) {

        case ADMIN_CLASS:

                //printk(KERN_INFO "nslink: comtrol_rcv ... ADMIN_CLASS packet\n");

                ahdr = (struct adminhdr *) hdr;
                admin_data = skb_pull(skb, sizeof(struct adminhdr));

                switch (ahdr->admin_type) {
                case ADMIN_SEC_LOAD_TYPE:
                        break;

                case ADMIN_PRD_ID_REP_TYPE: {
                        //printk(KERN_INFO "nslink: comtrol_rcv ... ADMIN_PRD_ID_REP_TYPE \n");

#ifdef NSLINK_DEBUG_DEVICE_ID
                        struct mac_id_response *idresp = (struct mac_id_response*)admin_data;
                        int i;
                        int len = (ahdr->len_high << 8) + ahdr->len_low;

                        printk(KERN_INFO "nslink: ID response [len=%d] device type %02x, MAC addr "
                               "%02x:%02x:%02x:%02x:%02x:%02x %s\n",
                               len,
                               chdr->product_id, admin_data[0],
                               admin_data[1], admin_data[2],
                               admin_data[3], admin_data[4],
                               admin_data[5], admin_data[6] ? "active" : "loading");

                        printk(KERN_INFO "nslink: ID response data:\n");
                        for (i = 0; i < len; ++i) {
                                if (!(i&0x07))
                                        printk(KERN_INFO "\n");
                                printk(KERN_INFO "%02x ", admin_data[i]);
                        }
                        printk(KERN_INFO "\n");

                        if (len >= sizeof *idresp) {
                                unsigned char *AppData;
                                unsigned char AppDataCount;

                                printk(KERN_INFO "nslink:        Model = %d\n", ntohl(idresp->ModelId));
                                printk(KERN_INFO "nslink:         Arch = %d\n", idresp->ArchId);
                                printk(KERN_INFO "nslink:    Num Ports = %d\n", idresp->NumPorts);
                                printk(KERN_INFO "nslink:   Program Id = %d\n", ntohs(idresp->ProgramId));
                                printk(KERN_INFO "nslink:    Board Rev = %02x\n",
                                       idresp->BoardRev + 'A' - 1);
                                printk(KERN_INFO "nslink:     Boot Rev = %d.%02d\n",
                                       idresp->BootRevMajor, idresp->BootRevMinor);
                                printk(KERN_INFO "nslink:     Num Apps = %d\n", idresp->NumApps);
                                for (i = 0; i < idresp->NumApps; ++i) {
                                        printk(KERN_INFO "nslink:         App[%d] Id = %5d  ", i,
                                               htons(idresp->AppRecord[i].AppId));
                                        printk(KERN_INFO "version %d.%02d\n",
                                               idresp->AppRecord[i].AppRevMajor,
                                               idresp->AppRecord[i].AppRevMinor);
                                }

                                AppData =
                                        &idresp->NumApps +
                                        (idresp->NumApps * sizeof idresp->AppRecord[0]) + 1;
                                AppDataCount = AppData[-1];

                                printk(KERN_INFO "nslink:     App Data = [%d]: ", AppDataCount);
                                for (i = 0; i < AppDataCount; ++i)
                                        printk(KERN_INFO "%02x ", AppData[i]);
                                printk(KERN_INFO "\n");
                        }
#endif

                        //printk(KERN_INFO "nslink: comtrol_rcv ... ADMIN_PRD_ID_REP_TYPE: fetching si struct \n");

                        /*
                         * note:  admin_data should be same as
                         * ehdr->h_source!
                         */
                        // Get ptr to hub config/info.
                        si = find_si_struct(admin_data);
                        if (si && si->state == SI_STATE_BOOTED) {

                                //printk(KERN_INFO "nslink: comtrol_rcv ... ADMIN_PRD_ID_REP_TYPE: SI_STATE_BOOTED \n");

                                if (admin_data[6]) {
                                        //printk(KERN_INFO "nslink: comtrol_rcv ... ADMIN_PRD_ID_REP_TYPE: SI_STATE_CONNECTING \n");
                                        si->state = SI_STATE_CONNECTING;
                                } else {
                                        //printk(KERN_INFO "nslink: comtrol_rcv ... ADMIN_PRD_ID_REP_TYPE: SI_STATE_RESET \n");
                                        si->state = SI_STATE_RESET;
                                }
                        }

                        //this is set in init_hub_state from daemon_si_box_config copied down to si_box_config
                        //si->modelID = ntohl(idresp->ModelId);

                        if ((num_box_rpts < MAX_NRP_SIS) &&
                            (!rpt_loading_only || !admin_data[6])) {
                                rpt = si_box_rpts + num_box_rpts;
                                memcpy(rpt->ether_addr, ehdr->h_source, 6);
                                rpt->product_id = chdr->product_id;
                                rpt->flags = admin_data[6];
                                num_box_rpts++;
                        }
                }

                //printk(KERN_INFO "nslink: comtrol_rcv...finished processing ADMIN_PRD_ID_REP_TYPE\n");

                break;   //  case ADMIN_PRD_ID_REP_TYPE:
                }          //  end of switch (ahdr->admin_type)


                break;  // switch (chdr->packet_class) case ADMIN_CLASS:

                // "Async" class packet, process...
        case SI_CLASS:
                if (si) {
                        //printk(KERN_INFO "nslink: comtrol_rcv...SI_CLASS: si good -> now do hdlc pkt\n");
                        process_hdlc_packet(skb, si);
                }
                break;

        }

drop:
        kfree_skb(skb);
        return (0);
}

/*
 *  The timer function happens here, only used for HDLC connections.  This function
 *  is not only the timer bottom half,  it is called by many other functions when
 *  something needs to be sent to the hub.  The atomic timer_active variable provides
 *  a means by which it can only be run one at a time.  If another process/thread/whatever
 *  calls this function while it is already being run, it will return immediately.  This
 *  opens up the possible delay of a needed action, but solves the reentrancy problem.
 */
static void si_timer(unsigned long arg)
{
        struct si_state *si = (struct si_state *) arg;
        unsigned long tmp;
        unsigned long next_time = T_INFINITE;
        int i;

        if (atomic_dec_and_test(&si->timer_active) == 0) {
                /* The timer must be already running */
                atomic_inc(&si->timer_active);
                return;
        }

#ifdef NSLINK_DEBUG_TIMER
        printk(KERN_INFO "nslink: In si_timer (%d)...", atomic_read(&si->timer_active));
#endif

        if (si->state < SI_STATE_CONNECTED) {
                if (si->state == SI_STATE_RESET) {
                        // signal approprate daemon that we need the box booted;
                        wake_up_interruptible(&si->waitqueue);
                }

                if (si->state == SI_STATE_BOOTED) {
                        send_query_packet(si->dev, si);
                        if ((5 * HZ) < next_time)
                                next_time = 5 * HZ;
                        si->send_connect_time = jiffies;
                }

                /*
                 * If we're in the state SI_STATE_CONNECTING, then send the
                 * HDLC connect packet.  Once we receive the connect packet,
                 * we will enter the SI_STATE_CONNECTED.
                 */
                if (si->state == SI_STATE_CONNECTING) {
                        if (IsPast(si->send_connect_time)) {
                                if (si->resend_counter >= 5) {
                                        si->state = SI_STATE_RESET;
                                        si->resend_counter = 0;
#ifdef NSLINK_DEBUG_TIMER
                                        printk(KERN_INFO "nslink: assign timeout\n");
#endif
                                } else {
                                        si->resend_counter++;
                                        hdlc_assign_ports(si);
                                        si->send_connect_time = jiffies + hdlc_retrans_time;
                                }
                                if (hdlc_retrans_time < next_time)
                                        next_time = hdlc_retrans_time;
                        } else {
                                tmp = si->send_connect_time - jiffies;
                                if (tmp < next_time)
                                        next_time = tmp;
                        }
                }
        }
#ifdef NSLINK_DEBUG_TIMER
        printk(KERN_INFO "nslink: hdlc_state_flags=%d...\n", si->hdlc_state_flags);
#endif

        if (si->state == SI_STATE_CONNECTED && si->linkTimeoutJiffies) {
                /* need to send RK_CONNECT_CHECK ? */
                if (IsPast(si->keep_alive_tx_timer)) {
#ifdef NSLINK_DEBUG_KEEPALIVE
                        printk(KERN_INFO "nslink: tx RK_CONNECT_CHECK: %d\n",si->conc_id);
#endif
                        si->need_to_send = 1;
                        si->need_to_send_connect_check = 1;
                        si->keep_alive_tx_timer += si->linkTimeoutJiffies/2;
                        if (si->keep_alive_tx_timer - jiffies < next_time)
                                next_time = si->keep_alive_tx_timer - jiffies;
                } else {
                        tmp = si->keep_alive_tx_timer - jiffies;
                        if (tmp < next_time)
                                next_time = tmp;
                }

                /* link timeout? */
                if (IsPast(si->keep_alive_timeout)) {
                        printk(KERN_INFO "nslink: hdlc link timeout on hub %d\n",si->conc_id);
                        si->state = SI_STATE_RESET;
                        si->hdlc_state_flags = 0;
                        if ((5 * HZ) < next_time)
                                next_time = 5 * HZ;
                        set_otherclose_all_tty(si);
                        wake_all_tty(si);
                } else {
                        tmp = si->keep_alive_timeout - jiffies;
                        if (tmp < next_time)
                                next_time = tmp;
                }

        }

        /*
         * Check the state again in case we declared the device as
         * dead.
         */
        if (si->state == SI_STATE_CONNECTED) {
                /*
                 * If there are characters in the receive buffer,
                 * try to send them to the line discpline.
                 */
                for (i = 0; i < si->num_ports; i++)
                        send_recv_buf_to_ldisc(nrp_table[i + si->base_port]);

                /*
                 * See if we need to send commands to the remote.
                 */
                if (si->need_to_send) {
                        int pkts_out;
                        if (si->out_snd_index >= si->in_ack_index)
                                pkts_out = (int) (si->out_snd_index - si->in_ack_index);
                        else
                                pkts_out = (int) (si->out_snd_index + 1 + (0xff - si->in_ack_index));

                        if (pkts_out < HDLC_MAX_UNACKED_COUNT)
                                hdlc_send_remote_cmds(si);
                }
        }

        /*
         * If we haven't transmited any HDLC packets within the ack
         * timeout period, send a bare ack packet to prevent the
         * remote from resending packets at us.
         */

        if (si->hdlc_state_flags & HDLC_RCV_TIMER_ACTIVE) {
#ifdef NSLINK_DEBUG_TIMER
                printk(KERN_INFO "nslink: rcv timer...");
#endif
                if (IsPast(si->receiver_ack_timer)) {
#ifdef NSLINK_DEBUG_TIMER
                        printk(KERN_INFO "nslink: hdlc timeout...");
#endif
                        hdlc_send_ack(si, CONTROL_UFRAME);
                } else {
                        tmp = si->receiver_ack_timer - jiffies;
                        if (tmp < next_time)
                                next_time = tmp;
                }
        }
        /*
         * If we haven't gotten an ack for the packets which we have
         * sent, resend the packets in the retransmit queue
         */
        if (si->hdlc_state_flags & HDLC_SND_TIMER_ACTIVE) {
#if 0
                printk(KERN_INFO "nslink: ack_timer: %ld, %ld...", si->sender_ack_timer, jiffies);
#endif
                if (IsPast(si->sender_ack_timer)) {
                        hdlc_retransmit(si);
                        if (si->hdlc_state_flags & HDLC_SND_TIMER_ACTIVE) {
                                si->sender_ack_timer = jiffies + hdlc_retrans_time;
                                if (hdlc_retrans_time < next_time)
                                        next_time = hdlc_retrans_time;
                        }
                } else {
                        tmp = si->sender_ack_timer - jiffies;
                        if (tmp < next_time)
                                next_time = tmp;
                }
        }

        if (next_time != T_INFINITE)
                si_set_timer(si, next_time);

        atomic_inc(&si->timer_active);

#ifdef NSLINK_DEBUG_TIMER

        printk(KERN_INFO "nslink: End si_timer (next_time = %lu)\n", next_time);
#endif
}

static void si_set_timer(struct si_state *si, unsigned long ticks)
{
        unsigned long new_time;

#ifdef NSLINK_DEBUG_TIMER

        printk(KERN_INFO "nslink: set_timer(%lu)...", ticks);
#endif

        if (si->use_tcp)
                return;

        new_time = jiffies + ticks;

        if ((!timer_pending(&si->timer)) ||
            ((si->timer.expires != new_time && ((si->timer.expires - jiffies) > ticks))))
                mod_timer(&si->timer,new_time);
}

static struct packet_type comtrol_packet_type = {
        .type	= __constant_htons(ETH_P_COMTROL),
        .func	= comtrol_rcv,
};

/********************************************************************************/
/*                                                                              */
/*        MAC / TCP COMMON FUNCTIONS                                            */
/*                                                                              */
/********************************************************************************/

/*
 *  Process the data portion of an incoming (HDLC or TCP) packet.  We do this by
 *  walking through the received data buffer, processing the commands until the terminating
 *  byte is reached.  Note that every command has a length (cmd + associated data).  If the
 *  packet length is too short for the command, this is a fragment (caused by tcp).  Keep
 *  this fragment in a different buffer for processing when the next packet is received.
 */
static void si_parse_input_pkt(struct si_state *si,
                               unsigned char *buf,
                               unsigned long len)
{
        int frcnt, portnum=-1;
        unsigned int cmdval;
        struct nr_port *info;
        struct si_port_state *pstate;
        unsigned long flags;

        info = NULL;
        pstate = NULL;

        // might have fragmented a packet so we need to remember our
        // port number from last time
        if (si->use_tcp) {
                portnum = si->tcp_last_port;
                if ((portnum >= 0) && (portnum < si->num_ports)) {
                        info = nrp_table[portnum + si->base_port];
                        pstate = si->port_state + portnum;
                }
        }

        //printk(KERN_INFO "nslink: parse_input_packet(%p,%p,%d)\n",si,buf,len);
        //printk(KERN_INFO "nslink: info=%p pstate=%p\n",info,pstate);

        while ((len > 0) && (*buf)) {
                switch (*buf) {
                case RK_CONNECT_CHECK:
#if defined(NSLINK_DEBUG_IN_CMDS) || defined(NSLINK_DEBUG_KEEPALIVE)

                        printk(KERN_INFO "nslink: rx RK_CONNECT_CHECK: %d\n",si->conc_id);
#endif

                        if (si->linkTimeoutJiffies) {
                                si->keep_alive_timeout = jiffies + si->linkTimeoutJiffies;
                                if (!si->use_tcp)
                                        si_set_timer(si, si->linkTimeoutJiffies);
                        }
                        frcnt = 1;
                        break;
                case RK_PORT_SET:
                        if (len < 2) {
                                if (si->use_tcp)
                                        tcp_store_fragment(si, buf, len);
                                return;
                        }
                        portnum = buf[1];
                        /* printk(KERN_INFO "nslink: RK_PORT_SET: %d\n",portnum); */
                        if ((portnum >= 0) && (portnum < si->num_ports)) {
                                si->tcp_last_port = portnum;
                                info = nrp_table[portnum + si->base_port];
                                pstate = si->port_state + portnum;
                                frcnt = 2;
                        } else {
                                printk(KERN_INFO "nslink: RK_PORT_SET: invalid port %d\n", portnum);
                                info = NULL;
                                pstate = NULL;
                                frcnt = len;
                        }
                        break;
                case RK_MSR_SET:
                        if (len < 3) {
                                if (si->use_tcp)
                                        tcp_store_fragment(si, buf, len);
                                return;
                        }
                        cmdval = buf[1];
                        cmdval += buf[2] << 8;

                        //	  printk(KERN_INFO "nslink: RK_MSR_SET: 0x%x\n",cmdval);

                        if (pstate)
                                pstate->modemStatusRegister = cmdval;

                        if (info && info->tty && (info->flags & ASYNC_INITIALIZED)) {
                                if ((cmdval & MSR_CD_ON) && (!info->cd_status)) {
                                        info->cd_status = 1;
                                        wake_up_interruptible(&info->open_wait);
                                } else if (!(cmdval & MSR_CD_ON) && (info->cd_status)) {
                                        info->cd_status = 0;
                                        if (!C_CLOCAL(info->tty))
                                                tty_hangup(info->tty);
                                }
                        }
                        frcnt = 3;
                        break;
                case RK_ESR_SET:
                        if (len < 3) {
                                if (si->use_tcp)
                                        tcp_store_fragment(si, buf, len);
                                return;
                        }
                        cmdval = buf[1];
                        cmdval += buf[2] << 8;
                        /* printk(KERN_INFO "nslink: RK_ESR_SET: 0x%x\n",cmdval); */
                        if (pstate) {
                                pstate->errorStatusRegister = cmdval;

                                if (pstate->errorStatusRegister & ESR_FRAME_ERROR)
                                        info->framingErrors++;
                                if (pstate->errorStatusRegister & ESR_PARITY_ERROR)
                                        info->parityErrors++;
                                if (pstate->errorStatusRegister & ESR_OVERFLOW_ERROR)
                                        info->overflowErrors++;

                        }
                        frcnt = 3;
                        break;
                case RK_ACTION_ACK:
                        if (len < 3) {
                                if (si->use_tcp)
                                        tcp_store_fragment(si, buf, len);
                                return;
                        }
                        cmdval = buf[1];
                        cmdval += buf[2] << 8;
                        /* printk(KERN_INFO "nslink: RK_ACTION_ACK: 0x%x\n",cmdval); */
                        if (pstate)
                                pstate->actionResponse = cmdval;
                        frcnt = 3;
                        break;
                case RK_QIN_STATUS:
                        if (len < 3) {
                                if (si->use_tcp)
                                        tcp_store_fragment(si, buf, len);
                                return;
                        }
                        cmdval = buf[1];
                        cmdval += buf[2] << 8;
                        /* printk(KERN_INFO "nslink: RK_QIN_STATUS: 0x%x\n",cmdval); */
                        if (pstate) {
                                pstate->nGetRemote = cmdval;
                                TraceQueue(info,4,cmdval);
                                if (info && info->tty &&
                                    (info->flags & ASYNC_INITIALIZED) &&
                                    info->xmit_cnt &&
                                    get_tx_space_remote(pstate))
                                        si->need_to_send = 1;
                        }
                        frcnt = 3;
                        break;
                case RK_DATA_BLK:
                        if (len < 3) {
                                if (si->use_tcp)
                                        tcp_store_fragment(si, buf, len);
                                return;
                        }
                        cmdval = buf[1];
                        cmdval += buf[2] << 8;
                        if (len < (cmdval + 3)) {
                                if (si->use_tcp)
                                        tcp_store_fragment(si, buf, len);
                                return;
                        }
                        if (info &&
                            info->tty &&
                            !(info->flags & ASYNC_CLOSING) &&
                            (info->flags & ASYNC_INITIALIZED)) {
                                info->rxBytes += cmdval;
                                if (info->recv_cnt)
                                        push_chars_to_recv_buf(info, &buf[3], cmdval);
                                else {
                                        int cnt = tty_insert_flip_string(info->tty, &buf[3],cmdval);
                                        if (cnt < cmdval)
                                                push_chars_to_recv_buf(info, &buf[3+cnt], cmdval-cnt);
                                        if (cnt)
                                                tty_flip_buffer_push(info->tty);
                                        spin_lock_irqsave(&pstate->nGetLocalLock, flags);
                                        pstate->nGetLocal += cnt;
                                        spin_unlock_irqrestore(&pstate->nGetLocalLock, flags);

                                        pstate->update_queue = 1;
                                        si->need_to_send = 1;
                                }
                        } else {
                                if (pstate) {
                                        spin_lock_irqsave(&pstate->nGetLocalLock, flags);
                                        pstate->nGetLocal += cmdval;
                                        spin_unlock_irqrestore(&pstate->nGetLocalLock, flags);

                                        pstate->update_queue = 1;
                                        si->need_to_send = 1;
                                }
                        }
                        frcnt = 3 + cmdval;
                        break;
                case RK_ASSIGN_REPLY:
                        if (si->state == SI_STATE_CONNECTING_WAIT ||
                            si->state == SI_STATE_CONNECTING) {
                                printk(KERN_INFO "nslink: Connected -- %d.\n", si->base_port);
                                si->update_timeout = 1;
                                si->need_to_send = 1;
#ifdef NSLINK_DEBUG_KEEPALIVE
                                printk(KERN_INFO "nslink: init link timer %d: %ld jiffies\n",si->conc_id,si->linkTimeoutJiffies/2);
#endif
                                if (si->linkTimeoutJiffies) {
                                        si->keep_alive_tx_timer = jiffies + si->linkTimeoutJiffies/2;
                                        si->keep_alive_timeout  = jiffies + si->linkTimeoutJiffies;
                                        if (!si->use_tcp)
                                                si_set_timer(si, si->linkTimeoutJiffies/2);
                                }
                                clr_otherclose_all_tty(si);
                                port_resync_all(si);
                                si->state = SI_STATE_CONNECTED;
                        } else
                                printk(KERN_INFO "nslink: Unexpected RPSH-Si connection assign reply!\n");
                        frcnt = 1;
                        break;
                case RK_IC_TIMEOUT:
                        //printk(KERN_INFO "nslink: RK_IC_TIMEOUT received, setting MSR_ICTO_OCCURRED \n");
                        info->si->port_state[info->portnum].modemStatusRegister |= MSR_ICTO_OCCURRED;
                        frcnt = 1;
                        break;
                case RK_REMOTE_STATUS:
                        if (len < 2) {
                                if (si->use_tcp)
                                        tcp_store_fragment(si, buf, len);
                                return;
                        }
                        if (buf[1] == REM_PORT_ASSIGNED) {
                                /* assign ok */
                                info->remote_status = REM_PORT_ASSIGNED;
                                cmdval = buf[2];
                                cmdval += buf[3] << 8;

                                //	      printk(KERN_INFO "nslink: RK_REMOTE_STATUS: 0x%x\n",cmdval);

                                if (pstate)
                                        pstate->modemStatusRegister = cmdval;
                                if (info && info->tty && (info->flags & ASYNC_INITIALIZED)) {
                                        if ((cmdval & MSR_CD_ON) && (!info->cd_status)) {
                                                wake_up_interruptible(&info->open_wait);
                                                info->cd_status = 1;
                                        } else if (!(cmdval & MSR_CD_ON) && (info->cd_status)) {
                                                if (!C_CLOCAL(info->tty))
                                                        tty_hangup(info->tty);
                                                info->cd_status = 0;
                                        }
                                }
                                frcnt = 4;
                        } else {
                                /* port not available */
                                //printk(KERN_INFO "nslink: port %d unavailable\n",portnum);
                                //printk(KERN_INFO "nslink: info = %p\n",info);
                                //printk(KERN_INFO "nslink: pstate = %p\n",pstate);
                                if (info) {
                                        int old_status = info->remote_status;
                                        info->conn_refused = 1;

                                        if (buf[1] == 0x11)
                                                info->remote_status = REM_PORT_BUSY;
                                        else {
                                                info->remote_status = REM_PORT_AVAIL;
                                                info->force_release_acked = 1;
                                                wake_up_interruptible(&info->release_wait);
                                        }

                                        if (old_status != REM_PORT_BUSY && old_status != REM_PORT_AVAIL) {
                                                if (info->tty) {
                                                        set_bit(TTY_OTHER_CLOSED, &info->tty->flags);
                                                        wake_tty(info->tty);
                                                }
                                                port_resync(info, pstate);
                                        }
                                }
                                frcnt = 2;
                        }
                        break;
                default:
                        printk(KERN_INFO "nslink: ParseInputPkt invalid cmd 0x%x, len %ld\n", *buf, len);
                        frcnt = len;
                        break;
                }
                len -= frcnt;
                buf += frcnt;
        }

        return;
}


static void reinit_hub_state(struct si_state *si)
{
        si->state = 0;
        si->need_to_send = 0;
        si->send_connect_time = 0;

        si->sender_ack_timer = 0;
        si->receiver_ack_timer = 0;
        si->rexmit_timer = 0;
        si->keep_alive_tx_timer = 0;
        si->keep_alive_timeout = 0;
        si->need_to_send_connect_check = 0;
        si->update_timeout = 0;
        si->retransCount = 0;
        si->rxBadOrderCount = 0;

        si->tcp_timer = 0;
        si->tcp_frag_len = 0;
        si->tcp_sock = NULL;
        si->tcp_sk = NULL;

        si->tcp_last_port = 0;

        si->taskPtr = NULL;

        if (si->ip_addr)
                si->use_tcp = 1;
        else
                si->use_tcp = 0;

        sema_init(&si->write_sem, 1);
        hdlc_resync(si);
}


static int init_hub_state(struct si_box_config *in, int from_user)
{
        struct si_state *si;
        struct NET_DEVICE *dev;
        int i;
        unsigned int size;
        struct si_box_config *cfg, tmp;


        if (from_user) {
                if (copy_from_user(&tmp, (void *) in, sizeof(struct si_box_config)))
                        return -EFAULT;
                cfg = &tmp;
        } else
                cfg = in;

        if (cfg->boxid >= numBoxes)
                return -EINVAL;

        if (conc_state[cfg->boxid]) {
                // reinitialize box that has already been initialized once
                si = conc_state[cfg->boxid];
                reinit_hub_state(si);
                // don't set timer if TCP/IP
                if (si->dev)
                        si_set_timer(si, HZ);
                return 0;
        }

        if (cfg->ip_addr)
                dev = NULL;
        else {
                dev = DevGetByName(cfg->interface);
                if (!dev) {
                        printk(KERN_INFO "nslink: can't find network device '%s'\n",cfg->interface);
                        return -ENOENT;
                }
        }

        if (cfg->num_ports == 0)
                return 0;

        si = kmalloc(sizeof(struct si_state), GFP_KERNEL);
        if (!si)
                return -ENOMEM;
        memset(si, 0, sizeof(struct si_state));

        init_timer(&si->timer);
        memcpy(si->ether_addr, cfg->ether_addr, 6);
        si->dev = dev;
        skb_queue_head_init(&si->rtq);
        si->conc_id = cfg->boxid;
        si->base_port = cfg->base_port;
        si->num_ports = cfg->num_ports;
        memcpy(si->rsmode, cfg->rsmode, MAX_NRP_PORTS);
        si->link_timeout = cfg->link_timeout;
        si->scan_period = cfg->scan_period;
        si->rexmit_timer = cfg->rexmit_timer;
        si->arch_id = cfg->arch_id;
        si->linkTimeoutJiffies = si->link_timeout * HZ;
        si->modelID = cfg->ModelId;
        sema_init(&si->write_sem, 1);

        hdlc_retrans_time = cfg->rexmit_timer;
        hdlc_ack_time = (cfg->rexmit_timer/2);

#ifdef NSLINK_DEBUG_KEEPALIVE
        printk(KERN_INFO "nslink: linkTimeout for hub %d = %d seconds\n",si->conc_id,si->link_timeout);
#endif

        si->ip_addr = cfg->ip_addr;
        if (si->ip_addr)
                si->use_tcp = 1;
        else
                si->use_tcp = 0;
        si->timer.data = (unsigned long) si;
        si->timer.function = si_timer;
        atomic_set(&si->timer_active, 1);

        size = cfg->num_ports * sizeof(struct si_port_state);
        si->port_state = kmalloc(size, GFP_KERNEL);
        if (!si->port_state) {
                kfree(si);
                return -ENOMEM;
        }
        memset(si->port_state, 0, size);

        init_waitqueue_head(&si->waitqueue);

        for (i = 0; i < cfg->num_ports; i++)
                init_nr_port(si, i, cfg->inter_char_timeout[i], cfg->low_latency[i], cfg->rx_fifo_disable[i]);

        conc_state[cfg->boxid] = si;

        if (si->dev) {
                /* do not set timer yet if there is not device, i.e., IP addressing. */
                si_set_timer(si, HZ);
        }

        return 0;
}

/*
 *  This function initializes the nr_port structure for one serial port.
 *  Inputs:  si-> ptr to si struct for the hub, portnum (0-31) relative port number
 *           (not the ttySIx number)
 */
static void init_nr_port(struct si_state *si, int portnum, int ic_timeout, int low_latency, int rx_fifo_disable)
{
        struct nr_port *info;
        struct si_port_state *pstate;
        int line;

        line = si->base_port + portnum;
        pstate = si->port_state + portnum;

        info = kmalloc(sizeof(struct nr_port), GFP_KERNEL);
        if (!info) {
                printk(KERN_INFO "nslink: Couldn't allocate info struct for line #%d\n", line);
                return;
        }
        memset(info, 0, sizeof(struct nr_port));

        info->magic = RPORT_MAGIC;
        info->line = line;
        info->si = si;
        info->portnum = portnum;
        info->closing_wait = 30000;  //ms
        info->close_delay = 500;     //ms
        info->baud_rate = 9600;
        info->baud_base = 0;
        // don't set low latency flag on MAC-mode ports, or it causes
        // problems when tty_flip_buffer_push() is called
        if (low_latency && si->use_tcp)
                info->flags |= ASYNC_LOW_LATENCY;
        else
                info->flags &= ~ASYNC_LOW_LATENCY;
        info->rx_fifo_disable = rx_fifo_disable;
        info->inter_char_timeout = ic_timeout;
        info->normal_termios = nslink_driver->init_termios;
        init_waitqueue_head(&info->open_wait);
        init_waitqueue_head(&info->close_wait);
        init_waitqueue_head(&info->release_wait);

        spin_lock_init(&info->slock);
        spin_lock_init(&pstate->nGetLocalLock);
        sema_init(&info->write_sem, 1);
        nrp_table[line] = info;

#if LINUX_VERSION_CODE > VERSION_CODE(2,5,0)
        tty_register_device(nslink_driver, line, NULL);
#endif

}


/********************************************************************************/
/*                                                                              */
/*        TTY FUNCTIONS                                                         */
/*                                                                              */
/********************************************************************************/

/*
 * Utility function.
 */
#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

/*
 * If there is characters in the nr_port recv_buffer, try to send it out to
 * the tty line discpline.
 */
static void send_recv_buf_to_ldisc(struct nr_port *info)
{
        struct tty_struct *tty;
        int c, cnt, total_sent = 0;
        struct si_port_state *pstate;
        unsigned long flags;

        if (!info || !info->tty || !info->recv_cnt || !info->recv_buf)
                return;

        tty = info->tty;

        //  Copy the data into tty bufs, spinlock to protect recv_tail and recv_cnt
        spin_lock_irqsave(&info->slock, flags);
        while (1) {
                c = info->recv_cnt;

                if (c > (RECV_BUF_SIZE - info->recv_tail))
                        c = RECV_BUF_SIZE - info->recv_tail;

                // done if recv_buf is empty
                if (c <= 0)
                        break;

                cnt = tty_insert_flip_string(info->tty, info->recv_buf + info->recv_tail, c);
                if (cnt)
                        tty_flip_buffer_push(tty);

                info->recv_tail += cnt;
                info->recv_tail &= RECV_BUF_SIZE - 1;
                info->recv_cnt -= cnt;
                total_sent += cnt;

                // done if tty layer is full
                if (cnt < c)
                        break;
        }
        spin_unlock_irqrestore(&info->slock, flags);


        if (total_sent) {
                //printk(KERN_INFO "nslink: copied %d chars from recv_buf to tty layer\n", total_sent);
                pstate = info->si->port_state + info->portnum;
                spin_lock_irqsave(&pstate->nGetLocalLock, flags);
                pstate->nGetLocal += total_sent;
                spin_unlock_irqrestore(&pstate->nGetLocalLock, flags);
                pstate->update_queue = 1;
                info->si->need_to_send = 1;
        }

        // If there are still more characters to send, try again 0.5 seconds later.
        if (info->recv_cnt)
                si_set_timer(info->si, HZ / 2);
}

/*
 *  Push characters to the nr_port recv_buffer.  Data is from a received
 *  ethernet packet (either MAC mode or TCP).
 */
static void push_chars_to_recv_buf(struct nr_port *info, unsigned char *cp, int len)
{
        struct tty_struct *tty;
        int c;
        unsigned long flags;

        if (!info || !info->tty || !info->recv_buf)
                return;

        tty = info->tty;

        //printk(KERN_INFO "nslink: pushing %d chars to recv_buf...", len);

        //  Copy the data into port buf, spinlock to protect recv_head and xmit_cnt
        spin_lock_irqsave(&info->slock, flags);
        while (1) {
                c = RECV_BUF_SIZE - info->recv_cnt - 1;
                if (c > len)
                        c = len;
                if (c > RECV_BUF_SIZE - info->recv_head)
                        c = RECV_BUF_SIZE - info->recv_head;
                if (c <= 0)
                        break;
                memcpy(info->recv_buf + info->recv_head, cp, c);
                info->recv_head = (info->recv_head + c) & (RECV_BUF_SIZE - 1);
                info->recv_cnt += c;
                cp += c;
                len -= c;
        }
        spin_unlock_irqrestore(&info->slock, flags);


        info->si->need_to_send = 1;
        if (info->si->use_tcp == 0)
                si_timer((unsigned long) info->si);
}

static inline int nslink_paranoia_check(struct nr_port *info, const char *routine)
{
        if (!info) {
                printk(KERN_INFO "nslink: %d nslink_paranoia_check: info=NULL in %s\n",current->pid, routine);
                return 1;
        }
        if (info->magic != RPORT_MAGIC) {
                printk(KERN_INFO "nslink: %d nslink_paranoia_check: bad info->magic in %s\n", current->pid, routine);
                return 1;
        }
        return 0;
}

/*
 * This routine configures an nslink port according to its termio settings.
 */
static void configure_nr_port(struct nr_port *info)
{
        unsigned cflag;
        int i, bits, baud, control_settings;

        i = bits = baud = control_settings = 0;

        if (!info->tty || !info->tty->termios)
                return;
        cflag = info->tty->termios->c_cflag;

        /* Byte size and parity */
        if ((cflag & CSIZE) == CS8) {
                control_settings |= SC_DATABITS_8;
                bits = 10;
        } else {
                control_settings |= SC_DATABITS_7;
                bits = 9;
        }
        if (cflag & CSTOPB) {
                control_settings |= SC_STOPBITS_2;
                bits++;
        } else
                control_settings |= SC_STOPBITS_1;

        if (cflag & PARENB) {
                bits++;
                if (cflag & PARODD)
                        control_settings |= SC_PARITY_ODD;
                else
                        control_settings |= SC_PARITY_EVEN;
        }

        if (cflag & CRTSCTS)
                control_settings |= SC_FLOW_CTS_AUTO+SC_FLOW_RTS_AUTO;
        if ((info->si->rsmode[info->portnum] == SI_RSMODE_485) ||
            (info->si->rsmode[info->portnum] == SI_RSMODE_485FDS))
                control_settings |= SC_FLOW_RTS_ARS485;

        if (I_IXON(info->tty))
                control_settings |= SC_FLOW_XON_TX_AUTO;

        if (I_IXOFF(info->tty))
                control_settings |= SC_FLOW_XON_RX_AUTO;

        if (I_IXANY(info->tty))
                control_settings |= SC_FLOW_IXANY;

        if (control_settings != info->control_settings) {
                info->control_settings = control_settings;
                info->update_control_settings = 1;
        }

#if (LINUX_VERSION_CODE < VERSION_CODE(2,5,0))
        // old baud rate detection method --> DONT USE IT!
        i = cflag & CBAUD;
        if (i & CBAUDEX) {
                i &= ~CBAUDEX;
                if (i < 1 || i > 4)
                        info->tty->termios->c_cflag &= ~CBAUDEX;
                else
                        i += 15;
        }
        if (i == 15) {
                if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_HI)
                        i += 1;
                if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_VHI)
                        i += 2;
                if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_SHI)
                        i += 3;
                if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_WARP)
                        i += 4;
        }
        baud = baud_table[i] ? baud_table[i] : 9600;
#else
        baud = tty_get_baud_rate(info->tty);
#endif
        //printk(KERN_INFO "nslink: baud=%d  info->baud=%d\n", baud, info->baud_rate);
        //printk(KERN_INFO "nslink: isDM500 = %d, Model = %d\n", isDM500(info->si->modelID), info->si->modelID);

        //WindowsOS dm500 hack workaround
        if ((baud == 300) && isDM500(info->si->modelID)) {
                baud = 500000;
                printk(KERN_INFO "nslink: 300 Baud remapped to 500K\n");
        }

        if ((baud == 38400) && ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_CUST)) {
                if (info->custom_divisor)
                        baud = info->baud_base / info->custom_divisor;
                else
                        baud = info->baud_base;
        }

        info->cps = baud / bits;

        if (baud != info->baud_rate) {
                info->baud_rate = baud;
                info->update_baud_rate = 1;
        }

        if (info->rx_xon != START_CHAR(info->tty) ||
            info->rx_xoff != STOP_CHAR(info->tty) ||
            info->tx_xon != START_CHAR(info->tty) ||
            info->tx_xoff != STOP_CHAR(info->tty)) {
                info->rx_xon = info->tx_xon = START_CHAR(info->tty);
                info->rx_xoff = info->tx_xoff = STOP_CHAR(info->tty);
                info->update_special_charset = 1;
        }

        if (info->update_baud_rate || info->update_control_settings) {
                info->si->need_to_send = 1;
                if (info->si->use_tcp == 0)
                        si_timer((unsigned long) info->si);
        }
}

static int block_til_ready(struct tty_struct *tty,
                           struct file *filp,
                           struct nr_port *info)
{
        wait_queue_t wait;
        int retval;
        int do_clocal = 0, extra_count = 0;
        unsigned long flags;

        init_waitqueue_entry(&wait,current);

        /*
         * If the device is in the middle of being closed, then block
         * until it's done, and then try again.
         */
        if (tty_hung_up_p(filp))
                return ((info->flags & ASYNC_HUP_NOTIFY) ? -EAGAIN : -ERESTARTSYS);

        // If we are in the process of closing, wait for it to complete, then return
        if (info->flags & ASYNC_CLOSING) {
                wait_event_interruptible(info->close_wait, !(info->flags&ASYNC_CLOSING));
                return ((info->flags & ASYNC_HUP_NOTIFY) ? -EAGAIN : -ERESTARTSYS);
        }

        /*
         * If non-blocking mode is set, or the port is not enabled,
         * then make the check up front and then exit.
         */
        if ((filp->f_flags & O_NONBLOCK) || (tty->flags & (1 << TTY_IO_ERROR))) {
                info->flags |= ASYNC_NORMAL_ACTIVE;
                return 0;
        }

        if (tty->termios->c_cflag & CLOCAL)
                do_clocal = 1;

        /*
         * Block waiting for the carrier detect and the line to become
         * free.  While we are in this loop, info->count is dropped by one, so that
         * nrp_close() knows when to free things.  We restore it upon
         * exit, either normal or abnormal.
         */
        retval = 0;
        add_wait_queue(&info->open_wait, &wait);
        DebugOpen("%d block_til_ready ttySI%d before block count = %d\n", current->pid, info->line, info->count);


        //  Use spinlock to protect info->count
        spin_lock_irqsave(&info->slock, flags);
        if (!tty_hung_up_p(filp)) {
                extra_count = 1;
                info->count--;
        }
        spin_unlock_irqrestore(&info->slock, flags);

        info->blocked_open++;

        while (1) {
                if (tty->termios->c_cflag & CBAUD) {
                        if ((info->si->rsmode[info->portnum] != SI_RSMODE_485) &&
                            (info->si->rsmode[info->portnum] != SI_RSMODE_485FDS) &&
                            !(info->modem_control_register & MCR_DTR_SET_MASK)) {
                                info->modem_control_register |= MCR_DTR_SET_ON | MCR_RTS_SET_ON;
                                info->update_mcr = 1;
                                info->si->need_to_send = 1;
                                if (info->si->use_tcp == 0)
                                        si_timer((unsigned long) info->si);
                        }
                }
                set_current_state(TASK_INTERRUPTIBLE);
                if (tty_hung_up_p(filp) || !(info->flags & ASYNC_INITIALIZED)) {
                        if (info->flags & ASYNC_HUP_NOTIFY)
                                retval = -EAGAIN;
                        else
                                retval = -ERESTARTSYS;
                        break;
                }

                if ((!(info->flags & ASYNC_CLOSING)) && (do_clocal || info->cd_status))
                        break;

                if (signal_pending(current)) {
                        retval = -ERESTARTSYS;
                        break;
                }

                DebugOpen("%d block_til_ready ttySI%d blocking with count=%d, flags=0x%0x\n", current->pid, info->line, info->count, info->flags);
                schedule();
        }
        set_current_state(TASK_RUNNING);
        remove_wait_queue(&info->open_wait, &wait);

        //  Use spinlock to protect info->count
        spin_lock_irqsave(&info->slock, flags);
        if (extra_count)
                info->count++;
        spin_unlock_irqrestore(&info->slock, flags);

        info->blocked_open--;
        DebugOpen("%d block_til_ready ttySI%d after block count=%d\n", current->pid, info->line, info->count);

        if (retval)
                return retval;

        info->flags |= ASYNC_NORMAL_ACTIVE;
        return 0;
}

/*
 *  TTY port open handler
 */
static int nrp_open(struct tty_struct *tty, struct file *filp)
{
        struct si_port_state *pstate;
        struct nr_port *info;
        int line, timeout;
        unsigned long page, page2, jifval;
        unsigned long flags;
        int count, retval;

        // 'line' is the port number 'xx' /dev/ttySIxx
        line = TTY_GET_LINE(tty);

        DebugOpen("%d nrp_open ttySI%d tty=%p \n", current->pid, line, tty);

        if ((line < 0) || (line >= MAX_NRP_TOTAL_PORTS))
                return -ENODEV;

        // 'info' is ptr to global port config/info data
        info = nrp_table[line];

        //Increment port use count and module port open count

        DebugOpen("%d nrp_open ttySI%d info=%p \n", current->pid, line, info);

        if (info) {
                tty->driver_data = info;
                spin_lock_irqsave(&info->slock, flags);
                if (tty_hung_up_p(filp)) {
                        count = info->count;
                        spin_unlock_irqrestore(&info->slock, flags);
                } else {
                        ++info->count;
                        count = info->count;
                        spin_unlock_irqrestore(&info->slock, flags);
                        if (count == 1) {
#if ((LINUX_VERSION_CODE < VERSION_CODE(2,3,0)) && defined(MODULE))
                                MOD_INC_USE_COUNT;
#endif
                                atomic_inc(&nrp_num_ports_open);
                                DebugOpen("%d nrp_open ttySI%d nslink new ports_open = %d\n", current->pid, line, atomic_read(&nrp_num_ports_open));
                        }
                }

                DebugOpen("%d nrp_open ttySI%d count is now %d\n",current->pid,line,count);
        }

        if (!info || !info->si || info->si->rsmode[info->portnum] == SI_RSMODE_OFF) {
                if (info) {
                        DebugOpen("info->si=%p ",info->si);
                        if (info->si)
                                DebugOpen("rsmode=%d ",info->si->rsmode[info->portnum]);
                }
                DebugOpen("returning -ENODEV\n");
                return -ENODEV;
        }

        if (count > 1 && info->remote_status == REM_PORT_ASSIGNED) {
                // port already open, so we're done
                DebugOpen("%d nrp_open ttySI%d already open: returning 0\n", current->pid, line);
                return 0;
        }

        page = __get_free_page(GFP_KERNEL);
        if (!page)
                return -ENOMEM;

        page2 = __get_free_page(GFP_KERNEL);
        if (!page2) {
                free_page(page);
                return -ENOMEM;
        }

        info->remote_status = REM_OPEN_PORT;
        info->si->need_to_send = 1;
        if (info->si->use_tcp == 0)
                si_timer((unsigned long) info->si);

        //  Wait a sec (less time if signalled)
        jifval = jiffies + (HZ);

        while (!IsPast(jifval) && (info->remote_status & (REM_OPEN_PORT | REM_WAITING_OPEN))) {
                set_current_state(TASK_INTERRUPTIBLE);
                schedule_timeout((HZ / 10));
                if (signal_pending(current))
                        break;
        }

        if (info->remote_status != REM_PORT_ASSIGNED) {
                free_page(page2);
                free_page(page);
                DebugOpen("%d nrp_open ttySI%d remote_status=%d -- returning EBUSY\n", current->pid, line, info->remote_status);
                return -EBUSY;
        }


        // If we are in the process of closing, wait for it to complete, then return
        if (tty_hung_up_p(filp) || info->flags & ASYNC_CLOSING) {
                DebugOpen("%d nrp_open ttySI%d is closing, waiting for close\n", current->pid, line);
                wait_event_interruptible(info->close_wait, !(info->flags & ASYNC_CLOSING));
                free_page(page2);
                free_page(page);
                DebugOpen("%d nrp_open ttySI%d was closing, returning %s\n", current->pid, line,(info->flags & ASYNC_HUP_NOTIFY) ? "EAGAIN" : "ERESTARTSYS");
                return (info->flags & ASYNC_HUP_NOTIFY) ? -EAGAIN : -ERESTARTSYS;
        }

        /*
         * We must not sleep from here until the port is marked fully
         * in use.
         */
        if (info->xmit_buf)
                free_page(page);
        else
                info->xmit_buf = (unsigned char *) page;

        if (info->recv_buf)
                free_page(page2);
        else {
                info->recv_buf = (unsigned char *) page2;
                info->recv_cnt = 0;
                info->recv_head = 0;
                info->recv_tail = 0;
        }

        info->tty = tty;

        // If we are in the process of closing, wait for it to complete, then return
        if (info->flags & ASYNC_CLOSING) {
                DebugOpen("%d nrp_open ttySI%d is closing, waiting for close\n", current->pid, line);
                wait_event_interruptible(info->close_wait, !(info->flags&ASYNC_CLOSING));
                DebugOpen("%d nrp_open ttySI%d was closing, returning %s\n", current->pid, line,(info->flags & ASYNC_HUP_NOTIFY) ? "EAGAIN" : "ERESTARTSYS");
                return ((info->flags & ASYNC_HUP_NOTIFY) ? -EAGAIN : -ERESTARTSYS);
        }


        pstate = info->si->port_state + info->portnum;
        if (pstate->modemStatusRegister & MSR_CD_ON)
                info->cd_status = 1;
        else
                info->cd_status = 0;

        /*
         * XXX Shouldn't be needed, but embedded box sometimes drops a
         * transmitted data subpacket request (while still ack'ing the
         * packet at the HDLC level!)  We resync on an initial open to
         * make sure things are sane...
         */
        timeout = get_tx_cnt_remote(pstate);
        if (timeout) {
                timeout = (get_tx_cnt_remote(pstate) + 1) * HZ / 1200;
                if (timeout < HZ)
                        timeout = HZ;
                nrp_wait_until_sent(tty, timeout);
        }

        TraceQueue(info,3,0);

        pstate->actionResponse = 0;
        info->action_register = ACT_FLUSH_INPUT | ACT_FLUSH_OUTPUT;
        if (info->rx_fifo_disable)
                info->action_register |= ACT_RXFIFO_DISABLE;
        info->update_action = 1;

        if (info->si->arch_id == 3 || info->si->arch_id == 2)
                info->update_ic_timeout = 1;

        /*
         * Set up the tty->alt_speed kludge
         */
        if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_HI)
                info->tty->alt_speed = 57600;
        if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_VHI)
                info->tty->alt_speed = 115200;
        if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_SHI)
                info->tty->alt_speed = 230400;
        if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_WARP)
                info->tty->alt_speed = 460800;

        configure_nr_port(info);

        if ((info->si->rsmode[info->portnum] != SI_RSMODE_485) &&
            (info->si->rsmode[info->portnum] != SI_RSMODE_485FDS)) {
                if (tty->termios->c_cflag & CBAUD)
                        info->modem_control_register |= MCR_DTR_SET_ON;
                info->modem_control_register |= MCR_RTS_SET_ON;
                info->update_mcr = 1;
        }

        info->si->need_to_send = 1;

        if (info->si->use_tcp == 0)
                si_timer((unsigned long) info->si);

        // block until action response is received

        //  Wait a sec (less time if signalled)
        jifval = jiffies + (HZ);
        while (!IsPast(jifval) && (pstate->actionResponse == 0)) {
                set_current_state(TASK_INTERRUPTIBLE);
                schedule_timeout((HZ / 10));
                if (signal_pending(current))
                        break;
        }

        info->flags |= ASYNC_INITIALIZED;

        DebugOpen("%d nrp_open ttySI%d calling block_til_ready\n", current->pid, line);

        retval = block_til_ready(tty, filp, info);
        if (retval) {
                DebugOpen("%d nrp_open ttySI%d returning after block_til_ready with %d\n", current->pid, line, retval);
                return retval;
        }

        if ((count == 1) && (info->flags & ASYNC_SPLIT_TERMIOS)) {
                *tty->termios = info->normal_termios;
                configure_nr_port(info);
        }

        tty->low_latency = (info->flags & ASYNC_LOW_LATENCY) != 0;

        DebugOpen("%d nrp_open ttySI%d success: returning 0\n", current->pid, line);

        return 0;
}

/*
 *  TTY port close handler
 */
static void nrp_close(struct tty_struct *tty, struct file *filp)
{
        struct si_port_state *pstate;
        struct nr_port *info = (struct nr_port *) tty->driver_data;
        unsigned long flags, jifval;
        int timeout;

        if (nslink_paranoia_check(info, "nrp_close"))
                return;

        if (!info)
                return;

        if (tty_hung_up_p(filp))
                return;

        DebugOpen("%d nrp_close ttySI%d count=%d\n", current->pid, info->line, info->count);

        //  Use spinlock to protect info->count and info->flags
        spin_lock_irqsave(&info->slock, flags);
        if (tty_hung_up_p(filp)) {
                spin_unlock_irqrestore(&info->slock, flags);
                return;
        }

        if (--info->count < 0) {
                printk(KERN_WARNING "nslink %d nrp_close ttySI%d count = %d\n", current->pid, info->line, info->count);
                info->count = 0;
        }

        if (info->count) {
                spin_unlock_irqrestore(&info->slock, flags);
                DebugOpen("%d nrp_close ttySI%d count is now %d -- returning\n", current->pid, info->line, info->count);
                return;
        }

        DebugOpen("%d nrp_close ttySI%d setting ASYNC_CLOSING\n", current->pid, info->line);
        tty->closing = 1;
        info->flags |= ASYNC_CLOSING;
        spin_unlock_irqrestore(&info->slock, flags);

        info->normal_termios = *tty->termios;


        // If transmission was throttled by the application request, just
        // flush the xmit buffer.
        if (tty->flow_stopped)
                nrp_flush_buffer(tty);

        // Wait for the transmit buffer to clear
        if (info->closing_wait != ASYNC_CLOSING_WAIT_NONE) {
                DebugOpen("%d nrp_close ttySI%d calling tty_wait_until_sent(%d)\n", current->pid, info->line, info->closing_wait);
                tty_wait_until_sent(tty, msecs_to_jiffies(info->closing_wait));
                DebugOpen("%d nrp_close ttySI%d tty_wait_until_sent() returned\n", current->pid, info->line);
        }

        // Before we drop DTR, make sure the UART transmitter has completely
        // drained; this is especially important if there is a transmit
        // FIFO!
        pstate = info->si->port_state + info->portnum;
        if (info->cps == 0)
                timeout = 1;
        else {
                timeout = (get_tx_cnt_remote(pstate) + 1) * HZ / info->cps;
                if (timeout == 0)
                        timeout = 1;
        }

        if (info->closing_wait != ASYNC_CLOSING_WAIT_NONE) {
                DebugOpen("%d nrp_close ttySI%d calling nrp_wait_until_sent(%d)\n", current->pid, info->line, timeout);
                nrp_wait_until_sent(tty, timeout);
                DebugOpen("%d nrp_close ttySI%d nrp_wait_until_sent() returned\n", current->pid, info->line);
        }

        info->action_register = ACT_FLUSH_INPUT | ACT_FLUSH_OUTPUT;
        info->update_action = 1;

        info->modem_control_register &= ~MCR_RTS_SET_MASK;
        if (C_HUPCL(tty) && info->modem_control_register & MCR_DTR_SET_MASK)
                info->modem_control_register &= ~MCR_DTR_SET_MASK;
        info->update_mcr = 1;

        info->si->need_to_send = 1;

        DebugOpen("%d nrp_close ttySI%d sending final commands\n", current->pid, info->line);
        if (info->si->use_tcp == 0)
                si_timer((unsigned long) info->si);
        else
                tcp_send_remote_cmds(info->si);

        DebugOpen("%d nrp_close ttySI%d flushing tty buffer\n",  current->pid,info->line);

        if (TTY_DRIVER_FLUSH_BUFFER_EXISTS(tty))
                TTY_DRIVER_FLUSH_BUFFER(tty);

        DebugOpen("%d nrp_close ttySI%d flushing ldisc buffer\n", current->pid, info->line);

#if LINUX_VERSION_CODE < VERSION_CODE(2,6,26)
        if (tty->ldisc.flush_buffer)
                tty->ldisc.flush_buffer(tty);
#else
        if (TTY_LDISC_FLUSH_BUFFER_EXISTS(tty))
                TTY_LDISC_FLUSH_BUFFER(tty);
#endif			//kernel <2.6.26

        if (info->blocked_open) {
                DebugOpen("%d nrp_close ttySI%d blocked_open\n", current->pid, info->line);
                if (info->close_delay)
                        msleep_interruptible(info->close_delay);
                wake_up_interruptible(&info->open_wait);
        } else {
                DebugOpen("%d nrp_close ttySI%d freeing buffers\n", current->pid, info->line);
                if (info->xmit_buf) {
                        free_page((unsigned long) info->xmit_buf);
                        info->xmit_buf = 0;
                }
                if (info->recv_buf) {
                        free_page((unsigned long) info->recv_buf);
                        info->recv_buf = 0;
                        info->recv_cnt = 0;
                }
        }
        spin_lock_irqsave(&info->slock, flags);
        info->flags &= ~(ASYNC_INITIALIZED | ASYNC_CLOSING | ASYNC_NORMAL_ACTIVE);
        tty->closing = 0;
        wake_up_interruptible(&info->close_wait);
        spin_unlock_irqrestore(&info->slock, flags);


        info->remote_status = REM_RELEASE_PORT;
        info->si->need_to_send = 1;
        if (info->si->use_tcp == 0)
                si_timer((unsigned long) info->si);

        //  Wait a sec (less time if signalled)
        DebugOpen("%d nrp_close ttySI%d, waiting 1 second\n", current->pid, info->line);
        jifval = jiffies + (HZ);
        while (!IsPast(jifval) && (info->remote_status & (REM_RELEASE_PORT | REM_WAITING_CLOSE))) {
                set_current_state(TASK_INTERRUPTIBLE);
                schedule_timeout((HZ / 10));
                if (signal_pending(current))
                        break;
        }

        info->tty = NULL;

#if ((LINUX_VERSION_CODE < VERSION_CODE(2,3,0)) && defined(MODULE))
        MOD_DEC_USE_COUNT;
#endif
        atomic_dec(&nrp_num_ports_open);
        DebugOpen("%d nrp_close ttySI%d ports_open now = %d\n", current->pid, info->line,atomic_read(&nrp_num_ports_open));
        DebugOpen("%d nrp_close ttySI%d complete shutdown\n", current->pid, info->line);
}

#ifdef KTERMIOS_NEW
static void nrp_set_termios(struct tty_struct *tty, struct ktermios *old_termios)
#else
static void nrp_set_termios(struct tty_struct *tty, struct termios *old_termios)
#endif
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;
        unsigned cflag;

        if (nslink_paranoia_check(info, "nrp_set_termios"))
                return;

        cflag = tty->termios->c_cflag;

        // This driver doesn't support CS5 or CS6
        if (((cflag & CSIZE) == CS5) || ((cflag & CSIZE) == CS6))
                tty->termios->c_cflag = ((cflag & ~CSIZE) |
                                         (old_termios->c_cflag & CSIZE));

        configure_nr_port(info);

        /* Handle transition to B0 status */
        if ((old_termios->c_cflag & CBAUD) && !(tty->termios->c_cflag & CBAUD)) {
                info->modem_control_register &= ~(MCR_RTS_SET_MASK | MCR_DTR_SET_MASK);
                info->update_mcr = 1;
        }

        /* Handle transition away from B0 status */
        if ((info->si->rsmode[info->portnum] != SI_RSMODE_485) &&
            (info->si->rsmode[info->portnum] != SI_RSMODE_485FDS) &&
            !(old_termios->c_cflag & CBAUD) && (tty->termios->c_cflag & CBAUD)) {
                info->modem_control_register |= MCR_RTS_SET_ON | MCR_DTR_SET_ON;
                info->update_mcr = 1;
        }

        if ((old_termios->c_cflag & CRTSCTS) && !(tty->termios->c_cflag & CRTSCTS)) {
                tty->hw_stopped = 0;
                nrp_start(tty);
        }
}

/*
 *   TTY send break handler
 */
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,27)
static void nrp_break(struct tty_struct *tty, int break_state)
#else		//implies kernel >= 6,2,27
static int nrp_break(struct tty_struct *tty, int break_state)
#endif		//kernel < 2,6,27
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;
        int orig;
        int ret = 0;

        if (nslink_paranoia_check(info, "nrp_break"))
                goto end;

        orig = info->modem_control_register;

        if (break_state == -1)
                info->modem_control_register |= MCR_BREAK_SET_ON;
        else
                info->modem_control_register &= ~MCR_BREAK_SET_ON;

        if (orig != info->modem_control_register) {
                info->update_mcr = 1;
                info->si->need_to_send = 1;
                if (info->si->use_tcp == 0)
                        si_timer((unsigned long) info->si);
        }
        ret++;	// increment so return is non-zero
end:
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,27)
        return;
#else			//implies kernel >= 2,6,27
        return ret;
#endif			//kernel < 2,6,27
}

/*
 *  Use bits 0x200, 0x400 and 0x800 as termios Comtrol flags, the are undefined in Linux
 */
#define TIOCM_TX_FLOWED_OFF  0x0200
#define TIOCM_SET_TXOFF      0x0400
#define TIOCM_CLEAR_TXOFF    0x0800
#define TIOCM_ICTO_OCCURRED  0x1000
#define TIOCM_CLEAR_ICTO     0x2000

static int get_modem_info(struct nr_port *info, unsigned int *value)
{
        unsigned int result;
        int mcr, msr;

        mcr = info->modem_control_register;
        msr = info->si->port_state[info->portnum].modemStatusRegister;

        result = ((mcr & MCR_RTS_SET_ON) ? TIOCM_RTS : 0)
                 | ((mcr & MCR_DTR_SET_ON) ? TIOCM_DTR : 0)
                 | ((msr & MSR_CD_ON) ? TIOCM_CAR : 0)
                 | ((msr & MSR_RING_ON) ? TIOCM_RNG : 0)
                 | ((msr & MSR_DSR_ON) ? TIOCM_DSR : 0)
                 | ((msr & MSR_CTS_ON) ? TIOCM_CTS : 0)
                 | ((msr & MSR_TX_FLOWED_OFF) ? TIOCM_TX_FLOWED_OFF : 0)
                 | ((msr & MSR_ICTO_OCCURRED) ? TIOCM_ICTO_OCCURRED : 0);

        if (copy_to_user(value, &result, sizeof(int)))
                return -EFAULT;
        return 0;
}

static int set_modem_info(struct nr_port *info, unsigned int cmd, unsigned int *value)
{
        unsigned int arg;
        int mcr;

        if (copy_from_user(&arg, value, sizeof(int)))
                return -EFAULT;

        mcr = info->modem_control_register;

        switch (cmd) {
        case TIOCMBIS:
                if ((info->si->rsmode[info->portnum] != SI_RSMODE_485) &&
                    (info->si->rsmode[info->portnum] != SI_RSMODE_485FDS)) {
                        if (arg & TIOCM_RTS) {
                                mcr |= MCR_RTS_SET_ON;
                        }
                        if (arg & TIOCM_DTR)
                                mcr |= MCR_DTR_SET_ON;
                }
                break;
        case TIOCMBIC:
                if (arg & TIOCM_RTS)
                        mcr &= ~MCR_RTS_SET_ON;
                if (arg & TIOCM_DTR)
                        mcr &= ~MCR_DTR_SET_ON;
                break;
        case TIOCMSET:
                if ((info->si->rsmode[info->portnum] != SI_RSMODE_485) &&
                    (info->si->rsmode[info->portnum] != SI_RSMODE_485FDS)) {
                        mcr = ((mcr & ~(MCR_RTS_SET_ON | MCR_DTR_SET_ON))
                               | ((arg & TIOCM_RTS) ? MCR_RTS_SET_ON : 0)
                               | ((arg & TIOCM_DTR) ? MCR_DTR_SET_ON : 0));

                        if (arg & TIOCM_SET_TXOFF) {
                                info->action_register = ACT_SET_TX_XOFF;
                                info->update_action = 1;
                        }

                        if (arg & TIOCM_CLEAR_TXOFF) {
                                info->action_register = ACT_CLEAR_TX_XOFF;
                                info->update_action = 1;
                        }

                        if (arg & TIOCM_CLEAR_ICTO) {
                                //	      printk(KERN_INFO "nslink: Clearing MSR_ICTO_OCCURRED \n");
                                info->si->port_state[info->portnum].modemStatusRegister &= ~MSR_ICTO_OCCURRED;
                        }
                }
                break;
        default:
                return -EINVAL;
        }

        if (mcr != info->modem_control_register) {
                info->modem_control_register = mcr;
                info->update_mcr = 1;
                info->si->need_to_send = 1;
                if (info->si->use_tcp == 0)
                        si_timer((unsigned long) info->si);
        }

        return 0;
}

#if LINUX_VERSION_CODE > VERSION_CODE(2,5,0)

/*
 *  Returns the state of the serial modem control lines.  These next 2 functions
 *  are the way kernel versions > 2.5 handle modem control lines rather than IOCTLs.
 */
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,39)
static int nrp_tiocmget(struct tty_struct *tty, struct file *file)
#else
static int nrp_tiocmget(struct tty_struct *tty)
#endif
{
        struct nr_port *info = (struct nr_port *)tty->driver_data;
        unsigned int result;
        int mcr, msr;

        mcr = info->modem_control_register;
        msr = info->si->port_state[info->portnum].modemStatusRegister;

        result = ((mcr & MCR_RTS_SET_ON) ? TIOCM_RTS : 0)
                 | ((mcr & MCR_DTR_SET_ON) ? TIOCM_DTR : 0)
                 | ((msr & MSR_CD_ON) ? TIOCM_CAR : 0)
                 | ((msr & MSR_RING_ON) ? TIOCM_RNG : 0)
                 | ((msr & MSR_DSR_ON) ? TIOCM_DSR : 0)
                 | ((msr & MSR_CTS_ON) ? TIOCM_CTS : 0);

        return result;
}

/*
 *  Sets the modem control lines
 */
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,39)
static int nrp_tiocmset(struct tty_struct *tty, struct file *file, unsigned int set, unsigned int clear)
#else
static int nrp_tiocmset(struct tty_struct *tty, unsigned int set, unsigned int clear)
#endif
{
        struct nr_port *info = (struct nr_port *)tty->driver_data;
        int mcr;

        mcr = info->modem_control_register;

        //  If the port is not in RS485 mode, set the modem control lines
        if ((info->si->rsmode[info->portnum] != SI_RSMODE_485) &&
            (info->si->rsmode[info->portnum] != SI_RSMODE_485FDS)) {
                if (set & TIOCM_RTS)
                        mcr |= MCR_RTS_SET_ON;
                if (set & TIOCM_DTR)
                        mcr |= MCR_DTR_SET_ON;
                if (clear & TIOCM_RTS)
                        mcr &= ~MCR_RTS_SET_ON;
                if (clear & TIOCM_DTR)
                        mcr &= ~MCR_DTR_SET_ON;
        }

        if (mcr != info->modem_control_register) {
                info->modem_control_register = mcr;
                info->update_mcr = 1;
                info->si->need_to_send = 1;
                if (info->si->use_tcp == 0)
                        si_timer((unsigned long) info->si);
        }

        return 0;
}

#endif /*  Linux > 2.5 */


static int get_serial_info(struct nr_port *info,
                           struct serial_struct * retinfo)
{
        struct serial_struct tmp;
        if (!retinfo)

                return -EFAULT;

        memset(&tmp, 0, sizeof(tmp));

        tmp.flags              = info->flags;
        // convert times from milliseconds to 1/100s
        tmp.close_delay        = info->close_delay / 10;
        tmp.closing_wait       = info->closing_wait == ASYNC_CLOSING_WAIT_NONE ? ASYNC_CLOSING_WAIT_NONE : info->closing_wait / 10;
        tmp.baud_base          = info->baud_base;
        tmp.custom_divisor     = info->custom_divisor;
        tmp.line               = info->portnum;
        tmp.irq                = info->inter_char_timeout;

        if (info->si->use_tcp)
                tmp.port = ntohl(info->si->ip_addr);
        else
                tmp.port = ((info->si->ether_addr[2] << 24)|
                            (info->si->ether_addr[3] << 16)|
                            (info->si->ether_addr[4] << 8)|
                            (info->si->ether_addr[5] << 0));

        if (copy_to_user(retinfo,&tmp,sizeof(*retinfo)))
                return -EFAULT;
        return 0;
}

static int set_serial_info(struct nr_port *info,
                           struct serial_struct * new_info)
{
        struct serial_struct new_serial;
        if (copy_from_user(&new_serial,new_info,sizeof(new_serial)))
                return -EFAULT;

        // convert times from 1/100 second to milliseconds;
        new_serial.close_delay *= 10;
        if (new_serial.closing_wait != ASYNC_CLOSING_WAIT_NONE)
                new_serial.closing_wait *= 10;

        // don't allow low_latency mode in MAC mode since it causes problems
        // when tty_flip_buffer_push() is called.
        if (!info->si->use_tcp)
                new_serial.flags &= ~ASYNC_LOW_LATENCY;

#ifdef CAP_SYS_ADMIN
        if (!capable(CAP_SYS_ADMIN))
#else
        if (!suser())
#endif
        {
                if (new_serial.close_delay != info->close_delay ||
                    (new_serial.flags & ASYNC_FLAGS & ~ASYNC_USR_MASK) != (info->flags & ASYNC_FLAGS & ~ASYNC_USR_MASK))
                        return -EPERM;
                info->flags = (info->flags & ~ASYNC_USR_MASK) | (new_serial.flags & ASYNC_USR_MASK);
        } else {
                info->flags = (info->flags & ~ASYNC_FLAGS) | (new_serial.flags & ASYNC_FLAGS);
                info->close_delay = new_serial.close_delay;
                info->closing_wait = new_serial.closing_wait;
        }

        info->baud_base = new_serial.baud_base;
        info->custom_divisor = new_serial.custom_divisor;

        if (info->tty)
                info->tty->low_latency =  (info->flags & ASYNC_LOW_LATENCY) != 0;

        info->inter_char_timeout = new_serial.irq;
        if (info->si->arch_id == 3 || info->si->arch_id ==2)
                info->update_ic_timeout = 1;

        return 0;
}

/*
 *  Handler for IOCTL calls made through the tty device
 */
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,39)
static int nrp_ioctl(struct tty_struct *tty, struct file *file, unsigned int cmd, unsigned long arg)
#else
static int nrp_ioctl(struct tty_struct *tty, unsigned int cmd, unsigned long arg)
#endif
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;

        if (nslink_paranoia_check(info, "nrp_ioctl"))
                return -ENODEV;

        if (info->si->state != SI_STATE_CONNECTED ||
            info->remote_status != REM_PORT_ASSIGNED) {
                msleep_interruptible(10);
                return -EIO;
        }

        switch (cmd) {
        case TIOCGSERIAL:
                return get_serial_info(info,(struct serial_struct*)arg);
        case TIOCSSERIAL:
                return set_serial_info(info,(struct serial_struct*)arg);
        case TIOCMGET:
                return get_modem_info(info, (unsigned int *) arg);
        case TIOCMBIS:
        case TIOCMBIC:
        case TIOCMSET:
                return set_modem_info(info, cmd, (unsigned int *) arg);
        case SI_GET_CTL_MAJOR:
                if (copy_to_user((void *) arg, &control_major, sizeof(int)))
                        return -EFAULT;
        case SI_SET_BOX_CONFIG:
                return init_hub_state((struct si_box_config *) arg, 1);

        case SI_GET_BOX_CONFIG:
                return get_box_config((struct si_box_config *) arg);

        case SI_GET_BOX_REPORT:
                return get_box_report((struct si_box_report *) arg);

        case SI_LOAD_CODE4_8:
        case SI_LOAD_CODE2:
        default:
                return -ENOIOCTLCMD;
        }
        return 0;
}

#if (defined(NSLINK_DEBUG_FLOW) || defined(NSLINK_DEBUG_THROTTLE))
static char *nrp_tty_name(struct tty_struct *tty, char *buf)
{
        if (tty)
                sprintf(buf, "%s%d", TTY_DRIVER_NAME(tty),
                        TTY_DRIVER_MINOR_START(tty) + TTY_DRIVER_NAME_BASE(tty));
        else
                strcpy(buf, "NULL tty");
        return buf;
}
#endif

static void nrp_send_xchar(struct tty_struct *tty, char ch)
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;

        if (nslink_paranoia_check(info, "nrp_send_xchar"))
                return;

        info->x_char = ch;
        info->si->need_to_send = 1;
        if (info->si->use_tcp == 0)
                si_timer((unsigned long) info->si);
}

static void nrp_throttle(struct tty_struct *tty)
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;

        if (nslink_paranoia_check(info, "nrp_throttle"))
                return;

        if (I_IXOFF(tty))
                nrp_send_xchar(tty, STOP_CHAR(tty));
}

static void nrp_unthrottle(struct tty_struct *tty)
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;

        if (nslink_paranoia_check(info, "nrp_throttle"))
                return;

        if (I_IXOFF(tty))
                nrp_send_xchar(tty, START_CHAR(tty));
}

/*
 * ------------------------------------------------------------
 * nrp_stop() and nrp_start()
 *
 * This routines are called before setting or resetting tty->stopped.
 * They enable or disable transmitter interrupts, as necessary.
 * ------------------------------------------------------------
 */
static void nrp_stop(struct tty_struct *tty)
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;
#ifdef NSLINK_DEBUG_FLOW

        char buf[64];

        printk(KERN_INFO "nslink: stop %s: %d....\n", nrp_tty_name(tty, buf), info->xmit_cnt);
#endif

        if (nslink_paranoia_check(info, "nrp_stop"))
                return;

        /*
         * Would be nice if there was a way to stop the transmitter
         * immediately.
         */
}

static void nrp_start(struct tty_struct *tty)
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;
#ifdef NSLINK_DEBUG_FLOW

        char buf[64];

        printk(KERN_INFO "nslink: start %s: %d....\n", nrp_tty_name(tty, buf),
               info->xmit_cnt);
#endif

        if (nslink_paranoia_check(info, "nrp_start"))
                return;

        /*
         * Start the transmitter remotely, if we ever add a stop
         * transmitter to nrp_stop.
         */

        if (info->xmit_cnt <= 0 || tty->stopped || tty->hw_stopped ||
            !info->xmit_buf)
                return;

        info->si->need_to_send = 1;
        if (info->si->use_tcp == 0)
                si_timer((unsigned long) info->si);
}

/*
 *  Waits until the serial port in the remote hub had transmitted all
 *  the data that this tty driver has sent to it.
 */
static void nrp_wait_until_sent(struct tty_struct *tty, int timeout)
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;
        unsigned long orig_jiffies;
        int check_time, exit_time=0;
        int txcnt=0;

        if (nslink_paranoia_check(info, "nrp_wait_until_sent"))
                return;

        if (info->remote_status != REM_PORT_ASSIGNED)
                return;

        orig_jiffies = jiffies;
#ifdef NSLINK_DEBUG_WAIT_UNTIL_SENT
        printk(KERN_INFO "nslink: entering nrp_wait_until_sent(%d) (jiff=%lu) cps=%d\n", timeout, jiffies, info->cps);
#endif

        while (1) {
#ifdef NSLINK_DEBUG_WAIT_UNTIL_SENT
                printk(KERN_INFO "nslink: nrp_wait_until_sent: info=%p info->si=%p\n",info,info->si);
#endif

                if (!info) {
                        printk(KERN_INFO "nslink: nrp_wait_until_sent info==NULL!\n");
                        break;
                }
                if (!info->si) {
                        printk(KERN_INFO "nslink: nrp_wait_until_sent info->si==NULL!\n");
                        break;
                }
                if (info->portnum<0 || info->portnum >info->si->num_ports) {
                        printk(KERN_INFO "nslink: nrp_wait_until_sent: bad portnum: %d\n",info->portnum);
                        break;
                }

                //printk(KERN_INFO "nslink: nrp_wait_until_sent info=%p info->si=%p\n", info, info->si);
                txcnt = get_tx_cnt_remote(&info->si->port_state[info->portnum]);
                //printk(KERN_INFO "nslink: nrp_wait_until_sent txcnt=%d\n", txcnt);

                if (!txcnt) {
                        // Ideal to check if the last character has shifted all the
                        // way out, but there's no way to do that right now.
                        break;
                }

                // check in 1/10 second intervals.  Used to base interval on
                // tx byte count, but if there is a flush in progress,
                // that's no good.
                check_time = HZ/10;

                if (timeout) {
                        // work-around for broken tty-layer code that passes
                        // 64-bit value resulting in our seeing -1 for a
                        // timeout instead of max signed value.
                        if (timeout > 0)
                                exit_time = orig_jiffies + timeout - jiffies;
                        else
                                exit_time = ((unsigned)~0)>>1;

                        if (exit_time <= 0)
                                break;

                        if (exit_time < check_time)
                                check_time = exit_time;
                }

                if (check_time == 0)
                        check_time = 1;

#ifdef NSLINK_DEBUG_WAIT_UNTIL_SENT
                printk(KERN_INFO "nslink: nrp_wait_until_sent: txcnt = %d (jiff=%lu,check=%d,exit=%d)\n", txcnt, jiffies, check_time, exit_time);
#endif
                set_current_state(TASK_INTERRUPTIBLE);
                schedule_timeout(check_time);
                if (signal_pending(current))
                        break;
        }
        set_current_state(TASK_RUNNING);
#ifdef NSLINK_DEBUG_WAIT_UNTIL_SENT
        printk(KERN_INFO "nslink: txcnt = %d (jiff=%lu)...done\n", txcnt, jiffies);
#endif
}

/*
 * nrp_hangup() --- called by tty_hangup() when a hangup is signaled.
 */
static void nrp_hangup(struct tty_struct *tty)
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;
        unsigned long flags;

        if (nslink_paranoia_check(info, "nrp_hangup"))
                return;

#if (defined(NSLINK_DEBUG_OPEN) || defined(NSLINK_DEBUG_HANGUP))
        printk(KERN_INFO "nslink: nrp_hangup of ttySI%d...", info->line);
#endif
        /*
         * If the port is in the process of being closed, just force
         * the transmit buffer to be empty, and let nrp_close handle
         * the clean up.
         */
        if (info->flags & ASYNC_CLOSING) {
                spin_lock_irqsave(&info->slock, flags);
                info->xmit_cnt = info->xmit_head = info->xmit_tail = 0;
                spin_unlock_irqrestore(&info->slock, flags);

                wake_up_interruptible(&tty->write_wait);
                return;
        }

        //  Use spinlock to protect info->count
        spin_lock_irqsave(&info->slock, flags);
        if (info->count) {
                atomic_dec(&nrp_num_ports_open);
        }

        info->count = 0;
        spin_unlock_irqrestore(&info->slock, flags);

        info->flags &= ~ASYNC_NORMAL_ACTIVE;
        info->tty = 0;

        info->action_register = ACT_FLUSH_INPUT | ACT_FLUSH_OUTPUT;
        info->update_action = 1;
        info->si->need_to_send = 1;

        if (info->si->use_tcp == 0)
                si_timer((unsigned long) info->si);

        info->flags &= ~ASYNC_INITIALIZED;

        wake_up_interruptible(&info->open_wait);
}

/*
 *  TTY write char routine.  Write buffer and counters protected by spinlocks.
 */
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,26)
static void nrp_put_char(struct tty_struct *tty, unsigned char ch)
#else
static int nrp_put_char(struct tty_struct *tty, unsigned char ch)
#endif
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;
        unsigned long flags;
        int ret = 0;

// the various test condition results now use goto end instead of just return.

        if (nslink_paranoia_check(info, "nrp_put_char"))
                goto end;

        if (info->remote_status != REM_PORT_ASSIGNED)
                goto end;

        if (info->xmit_cnt >= XMIT_BUF_SIZE - 1)
                goto end;

#ifdef NSLINK_DEBUG_WRITE

        printk(KERN_INFO "nslink: nrp_put_char %c...", ch);
#endif

        // Lock the port semaphore
        // returns -EINTR if interrupted. Non-zero if the
        // sem cannot be acquired.
        if (down_trylock(&info->write_sem)) {
                goto end;
        }

        spin_lock_irqsave(&info->slock, flags);

        info->xmit_buf[info->xmit_head++] = ch;
        info->xmit_head &= SERIAL_XMIT_SIZE - 1;
        info->xmit_cnt++;
        ret++;	// increment so return is non-zero
        spin_unlock_irqrestore(&info->slock, flags);
        up(&info->write_sem);

end:

#if LINUX_VERSION_CODE < VERSION_CODE(2,6,26)
        return;
#else
        return ret;
#endif

}//end nrp_put_char

/*
 *  TTY flush chars routine
 */
static void nrp_flush_chars(struct tty_struct *tty)
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;
        struct si_port_state *pstate;
        int space = 0;

        if (nslink_paranoia_check(info, "nrp_flush_chars"))
                return;

        pstate = &info->si->port_state[info->portnum];
        if (pstate)
                space = get_tx_space_remote(pstate);

        if (space <= WAKEUP_CHARS || info->xmit_cnt <= 0 ||
            tty->stopped || tty->hw_stopped || !info->xmit_buf)
                return;

        info->si->need_to_send = 1;
        if (info->si->use_tcp == 0) {
                if (info->xmit_cnt > 64)
                        si_timer((unsigned long) info->si);
                else
                        si_set_timer(info->si, 0);
        }
}

/*
 *  TTY write routine, called when user (or kernel) app writes to a serial port.  Data
 *  is moved into the write buffer for the port (in the global info-> struct), then
 *  the HDLC or TCP function is called that causes the bytes to go over the ethernet
 *  to the device.
 *
 *  A per port write semaphore is used to protect from another process writing to
 *  this port at the same time.  This other process could be running on the other CPU
 *  or get control of the CPU if the copy_from_user() blocks due to a page fault (swapped out).
 *  Spinlocks protect the info xmit members.
 *
 *  Inputs:  from_user set if write called from user space, data buf and count requested amount
 *           to write
 *  Returns: Number of bytes actually written.
 *
 *  Our open source friends changed the tty layer in kernel version 2.6.10, removing
 *  the from_user parameter, presumable the memcpy() functions now can deal with
 *  copies to/from user space.  There is therefore 2 versions of the following function.
 */
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,10)

static int nrp_write(struct tty_struct *tty, int from_user, const unsigned char *buf,  int count)
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;
        int c, retval = 0;
        unsigned long flags;

        if (count <= 0 || nslink_paranoia_check(info, "nrp_write"))
                return 0;

        if (info->si->state != SI_STATE_CONNECTED) {
                msleep_interruptible(10);
                return -EIO;
        }

        if (info->remote_status == REM_PORT_BUSY) {
                msleep_interruptible(10);
                return -ENOTCONN;
        }

        if (info->remote_status != REM_PORT_ASSIGNED) {
                msleep_interruptible(10);
                return -EIO;
        }

#ifdef NSLINK_DEBUG_WRITE

        printk(KERN_INFO "nslink: nrp_write %d chars...", count);
#endif

        // Lock the port semaphore
        // returns -EINTR if interrupted. Non-zero if the
        // sem cannot be acquired.
        retval = down_interruptible(&info->write_sem);
        if (retval) {
                return retval;
        }

        //  Loop while there is TTY data to be transfered into the driver
        while (1) {
                if (info->tty == 0)
                        goto end;

                c = MIN(count, MIN(XMIT_BUF_SIZE - info->xmit_cnt - 1,
                                   XMIT_BUF_SIZE - info->xmit_head));
                if (c <= 0)
                        break;

                info->txBytes += c;

                //  Copy data from the app into the port's write buffer
                if (from_user) {
                        if (copy_from_user(info->xmit_buf + info->xmit_head, buf, c)) {
                                retval = -EFAULT;
                                goto end_intr;
                        }
                } else {
                        memcpy(info->xmit_buf + info->xmit_head, buf, c);
                }

                spin_lock_irqsave(&info->slock, flags);
                info->xmit_head = (info->xmit_head + c) & (XMIT_BUF_SIZE - 1);
                info->xmit_cnt += c;
                spin_unlock_irqrestore(&info->slock, flags);

                buf += c;
                count -= c;
                retval += c;
        }

end_intr:
        nrp_flush_chars(tty);

end:
        up(&info->write_sem);

        //  Call the routines to send data to hub
        if (info->tty->low_latency) {
                info->si->need_to_send = 1;
                if (info->si->use_tcp == 0)
                        si_timer((unsigned long)info->si);
                else
                        tcp_send_remote_cmds(info->si);
        }
#ifdef NSLINK_DEBUG_WRITE
        printk(KERN_INFO "ret=%d\n", retval);
#endif

        return retval;
}

#else  //  Linux 2.6.10 or greater

static int nrp_write(struct tty_struct *tty, const unsigned char *buf,  int count)
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;
        int c, retval = 0;
        unsigned long flags;

        if (count <= 0 || nslink_paranoia_check(info, "nrp_write"))
                return 0;

        if (info->si->state == SI_STATE_RESET) {
                msleep_interruptible(10);
                return -EIO;
        }

        if (info->remote_status == REM_PORT_BUSY) {
                msleep_interruptible(10);
                return -ENOTCONN;
        }

        if (info->remote_status != REM_PORT_ASSIGNED) {
                msleep_interruptible(10);
                return -EIO;
        }

#ifdef NSLINK_DEBUG_WRITE

        printk(KERN_INFO "nslink: nrp_write %d chars...", count);
#endif

        // Lock the port semaphore
        // returns -EINTR if interrupted. Non-zero if the
        // sem cannot be acquired.
        retval = down_interruptible(&info->write_sem);
        if (retval) {
                return retval;
        }

        //  Loop while there is TTY data to be transfered into the driver
        while (1) {
                if (info->tty == 0)
                        goto end;

                c = MIN(count, MIN(XMIT_BUF_SIZE - info->xmit_cnt - 1,
                                   XMIT_BUF_SIZE - info->xmit_head));
                if (c <= 0)
                        break;

                info->txBytes += c;

                //  Copy data from the app into the port's write buffer
                memcpy(info->xmit_buf + info->xmit_head, buf, c);

                spin_lock_irqsave(&info->slock, flags);
                info->xmit_head = (info->xmit_head + c) & (XMIT_BUF_SIZE - 1);
                info->xmit_cnt += c;
                spin_unlock_irqrestore(&info->slock, flags);

                buf += c;
                count -= c;
                retval += c;
        }

        nrp_flush_chars(tty);

end:
        up(&info->write_sem);

        //  Call the routines to send data to hub
        if (info->tty->low_latency) {
                info->si->need_to_send = 1;
                if (info->si->use_tcp == 0)
                        si_timer((unsigned long)info->si);
                else
                        tcp_send_remote_cmds(info->si);
        }
#ifdef NSLINK_DEBUG_WRITE
        printk(KERN_INFO "ret=%d\n", retval);
#endif

        return retval;
}

#endif  // Linux 2.6.10 or greater


/*
 * Return the number of characters that can be sent.  We estimate
 * only using the in-memory transmit buffer only, and ignore the
 * potential space in the remote end.
 */
static int nrp_write_room(struct tty_struct *tty)
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;
        int ret;

        if (nslink_paranoia_check(info, "nrp_write_room"))
                return 0;

        ret = XMIT_BUF_SIZE - info->xmit_cnt - 1;
        if (ret < 0)
                ret = 0;
#ifdef NSLINK_DEBUG_WRITE

        printk(KERN_INFO "nslink: nrp_write_room returns %d...", ret);
#endif

        return ret;
}

/*
 * Return the number of characters in the buffer.  Again, this only
 * counts those characters in the in-memory transmit buffer.
 */
static int nrp_chars_in_buffer(struct tty_struct *tty)
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;

        if (nslink_paranoia_check(info, "nrp_chars_in_buffer"))
                return 0;

#ifdef NSLINK_DEBUG_WRITE

        printk(KERN_INFO "nslink: nrp_chars_in_buffer returns %d...", info->xmit_cnt);
#endif

        return info->xmit_cnt;
}

/*
 *  TTY flush buffer routine.  Removes all bytes from the nr_port xmit_buf.
 */
static void nrp_flush_buffer(struct tty_struct *tty)
{
        struct nr_port *info = (struct nr_port *) tty->driver_data;
        unsigned long flags;

        if (nslink_paranoia_check(info, "nrp_flush_buffer"))
                return;

        info->action_register = ACT_FLUSH_OUTPUT;
        info->update_action = 1;
        info->si->need_to_send = 1;
        if (info->si->use_tcp == 0)
                si_timer((unsigned long) info->si);

        spin_lock_irqsave(&info->slock, flags);
        info->xmit_cnt = info->xmit_head = info->xmit_tail = 0;
        spin_unlock_irqrestore(&info->slock, flags);

        wake_tty_write(tty);
}

#if LINUX_VERSION_CODE < VERSION_CODE(2,6,30)
/*
 * /proc fs routines
 */
static inline int line_info(char *buf, struct si_state *si, int line)
{
        struct si_port_state *pstate;
        struct nr_port *info;
        char stat_buf[40];
        int ret;
        unsigned int control, status;

        ret = sprintf(buf, "%d: ", line + si->base_port);

        info = nrp_table[line + si->base_port];
        pstate = si->port_state + line;

        /*
         * Figure out the current RS-232 lines
         */
        status = pstate->modemStatusRegister;
        control = info ? info->modem_control_register : 0;
        stat_buf[0] = 0;
        stat_buf[1] = 0;
        if (control & MCR_DTR_SET_MASK)
                strcat(stat_buf, "|RTS");
        if (status & MSR_CTS_ON)
                strcat(stat_buf, "|CTS");
        if (control & MCR_DTR_SET_MASK)
                strcat(stat_buf, "|DTR");
        if (status & MSR_DSR_ON)
                strcat(stat_buf, "|DSR");
        if (status & MSR_CD_ON)
                strcat(stat_buf, "|CD");
        if (status & MSR_TX_FLOWED_OFF)
                strcat(stat_buf, "|TXOFF");
        if (status & MSR_RING_ON)
                strcat(stat_buf, "|RING");

        if (info && info->baud_rate)
                ret += sprintf(buf + ret, " baud:%d", info->baud_rate);

#if 0
        ret += sprintf(buf + ret, " tx:%d rx:%d",
                       state->icount.tx, state->icount.rx);

        if (state->icount.frame)
                ret += sprintf(buf + ret, " fe:%d", state->icount.frame);

        if (state->icount.parity)
                ret += sprintf(buf + ret, " pe:%d", state->icount.parity);

        if (state->icount.brk)
                ret += sprintf(buf + ret, " brk:%d", state->icount.brk);

        if (state->icount.overrun)
                ret += sprintf(buf + ret, " oe:%d", state->icount.overrun);
#endif

        /*
         * Last thing is the RS-232 status lines
         */
        ret += sprintf(buf + ret, " %s\n", stat_buf + 1);
        return ret;
}

static int nrp_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
        int i, j, len = 0, l;
        off_t begin = 0;
        struct si_state *si;

        len += sprintf(page, "serinfo:1.0 driver:%s\n", SI_VERSION);
        for (i = 0; i < numBoxes; i++) {
                si = conc_state[i];
                if (!si)
                        continue;
                for (j = 0; j < si->num_ports; j++) {
                        l = line_info(page + len, si, j);
                        len += l;
                        if (len + begin > off + count)
                                goto done;
                        if (len + begin < off) {
                                begin += len;
                                len = 0;
                        }
                }
        }
        *eof = 1;
done:
        if (off >= len + begin)
                return 0;
        *start = page + (begin - off);
        return ((count < begin + len - off) ? count : begin + len - off);
}
#endif	// kernel < 2,6,30

/********************************************************************************/
/*                                                                              */
/*        NSLinkctl device functions                                            */
/*                                                                              */
/********************************************************************************/

static int ctl_open(struct inode *inode, struct file *file)
{
        return 0;
}

static int ctl_close(struct inode *inode, struct file *file)
{
        return 0;
}

static unsigned int ctl_poll(struct file *file, poll_table * wait)
{
        struct si_state *si;
        unsigned int mask;
        int i;

        poll_wait(file, &wake_daemon_wait, wait);
        mask = 0;

        for (i = 0; i < numBoxes; i++) {
                si = conc_state[i];
                if (si && si->state == SI_STATE_DEAD)
                        mask |= POLLOUT | POLLWRNORM;
        }
        return mask;
}

static int get_box_config(struct si_box_config *retinfo)
{
        struct si_box_config tmp;
        struct si_state *si;

        if (copy_from_user(&tmp, retinfo, sizeof(tmp)))
                return -EFAULT;

        if ((tmp.boxid >= numBoxes) || !((si = conc_state[tmp.boxid])))
                return -EINVAL;

        tmp.num_ports = si->num_ports;
        tmp.state = si->state;
        memcpy(tmp.ether_addr, si->ether_addr, 6);
        strncpy(tmp.interface, si->dev->name, sizeof(tmp.interface));

        if (copy_to_user(retinfo, &tmp, sizeof(tmp)))
                return -EFAULT;
        return 0;
}

static int get_box_report(struct si_box_report *retinfo)
{
        struct si_box_report tmp;
        int unit;

        if (copy_from_user(&tmp, retinfo, sizeof(tmp)))
                return -EFAULT;

        unit = tmp.flags;		/* kludge, but easist way to handle it */

        if ((unit >= num_box_rpts) || (unit >= numBoxes))
                return -EINVAL;

        tmp = si_box_rpts[unit];

        if (copy_to_user(retinfo, &tmp, sizeof(tmp)))
                return -EFAULT;
        return 0;
}

static int get_problem_boxes(int *ret_status)
{
        struct si_state *si;
        int i, status = 0;

        for (i = 0; i < numBoxes; i++) {
                si = conc_state[i];
                if (si && si->state == SI_STATE_DEAD)
                        status |= 1 << i;
        }
        if (copy_to_user(ret_status, &status, sizeof(status)))
                return -EFAULT;
        return 0;
}

static int force_port_release(int line)
{
        struct nr_port *info;
        if (line < 0 || line > MAX_NRP_TOTAL_PORTS)
                return -EINVAL;
        info = nrp_table[line];
        if (!info || !info->si)
                return -EINVAL;
        printk(KERN_INFO "nslink: Forcing release of /dev/ttySI%d\n",line);
        info->send_force_release = 1;
        info->force_release_acked = 0;
        info->si->need_to_send = 1;
        if (info->si->use_tcp == 0)
                mod_timer(&info->si->timer, jiffies + HZ/50);  //  Set the timer to 20ms from now
        interruptible_sleep_on_timeout(&info->release_wait,3*HZ);
        if (info->force_release_acked)
                return 0;
        else
                return -ETIME;
}

static int debug_misc(void)
{
#if 0
        struct si_state *si;

        si = conc_state[0];
        printk(KERN_INFO "nslink: si->rtq.qlen = %d\n", si->rtq.qlen);
#endif
        return 0;
}

/*
 *  Sets number of open ports to zero so the can_unload() returns true, allowing the module
 *  to unload.  Kind of a hack, we have continual problems with this use count stuff.  Be careful
 *  not to do this when ports really are open.
 */
static int release_module(void)
{
        atomic_set(&nrp_num_ports_open, 0);
        return 0;
}

/*
 *  Handler for IOCTL calls made through the NSLinkctl device
 */
static int ctl_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
        switch (cmd) {
        case SI_RELEASE_MODULE:
                return release_module();

        case SI_SET_BOX_CONFIG:
                return init_hub_state((struct si_box_config *) arg, 1);

        case SI_GET_BOX_CONFIG:
                return get_box_config((struct si_box_config *) arg);

        case SI_GET_BOX_REPORT:
                return get_box_report((struct si_box_report *) arg);

        case SI_GET_PROBLEM_BOXES:
                return get_problem_boxes((int *) arg);

        case SI_DEBUG_MISC:
                return debug_misc();

        case SI_SET_TCP:
                return set_tcp((struct tcp_mode_params *) arg);
                break;

        case SI_TCP_TXQUEUE_WAIT:
                return tcp_txqueue_wait((tTcpTxQueueWait*) arg);
                break;

        case SI_SET_MAC:
                return set_mac((struct mac_mode_params *) arg);
                break;

        case SI_REGISTER_PORTS:
                return register_tty_driver((tBoxPortCount *)arg);
                break;

        case SI_FORCE_PORT_RELEASE:
                return force_port_release(arg);
                break;

        case SI_LOAD_CODE4_8:
        case SI_LOAD_CODE2:
        default:
                return -EINVAL;
        }
        return 0;
}

// In newer kernels, unlocked_ioctl() has replaced ioctl(), and calls
// are made with the kernel unlocked.  In older kernels ioctl calls
// were made with the kernel locked.  The routine below implements
// that older behavior for new kernels.  We should probably eliminate
// the lock/unlock_kernel() calls be doing appropriate locking within
// the code the implements the ioctl commands, but since control ioctl
// calls only happen on startup and shutdown, we're not going to
// bother at this point.

#if LINUX_VERSION_CODE >= VERSION_CODE(2,6,36)
# if LINUX_VERSION_CODE >= VERSION_CODE(2,6,39)
#  define lock_kernel()  /* noop */
#  define unlock_kernel()  /* noop */
# endif
static long ctl_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        long r;
        lock_kernel();
        r = ctl_ioctl(NULL, file, cmd, arg);
        unlock_kernel();
        return r;
}
#endif

/*
 *  Sets the MAC mode parameters for the driver.  Called from the NSLinkctl device IOCTL.
 */
static int set_mac(struct mac_mode_params *in)
{
        struct si_state *si;
        struct mac_mode_params mac_params;

        if (!driver_registered) {
                printk(KERN_INFO "nslink: Error -- attempt to register device before driver is registered (out of date daemon?)\n");
                return -EINVAL;
        }

        if (copy_from_user(&mac_params, (void *) in, sizeof(struct mac_mode_params)))
                return -EFAULT;

        if (mac_params.boxid >= numBoxes)
                return -EINVAL;

        si = conc_state[mac_params.boxid];

        if (!si)
                return -EFAULT;

        si->state = SI_STATE_BOOTED;
        si_set_timer(si, HZ / 50);         //  20ms wakeup call

        //  Sleep until woken.  The daemon configures the box, then sends the set_mac
        //  ioct.  At this point, the hub begins functioning and the daemon sleeps (here).
        //  It is woken (in si_timer) only if the state goes to SI_STATE_RESET, at which
        //  point it it the daemon's job to reboot the hub and restart it.
        wait_event_interruptible(si->waitqueue, (si->state == SI_STATE_RESET));
        return 0;
}

/*
 *  File operations struct for the NSLinkctl device
 */
static struct file_operations ctl_fops = {
owner:
        THIS_MODULE,
poll:
        ctl_poll,
#if LINUX_VERSION_CODE >= VERSION_CODE(2,6,36)
unlocked_ioctl:
        ctl_unlocked_ioctl,
#else
ioctl:
        ctl_ioctl,
#endif
open:
        ctl_open,
release:
        ctl_close,
};

/********************************************************************************/
/*                                                                              */
/*        TCP FUNCTIONS                                                         */
/*                                                                              */
/********************************************************************************/

/*
 *  Stores a fragment in the fragment buffer.  A fragment is when a partial
 *  admin/async command is received, which happens in tcp mode.  For example,
 *  a RK_PORT_SET command is the command which should have 1 byte of associated
 *  data (total length 2).  If the data length is 1, then this is a fragment, the
 *  next byte must be in the next tcp packet.  This function stores the fragment
 *  and set the tcp_frag_len.  This variable acts as a flag indicating that there
 *  is a stored fragment.
 */
static void tcp_store_fragment(struct si_state *si, unsigned char *buf, int len)
{
        memcpy(si->tcp_k_rfrag, buf, len);
        si->tcp_frag_len = len;
        return;
}

static void tcp_process_packet(int len, struct si_state *si)
{

        unsigned char *buf;

        //printk(KERN_INFO "nslink: tcp_process_packet(%d,%p)\n",len,si);

        if (si->tcp_frag_len) {
                buf = si->tcp_k_rfrag;
                len += si->tcp_frag_len;
                si->tcp_frag_len = 0;
        } else
                buf = si->tcp_k_rbuf;

        si_parse_input_pkt(si, buf, len);

        return;
}

static void tcp_check_rx(struct si_state *si)
{
        struct iovec iov;
        struct msghdr msg;
        int rc, amount, pktlen;

        rc = si->SOCK_PROT(tcp_sk)->ioctl(si->tcp_sk, TIOCINQ, (unsigned long) si->tcp_u_rbuf);

        if (rc) {
                printk(KERN_INFO "nslink: TIOCINQ err %d\n", rc);
                return;
        }
        if (copy_from_user(&amount, si->tcp_u_rbuf, sizeof(int))) {
                printk(KERN_INFO "nslink: CheckRxTcp copy from user failed\n");
                return;
        }

        while (amount) {
                if (amount > si->tcp_rbuf_size)
                        pktlen = si->tcp_rbuf_size;
                else
                        pktlen = amount;

                iov.iov_base = si->tcp_u_rbuf;
                iov.iov_len = pktlen;
                msg.msg_name = NULL;
                msg.msg_iov = &iov;
                msg.msg_iovlen = 1;
                msg.msg_control = NULL;

#if LINUX_VERSION_CODE > VERSION_CODE(2,5,0)

                rc = si->SOCK_PROT(tcp_sk)->recvmsg(NULL, si->tcp_sk, &msg, pktlen, 1, 0, NULL);
#else

                rc = si->SOCK_PROT(tcp_sk)->recvmsg(si->tcp_sk, &msg, pktlen, 1, 0, NULL);
#endif

                if (rc < 0) {
                        printk(KERN_INFO "nslink: CheckRxTcp recvmsg err %d\n", rc);
                        return;
                }
                if (rc != pktlen)
                        printk(KERN_INFO "nslink: CheckRxTcp rc %d, pktlen %d\n", rc, pktlen);

                /*
                 *  If tcp_frag_len is non-zero, then there is a stored fragment from the last
                 *  packet.  Add the incoming data to this fragment and process that.  If no fragment,
                 *  process the incoming data.
                 */
                if (si->tcp_frag_len)
                        rc = copy_from_user(&si->tcp_k_rfrag[si->tcp_frag_len], si->tcp_u_rbuf, pktlen);
                else
                        rc = copy_from_user(si->tcp_k_rbuf, si->tcp_u_rbuf, pktlen);

                if (rc) {
                        printk(KERN_INFO "nslink: CheckRxTcp copy from user failed\n");
                        return;
                }

                tcp_process_packet(pktlen, si);
                amount -= pktlen;
        }

        return;
}

/*
 *  Send data using a kernel mode TCP socket.  Data to send is int si->tcp_k_wbuf.
 */
static void tcp_send_packet(struct si_state *si, int len)
{
        int rc;
        struct iovec iov;
        struct msghdr msg;
        mm_segment_t oldfs;

        //printk(KERN_INFO "nslink: tcp_send_packet(%p, %d)\n",si,len);

        if (!len)
                return;

        iov.iov_base = si->tcp_k_wbuf;
        iov.iov_len = len;
        msg.msg_name = NULL;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = NULL;
        msg.msg_flags = 0;

        oldfs = get_fs();
        set_fs(KERNEL_DS);

#if LINUX_VERSION_CODE > VERSION_CODE(2,5,0)
#if LINUX_VERSION_CODE > VERSION_CODE(2,6,21)
        rc = si->tcp_sock->ops->sendmsg(NULL, si->tcp_sock, &msg, len);
#else
        rc = si->SOCK_PROT(tcp_sk)->sendmsg(NULL, si->tcp_sk, &msg, len);
#endif
#else
        rc = si->SOCK_PROT(tcp_sk)->sendmsg(si->tcp_sk, &msg, len);
#endif

        set_fs(oldfs);

        if (rc != len)
                printk(KERN_INFO "nslink: TcpSendPacket sendmsg err, %d\n", rc);

        return;
}

static int tcp_room(struct si_state *si, int bi, int cnt)
{
        if ((bi + cnt) > si->tcp_wbuf_size) {
                tcp_send_packet(si, bi);
                return 1;
        }
        return 0;
}

static int tcp_out_data(unsigned char *buf, int bi, struct nr_port *info,
                        struct si_port_state *pstate, struct si_state *si)
{
        int lenbi, xmitcnt, space, totlen;
        struct tty_struct *tty;
        unsigned long flags;

        if (si->state != SI_STATE_CONNECTED || info->remote_status != REM_PORT_ASSIGNED)
                return 0;

        buf[bi++] = RK_DATA_BLK;
        lenbi = bi;
        bi += 2;

        totlen = 0;
        if (info->x_char) {
                buf[bi++] = info->x_char;
                totlen += 1;
                pstate->nPutRemote += 1;
                info->x_char = 0;
        }

        //  Copy the data into buf, spinlock to protect xmit_tail and xmit_cnt
        spin_lock_irqsave(&info->slock, flags);
        do {
                xmitcnt = si->tcp_wbuf_size - bi;
                if (xmitcnt > info->xmit_cnt)
                        xmitcnt = info->xmit_cnt;

                if (xmitcnt > (XMIT_BUF_SIZE - info->xmit_tail))
                        xmitcnt = XMIT_BUF_SIZE - info->xmit_tail;

                space = get_tx_space_remote(pstate);

                if (xmitcnt > space)
                        xmitcnt = space;

                if (xmitcnt) {
                        memcpy(&buf[bi], (info->xmit_buf + info->xmit_tail), xmitcnt);
                        bi += xmitcnt;
                        info->xmit_tail += xmitcnt;
                        info->xmit_tail &= (XMIT_BUF_SIZE - 1);
                        info->xmit_cnt -= xmitcnt;
                        totlen += xmitcnt;
                        pstate->nPutRemote += (__u16) xmitcnt;
                }
        } while (xmitcnt);
        spin_unlock_irqrestore(&info->slock, flags);

        if (totlen) {
                buf[lenbi++] = totlen & 0xff;
                buf[lenbi++] = totlen >> 8;
                totlen += 3;
        }

        tty = info->tty;
        if ((info->xmit_cnt < WAKEUP_CHARS) && tty)
                wake_tty_write(tty);

        return totlen;
}

/*
 *  Depending on the si-> tcp flags, puts cmds into the tcp write buffer,
 *  then sends the tcp packet.
 */
static void tcp_send_remote_cmds(struct si_state *si)
{
        int pi, bi, portset, dc;
        unsigned char *buf;
        struct si_port_state *pstate;
        struct nr_port *info;

        //printk(KERN_INFO "nslink: tcp_send_remote_cmds(%p)\n",si);

        if (!si->tcp_sk)
                return;

        if (down_interruptible(&si->write_sem))
                return;

        if (!sock_wspace(si->tcp_sk)) {
                printk(KERN_INFO "nslink: TcpLoop OUTQ empty\n");
                up(&si->write_sem);
                return;
        }

        buf = si->tcp_k_wbuf;
        si->need_to_send = 0;
        bi = 0;

        if (si->need_to_send_connect_check) {
                buf[bi++] = RK_CONNECT_CHECK;
                si->need_to_send_connect_check = 0;
#ifdef NSLINK_DEBUG_KEEPALIVE
                printk(KERN_INFO "nslink: tx RK_CONNECT_CHECK: %d\n",si->conc_id);
#endif

        }

        if (si->update_timeout) {
                if (tcp_room(si, bi, 9))	//was 7
                        bi = 0;
                buf[bi++] = RK_TIMEOUT_SET;
                // whoever decided to put stuff on the wire in */
                // something other than network order ought to be
                // hunted down and beaten bloody.
                buf[bi++] = si->link_timeout;
                buf[bi++] = si->link_timeout >> 8;
                buf[bi++] = si->link_timeout >> 16;
                buf[bi++] = si->link_timeout >> 24;
                if ( si->scan_period && (si->arch_id == 3 || si->arch_id ==2)) {
                        buf[bi++] = RK_SCANPERIOD_SET;
                        buf[bi++] = si->scan_period;
                }
                if ( si->rexmit_timer && (si->arch_id == 3 || si->arch_id ==2)) {
                        buf[bi++] = RK_RETRANS_TIMER_SET;
                        buf[bi++] = si->rexmit_timer;
                }
                si->update_timeout = 0;
        }

        pi = 0;
        while (pi < si->num_ports) {
                portset = 0;
                buf[bi++] = RK_PORT_SET;
                buf[bi++] = pi;

                info = nrp_table[pi + si->base_port];
                if (!info)
                        continue;

                if (info->remote_status == REM_PORT_ASSIGNED) {
                        pstate = si->port_state + pi;
                        if (pstate->update_queue) {
                                if (tcp_room(si, bi, 3)) {
                                        bi = 0;
                                        continue;
                                }
                                portset = 1;
                                buf[bi++] = RK_QIN_STATUS;
                                *(unsigned short *) &buf[bi] = pstate->nGetLocal;
                                bi += 2;
                                pstate->update_queue = 0;
                        }
                        if (info->rsmode != si->rsmode[pi]) {
                                if (tcp_room(si, bi, 2)) {
                                        bi = 0;
                                        continue;
                                }

                                if (si->rsmode[pi] != SI_RSMODE_232) {
                                        if (UnitHasNo485(si->modelID)) {
                                                printk(KERN_INFO "nslink: Error - attempting to change interface mode for port ttySI%d on a model that only has RS232 ports \n", info->line);
                                                si->rsmode[pi] = SI_RSMODE_232;
                                        }
                                }

                                if ((si->rsmode[pi] == SI_RSMODE_485FDM) ||
                                    (si->rsmode[pi] == SI_RSMODE_485FDS)) {
                                        if (!UnitHas485FD(si->modelID)) {
                                                printk(KERN_INFO "nslink: Error - attempting to change interface mode for port ttySI%d on a model that does not support RS485 full duplex mode \n", info->line);
                                                si->rsmode[pi] = SI_RSMODE_232;
                                        }
                                }

                                info->rsmode = si->rsmode[pi];
                                portset = 1;
                                buf[bi++] = RK_RSMODE_SET;
                                buf[bi++] = info->rsmode;
                                if ((info->rsmode == SI_RSMODE_485) ||
                                    (info->rsmode == SI_RSMODE_485FDS)) {
                                        /* RS485 turn on RTS toggle */
                                        info->update_control_settings = 1;
                                        info->control_settings &= ~SC_FLOW_RTS_MASK;
                                        info->control_settings |= SC_FLOW_RTS_ARS485;
                                }
                        }
                        if (info->update_baud_rate) {
                                if (tcp_room(si, bi, 5)) {
                                        bi = 0;
                                        continue;
                                }
                                portset = 1;
                                buf[bi++] = RK_BAUD_SET;
                                *(uint32_t *) &buf[bi] = info->baud_rate;
                                bi += 4;
                                info->update_baud_rate = 0;
                        }
                        if (info->update_special_charset) {
                                if (tcp_room(si, bi, 7)) {
                                        bi = 0;
                                        continue;
                                }
                                portset = 1;
                                buf[bi++] = RK_SPECIAL_CHAR_SET;
                                buf[bi++] = info->rx_xon;
                                buf[bi++] = info->rx_xoff;
                                buf[bi++] = info->tx_xon;
                                buf[bi++] = info->tx_xoff;
                                buf[bi++] = 0;
                                buf[bi++] = 0;
                                info->update_special_charset = 0;
                        }
                        if (info->update_control_settings) {
                                if (tcp_room(si, bi, 3)) {
                                        bi = 0;
                                        continue;
                                }
                                portset = 1;
                                buf[bi++] = RK_CONTROL_SET;
                                *(unsigned short *) &buf[bi] = info->control_settings;
                                bi += 2;
                                info->update_control_settings = 0;
                        }
                        if (info->update_mcr) {
                                if (tcp_room(si, bi, 3)) {
                                        bi = 0;
                                        continue;
                                }
                                portset = 1;
                                buf[bi++] = RK_MCR_SET;
                                *(unsigned short *) &buf[bi] = info->modem_control_register;
                                bi += 2;
                                info->update_mcr = 0;
                        }
                        if (info->update_ic_timeout) {
                                if (tcp_room(si, bi, 5)) {
                                        bi = 0;
                                        continue;
                                }
                                portset = 1;
                                buf[bi++] = RK_IC_TIMEOUT_SET;
                                *(uint32_t *)&buf[bi] = info->inter_char_timeout;
                                bi += 4;
                                info->update_ic_timeout = 0;
                        }
                        // make sure update action is last of the setup commands
                        // since we use action response to synchronize open().
                        if (info->update_action) {
                                if (tcp_room(si, bi, 3)) {
                                        bi = 0;
                                        continue;
                                }
                                portset = 1;
                                buf[bi++] = RK_ACTION_SET;
                                *(unsigned short *) &buf[bi] = info->action_register;
                                bi += 2;
                                info->update_action = 0;
                                info->action_register = 0;
                        }
                        if (info->x_char ||
                            (info->xmit_cnt && info->tty &&
                             !info->tty->hw_stopped && !info->tty->stopped)) {
                                if (tcp_room(si, bi, 4)) {
                                        bi = 0;
                                        continue;
                                }
                                dc = tcp_out_data(buf, bi, info, pstate, si);
                                if (dc) {
                                        bi += dc;
                                        portset = 1;
                                }
                        }
                } else if (info->remote_status == REM_OPEN_PORT) {
                        if (tcp_room(si, bi, 2)) {
                                bi = 0;
                                continue;
                        }
                        portset = 1;
                        buf[bi++] = RK_REMOTE_STATUS;
                        buf[bi++] = REM_OPEN_PORT;
                        info->remote_status = REM_WAITING_OPEN;
                } else if (info->remote_status == REM_RELEASE_PORT) {
                        if (tcp_room(si, bi, 2)) {
                                bi = 0;
                                continue;
                        }
                        portset = 1;
                        buf[bi++] = RK_REMOTE_STATUS;
                        buf[bi++] = REM_RELEASE_PORT;
                        info->remote_status = REM_WAITING_CLOSE;
                }

                if (info->send_force_release) {
                        if (tcp_room(si,bi,1)) {
                                bi = 0;
                                continue;
                        }
                        portset = 1;
                        buf[bi++] = RK_RELEASE_REQ;
                        info->force_release_acked = 0;
                        info->send_force_release = 0;
                }

                if (portset) {
                        if (bi == si->tcp_wbuf_size) {
                                tcp_send_packet(si, bi);
                                bi = 0;
                                continue;
                        }
                } else
                        bi -= 2;

                ++pi;
        }

        if (bi)
                tcp_send_packet(si, bi);

        up(&si->write_sem);
        return;
}

#define TRACE() printk(KERN_INFO "nslink: %s line %d\n",__FILE__,__LINE__)

// Hack Alert!
//
// Since the TCP admin stuff expects a datagram service, we
// need a way to make sure write()s don't get contatenated some
// where in the network.  This is an ioctl() that can be use
// to wait until a TCP tx queue is empty (all data has been
// ACK'ed).

static int tcp_txqueue_wait(tTcpTxQueueWait *in)
{
        tTcpTxQueueWait req;
#if LINUX_VERSION_CODE >= VERSION_CODE(2,5,0)
        int err;
#endif
        struct file *file;
        struct socket *socket;
        struct sock *sock;
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,11)

        struct tcp_opt *tp;
#else

        struct tcp_sock *tp;
#endif

        int timeout;
#if LINUX_VERSION_CODE < VERSION_CODE(2,5,0)

        struct inode *inode;
#endif

        if (copy_from_user(&req, (void *)in, sizeof req))
                return -EFAULT;

#if LINUX_VERSION_CODE < VERSION_CODE(2,5,0)

        file = fget(req.sock);

        if (!file)
                return EFAULT;

        inode = file->f_dentry->d_inode;
        if (!inode || !inode->i_sock) {
                fput(file);
                return -EFAULT;
        }

        socket = &inode->u.socket_i;
        if (socket->file != file) {
                fput(file);
                return -EFAULT;
        }
#else
        socket = sockfd_lookup(req.sock, &err);
        if ((err == -EBADF) || (socket == NULL) || (!socket))
                return err;
        file = socket->file;
#endif // if kernel < 2.5

        sock = (struct sock*)socket->sk;
        if (!sock) {
                fput(file);
                return -EFAULT;
        }

        if (SOCK_STATE(sock) == TCP_TIME_WAIT) {
                fput(file);
                return 0;
        }

#if LINUX_VERSION_CODE < VERSION_CODE(2,5,0)
        tp = &sock->tp_pinfo.af_tcp;
#else

        tp = tcp_sk(sock);
#endif // if kernel < 2.5

        timeout = req.msTimeout / (1000/HZ);

        while ((tp->write_seq - tp->snd_una) && (timeout > 0)) {
                set_current_state(TASK_INTERRUPTIBLE);
                timeout -= (10 - schedule_timeout(10));
        }

        set_current_state(TASK_RUNNING);
        fput(file);
        if (tp->write_seq - tp->snd_una)
                return -ETIME;
        return 0;
}


static void tcp_data_ready(struct sock *sk, int count)
{
        struct si_state *si = (struct si_state *)SOCK_DATA(sk);

        if (si && si->taskPtr)
                wake_up_process(si->taskPtr);
}


/*
 *  Sets the TCP parameters for the driver for one hub.  Then goes into a polling
 *  loop, reading/writing TCP packets.  Note that the task is interruptible when
 *  schedule() is called, and the TCP stack network callback can signal this function
 *  when a TCP packet for us is received (using tcp_data_ready()).
 *
 *  Called from the NSLinkctl device IOCTL.  The nslinkd daemon starts a process
 *  for each hub, which ends up calling this function as a TCP polling loop.
 */
static int set_tcp(struct tcp_mode_params *in)
{
        struct si_state *si;
        struct tcp_mode_params tcp_params;
        struct file *file = NULL;
        struct nr_port *info;
        int i;
#if LINUX_VERSION_CODE < VERSION_CODE(2,5,0)

        struct inode *inode;
#endif


        if (!driver_registered) {
                printk(KERN_INFO "nslink: Error -- attempt to register device before driver is registered (out of date daemon?)\n");
                return -EINVAL;
        }

        if (copy_from_user(&tcp_params, (void *) in, sizeof(struct tcp_mode_params))) {
                printk(KERN_INFO "nslink: set_tcp() - error in copy_from_user \n");
                return -EFAULT;
        }

        if (tcp_params.boxid >= numBoxes) {
                printk(KERN_INFO "nslink: set_tcp() - error boxid > max_nrp_sis \n");
                return -EINVAL;
        }

        si = conc_state[tcp_params.boxid];
        if (!si) {
                printk(KERN_INFO "nslink: set_tcp() - NULL si_state struct \n");
                return -EFAULT;
        }

#if LINUX_VERSION_CODE < VERSION_CODE(2,5,0)
        file = fget(tcp_params.sk);
        if (!file) {
                printk(KERN_INFO "nslink: set_tcp() - unable to fget tcp_params file \n");
                return -EFAULT;
        }

        inode = file->f_dentry->d_inode;
        if (!inode || !inode->i_sock) {
                fput(file);
                return -EFAULT;
        }

        si->tcp_sock = &inode->u.socket_i;
        if (si->tcp_sock->file != file) {
                fput(file);
                return -EFAULT;
        }
#else
        si->tcp_sock = sockfd_lookup(tcp_params.sk, &i);
        if ((i == -EBADF) || (si->tcp_sock == NULL) || (!si->tcp_sock)) {
                printk(KERN_INFO "nslink: set_tcp() - socketfd_lookup error (%d) \n", i);
                return -EFAULT;
        }
        file = si->tcp_sock->file;
#endif // if kernel < 2.5

        si->tcp_sk = si->tcp_sock->sk;
        if (!si->tcp_sk) {
                printk(KERN_INFO "nslink: set_tcp() - invalid tcp_sk \n");
                fput(file);
                return -EFAULT;
        }

        si->state = SI_STATE_CONNECTING;
        si->tcp_rbuf_size = tcp_params.rbuf_size;
        si->tcp_u_rbuf = tcp_params.rbuf;

#if LINUX_VERSION_CODE < VERSION_CODE(2,5,0)

        si->saddr = si->tcp_sk->saddr;
        si->sport = si->tcp_sk->num;
        si->daddr = si->tcp_sk->daddr;
#elif LINUX_VERSION_CODE < VERSION_CODE(2,6,33)
        si->saddr = inet_sk(si->tcp_sk)->rcv_saddr;
        si->sport = inet_sk(si->tcp_sk)->num;
        si->daddr = inet_sk(si->tcp_sk)->daddr;
#else
        si->saddr = inet_sk(si->tcp_sk)->inet_rcv_saddr;
        si->sport = inet_sk(si->tcp_sk)->inet_num;
        si->daddr = inet_sk(si->tcp_sk)->inet_daddr;
#endif // if kernel < 2.5

        si->tcp_k_rbuf = kmalloc(si->tcp_rbuf_size, GFP_KERNEL);
        if (!si->tcp_k_rbuf)
                si->tcp_sk = NULL;

        si->tcp_wbuf_size = 1400;
        si->tcp_k_wbuf = kmalloc(1400, GFP_KERNEL);

        if (!si->tcp_k_wbuf)
                si->tcp_sk = NULL;

        si->tcp_k_rfrag = kmalloc((si->tcp_rbuf_size + 2003), GFP_KERNEL);

        if (!si->tcp_k_rfrag)
                si->tcp_sk = NULL;

        si->tcp_last_port = -1;
        si->taskPtr = current;

        si->SOCK_DATA(tcp_sk) = si;
        si->SOCK_DATA_RDY(tcp_sk) = tcp_data_ready;

        /* TCP Loop */
        while (si->tcp_sk) {
                set_current_state(TASK_INTERRUPTIBLE);
                schedule_timeout(1);

                if (signal_pending(current)) {
                        printk(KERN_INFO "nslink: TCP connection to hub %d terminated by signal\n",si->conc_id);
                        break;
                }

#if LINUX_VERSION_CODE < VERSION_CODE(2,5,0)
                if ((si->tcp_sk == NULL) ||
                    (si->tcp_sock->state != SS_CONNECTED) ||
                    (si->SOCK_STATE(tcp_sk) != TCP_ESTABLISHED) ||
                    (si->SOCK_SHUTDOWN(tcp_sk) & SEND_SHUTDOWN) ||
                    (si->tcp_sk->saddr != si->saddr) ||
                    (si->tcp_sk->num != si->sport) || (si->tcp_sk->daddr != si->daddr)) {
                        printk(KERN_INFO "nslink: TCP connection to hub %d terminated by %s\n",si->conc_id,
                               si->tcp_sk == NULL ? "socket==NULL" :
                               si->tcp_sock->state != SS_CONNECTED ? "ss_connected==0" :
                               si->SOCK_SHUTDOWN(tcp_sk) & SEND_SHUTDOWN ? "send_shutdown==1" :
                               si->tcp_sk->saddr != si->saddr ? "bad saddr" :
                               si->tcp_sk->daddr != si->daddr ? "bad daddr" :
                               si->tcp_sk->num != si->sport ? "bad num" :
                               "?"
                              );
                        si->tcp_sk = NULL;
                        break;
                }
#else
                if ((si->tcp_sk == NULL) ||
                    (si->tcp_sock->state != SS_CONNECTED) ||
                    (si->SOCK_STATE(tcp_sk) != TCP_ESTABLISHED) ||
                    (si->SOCK_SHUTDOWN(tcp_sk) & SEND_SHUTDOWN) ||
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,33)
                    (inet_sk(si->tcp_sk)->rcv_saddr != si->saddr) ||
                    (inet_sk(si->tcp_sk)->num != si->sport) ||
                    (inet_sk(si->tcp_sk)->daddr != si->daddr)) {
#else
                (inet_sk(si->tcp_sk)->inet_rcv_saddr != si->saddr) ||
                (inet_sk(si->tcp_sk)->inet_num != si->sport) ||
                (inet_sk(si->tcp_sk)->inet_daddr != si->daddr)) {
#endif
                        printk(KERN_INFO "nslink: TCP connection to hub %d terminated by %s\n",si->conc_id,
                               si->tcp_sk == NULL ? "socket==NULL" :
                               si->tcp_sock->state != SS_CONNECTED ? "ss_connected==0" :
                               si->SOCK_SHUTDOWN(tcp_sk) & SEND_SHUTDOWN ? "send_shutdown==1" :
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,33)
                               inet_sk(si->tcp_sk)->rcv_saddr != si->saddr ? "bad saddr" :
                               inet_sk(si->tcp_sk)->daddr != si->daddr ? "bad daddr" :
                               inet_sk(si->tcp_sk)->num != si->sport ? "bad num" :
#else
                inet_sk(si->tcp_sk)->inet_rcv_saddr != si->saddr ? "bad saddr" :
                inet_sk(si->tcp_sk)->inet_daddr != si->daddr ? "bad daddr" :
                inet_sk(si->tcp_sk)->inet_num != si->sport ? "bad num" :
#endif
                               "?"
                              );
                        si->tcp_sk = NULL;
                        break;
                }
#endif // if kernel < 2.5

                tcp_check_rx(si);

                switch (si->state) {
                case SI_STATE_RESET:
                        // box timed out.  return and let daemon try to start it
                        si->tcp_sk = NULL;
                        break;

                case SI_STATE_CONNECTING:
                        /* Send assign request */
                        //printk(KERN_INFO "nslink: sending ASSIGN req\n");
                        si->tcp_k_wbuf[0] = RK_ASSIGN_REQ;
                        tcp_send_packet(si, 1);
                        si->tcp_timer = jiffies + (5 * HZ);
                        si->state = SI_STATE_CONNECTING_WAIT;
                        break;

                case SI_STATE_CONNECTING_WAIT:
                        if (IsPast(si->tcp_timer)) {
                                printk(KERN_INFO "nslink: hub timed out\n");
                                si->state = SI_STATE_RESET;
                        }
                        break;

                case SI_STATE_CONNECTED:
                        if (si->linkTimeoutJiffies) {
                                if (IsPast(si->keep_alive_tx_timer)) {
                                        si->need_to_send = 1;
                                        si->need_to_send_connect_check = 1;
                                        si->keep_alive_tx_timer += si->linkTimeoutJiffies/2;
                                }
                                if (IsPast(si->keep_alive_timeout)) {
                                        printk(KERN_INFO "nslink: tcp keepalive timeout on hub %d\n",si->conc_id);
                                        si->state = SI_STATE_RESET;
                                        si->tcp_sk = NULL;
                                        set_otherclose_all_tty(si);
                                        wake_all_tty(si);
                                }
                        }

                        if (si->tcp_sk) {
                                for (i=0; i < si->num_ports; ++i) {
                                        info = nrp_table[i + si->base_port];
                                        if (info && (info->magic == RPORT_MAGIC))
                                                send_recv_buf_to_ldisc(info);
                                }
                                if (si->need_to_send)
                                        tcp_send_remote_cmds(si);
                        }
                        break;
                }
        }

        if (si->tcp_k_rbuf) {
                kfree(si->tcp_k_rbuf);
                si->tcp_k_rbuf = NULL;
        }

        if (si->tcp_k_wbuf) {
                kfree(si->tcp_k_wbuf);
                si->tcp_k_wbuf = NULL;
        }

        if (si->tcp_k_rfrag) {
                kfree(si->tcp_k_rfrag);
                si->tcp_k_rfrag = NULL;
        }

        set_current_state(TASK_RUNNING);

        si->tcp_u_rbuf = NULL;
        si->saddr = 0;
        si->sport = 0;
        si->daddr = 0;
        si->tcp_sk = NULL;
        si->tcp_sock = NULL;
        si->state = SI_STATE_RESET;
        si->taskPtr = NULL;

        fput(file);
        return 0;
}

static int free_si_box(int boxid)
{
        struct si_state *si;
        struct nr_port *info;
        int i;

        if (boxid >= numBoxes || !conc_state[boxid])
                return -EINVAL;

        si = conc_state[boxid];

        for (i = 0; i < si->num_ports; i++) {
                info = nrp_table[i + si->base_port];
                if (!info)
                        continue;
                if (info->count || info->blocked_open)
                        return -EBUSY;
        }

        if (timer_pending(&si->timer))
                del_timer(&si->timer);

        conc_state[boxid] = 0;

        for (i = 0; i < si->num_ports; i++) {
                info = nrp_table[i + si->base_port];
                if (info) {
                        nrp_table[i + si->base_port] = 0;
                        kfree(info);
#if LINUX_VERSION_CODE > VERSION_CODE(2,5,0)
                        tty_unregister_device(nslink_driver, i+si->base_port);
#endif

                }
        }

        if (si->port_state)
                kfree(si->port_state);
        kfree(si);
        return 0;
}

/********************************************************************************/
/*                                                                              */
/*        UTILITY FUNCTIONS                                                     */
/*                                                                              */
/********************************************************************************/
static char *macToStr(unsigned char *mac)
{
        static char b[20];
        sprintf(b, "%02x:%02x:%02x:%02x:%02x:%02x",
                mac[0],
                mac[1],
                mac[2],
                mac[3],
                mac[4],
                mac[5]);
        return b;
}

static char *siStateStr(int i)
{
        switch (i) {
        case SI_STATE_CONNECTING:
        case SI_STATE_CONNECTING_WAIT:
                return "Connecting";
        case SI_STATE_CONNECTED:
                return "Connected";
        case SI_STATE_DEAD:
                return "Dead";
        case SI_STATE_RESET:
                return "Reset";
        case SI_STATE_BOOTING:
                return "Booting";
        case SI_STATE_BOOTED:
                return "Booted";
        }
        return "????";
}

static char *ipToStr(uint32_t ip)
{
        static char b[20];
        uint32_t addr = htonl(ip);

        sprintf(b,"%d.%d.%d.%d",
                (int)((addr>>24)&0xff),
                (int)((addr>>16)&0xff),
                (int)((addr>> 8)&0xff),
                (int)((addr>> 0)&0xff));
        return b;
}


// Wake up all tty's for a hub
static void wake_all_tty(struct si_state *si)
{
        struct nr_port *info;
        int i;

        for (i = 0; i < si->num_ports; i++) {
                info = nrp_table[i + si->base_port];
                if (info && info->tty)
                        wake_tty(info->tty);
        }
}

// Set the TTY_OTHER_CLOSED bit for all tty's for a hub
static void set_otherclose_all_tty(struct si_state *si)
{
        struct nr_port *info;
        int i;

        for (i = 0; i < si->num_ports; i++) {
                info = nrp_table[i + si->base_port];
                if (info && info->tty)
                        set_bit(TTY_OTHER_CLOSED, &info->tty->flags);
        }
}

// Clear the TTY_OTHER_CLOSED bit for all tty's for a hub
static void clr_otherclose_all_tty(struct si_state *si)
{
        struct nr_port *info;
        int i;

        for (i = 0; i < si->num_ports; i++) {
                info = nrp_table[i + si->base_port];
                if (info && info->tty)
                        clear_bit(TTY_OTHER_CLOSED, &info->tty->flags);
        }
}


/********************************************************************************/
/*                                                                              */
/*        /proc FUNCTIONS                                                       */
/*                                                                              */
/********************************************************************************/


#if NSLINK_DEBUG_TRACE_QUEUE

static char *queueEventString[] = {
        "none",
        "SYNC",
        "SEND",
        "FLSH",
        "QIN",
};

// Implements the /proc/driver/nslink/queue-trace method.
static int queue_proc_read_func(char *buf, char **start, off_t offset,
                                int count, int *eof, void *data)
{
        struct queueTraceStruct *p;
        int i;
        static unsigned readIndex;
        int cnt = 0;
        int len = 0;

        *eof = 0;

        if (offset == 0)
                readIndex = (queueTraceIndex+1)&NumQueueMask;

        while (len < (count-128)) {
                signed short s;
                i = (readIndex+offset+cnt)&NumQueueMask;
                if (i == queueTraceIndex) {
                        *eof = 1;
                        break;
                }
                p = queueTraceBuffer+i;
                s = p->pstate.nPutRemote - p->pstate.nGetRemote;
                len += sprintf(buf+len,"%d: %lu %-6s %4x  %04x %04x %d\n",
                               p->port,p->timestamp,
                               queueEventString[p->event],
                               p->data,
                               p->pstate.nPutRemote,
                               p->pstate.nGetRemote,
                               s);
                ++cnt;
        }
        // "If *start is set to a small integer value, the caller will use it to
        // increment filp->f_pos independently of the amount of data you return,
        // thus making f_pos an internal record number of your read_proc or
        // get_infoprocedure." - http://www.xml.com/ldd/chapter/book/ch04.html
        // Cast cnt to long first to avoid compiler warnings on x86_64 system.
        *start = (char*)(long)cnt;
        return len;
}
#endif


#if NSLINK_DEBUG_TRACE_HDLC

static char *stopString[] = {
        "none",
        "STOP-BADSEQ",
        "STOP-RETRANS",
        "STOP-BADACK",
};

static char *eventString[] = {
        "none",
        "RXACK",
        "RECV",
        "SEND",
        "TXACK",
        "RXBAD",
        "RSSTART",
        "RESND",
};

// Implements the /proc/driver/nslink/hdlc-trace method.
static int ack_proc_read_func(char *buf, char **start, off_t offset,
                              int count, int *eof, void *data)
{
        struct ackTraceStruct *p;
        int i;
        static unsigned readIndex;
        int cnt = 0;
        int len = 0;

        *eof = 0;

        if (offset == 0)
                readIndex = (ackTraceIndex+1)&NumAckMask;

        while (len < (count-128)) {
                i = (readIndex+offset+cnt)&NumAckMask;
                if (i == ackTraceIndex) {
                        *eof = 1;
                        break;
                }
                p = ackTraceBuffer+i;
                len += sprintf(buf+len,"%d: %lu %-6s %02x os=%02x ia=%02x ni=%02x c=%d ",
                               p->boxid,p->timestamp,
                               p->event < 0 ? stopString[p->event] : eventString[p->event],
                               p->data[0],
                               p->out_snd_index, p->in_ack_index,
                               p->next_in_index,p->rtqCount);
                if (p->head == -1)
                        len += sprintf(buf+len,"h=-- ");
                else
                        len += sprintf(buf+len,"h=%02x ",p->head&0xff);
                if (p->tail == -1)
                        len += sprintf(buf+len,"t=-- ");
                else
                        len += sprintf(buf+len,"t=%02x ",p->tail&0xff);
                len += sprintf(buf+len,"\n");
                ++cnt;
        }
        // "If *start is set to a small integer value, the caller will use it to
        // increment filp->f_pos independently of the amount of data you return,
        // thus making f_pos an internal record number of your read_proc or
        // get_infoprocedure." - http://www.xml.com/ldd/chapter/book/ch04.html
        // Cast cnt to long first to avoid compiler warnings on x86_64 system.
        *start = (char*)(long)cnt;
        return len;
}
#endif

// Implements the /proc/driver/nslink/port method.
static int port_proc_read_func(char *buf, char **start, off_t offset,
                               int count, int *eof, void *data)
{
        struct nr_port *info;
        struct si_port_state *pstate;
        int cnt = 0, len = 0, i;
        int allInfo = 0;

        while (len < (count - 512)) {
                i = offset + cnt;
                if (i == 0) {
                        len += sprintf(buf + len,"Port Open mode  Baud  Ctrl  FC    TxBytes    RxBytes  ParErr  FramErr  OverErr");
                        if (allInfo)
                                len += sprintf(buf + len,"  ctrl  mcr  err getl putr getr  cntr");
                        len += sprintf(buf + len," \n");
                }
                if (i >= (int)((sizeof nrp_table)/(sizeof nrp_table[0]))) {
                        *eof = 1;
                        break;
                }
                info = nrp_table[i];
                if (info) {
                        len += sprintf(buf + len,"%3d %4d   %02x %6d   ",
                                       i,
                                       info->count,
                                       info->rsmode,
                                       info->baud_rate);

                        // Ctrl ...
                        if (info->control_settings & SC_DATABITS_7)
                                len += sprintf(buf + len,"7");
                        else
                                len += sprintf(buf + len,"8");

                        if (info->control_settings & SC_PARITY_ODD)
                                len += sprintf(buf + len,"O");
                        else if (info->control_settings & SC_PARITY_EVEN)
                                len += sprintf(buf + len,"E");
                        else
                                len += sprintf(buf + len,"N");

                        if (info->control_settings & SC_STOPBITS_2)
                                len += sprintf(buf + len,"2  ");
                        else
                                len += sprintf(buf + len,"1  ");

                        // FC ...
                        if (info->control_settings & SC_FLOW_CTS_AUTO)
                                len += sprintf(buf + len,"HW ");
                        else if (info->control_settings & SC_FLOW_XON_TX_AUTO)
                                len += sprintf(buf + len,"SW ");
                        else
                                len += sprintf(buf + len,"-- ");

                        //  TX RX Errors
                        len += sprintf(buf + len,"  %8d   %8d  %6d   %6d   %6d  ",
                                       info->txBytes,
                                       info->rxBytes,
                                       info->parityErrors,
                                       info->framingErrors,
                                       info->overflowErrors);

                        if (allInfo) {
                                pstate = info->si->port_state+info->line;
                                len += sprintf(buf+len,"%04x %4x %4x %4x %5d",
                                               pstate->errorStatusRegister,
                                               pstate->nGetLocal,
                                               pstate->nPutRemote,
                                               pstate->nGetRemote,
                                               get_tx_cnt_remote(pstate));
                        }
                        len += sprintf(buf+len,"\n");
                }
                ++cnt;
        }
        // "If *start is set to a small integer value, the caller will use it to
        // increment filp->f_pos independently of the amount of data you return,
        // thus making f_pos an internal record number of your read_proc or
        // get_infoprocedure." - http://www.xml.com/ldd/chapter/book/ch04.html
        // Cast cnt to long first to avoid compiler warnings on x86_64 system.
        *start = (char*)(long)cnt;
        return len;
}


// Implements the /proc/driver/nslink/hdlc method.
static int hdlc_proc_read_func(char *buf, char **start, off_t offset,
                               int count, int *eof, void *data)
{
        int len = 0;
        struct sk_buff *h,*t;
        int j,cnt=0;
        struct si_state *p;

        while (len < (count-512)) {
                int i = offset+cnt;
                if (i >= numBoxes) {
                        *eof = 1;
                        break;
                }
                p = conc_state[i];
                if (!p)
                        len += sprintf(buf+len,"%d: xx:xx:xx:xx:xx:xx\n",i);
                else {
                        len += sprintf(buf+len,"%d: %02x:%02x:%02x:%02x:%02x:%02x\n",i,p->ether_addr[0],p->ether_addr[1],p->ether_addr[2],p->ether_addr[3],p->ether_addr[4],p->ether_addr[5]);
                        len += sprintf(buf+len," state %08x\n",p->hdlc_state_flags);
                        len += sprintf(buf+len," out_snd_index %02x\n",p->out_snd_index);
                        len += sprintf(buf+len," in_ack_index %02x\n",p->in_ack_index);
                        len += sprintf(buf+len," next_in_index %02x\n",p->next_in_index);
                        len += sprintf(buf+len," rtqCount %d\n", p->rtqCount);
                        h = skb_peek(&p->rtq);
                        t = skb_peek_tail(&p->rtq);
                        for (j=0; j<16 && h; ++j) {
                                len += sprintf(buf+len,"  %02x\n",((struct HDLChdr*)(skb_transport_header(h)+3))->out_snd_index);

                                if (!t || t==h)
                                        break;
                                h = h->next;
                        }
                }
                ++cnt;
        }
        // "If *start is set to a small integer value, the caller will use it to
        // increment filp->f_pos independently of the amount of data you return,
        // thus making f_pos an internal record number of your read_proc or
        // get_infoprocedure." - http://www.xml.com/ldd/chapter/book/ch04.html
        // Cast cnt to long first to avoid compiler warnings on x86_64 system.
        *start = (char*)(long)cnt;
        return len;
}


// Implements the /proc/driver/nslink/hdlc-queue method.
static int hdlc_queue_proc_read_func(char *buf, char **start, off_t offset,
                                     int count, int *eof, void *data)
{
        int len = 0;
        struct sk_buff *skb, *tail, *head=NULL;
        unsigned long flags;
        struct si_state *si = conc_state[0];

        // grab a copy of retransmit queue

        spin_lock_irqsave(&si->rtq.lock, flags);
        skb = skb_peek(&si->rtq);
        if (skb) {
                tail = head = skb_clone(skb, GFP_ATOMIC);
                while (tail && (skb->next != (struct sk_buff *)(&si->rtq))) {
                        tail->next = skb_clone(skb->next, GFP_ATOMIC);
                        if (tail->next == NULL) {
                                printk(KERN_INFO "nslink: skb_clone failed!\n");
                                break;
                        }
                        tail = tail->next;
                        skb = skb->next;
                }
                tail->next = NULL;
        }
        spin_unlock_irqrestore(&si->rtq.lock, flags);

        if (head) {
                len += sprintf(buf+len, "sq  len tlrm\n");
                skb = head;
                while (skb) {
                        struct sk_buff *next;
                        len += sprintf(buf+len,"%02x ", skb->head[20]);
                        len += sprintf(buf+len,"%4u ", skb->len);
                        len += sprintf(buf+len,"%4u\n", skb_tailroom(skb));
                        next = skb->next;
                        skb->next = NULL;
                        kfree_skb(skb);
                        skb = next;
                }
        }

        *start = (char*)(long)0;
        *eof = 1;
        return len;
}


// Implements the /proc/driver/nslink/status method.
static int proc_read_func(char *buf, char **start, off_t offset,
                          int count, int *eof, void *data)
{
        int i,n,len;
        struct si_state *si;
#define LIMIT (PAGE_SIZE-81)

        len = 0;

        len += sprintf(buf+len, "Comtrol NS-Link -- version %s (%s)\n",SI_VERSION,SI_DATE);
        len += sprintf(buf+len, "Built %s  %s\n\n",__DATE__,__TIME__);
        len += sprintf(buf+len, "Hub State        Address            Ports\n");
        len += sprintf(buf+len, "--- ------------ ------------------ -----\n");

        n=0;

        for (i=0; i<numBoxes; ++i) {
                if (len > (int)LIMIT) {
                        len += sprintf(buf+len,"[output truncated]\n");
                        break;
                }

                si = conc_state[i];

                if (!si)
                        continue;

                len += sprintf(buf+len, "%3d %-12s",i,siStateStr(si->state));
                len += sprintf(buf+len, " %-18s",si->use_tcp ? ipToStr(si->ip_addr) : macToStr(si->ether_addr));
                len += sprintf(buf+len, " %2d-%-2d",si->base_port,si->base_port+si->num_ports-1);

                len += sprintf(buf+len, "\n");

                ++n;
        }

        len += sprintf(buf+len, "\n%d hub%s running.\n",n,n==1?"":"s");
        *eof = (len <= (int)LIMIT);
        // len += sprintf(buf+len, "[%d]\n",len);
        return len;
}

/********************************************************************************/
/*                                                                              */
/*        MODULE INIT/CLEANUP FUNCTIONS                                         */
/*                                                                              */
/********************************************************************************/

#ifdef MODULE

#if LINUX_VERSION_CODE < VERSION_CODE(2,4,99)
static int can_unload(void)
{
        if (atomic_read(&nrp_num_ports_open)) {
                return 1;
        } else {
                return 0;
        }
}
#endif

#if LINUX_VERSION_CODE >= VERSION_CODE(2,6,30)
static void nrp_line_info(struct seq_file *m, struct si_state *si, int line)
{
        struct si_port_state *pstate;
        struct nr_port *info;
        unsigned int control, status;

        seq_printf(m, "%d: ", line + si->base_port);

        info = nrp_table[line + si->base_port];
        pstate = si->port_state + line;

        if (info && info->baud_rate)
                seq_printf(m, " baud:%d", info->baud_rate);

        /*
         * Last thing is the RS-232 status lines
         */
        status = pstate->modemStatusRegister;
        control = info ? info->modem_control_register : 0;
        if (control & MCR_DTR_SET_MASK)
                seq_puts(m, "|RTS");
        if (status & MSR_CTS_ON)
                seq_puts(m, "|CTS");
        if (control & MCR_DTR_SET_MASK)
                seq_puts(m, "|DTR");
        if (status & MSR_DSR_ON)
                seq_puts(m, "|DSR");
        if (status & MSR_CD_ON)
                seq_puts(m, "|CD");
        if (status & MSR_TX_FLOWED_OFF)
                seq_puts(m, "|TXOFF");
        if (status & MSR_RING_ON)
                seq_puts(m, "|RING");
        seq_puts(m, "\n");
}

static int nrp_proc_show(struct seq_file *m, void *v)
{
        struct si_state *si;
        int i, j;

        seq_printf(m, "serinfo:1.0 driver:%s\n", SI_VERSION);
        for (i = 0; i < numBoxes; i++) {
                si = conc_state[i];
                if (!si)
                        continue;
                for (j = 0; j < si->num_ports; j++) {
                        nrp_line_info(m, si, j);
                }
        }
        return 0;
}

static int nrp_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, nrp_proc_show, NULL);
}

static const struct file_operations nrp_proc_fops = {
        .owner		= THIS_MODULE,
        .open		= nrp_proc_open,
        .read		= seq_read,
        .llseek		= seq_lseek,
        .release	= single_release,
};
#endif	// kernel >= 2,6,30

#if LINUX_VERSION_CODE > VERSION_CODE(2,4,99)
static struct tty_operations nslink_ops = {
        .open = nrp_open,
        .close = nrp_close,
        .write = nrp_write,
        .put_char = nrp_put_char,
        .write_room = nrp_write_room,
        .chars_in_buffer = nrp_chars_in_buffer,
        .flush_buffer = nrp_flush_buffer,
        .ioctl = nrp_ioctl,
        .throttle = nrp_throttle,
        .unthrottle = nrp_unthrottle,
        .set_termios = nrp_set_termios,
        .stop = nrp_stop,
        .start = nrp_start,
        .hangup = nrp_hangup,
        .break_ctl = nrp_break,
        .send_xchar = nrp_send_xchar,
        .wait_until_sent = nrp_wait_until_sent,
        .tiocmget = nrp_tiocmget,
        .tiocmset = nrp_tiocmset,
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,30)
        .read_proc = nrp_read_proc,
#else	// implies kernel >= 2,6,30
        .proc_fops = &nrp_proc_fops,
#endif	// kernel < 2,6,30
};
#endif

int register_tty_driver(tBoxPortCount *userbp)
{
        int retval;
        tBoxPortCount bp;

        if (copy_from_user(&bp, userbp, sizeof bp))
                return -EFAULT;

        numBoxes = bp.boxCount;

        // Set up the tty driver structure and then register this
        // driver with the tty layer.
        nslink_driver = alloc_tty_driver((MAX_NRP_TOTAL_PORTS - 1));

        nslink_driver->magic = TTY_DRIVER_MAGIC;
        nslink_driver->driver_name = "NS-Link";
        nslink_driver->name = "ttySI";
        nslink_driver->num = bp.portCount;
        nslink_driver->major = 0; // dynamically allocated major number
        nslink_driver->minor_start = 0;
        nslink_driver->type = TTY_DRIVER_TYPE_SERIAL;
        nslink_driver->subtype = SERIAL_TYPE_NORMAL;
        nslink_driver->init_termios = tty_std_termios;
        nslink_driver->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
#ifdef KTERMIOS_NEW
        nslink_driver->init_termios.c_ispeed = 9600;
        nslink_driver->init_termios.c_ospeed = 9600;
#endif
#if defined(TTY_DRIVER_NO_DEVFS)

        nslink_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_NO_DEVFS;
#elif defined(TTY_DRIVER_DYNAMIC_DEV)

        nslink_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
#else
# error "Ouch.  Somebody pulled the rug out from under us: don't know how to register dynamic serial devices"
#endif

#if (LINUX_VERSION_CODE > VERSION_CODE(2,4,99)) && (LINUX_VERSION_CODE < VERSION_CODE(2,6,18))

        nslink_driver->devfs_name = "tts/SI";
#endif

#if LINUX_VERSION_CODE < VERSION_CODE(2,5,0)

        nslink_driver->refcount = &nslink_refcount;
        nslink_driver->table = nslink_table;
        nslink_driver->termios = nslink_termios;
        nslink_driver->termios_locked = nslink_termios_locked;
        nslink_driver->open = nrp_open;
        nslink_driver->close = nrp_close;
        nslink_driver->write = nrp_write;
        nslink_driver->put_char = nrp_put_char;
        nslink_driver->flush_chars = nrp_flush_chars;
        nslink_driver->write_room = nrp_write_room;
        nslink_driver->chars_in_buffer = nrp_chars_in_buffer;
        nslink_driver->flush_buffer = nrp_flush_buffer;
        nslink_driver->ioctl = nrp_ioctl;
        nslink_driver->throttle = nrp_throttle;
        nslink_driver->unthrottle = nrp_unthrottle;
        nslink_driver->set_termios = nrp_set_termios;
        nslink_driver->stop = nrp_stop;
        nslink_driver->start = nrp_start;
        nslink_driver->hangup = nrp_hangup;
        nslink_driver->break_ctl = nrp_break;
        nslink_driver->read_proc = nrp_read_proc;
        nslink_driver->send_xchar = nrp_send_xchar;
        nslink_driver->wait_until_sent = nrp_wait_until_sent;
#endif

#if (LINUX_VERSION_CODE > VERSION_CODE(2,5,0))
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,26)
        nslink_driver->tiocmget = nrp_tiocmget;
        nslink_driver->tiocmset = nrp_tiocmset;
#endif
        nslink_driver->owner = THIS_MODULE;
        tty_set_operations(nslink_driver, &nslink_ops);
#endif /* Kernel > 2.5 */

        retval = tty_register_driver(nslink_driver);
        if (retval < 0) {
                printk(KERN_INFO "Couldn't install tty NS-Link driver (error %d)\n", -retval);
#if LINUX_VERSION_CODE >= VERSION_CODE(2,6,0)
                put_tty_driver(nslink_driver);
#endif
                return retval;
        }
        driver_registered = 1;
        printk(KERN_INFO "NS-Link driver is major %d\n", nslink_driver->major);
        return 0;
}



#if LINUX_VERSION_CODE < VERSION_CODE(2,5,0)
int init_module(void)
#else
int nrp_init(void)
#endif
{
        static struct proc_dir_entry *procdir;

        printk(KERN_INFO "nslink: NS-Link driver, version %s %s\n",SI_VERSION, SI_DATE);
        printk(KERN_INFO "nslink: Built %s %s\n", __DATE__, __TIME__);

#if LINUX_VERSION_CODE < VERSION_CODE(2,5,0)

        __this_module.can_unload = can_unload;
#endif

        // Initialize the array of pointers to our own internal state structures.
        memset(nrp_table, 0, sizeof(nrp_table));

        // Register the control driver NSLinkctl, used by the daemon to talk to the driver
        control_major = register_chrdev(0, ctl_name, &ctl_fops);
        if (control_major < 0) {
                printk(KERN_INFO "nslink: Failed to register the control registry.\n");
                return (control_major);
        }

        // Create the /proc/driver/nslink entries.
        procdir = proc_mkdir("driver/nslink",NULL);
        if (procdir) {
#if LINUX_VERSION_CODE < VERSION_CODE(2,6,30)
                procdir->owner = THIS_MODULE;
#endif
                create_proc_read_entry("status", 0, procdir, proc_read_func, NULL);
                create_proc_read_entry("hdlc", 0, procdir, hdlc_proc_read_func, NULL);
                create_proc_read_entry("ports", 0, procdir, port_proc_read_func, NULL);
                create_proc_read_entry("txqueue", 0, procdir, hdlc_queue_proc_read_func, NULL);
#if NSLINK_DEBUG_TRACE_HDLC
                create_proc_read_entry("hdlc-trace", 0, procdir, ack_proc_read_func, NULL);
#endif
#if NSLINK_DEBUG_TRACE_QUEUE
                create_proc_read_entry("queue-trace", 0, procdir, queue_proc_read_func, NULL);
#endif

        }

        // Add the protocol handler for 0x11FE packets to the networking stack
        dev_add_pack(&comtrol_packet_type);

        memset(&conc_state, 0, sizeof(conc_state));
        return 0;
}

#if LINUX_VERSION_CODE < VERSION_CODE(2,5,0)
void cleanup_module(void)
#else
static void nrp_cleanup_module(void)
#endif
{
        int retval, i;

        remove_proc_entry("driver/nslink/status",0);
        remove_proc_entry("driver/nslink/hdlc",0);
        remove_proc_entry("driver/nslink/ports",0);
        remove_proc_entry("driver/nslink/txqueue",0);
#if NSLINK_DEBUG_TRACE_HDLC
        remove_proc_entry("driver/nslink/hdlc-trace",0);
#endif
#if NSLINK_DEBUG_TRACE_QUEUE
        remove_proc_entry("driver/nslink/queue-trace",0);
#endif
        remove_proc_entry("driver/nslink",0);

        dev_remove_pack(&comtrol_packet_type);

        if (control_major > 0) {
#if LINUX_VERSION_CODE >= VERSION_CODE(2,6,23)
                unregister_chrdev(control_major, ctl_name);
#else

                retval = unregister_chrdev(control_major, ctl_name);
                if (retval)
                        printk(KERN_INFO "nslink: Error %d while trying to unregister "
                               "control device.\n", -retval);
                else
#endif

                control_major = 0;
        }

        for (i = 0; i < numBoxes; i++)
                free_si_box(i);

        if (driver_registered) {
                retval = tty_unregister_driver(nslink_driver);
#if LINUX_VERSION_CODE >= VERSION_CODE(2,6,0)
                put_tty_driver(nslink_driver);
#endif
                if (retval)
                        printk(KERN_INFO "nslink: Error %d while trying to unregister Nslink driver\n", -retval);
        }

        return;
}

/*
 *  Returns 1 if the device is an RS232 device only (DeviceMaster Serial Hub),
 *  else 0.
 */
static int UnitHasNo485(int modelID)
{
        switch (modelID) {
        case 5002115:
        case 5002315:
                return 1;
        default:
                return 0;
        }
}

/*
 *  Returns 1 if the device supports 485 full duplex mode,
 *  else 0.
 */
static int UnitHas485FD(int modelID)
{
        switch (modelID) {
        case 5002525:           // DM RTS 2P Configuration 1E
        case 5002535:           // DM RTS 2P Configuration 2E
        case 5002595:           // DM RTS 2P Configuration 1E DB9
        case 5002600:           // DM RTS 2P Configuration 2E DB9
                return 1;
        default:
                return 0;
        }
}

static int isDM500(int modelID)
{
        switch (modelID) {
        case 5002363:	// 1P boxed
        case 5002362:	// 1P embedded
        case 5002117:	// 4P boxed
                return 1;
        default:
                return 0;
        }
}

#endif /* MODULE */
