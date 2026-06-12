/*
 *  NS-Link support daemon for Comtrol SI/IA & DeviceMaster products.
 *
 *  Copyright (C) Comtrol, Inc.
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <syslog.h>
#include <grp.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/if.h>

#include "nslink.h"
#include "nslinkd.h"
#include "version.h"

char *dfl_boot_file4_8 = "/etc/rpshsi.bin";
char *dfl_boot_file2 = "/etc/rpshsi2p.bin";
const char *not_loaded = "RPSH-Si driver not loaded?";
const char *pid_file = "/var/run/nslinkd.pid";

static pid_t child_pids[MAX_NRP_SIS];
static pid_t parent_pid;
static int child_socket;

char *mactoa(unsigned char *m);
char *iptoa(unsigned int addr);

static int get_major_number(const char *name)
{
        FILE *f;
        char buf[80], devname[80];
        int n, major = -1;

        f = fopen("/proc/devices", "r");
        if (!f)
                return -1;
        while (!feof(f)) {
                if (!fgets(buf, sizeof(buf), f))
                        break;
                if (sscanf(buf, "%d %s", &n, devname) != 2)
                        continue;
                if (!strcmp(devname, name)) {
                        major = n;
                        break;
                }
        }
        fclose(f);
        return major;
}

// return a random, unused temporary device name

static char *myTempName(void)
{
        int tries;
        static char path[256];
        struct stat statbuf;

        srandom(time(NULL));
        for (tries=0; tries<10; ++tries) {
                sprintf(path,"/dev/nslink.%08lx.%08lx",(long)time(NULL),(long)random());
                if (stat(path,&statbuf))
                        return path;
        }
        return NULL;
}

static int open_dev(dev_t dev, int mode)
{
        char *fn;
        int fd;

        if ((fn = myTempName()) == NULL) {
                fprintf(stderr,"myTempName() failed\n");
                return -1;
        }
        if (mknod(fn, S_IFCHR | 0600, dev) != 0) {
                fprintf(stderr, "mknod(\"%s\", %04x, %lu) failed\n", fn, S_IFCHR | 0600, (unsigned long)dev);
                perror("mknod");
                return -1;
        }
        fd = open(fn, mode);
        if (fd < 0) {
                fprintf(stderr,"open(\"%s\",%d) failed\n",fn,mode);
                perror("open");
        }
        unlink(fn);
        return fd;
}

int config_si(int fd,   struct daemon_si_box_config *config, int boxid)
{
        struct si_box_config cfg;
        int ret, i;

        //syslog(LOG_INFO, "config_si(fd=%d, boxid=%d, ...)\n",fd,boxid);  /* debug */

        memset(&cfg, 0, sizeof cfg);
        cfg.boxid = boxid;
        cfg.base_port = config->base_port;
        cfg.num_ports = config->num_lines;
        memcpy(cfg.ether_addr, config->ether_addr, 6);
        strcpy(cfg.interface, config->interface);
        cfg.ip_addr = config->ip_addr;
        cfg.link_timeout = config->link_timeout;
        cfg.scan_period = config->scan_period;
        cfg.rexmit_timer = config->rexmit_timer;
        cfg.arch_id = config->arch_id;
        cfg.ModelId = config->ModelId;
        for (i=0; i<MAX_NRP_PORTS; ++i) {
                cfg.rsmode[i] = config->rsmode[i];
                cfg.low_latency[i] = config->low_latency[i];
                cfg.inter_char_timeout[i] = config->inter_char_timeout[i];
                cfg.rx_fifo_disable[i] = config->rx_fifo_disable[i];
        }
        //syslog(LOG_INFO,"boxid %d link timeout = %d seconds\n",boxid,config->link_timeout);  /* debug */
        ret = ioctl(fd, SI_SET_BOX_CONFIG, &cfg);
        if (ret) {
                syslog(LOG_ERR, "Couldn't config NS-Link box #%d (errno %d)",boxid,errno);
                return errno;
        }
        //syslog(LOG_INFO,"config_si return\n");  /* debug */
        return 0;
}


int become_daemon()
{
        int child_pid, fd;

        child_pid = fork();
        if (child_pid == -1)
                return -1;
        if (child_pid)
                exit(0);
        (void) chdir("/");
        fd = open("/dev/null", O_RDWR, 0);
        if (fd >= 0) {
                (void) dup2(fd, 0);
                (void) dup2(fd, 1);
                (void) dup2(fd, 2);
                if (fd > 2)
                        (void) close(fd);
        }
        return (0);
}

void check_pid_file()
{
        FILE *f;
        int pid;

        if (access(pid_file, R_OK) == 0) {
                f = fopen(pid_file, "r");
                if (f) {
                        pid = 0;
                        fscanf(f, "%d", &pid);
                        if (pid && !kill(pid, 0)) {
                                fprintf(stderr, "NS-Link daemon already running!\n");
                                syslog(LOG_ERR, "NS-Link daemon already running");
                                exit(1);
                        }
                        fclose(f);
                }
        }
}

void put_pid_file()
{
        FILE *f;

        f = fopen(pid_file, "w");
        if (!f) {
                syslog(LOG_WARNING, "Warning: couldn't write pid file %s", pid_file);
                return;
        }
        parent_pid = getpid();
        fprintf(f, "%d\n", parent_pid);
        fclose(f);
}

/*
 * Create a device if it doesn't exist and have the correct device
 * number or major/minor number.
 */
#define MKDEV(a,b) ((int)((((a) & 0xff) << 8) | ((b) & 0xff)))


void create_device(const char *prefix, int major, int num, gid_t def_group)
{
        char name[80];
        struct stat st;
        dev_t device;
        mode_t mode = 0620;  // default file protection
        uid_t  owner = 0;    // default owner
        gid_t  group = def_group;

        sprintf(name, "%s%d", prefix, num);
        // syslog(LOG_INFO, "Device created %s\n", name);
        device = MKDEV(major, num);
        if (lstat(name, &st) == 0) {
                if ((st.st_rdev == device) && S_ISCHR(st.st_mode))
                        return;
#if 0

                syslog(LOG_INFO, "%s has dev %d, mode %o wanted %d",
                       name, st.st_rdev, st.st_mode, device);
#endif
                // device exists, but doesn't have right major/minor numbers,
                // so remember owner/group/mode so we can create new device
                // with same properties as old one except w/ right major/minor.

                mode = st.st_mode & 0777;
                owner = st.st_uid;
                group = st.st_gid;

                (void) unlink(name);
        }

        umask(0);

        if (mknod(name, S_IFCHR | mode, device) < 0) {
                perror(name);
                syslog(LOG_ERR, "Couldn't create device %s: %m", name);
        }
        if (chown(name, owner, group) < 0) {
                perror(name);
                syslog(LOG_ERR, "Couldn't chmod device %s: %m", name);
        }
}

void unlink_device(const char *prefix, int num)
{
        char name[80];
        sprintf(name,"%s%d",prefix,num);
        unlink(name);
}


/*
 * This function creates all of the devices
 */
void create_devices(int ctlfd)
{
        int i, num_ports;
        struct group *grp;
        gid_t tty_group = 0;
        int major, callout_major;
        tBoxPortCount bp;

        num_ports = 0;
        for (i = 0; i < num_box_config; i++)
                num_ports += box_config[i].num_lines;

        syslog(LOG_INFO, "Registering %d devices, %d ports\n", num_box_config, num_ports);

        bp.boxCount = num_box_config;
        bp.portCount = num_ports;

        if (ioctl(ctlfd,SI_REGISTER_PORTS,&bp))
                syslog(LOG_ERR, "Couldn't register ports\n");

        syslog(LOG_INFO, "Creating devices\n");

        major = get_major_number("ttySI");
        callout_major = get_major_number("cuSI");

        if (major == 0 && callout_major == 0)
                return;

        grp = getgrnam("tty");
        if (grp)
                tty_group = grp->gr_gid;

        for (i=0; i<num_ports; ++i) {
                if (major)
                        create_device("/dev/ttySI", major, i, tty_group);
                if (callout_major)
                        create_device("/dev/cuSI", callout_major, i, tty_group);
        }
}

void sigterm_handler()
{
        int i, rc;

        if (parent_pid == getpid()) {
                for (i = 0; i < num_box_config; i++)
                        if (child_pids[i])
                                kill(child_pids[i], SIGTERM);
                syslog(LOG_NOTICE, "Exiting");
                (void) unlink(pid_file);
        } else {
                close(child_socket);
        }
        exit(0);
}

void sighup_handler()
{
        signal(SIGHUP, sighup_handler);
        return;
}

int socketReadTimeout(int socket, void *buf, int count, unsigned usTimeout)
{
        fd_set r,e;
        struct timeval tv;
        int s;

        FD_ZERO(&r);
        FD_ZERO(&e);
        FD_SET(socket,&r);
        FD_SET(socket,&e);
        tv.tv_sec = usTimeout/1000000;
        tv.tv_usec = usTimeout % 1000000;

        s = select(socket+1,&r,NULL,&e,&tv);

        if (s==0)
                return 0;

        if (!FD_ISSET(socket,&r))
                return 0;

        return read(socket,buf,count);
}


int packetReadTimeout(int socket, unsigned char *macAddr, unsigned char index, void *b, int count,  unsigned usTimeout)
{
        unsigned char *buf = b;
        fd_set r,e;
        struct timeval tv;
        int s;
        int n;


        FD_ZERO(&r);
        FD_ZERO(&e);
        FD_SET(socket,&r);
        FD_SET(socket,&e);
        tv.tv_sec = usTimeout/1000000;
        tv.tv_usec = usTimeout % 1000000;

        while (1) {
                s = select(socket+1,&r,NULL,&e,&tv);

                if (s==0)
                        return 0;

                if (!FD_ISSET(socket,&r))
                        return 0;

                n = read(socket,buf,count);

                if (memcmp(buf+6,macAddr,6))
                        continue;  // not from the right box: count on fact that tv is time remaining

                if (buf[15] != index)
                        continue;  // not intended for daemon

                return n;
        }
}

typedef struct {
        uint32_t macLoadAddress;
        uint32_t tcpLoadAddress;
        uint32_t startAddress;
        char *bootFile;
}
tArchData;

// Whoever decided to use multiple formats to denote the EXACT
// SAME ADDRESS in the 8086 hub has some explaining to do... ;)

tArchData archData[] = {
        {0,0,0,NULL},                                       // not used
        {0x00010000, 0x10000000, 0x10000000, "/etc/rpshsi.bin"},      // 8086 hubs
        {0x00100000, 0x00100000, 0x00100000, "/etc/rpshsi2p.bin"},    // original 2-port ARM
        {0x00000000, 0x00000000, 0x00000000, "/etc/devmast.bin"}     // deviceMaster
};

unsigned short IntelOrderS(unsigned short v)
{
        unsigned char t;
        union {
                unsigned short s;
                unsigned char  b[2];
        } u;

        u.s = htons(v);
        t = u.b[1];
        u.b[1] = u.b[0];
        u.b[0] = t;
        return u.s;
}

uint32_t IntelOrderL(uint32_t v)
{
        union {
                uint32_t l;
                unsigned char  b[4];
        } in,out;

        in.l = htonl(v);

        out.b[0] = in.b[3];
        out.b[1] = in.b[2];
        out.b[2] = in.b[1];
        out.b[3] = in.b[0];

        return out.l;
}


uint32_t GoofyOrderL(uint32_t v)
{
        union {
                uint32_t l;
                unsigned char  b[4];
        } in,out;

        in.l = htonl(v);

        out.b[0] = in.b[1];
        out.b[1] = in.b[0];
        out.b[2] = in.b[3];
        out.b[3] = in.b[2];

        return out.l;
}

struct LoadRequest {
        tToolRequest tr;
        unsigned char lc[1460 - sizeof (tToolRequest)];
}
__attribute__((packed));
typedef struct LoadRequest tLoadRequest;



// Create a TCP socket and connect it to the specified hub.
// Download devmast binary if needed.
// Once devmast is running, return the connected socket.
// Return -1 on error.
//
// All commands have to go out in their own Ethernet packets.
//
// That means you have to delay between write() calls on the
// socket so that the network stack has time to send out the
// previous command.  Othewise the stack might concatenate the
// two write()s into a single Ethernet frame.
//
// This is a bug in the protocol design.  We are trying to use
// a stream service as a datagram service.  This is bad. We
// shouldn't do this.

int downloadTcp(int nslinkfd, unsigned ip_addr, struct daemon_si_box_config *config)
{
        unsigned char tcpBuf[sizeof (tLoadRequest)];
        tToolResponse *tr = (tToolResponse *)tcpBuf;
        tToolRequest  *trq = (tToolRequest *)tcpBuf;
        tLoadRequest  *lcreq = (tLoadRequest *)tcpBuf;
        struct sockaddr_in sockaddr;
        int sock, fd, n, ldAddr, archId;
        unsigned char ldIndex;
        tArchData *a;
        tTcpTxQueueWait tqwait;
        int needsDownload = 0;
        unsigned connectFailCount = 0;

        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons(ETH_P_COMTROL);
        sockaddr.sin_addr.s_addr = ip_addr;

        sock = -1;

        tqwait.msTimeout = 2000;

        while (1) {
                // open connection to hub and send ID request

                sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

                if (sock < 0) {
                        syslog(LOG_ERR, "Could not create socket for NS-Link device.");
                        exit(1);
                }

                if (connect(sock, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_in)) < 0) {
                        ++connectFailCount;
                        if ((connectFailCount) < 5 || (connectFailCount % 20 == 0))
                                syslog(LOG_ERR, "Could not connect to NS-Link device at %s",inet_ntoa(sockaddr.sin_addr));
                        sleep(1);
                        if (connectFailCount > 5)
                                sleep(5);
                        continue;
                }

                connectFailCount = 0;

                tqwait.sock = sock;

                trq->cmd = ID_TOOLREQ;
                trq->index = 0;
                trq->length = IntelOrderS(sizeof *trq);
                trq->addr = 0;

                write(sock,trq,sizeof *trq);

                // read and parse ID response

                if ((n=socketReadTimeout(sock, tcpBuf, sizeof tcpBuf, 200000)) <= 0) {
                        syslog(LOG_ERR, "TCP ID request timeout for %s",inet_ntoa(sockaddr.sin_addr));
                        close(sock);
                        sleep(1);
                        continue;
                }

                if (n < (int)sizeof(tOldToolResponse)) {
                        syslog(LOG_ERR, "TCP ID response too short for %s",inet_ntoa(sockaddr.sin_addr));
                        close(sock);
                        sleep(1);
                        continue;
                }

                if (n >= (int)sizeof(tToolResponse)) {
                        tr->ProgramId = ntohs(tr->ProgramId);
                        tr->ModelId   = ntohl(tr->ModelId);
                        syslog(LOG_INFO, "TCP ID response from %s '%s' %s Model=%u Arch=%d NumPorts=%d ProgId=%d BoardRev=%d",
                               mactoa(tr->MacAddr), tr->Version, iptoa(tr->IpAddr),
                               (unsigned int)tr->ModelId, tr->ArchId, tr->NumPorts, tr->ProgramId, tr->BoardRev);
                } else {
                        // must be old box!
                        syslog(LOG_INFO, "TCP ID response from %s %s %08x",
                               mactoa(tr->MacAddr), iptoa(tr->IpAddr),
                               (unsigned int)tr->AvailMemAddr);
                        // we have to look at memory values to figure out who we're talking to :(
                        if (tr->MacAddr[3] == 0x10 || tr->MacAddr[3] == 0x11) {
                                tr->ArchId = 2;         // old ARM 2-port or IA
                                if (tr->AvailMemAddr == 0x1000)
                                        tr->ProgramId = 0;      // bootloader
                                else
                                        tr->ProgramId = 1;      // RPSH-Si app
                        } else if (tr->MacAddr[3] == 0x06) {
                                tr->ArchId = 1;         // 8086
                                if (tr->AvailMemAddr == 0x10000000)
                                        tr->ProgramId = 0;      // bootloader
                                else
                                        tr->ProgramId = 1;      // RPSH-Si app
                        } else {
                                tr->ArchId = 0;         // don't know
                                tr->ProgramId = 0xff;   // not a bootloader
                        }
                }

                config->arch_id = tr->ArchId;
                config->ModelId = tr->ModelId;

                // if right program is running, return socket
                if (tr->ProgramId == 1)
                        return sock;

                // if wrong program is running, set flag noting that we need to
                // download, and send reset

                if (tr->ProgramId != 0) {
                        needsDownload = 1;
                        syslog(LOG_INFO, "Resetting device at %s\n", iptoa(tr->IpAddr));
                        trq->cmd = RESET_TOOLREQ;
                        trq->index = 0;
                        trq->length = IntelOrderS(sizeof *trq);
                        trq->addr = 0;
                        write(sock,trq,sizeof *trq);
                        if (ioctl(nslinkfd,SI_TCP_TXQUEUE_WAIT,&tqwait))
                                syslog(LOG_ERR,"txqueue_wait %s",strerror(errno));
                        close(sock);
                        sleep(4);
                        continue;
                }

                // if redboot is running, but we haven't seen app, then try again later
                if (tr->ProgramId == 0 && !needsDownload) {
                        close(sock);
                        sleep(5);
                        continue;
                }

                // if we get to here, then we've decide that we need to
                // download, and redboot is running, so do the download

                if (tr->ArchId <= 0 || tr->ArchId >= ((sizeof archData)/(sizeof archData[0]))) {
                        syslog(LOG_ERR, "Unknown architecture ID: %d", tr->ArchId);
                        close(sock);
                        return -1;
                }

                archId = tr->ArchId;

                a = archData + archId;
                fd = open(a->bootFile,O_RDONLY);

                if (fd < 0) {
                        syslog(LOG_ERR,"Can't open boot file %s", a->bootFile);
                        close(sock);
                        return -1;
                }

                syslog(LOG_INFO,"Downloading %s to device at %s\n", a->bootFile, iptoa(tr->IpAddr));

                // we're going to re-use tcpBuf (over-writing id response data)

                ldAddr = a->tcpLoadAddress;
                ldIndex = 0;

                while (1) {
                        int len;
                        lcreq->tr.cmd = LOAD_TOOLREQ;
                        lcreq->tr.index = ldIndex;
                        lcreq->tr.addr = IntelOrderL(ldAddr);
                        n = read(fd,lcreq->lc,(sizeof lcreq->lc) - 200);
                        if (n <= 0)
                                break;
                        len = n + sizeof (tToolRequest);
                        lcreq->tr.length = IntelOrderS(len);
                        if (len != write(sock, lcreq, len)) {
                                syslog(LOG_ERR,"Error writing to socket");
                                close(sock);
                                close(fd);
                                return -1;
                        }

                        usleep(5000);  // 5ms delay to let packet get sent

                        // wait for data to get acked (only needed for pre-DM hubs)

                        if (archId < 3)
                                if (ioctl(nslinkfd,SI_TCP_TXQUEUE_WAIT,&tqwait))
                                        syslog(LOG_ERR,"txqueue_wait %s",strerror(errno));

                        ++ldIndex;
                        ldAddr += n;
                }

                close(fd);

                trq->cmd = GOTO_TOOLREQ;
                trq->index = 0;
                trq->length = IntelOrderS(sizeof *trq);
                trq->addr = IntelOrderL(a->startAddress);

                write(sock,trq, sizeof *trq);
                if (archId < 3)
                        if (ioctl(nslinkfd,SI_TCP_TXQUEUE_WAIT,&tqwait))
                                syslog(LOG_ERR,"txqueue_wait %s",strerror(errno));
                close(sock);

                // wait for program to start
                sleep(4);
                continue;
        }
}


void setup_tcp_dev(int fd,  struct daemon_si_box_config *config, int boxIndex)
{
        int sk, rc;
        struct tcp_mode_params tcp_params;

        tcp_params.sk = 0;
        tcp_params.rbuf = NULL;
        tcp_params.rbuf_size = 0;
        tcp_params.wbuf = NULL;
        tcp_params.wbuf_size = 0;

        while (1) {
                // download executable binary file to hub if needed

                if ((sk=downloadTcp(fd, config->ip_addr, config)) < 0) {
                        sleep(5);
                        continue;
                }

                child_socket = sk;

                // hub should be up and running, turn the driver loose on it.

                tcp_params.boxid = boxIndex;
                tcp_params.sk = sk;
                tcp_params.rbuf = malloc(4096);
                tcp_params.rbuf_size = 4096;
                tcp_params.wbuf = malloc(1460);
                tcp_params.wbuf_size = 1460;

                if ((tcp_params.rbuf == NULL) || (tcp_params.wbuf == NULL)) {
                        syslog(LOG_ERR, "Could not allocate memory for NS-Link device, %s\n", iptoa(config->ip_addr));
                        exit(1);
                }

                //syslog(LOG_INFO,"Child %d configuring hub\n",boxIndex);

                config_si(fd, config, boxIndex);

                //syslog(LOG_INFO,"Child %d setting TCP\n",boxIndex);

                rc = ioctl(fd, SI_SET_TCP, &tcp_params);

                if (rc) {
                        syslog(LOG_ERR, "Could not find driver for TCP hub %s\n", iptoa(config->ip_addr));
                        exit(1);
                }

                // syslog(LOG_INFO,"Child %d shutting down TCP\n",i);
                rc = close(sk);
                free(tcp_params.wbuf);
                free(tcp_params.rbuf);
                sleep(1);
        }
}

char *mactoa(unsigned char *m)
{
        static char buf[20];
        sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",m[0],m[1],m[2],m[3],m[4],m[5]);
        return buf;
}

char *iptoa(unsigned int addr)
{
        static char buf[20];
        sprintf(buf, "%d:%d:%d:%d",
                addr & 0xFF, (addr >> 8) & 0xFF, (addr >> 16) & 0xFF, (addr >> 24) & 0xFF);
        return buf;
}

typedef struct {
        unsigned char   destMac[6];
        unsigned char   srcMac[6];
        unsigned short  proto;
        unsigned char   prodId;
        unsigned char   concId;
        unsigned char   packetClass;
        unsigned short  length;
        unsigned char   adminCmd;
        unsigned char   adminData[1470];
}
__attribute__((packed))tAdminPacket;

#define AdminHeaderLen 20

typedef struct {
        unsigned char  tilde;
        unsigned short length;
        unsigned short cmd;
        uint32_t       offset;
        unsigned char  data[1460];
}
__attribute__((packed)) tMacBootPacket;

typedef struct {
        unsigned char  tilde;
        unsigned short length;
        unsigned short cmd;
        uint32_t       ioctl;
        uint32_t       startAddr;
        unsigned char checksum;
}
__attribute__((packed)) tMacGoPacket;

typedef struct {
        unsigned char    macAddr[6];
        unsigned char    flags;
        unsigned char    unused0;
        unsigned char    unused1;
        unsigned char    unused2;
        uint32_t         modelId;
        unsigned char    archId;
        unsigned char    numPorts;
        unsigned short   progId;
        unsigned char    boardRev;
        unsigned char    bootVersMajor;
        unsigned char    bootVersMinor;
        unsigned char    numAppRecords;
}
__attribute__((packed)) tMacIdResp;

typedef struct {
        unsigned char    macAddr[6];
        unsigned char    flags;
        unsigned char    unused0;
        unsigned char    unused1;
        unsigned char    unused2;
}
__attribute__((packed)) tOldMacIdResp;

#define ADMIN_BOOT           0
#define ADMIN_BOOT_PACKET    1
#define ADMIN_ID_QUERY       2
#define ADMIN_ID_REPLY       3
#define ADMIN_LOOP           4
#define ADMIN_RESET          5
#define ADMIN_TRACE          6
#define ADMIN_INIT           7
#define ADMIN_READMEM        8
#define ADMIN_WRITEMEM       9
#define ADMIN_TEST           10
#define ADMIN_DATA           11
#define ADMIN_ELBACK         12
#define ADMIN_READIO         13
#define ADMIN_WRITEIO        14
#define ADMIN_RSMODE         15
#define ADMIN_COMPMEM        16
#define ADMIN_MOVEMEM        17
#define ADMIN_TIMXMIT        18

#define ADMIN_IPINFO_SET    32
#define ADMIN_IPINFO_GET    33
#define ADMIN_IPINFO_ERASE  34


// returns: -1: failure
//           0: right application is running

int downloadMac(unsigned char *destMac, int boxIndex, struct daemon_si_box_config *config)
{
        struct sockaddr_ll sockaddr;
        struct ifreq ifr;
        unsigned char myMac[6];
        int needDownload;
        tAdminPacket  cmd;
        tAdminPacket  rsp;
        tMacIdResp    *id =  (tMacIdResp*)&rsp.adminData;
        tMacBootPacket *bp = (tMacBootPacket*)&cmd.adminData;
        tMacGoPacket   *gp = (tMacGoPacket*)&cmd.adminData;
        int n,fd;
        int ldAddr,len;
        unsigned char ldIndex;
        tArchData *a;
        unsigned char *cp;
        unsigned char checksum;
        char hubMacStr[20];
        int  sk;
        size_t u;
        int idRetryCount;

        strncpy(hubMacStr,mactoa(destMac),sizeof hubMacStr);

        //syslog(LOG_INFO, "Child %d creating packet socket\n", boxIndex);
        sk = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_COMTROL));

        if (sk < 0) {
                syslog(LOG_ERR, "Could not create socket for NS-Link hub %d at %s.",boxIndex,hubMacStr);
                exit(1);
        }

        // look up interface index (needed for bind())

        strncpy(ifr.ifr_name, config->interface, sizeof(ifr.ifr_name));
        ifr.ifr_name[(sizeof(ifr.ifr_name))-1] = '\0';

        if (ioctl(sk, SIOCGIFINDEX, &ifr)) {
                syslog(LOG_ERR,"Child %d couldn't find interface '%s'\n",boxIndex,config->interface);
                exit(1);
        }

        sockaddr.sll_family = AF_PACKET;
        sockaddr.sll_protocol = htons(ETH_P_COMTROL);
        sockaddr.sll_ifindex = ifr.ifr_ifindex;

        // only want to see packets from specified ethernet interface

        if (bind(sk,(struct sockaddr*) &sockaddr, sizeof sockaddr)) {
                syslog(LOG_ERR,"Child %d couldn't bind packet socket to %s\n",boxIndex,config->interface);
                exit(1);
        }

        child_socket = sk;

        u = sizeof sockaddr;
        getsockname(sk, (struct sockaddr*) &sockaddr, &u);

        memcpy(myMac,sockaddr.sll_addr,6);

        //syslog(LOG_INFO, "Child %d bound socket to %s (%s)\n", boxIndex, config->interface,mactoa(myMac));

        // stuff common to all commands we will send
        memcpy(cmd.destMac,destMac,6);
        memcpy(cmd.srcMac,myMac,6);
        cmd.proto = htons(ETH_P_COMTROL);
        cmd.prodId = 0xaa;
        cmd.concId = 0x80 + boxIndex;
        cmd.packetClass = 1;

        idRetryCount = 0;
        needDownload = 0;

        while (1) {
                // During DM Air development, it was found that the wireless
                // adapter/AP need to set up a bridging table before packets
                // will cross.  In TCP/IP this is done by arp/reponse.  In MAC
                // mode, we need to send a broadcast ADMIN_ID_QUERY without
                // requesting ownership.  This sets up the wireless components
                // with the target MAC addresses.

                memset(cmd.destMac, 0xFF, 6);
                cmd.adminCmd = ADMIN_ID_QUERY;
                memcpy(cmd.adminData,myMac,6);
                cmd.adminData[6] = 1;    // flags (bit0 == 0 ==> request ownership)
                cmd.length = IntelOrderS(8);
                write(sk,&cmd,AdminHeaderLen+8);
                packetReadTimeout(sk, destMac, cmd.concId, &rsp, sizeof rsp, 200000);

                // Send ID request, request ownership
                memcpy(cmd.destMac,destMac,6);
                cmd.adminCmd = ADMIN_ID_QUERY;
                memcpy(cmd.adminData,myMac,6);
                cmd.adminData[6] = 0;    // flags (bit0 == 0 ==> request ownership)
                cmd.length = IntelOrderS(8);
                write(sk,&cmd,AdminHeaderLen+8);

                if ((n=packetReadTimeout(sk, destMac, cmd.concId, &rsp, sizeof rsp, 200000)) <= 0) {
                        ++idRetryCount;
                        if ((idRetryCount < 5) || ((idRetryCount % 20) == 0))
                                syslog(LOG_ERR, "MAC ID request timeout for hub %d at %s",boxIndex,hubMacStr);
                        sleep(1);
                        if (idRetryCount > 5)
                                sleep(5);
                        continue;
                }

                idRetryCount = 0;

                if (n < (AdminHeaderLen + (int)sizeof(tOldMacIdResp)) || IntelOrderS(rsp.length) < 7) {
                        syslog(LOG_ERR, "MAC ID response too short from hub %d at %s",boxIndex,hubMacStr);
                        sleep(5);
                        continue;
                }

                if (rsp.adminCmd != ADMIN_ID_REPLY) {
                        syslog(LOG_ERR, "MAC ID response has wrong cmd type from hub %d at %s",boxIndex,hubMacStr);
                        sleep(5);
                        continue;
                }

                id->progId = ntohs(id->progId);
                id->modelId = ntohl(id->modelId);

                if (n >= (int)sizeof(tMacIdResp) &&
                    IntelOrderS(rsp.length) >= 22 &&
                    id->macAddr[3] != 0x06) {
                        // find model/version string in extended response data
                        unsigned char *p = &id->numAppRecords + 1 + 4*id->numAppRecords;     // start of extended data
                        unsigned char *end = (unsigned char*)&rsp + IntelOrderS(rsp.length); // end of extended data
                        char *version = "";

                        while (p<end) {
                                unsigned tag = *p++;
                                unsigned len = *p++;
                                if (tag == 1 && len > 0) {
                                        version = (char*)p;
                                        break;
                                }
                                p += len;
                        }

                        syslog(LOG_INFO, "MAC ID response from hub %d at %s '%s' flags=%02x, Model=%u Arch=%d NumPorts=%d ProgId=%d BoardRev=%d",
                               boxIndex, hubMacStr, version, id->flags, (unsigned int)id->modelId, id->archId, id->numPorts, id->progId, id->boardRev);
                } else {
                        // must be old box!
                        syslog(LOG_INFO, "MAC ID response from hub %d at %s flags=%02x\n", boxIndex,hubMacStr,id->flags);

                        // we have to look at addr and flag values to figure out who we're talking to :(
                        // kdr  Bad Bad Bad...  Magic numbers....
                        if ((id->macAddr[3] == 0x10 || id->macAddr[3] == 0x11))
                                id->archId = 2;
                        else if (id->macAddr[3] == 0x06)
                                id->archId = 1;
                        else
                                id->archId = 0;         // don't know

                        if (id->flags & 0x01)
                                id->progId = 1;
                        else
                                id->progId = 0;
                }

                config->arch_id = id->archId;
                config->ModelId = id->modelId;

                // right app running?
                if (id->progId == 1) {
                        close(sk);
                        return 0;
                }

                if (id->progId != 0) {
                        // wrong app
                        syslog(LOG_INFO,"Resetting hub %d at %s\n",boxIndex, hubMacStr);
                        needDownload = 1;
                        cmd.adminCmd = ADMIN_RESET;
                        cmd.adminData[0] = 0x55;
                        cmd.adminData[1] = 0x55;
                        cmd.length = IntelOrderS(2);
                        write(sk,&cmd,AdminHeaderLen+2);
                        sleep(5);
                        continue;
                }

                // at this point, we know it's a bootloader we're talking to

                if (!needDownload) {
                        // wait to find out if default app is right one.
                        sleep(5);
                        continue;
                }

                // if we get to this point, we're talking to a bootloader, and
                // it needs downloading

                if (id->flags & 0x02) {
                        // already owned
                        syslog(LOG_INFO, "Somebody else is downloading to hub %d at %s",boxIndex,hubMacStr);
                        sleep(5);
                        continue;
                }

                // We "own" this hub now, so go ahead and download to it

                if (id->archId <= 0 || id->archId >= ((sizeof archData)/(sizeof archData[0]))) {
                        syslog(LOG_ERR, "Unknown architecture ID: %d from hub %d at %s", id->archId, boxIndex, hubMacStr);
                        close(sk);
                        return -1;
                }

                a = archData + id->archId;
                fd = open(a->bootFile,O_RDONLY);

                if (fd < 0) {
                        syslog(LOG_ERR,"Can't open boot file %s", a->bootFile);
                        close(sk);
                        return -1;
                }

                syslog(LOG_INFO,"Downloading %s to hub %d at %s\n",a->bootFile,boxIndex,hubMacStr);

                ldAddr = a->macLoadAddress;
                ldIndex = 0;

                while (1) {
                        cmd.adminCmd = ADMIN_BOOT_PACKET;
                        bp->tilde = '~';
                        bp->length = 0;   // fill in later
                        bp->cmd    = IntelOrderS(8);
                        bp->offset = IntelOrderL(ldAddr);

                        n = read(fd,bp->data,sizeof bp->data);

                        if (n <= 0)
                                break;

                        len = n + 6;

                        bp->length = IntelOrderS(len);
                        cmd.length = IntelOrderS(len+14);

                        checksum = 0;
                        for (cp=(unsigned char*)&bp->length; cp < bp->data+n; ++cp)
                                checksum += *cp;

                        bp->data[n] = ~checksum;

                        len = 1 + (&(bp->data[n]) - (unsigned char*)&cmd);

                        if (len != write(sk, &cmd, len)) {
                                syslog(LOG_ERR,"Error writing to socket for hub %d at %s:",boxIndex,hubMacStr,strerror(errno));
                                close(sk);
                                close(fd);
                                return -1;
                        }

                        ldAddr += n;

                        if ((n=packetReadTimeout(sk, destMac, cmd.concId, &rsp, sizeof rsp, 200000)) <= 0) {
                                syslog(LOG_ERR, "download response timeout from hub %d at %s",boxIndex,hubMacStr);
                                close(sk);
                                close(fd);
                                return -1;
                        }

                        if (n < 24) {
                                syslog(LOG_ERR, "download response too short from hub %d at %s",boxIndex,hubMacStr);
                                close(sk);
                                close(fd);
                                return -1;
                        }

                        if (rsp.adminCmd != ADMIN_BOOT_PACKET ||
                            rsp.adminData[0] != '|' ||
                            rsp.adminData[1] != 0x02 ||
                            rsp.adminData[2] != 0x00 ||
                            rsp.adminData[3] != 0x88) {
                                syslog(LOG_ERR, "download response invalid from hub %d at %s\n",boxIndex,hubMacStr);
                                syslog(LOG_ERR, "download response %02x: %02x %02x %02x %02x\n",
                                       rsp.adminCmd,
                                       rsp.adminData[0],
                                       rsp.adminData[1],
                                       rsp.adminData[2],
                                       rsp.adminData[3]);
                                close(sk);
                                close(fd);
                                return -1;
                        }
                }

                close(fd);

                // send "go" command
                gp->tilde  = '~';
                gp->length = IntelOrderS(10);
                gp->cmd    = 5;
                gp->ioctl  = IntelOrderL(12);
                if (id->archId < 3)
                        gp->startAddr = GoofyOrderL(a->startAddress);  // pre DM units used bizarre format for start address
                else
                        gp->startAddr = IntelOrderL(a->startAddress);
                checksum = 0;
                for (cp = (unsigned char*)&gp->length; cp < &gp->checksum; ++cp)
                        checksum += *cp;
                gp->checksum = ~checksum;
                len = 1 + (&gp->checksum - (unsigned char*)&cmd);
                write(sk, &cmd, len);

                // wait for box to start
                sleep(3);
                continue;
        }
}


void setup_mac_dev(int fd,   struct daemon_si_box_config *config, int boxIndex)
{
        int rc;
        struct mac_mode_params mac_params;

        while (1) {
                // download executable binary file to hub and restart it if needed

                if (downloadMac(config->ether_addr,boxIndex,config)) {
                        syslog(LOG_ERR,"Couldn't download to NS-Link device, %s.\n", mactoa(config->ether_addr));
                        sleep(5);
                        continue;
                }

                // hub should be up and running, turn the driver loose on it.

                //syslog(LOG_INFO,"Child %d configuring hub\n",boxIndex);

                rc = config_si(fd, config, boxIndex);

                if (rc && (rc != EBUSY)) {
                        syslog(LOG_ERR, "Could not configure unit...exiting!\n");
                        exit(1);
                }

                //syslog(LOG_INFO,"Child %d setting Mac\n",boxIndex);

                mac_params.boxid = boxIndex;
                rc = ioctl(fd, SI_SET_MAC, &mac_params);

                if (rc) {
                        syslog(LOG_ERR, "Could not set device, %s.\n", mactoa(config->ether_addr));
                        exit(1);
                }
        }
}


void setup_devices(int fd)
{
        int i;
        int newpid;
        struct daemon_si_box_config *config;

        for (i=0; i< num_box_config; ++i) {
                config = box_config + i;

                newpid = fork();

                if (newpid) {
                        /* parent */
                        child_pids[i] = newpid;
                        continue;
                }

                /* child */

                if (config->ip_addr)
                        setup_tcp_dev(fd,config,i);
                else
                        setup_mac_dev(fd,config,i);
        }//end for
}


int main(int argc, char **argv)
{
        int major, fd;

        if (getuid()) {
                fprintf(stderr, "You must be root to run NS-Link daemon!\n");
                exit(1);
        }

        openlog("nslinkd", LOG_NDELAY | LOG_PID, LOG_DAEMON);
        syslog(LOG_INFO, "Starting NS-Link daemon, version %s (%s)",SI_VERSION, SI_DATE);
        syslog(LOG_INFO, "Built %s %s\n",__DATE__,__TIME__);

        parse_config("/etc/nslink.conf");

        if (boot_file4_8)
                archData[1].bootFile = boot_file4_8;
        if (boot_file2)
                archData[2].bootFile = boot_file2;
        if (boot_fileDM)
                archData[3].bootFile = boot_fileDM;

        major = get_major_number("NSLinkctl");

        if (major < 0) {
                syslog(LOG_ERR, "Couldn't load control file");
                fprintf(stderr, "Couldn't find control file.  %s\n", not_loaded);
                exit(1);
        }

        fd = open_dev(major << 8, O_RDWR);
        if (fd < 0) {
                syslog(LOG_ERR, "Couldn't open control device: %s", strerror(errno));
                fprintf(stderr, "Couldn't open control device.  %s\n", not_loaded);
                perror("  open_dev()");
                exit(1);
        }

        check_pid_file();

        if (become_daemon() < 0) {
                syslog(LOG_ERR, "Couldn't become daemon");
                fprintf(stderr, "Error forking child process while "
                        "starting daemon mode\n");
                exit(1);
        }

        put_pid_file();
        signal(SIGHUP, sighup_handler);
        signal(SIGTERM, sigterm_handler);
        signal(SIGINT, sigterm_handler);
        create_devices(fd);
        setup_devices(fd);
        // JS - Has to add this delay otherwise the parent process of nslinkd
        // daemon gets killed at bootup on Ubuntu system. Don't know why though.
        sleep(3);
        pause();
        return 0;
}
