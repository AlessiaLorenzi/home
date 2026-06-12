#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <time.h>
#if (LINUX_VERSION_CODE <= 131107)
#include <linux/if_packet.h>
#else
#include <net/if_packet.h>
#endif
#include <arpa/inet.h>

#if defined(EnableReadline)
#include <readline/readline.h>
#include <readline/history.h>
#else
char *readline(char *prompt)
{
        char *buf = malloc(80);
        printf("%s",prompt);
        fflush(stdout);
        if (fgets(buf,80,stdin))
                return buf;
        else {
                free(buf);
                return NULL;
        }
}
#endif


#define PKT_DATA_LEN 1492
#define PRODUCT_ID_AA 0xaa
#define PRODUCT_ID_55 0x55
#define EthHdrLen 14

/* Packet classes */
#define ADMIN_CLASS	0x01
#define RPSHSI_CLASS	0x55

/* Admin packet type codes */
#define ADMIN_ID_BOOT		0
#define ADMIN_BOOT_PACKET	1
#define ADMIN_ID_QUERY		2
#define ADMIN_ID_REPLY		3
#define ADMIN_ID_LOOP		4
#define ADMIN_ID_RESET		5
#define ADMIN_ID_TRACE          6
#define ADMIN_ID_INIT           7
#define ADMIN_ID_READMEM        8
#define ADMIN_ID_WRITEMEM       9
#define ADMIN_ID_TEST           10
#define ADMIN_ID_DATA           11
#define ADMIN_ID_ELBACK         12
#define ADMIN_ID_READIO         13
#define ADMIN_ID_WRITEIO        14
#define ADMIN_ID_RSMODE         15
#define ADMIN_ID_COMPMEM        16
#define ADMIN_ID_MOVEMEM        17
#define ADMIN_ID_TIMXMIT        18

#define ADMIN_IPINFO_SET    32
#define ADMIN_IPINFO_GET    33
#define ADMIN_IPINFO_ERASE  34
#define ADMIN_ID_MACADDR_SET 35
#define ADMIN_ID_MACADDR_GET 36
#define ADMIN_IPINFO_GETCUR  37
#define ADMIN_IPINFO_DISABLE 38


struct PktHdr
{
        unsigned char	product_id;
        unsigned char	concentrator_index;
        unsigned char	packet_class;
        unsigned char length[2];
        unsigned char	type;
        unsigned char	crc;
        unsigned char	data;
}
__attribute__((packed));
typedef struct PktHdr PktHdr;

struct AdminHdr
{
        unsigned char	product_id;
        unsigned char	concentrator_index;
        unsigned char	packet_class;
        unsigned char length[2];
        unsigned char	packet_type;
}
__attribute__((packed));
typedef struct AdminHdr AdminHdr;

struct IpInfo
{
        uint32_t addr;
        uint32_t mask;
        uint32_t gate;
}
__attribute__((packed));
typedef struct IpInfo IpInfo;

struct IpInfoPkt
{
        AdminHdr hdr;
        IpInfo   ip;
}
__attribute__((packed));
typedef struct IpInfoPkt IpInfoPkt;

struct IdInfo
{
        unsigned char  MacAddr[6];
        unsigned char  state;
}
__attribute__((packed));
typedef struct IdInfo IdInfo;

struct IdInfoPkt
{
        AdminHdr hdr;
        IdInfo   id;
}
__attribute__((packed));
typedef struct IdInfoPkt IdInfoPkt;


typedef struct DiscoverNode_t
{
        struct DiscoverNode_t	*next;
        unsigned char	mac[6];
        char *displayString;
        char *versionString;
        int idle;
        int owned;
        int archId;
        int progId;
        int modelId;
        int numPorts;
        int boardRev;
        int majorVers;
        int minorVers;
}
DiscoverNode;

static void DumpIt(unsigned int offset, unsigned char *buf, int len);

static int StopNow=0;
static unsigned char ComtrolMac[3] = {0x00, 0xc0, 0x4e};
static unsigned char RemoteMac[6];
static unsigned char MyMac[6];
static unsigned char *ReadIt;
static unsigned char ReadBuf[1518];
static unsigned char WriteBuf[1518];
static int RemoteType=0;
#define REMOTE_VS1000     0x01
#define REMOTE_VS2000     0x20
#define REMOTE_RPSH       0x02
#define REMOTE_RPSHSI     0x04
#define REMOTE_RPSHSI2P	  0x08
#define REMOTE_RPSHSI2PIA 0x10
#define REMOTE_DEVICEMASTER 0x40
#define REMOTE_COMTROL_UNKNOWN    0x8000
#define SI_MASK			(REMOTE_RPSHSI|REMOTE_RPSHSI2P|REMOTE_RPSHSI2PIA|REMOTE_DEVICEMASTER)
static DiscoverNode *DiscoverTable=NULL;
static DiscoverNode *RemoteNode=NULL;

static char *ethdevname = "eth0";
static int debugLevel = 0;
static int interactive = 1;
static char *progName;

static unsigned char GetCrc( unsigned char *buf, int len )
{
        int i, cksum;
        unsigned char crc;

        cksum = 0;
        i = 0;
        while( i < len ) {
                cksum += *buf++;
                i += 1;
        }

        crc = (~cksum) & 0xff;

        return crc;
}

//  For a recieved packet, checks for bad length, that this is a MAC
//  mode packet to us from the device.  Data is in global array ReadBuf.
//  This function moves the global ptr ReadIt to the first data byte,
//  which is ReadBuf[14].
//  Input:  len (packet length)
//  Returns: 0 if error, else packet length
static int VerifyRead( int len, int docrc, int concIndex)
{
        PktHdr *hdr;
        unsigned char crc;
        int crclen;

        if (debugLevel > 2) {
                int i;

                printf("Verify: ");
                for (i=0; i<20; ++i)
                        printf("%02x ",ReadBuf[i]);
                printf("\n");
        }

        if ( len > 1518 ) {
                printf("Verify read Packet too long, %d\n",len);
                return 0;
        }

        //  Verify the packet contains our MAC, their MAC and 0x11FE (Comtrol ethernet protocol ID)
        if ( (len >= 22) &&
                        (ReadBuf[0] == MyMac[0]) &&
                        (ReadBuf[1] == MyMac[1]) &&
                        (ReadBuf[2] == MyMac[2]) &&
                        (ReadBuf[3] == MyMac[3]) &&
                        (ReadBuf[4] == MyMac[4]) &&
                        (ReadBuf[5] == MyMac[5]) &&
                        (ReadBuf[6] == RemoteMac[0]) &&
                        (ReadBuf[7] == RemoteMac[1]) &&
                        (ReadBuf[8] == RemoteMac[2]) &&
                        (ReadBuf[9] == RemoteMac[3]) &&
                        (ReadBuf[10] == RemoteMac[4]) &&
                        (ReadBuf[11] == RemoteMac[5]) &&
                        (ReadBuf[12] == 0x11) &&
                        (ReadBuf[13] == 0xfe) ) {
                if (debugLevel > 2)
                        printf("Verify: MAC match\n");

                ReadIt = &ReadBuf[14];
                len -= 14;

                hdr = (PktHdr *)ReadIt;

                if (concIndex >= 0 && hdr->concentrator_index != 0xff) {
                        if (debugLevel > 2)
                                printf("VerifyRead() - Invalid concentrator index in received packet, discarding\n");
                        return 0;
                }

                if (debugLevel > 2)
                        printf("Verify: ConcIndex OK\n");

                if ( docrc ) {
                        crclen = hdr->length[0] + (hdr->length[1]<<8);
                        if ( crclen < 1500 ) {
                                crc = GetCrc( ReadIt, crclen );
                                if ( crc ) {
                                        printf("Invalid packet CRC - %02x\n",crc);
                                        DumpIt( 0, ReadIt, crclen );
                                }
                        } else {
                                printf("Bad packet from network.\n");
                                DumpIt( 0, ReadBuf, 128 );
                                len = 0;
                        }
                }
                return len;
        }

        //  If we got here, this is a broadcast packet, asking for our ID
        if ( RemoteMac[1] == 0 ) {

                // kdr  We must remove this list, at model table and accessor functions
                //      should be used
                //  The remote MAC starts at ReadBuf[6].  [9] is the product differentiator byte.
                if ((ReadBuf[9] == 0x02) ||
                                (ReadBuf[9] == 0x05) ||
                                (ReadBuf[9] == 0x06) ||
                                (ReadBuf[9] == 0x07) ||
                                (ReadBuf[9] == 0x08) ||
                                (ReadBuf[9] == 0x09) ||
                                (ReadBuf[9] == 0x0a) ||
                                (ReadBuf[9] == 0x0b) ||
                                (ReadBuf[9] == 0x10) ||
                                (ReadBuf[9] == 0x11) ||
                                (ReadBuf[9] == 0x12) ||
                                (ReadBuf[9] == 0x13) ||
                                (ReadBuf[9] == 0x14) ||
                                (ReadBuf[9] == 0x15) ||
                                (ReadBuf[9] == 0x17) ||
                                (ReadBuf[9] == 0x19) ||
                                (ReadBuf[9] == 0x1C) ||
                                (ReadBuf[9] == 0x1D) ||
                                (ReadBuf[9] == 0x1E) ||
                                (ReadBuf[9] == 0x1F) ||
                                (ReadBuf[9] == 0x20) ||
                                (ReadBuf[9] == 0x21) ||
                                (ReadBuf[9] == 0x22) ||
                                (ReadBuf[9] == 0x23) ||
                                (ReadBuf[9] == 0x24) ||
                                (ReadBuf[9] == 0x26) ||
                                (ReadBuf[9] == 0x28) ||
                                (ReadBuf[9] == 0x29) ||
                                (ReadBuf[9] == 0x2A) ||
                                (ReadBuf[9] == 0x2B)) {
                        /*
                         * We must be broadcasting on the network.  Just give whatever back
                         * to the caller.
                         */
                        ReadIt = &ReadBuf[14];
                        len -= 14;
                        return len;
                } else {
                        if (debugLevel > 2)
                                printf("VerifyRead() - Invalid 4th MAC byte (unknown model) in received packet, discarding\n");
                        return 0;
                }
        }

#ifdef MTM
        if ((ReadBuf[6] == RemoteMac[0]) &&
                        (ReadBuf[7] == RemoteMac[1]) &&
                        (ReadBuf[8] == RemoteMac[2]) &&
                        (ReadBuf[9] == RemoteMac[3]) &&
                        (ReadBuf[10] == RemoteMac[4]) &&
                        (ReadBuf[11] == RemoteMac[5]) ) {
                printf("BadInput\n");
                DumpIt(0, ReadBuf,len);
        }
#endif
        return 0;
}


// time delta in ms
long tvDelta(struct timeval *t1, struct timeval *t2)
{
        long d;

        d = 1000 * (t1->tv_sec - t2->tv_sec);
        d += (t1->tv_usec - t2->tv_usec)/1000;

        return d;
}

//---------------------------------------
//  Read box response over ethernet, puts data in ReadBuf.  The global ptr
//  ReadIt is set to point to the first data byte (ReadBuf[14]).
//  Params:  sk (socket ptr), timeout (number of ms periods to wait),
//           docrc (1 to do crc)
//  Returns: Number of bytes read, or -1 if error
static int ReadRemote(int sk, int timeout, int docrc, int concIndex)
{
        int rderrs, rc;
        struct timespec sleeptime;
        struct timeval timeStart, timeNow;

        gettimeofday(&timeStart,NULL);
        memset(ReadBuf, 0, sizeof ReadBuf);

        rderrs = 0;
        do {
                rc = read(sk, ReadBuf, 1500);
                if (rc > 0) {
                        rc = VerifyRead(rc, docrc, concIndex);
                } else {
                        if (timeout==0 || StopNow)
                                return rc;

                        if (errno != EAGAIN)
                                perror("ReadRemote");
                        else {
                                rderrs += 1;
                                gettimeofday(&timeNow,NULL);

                                if (tvDelta(&timeNow,&timeStart) < timeout) {
                                        sleeptime.tv_sec = 0;
                                        sleeptime.tv_nsec = 10000000;	/* 10 ms */
                                        nanosleep( &sleeptime, NULL );
                                        continue;
                                }
                        }
                        return 0;
                }
        } while( rc <= 0 );

        return rc;
}

static void BuildHeader( unsigned char *buf )
{
        if ( RemoteMac[1] )
                memcpy( &buf[0], RemoteMac, 6 );
        else
                memset( &buf[0], 0xff, 6 );

        memcpy( &buf[6], MyMac, 6 );

        buf[12] = 0x11;
        buf[13] = 0xfe;

        return;
}

//--------------------------------------
// Send data to box over ethernet.  Data to send is an ethernet header
// (remote MAC | my MAC | 0x11FE) then the data.  Data is put in WriteBuf,
//  then sent out the socket.
//  Inputs:  sk (socket ptr), ibuf (buf ptr to data), len (length of data)
//  Returns:  rc from sendto(), number of bytes sent or -1 for error
static int SendRemote( int sk, unsigned char *ibuf, int len, int docrc )
{
        struct sockaddr_pkt;
        int rc;
        PktHdr *hdr;

        if ( docrc ) {
                hdr = (PktHdr *)ibuf;
                hdr->crc = 0;
                hdr->crc = GetCrc( ibuf, len );
        }

        if (debugLevel > 8)
                printf("SendRemote(%d,%p,%d,%d)\n",sk,ibuf,len,docrc);

        BuildHeader( WriteBuf );
        memcpy( &WriteBuf[14], ibuf, len );
        rc = write(sk,WriteBuf,len+14);

        if ( rc < 0 )
                perror("write");

        return rc;
}

//------------------------------
//  Sends a ADMIN_BOOT_PACKET command to the device, which
//  start the box running.  Binary must be loaded first.
static int StartRemote(int sk)
{
        unsigned char buf[48];
        int rc;

        if (debugLevel > 0)
                printf("Starting remote\n");

        if ( RemoteType & SI_MASK )
                buf[0] = PRODUCT_ID_AA;
        else
                buf[0] = PRODUCT_ID_55;

        buf[1] = 0xff;  //concentrator index
        buf[2] = ADMIN_CLASS;
        buf[3] = 0x18;
        buf[4] = 0x00;
        buf[5] = ADMIN_BOOT_PACKET;
        buf[6] = 0x7e;
        buf[7] = 0x0a;
        buf[8] = 0x00;
        buf[9] = 0x05;
        buf[10] = 0x00;
        /* id */
        buf[11] = 0x0c;
        buf[12] = 0x00;
        buf[13] = 0x00;
        buf[14] = 0x00;
        /* offset */
        if ( RemoteType == REMOTE_RPSHSI2P || RemoteType == REMOTE_RPSHSI2PIA) {
                /* 0x00100000 */
                buf[15] = 0x10;
                buf[16] = 0x00;
                buf[17] = 0x00;
                buf[18] = 0x00;
                /* crc */
                buf[19] = 0xd4;
        } else if ( RemoteType == REMOTE_RPSHSI ) {
                /* 0x1000:0000 */
                buf[15] = 0x00;
                buf[16] = 0x10;
                buf[17] = 0x00;
                buf[18] = 0x00;
                /* crc */
                buf[19] = 0xd4;
        } else if ( RemoteType == REMOTE_DEVICEMASTER ) {
                /* 0x0000:0000 */
                buf[15] = 0x00;
                buf[16] = 0x00;
                buf[17] = 0x00;
                buf[18] = 0x00;
                /* crc */
                buf[19] = 0xe4;
        }

        rc = SendRemote( sk, buf, 20, 0 );

        if ( rc < 0 )
                return 1;

        return 0;
}

//----------------------------------
//  Loads binary file to the device
static int LoadRemote(int sk, char *filename)
{
        unsigned char *cp, buf[1500];
        int rc, len, cksum, fd, idx;
        uint32_t offset, tmp;

        if (debugLevel > 0)
                printf("Loading remote\n");

        if ( RemoteType == REMOTE_RPSHSI2P || RemoteType == REMOTE_RPSHSI2PIA) {
                if (!filename)
                        filename = "/etc/rpshsi2p.bin";
                offset = 0x100000;
        } else if ( RemoteType == REMOTE_RPSHSI ) {
                if (!filename)
                        filename = "/etc/rpshsi.bin";
                offset = 0x10000;
        } else if ( RemoteType == REMOTE_RPSH || RemoteType == REMOTE_VS1000 || RemoteType == REMOTE_VS2000) {
                if (!filename)
                        filename = "/etc/vslinka.bin";
                offset = 0x10000;
        } else if ( RemoteType == REMOTE_DEVICEMASTER ) {
                if (!filename)
                        filename = "/etc/devmast.bin";
                offset = 0;
        }

        if (!filename) {
                printf("Unknown device type: don't know what binary to load\n");
                return -1;
        }

        fd = open( filename, O_RDONLY );
        if ( fd < 0 ) {
                printf("OpenBinImage: error opening file '%s'\n",filename);
                perror("");
                return -1;
        }

        do {
                len = read( fd, &buf[15], 1460 );
                if ( len > 0 ) {
                        cp = buf;

                        if ( RemoteType & SI_MASK )
                                *cp++ = PRODUCT_ID_AA;
                        else
                                *cp++ = PRODUCT_ID_55;

                        *cp++ = 0xff; // concentrator index
                        *cp++ = 0x01;
                        *cp++ = ((len+20) & 0xff);
                        *cp++ = ((len+20) >> 8);
                        *cp++ = 0x01;

                        cksum = 0;

                        *cp++ = '~';
                        *cp++ = (len + 6) & 0xff;
                        cksum += (len + 6) & 0xff;
                        *cp++ = (len + 6) >> 8;
                        cksum += (len + 6) >> 8;
                        *cp++ = 0x08;
                        cksum += 0x08;
                        *cp++ = 0;
                        tmp = offset;
                        *cp++ = tmp & 0xff;
                        cksum += tmp & 0xff;
                        tmp = tmp >> 8;
                        *cp++ = tmp & 0xff;
                        cksum += tmp & 0xff;
                        tmp = tmp >> 8;
                        *cp++ = tmp & 0xff;
                        cksum += tmp & 0xff;
                        tmp = tmp >> 8;
                        *cp++ = tmp & 0xff;
                        cksum += tmp & 0xff;
                        idx = 0;
                        while( idx < len ) {
                                cksum += *cp++;
                                idx += 1;
                        }
                        *cp = (~cksum) & 0xff;

                        rc = SendRemote( sk, buf, (len+16), 0 );

                        if ( rc < 0 )
                                return 1;

                        rc = ReadRemote(sk, 1000, 0, -1);
                        if ( rc < 10 ) {
                                printf("Could not read boot reply\n");
                                close( fd );
                                return 1;
                        }

                        if ( ((ReadIt[0] == PRODUCT_ID_55) ||
                                        (ReadIt[0] == PRODUCT_ID_AA)) &&
                                        (ReadIt[6] == '|') &&
                                        (ReadIt[7] == 0x02) &&
                                        (ReadIt[8] == 0x00) &&
                                        (ReadIt[9] == 0x88) ) {
                                if (debugLevel > 0) {
                                        printf(".");
                                        fflush(stdout);
                                }
                        } else {
                                close( fd );
                                return 1;
                        }

                        offset += len;
                }
        } while( len );

        close(fd);

        if (debugLevel > 0)
                printf("\n");

        return 0;
}

//------------------------------------------------------------------------
//  Sends the ADMIN_ID_RESET command to the device, which causes it to reset.
static int ResetRemote( int sk )
{
        PktHdr hdr;
        int rc;

        if (debugLevel > 0)
                printf("Resetting remote\n");

        if ( RemoteType & SI_MASK )
                hdr.product_id = PRODUCT_ID_AA;
        else
                hdr.product_id = PRODUCT_ID_55;

        hdr.concentrator_index = 0xff;
        hdr.packet_class = ADMIN_CLASS;
        hdr.length[0] = 0x03;
        hdr.length[1] = 0;
        hdr.type = ADMIN_ID_RESET;
        hdr.crc = 0x55;
        hdr.data = 0x55;

        rc = SendRemote( sk, (unsigned char *)&hdr, sizeof(hdr), 0 );

        if ( rc < 0 )
                return 1;
        else
                return 0;
}

//------------------------------------------------------------------------
//   SetRemoteType - Sets the global variable RemoteType.
//   Returns:  The remote type, 0 if error */
static int SetRemoteType(void)
{
        char *remote_type, *cp;

        // kdr  We must remove this list, at model table and accessor functions
        //      should be used

        if (memcmp(ComtrolMac,RemoteMac,3))
                RemoteType = 0;
        else
                switch(RemoteMac[3]) {
                case 0x02:
                        RemoteType = REMOTE_VS1000;
                        break;
                case 0x04:
                        RemoteType = REMOTE_VS2000;
                        break;
                case 0x05:
                        RemoteType = REMOTE_RPSH;
                        break;
                case 0x06:
                        RemoteType = REMOTE_RPSHSI;
                        break;
                case 0x10:
                        RemoteType = REMOTE_RPSHSI2P;
                        break;
                case 0x11:
                        RemoteType = REMOTE_RPSHSI2PIA;
                        break;
                case 0x07:
                case 0x08:
                case 0x09:
                case 0x0a:
                case 0x0b:
                case 0x12:
                case 0x14:
                case 0x15:
                case 0x17:
                case 0x19:
                case 0x1C:
                case 0x1D:
                case 0x1E:
                case 0x1F:
                case 0x20:
                case 0x21:
                case 0x22:
                case 0x23:
                case 0x24:
                case 0x28:
                case 0x29:
                case 0x2A:
                case 0x2B:
                        RemoteType = REMOTE_DEVICEMASTER;
                        break;
                default:
                        RemoteType = 0;
                }

        if (!RemoteType && interactive) {
                printf("\nI do not recognize that MAC address.  Please tell me\n");
                printf("what type of remote we are dealing with.\n\n");
                printf("\t%d)  VS1000\n",REMOTE_VS1000);
                printf("\t%d)  VS2000\n",REMOTE_VS2000);
                printf("\t%d)  RocketPort Serial Hub\n",REMOTE_RPSH);
                printf("\t%d)  RocketPort Serial Hub Si\n",REMOTE_RPSHSI);
                printf("\t%d)  RocketPort Serial Hub Si, 2 Port\n",REMOTE_RPSHSI2P);
                printf("\t%d)  RocketPort Serial Hub Si, 2 Port IA\n",REMOTE_RPSHSI2PIA);
                printf("\t%d)  DeviceMaster\n",REMOTE_DEVICEMASTER);
                printf("\t*)  exit\n");
                printf("\n");
                cp = readline("->");
                if (cp) {
                        RemoteType = atoi(cp);
                        free(cp);
                }
        } else if (!RemoteType && !interactive) {
                printf("Not a recognized Comtrol Ethernet address\n");
                exit(1);
        }

        switch( RemoteType ) {
        case REMOTE_VS1000:
                remote_type = "VS1000";
                break;
        case REMOTE_VS2000:
                remote_type = "VS2000";
                break;
        case REMOTE_RPSH:
                remote_type = "RocketPort Serial Hub";
                break;
        case REMOTE_RPSHSI:
                remote_type = "RocketPort Serial Hub Si";
                break;
        case REMOTE_RPSHSI2P:
                remote_type = "RocketPort Serial Hub Si, 2 Port";
                break;
        case REMOTE_RPSHSI2PIA:
                remote_type = "RocketPort Serial Hub Si IA, 2 Port";
                break;
        case REMOTE_DEVICEMASTER:
                remote_type = "DeviceMaster";
                break;
        case REMOTE_COMTROL_UNKNOWN:
                remote_type = "Other Comtrol Hub";
                break;
        default:
                return 0;
        }

        if (debugLevel > 0)
                printf("\n%s: %02x:%02x:%02x:%02x:%02x:%02x\n", remote_type, RemoteMac[0],
                       RemoteMac[1], RemoteMac[2], RemoteMac[3], RemoteMac[4], RemoteMac[5] );

        return RemoteType;
}


//------------------------------------------------------------------------
//  Sends the ADMIN_ID_QUERY packet to the device
static int SendIdQuery(int sk, unsigned char prodId)
{
        unsigned char buf[80];

        if (debugLevel > 2) {
                printf("Sending broadcast ID query to ");
                if (prodId == 0xAA)
                        printf("DeviceMaster/SI devices (0x%x) \n", prodId);
                else if (prodId == 0x55)
                        printf("VS100/SH devices (0x%x) \n", prodId);
                else
                        printf("other devices (0x%x) \n", prodId);
        }

        buf[0] = prodId;
        buf[1] = 0xff;  // concentrator index
        buf[2] = ADMIN_CLASS;
        buf[3] = 0x08;
        buf[4] = 0x00;
        buf[5] = ADMIN_ID_QUERY;
        memcpy(&buf[6],MyMac,6);
        buf[12] = 0x01;

        return SendRemote( sk, buf, 13, 0 ) < 0;
}


//------------------------------------------------------------------------
//  Processes the ADMIN_ID_REPLY packet from the device.  Puts the data
//  into a DiscoverNode struct.
//  Inputs:  dn (ptr to DiscoverNode struct), resp (the data)
void UpdateNode(DiscoverNode *dn, unsigned char* resp, int count)
{
        int len, cp;
        char displayString[256];

        memcpy(dn->mac, &resp[6], 6);
        len = resp[3] + (resp[4]<<8);
        if (len >= 23) {
                dn->modelId = (resp[16]<<24) + (resp[17]<<16) + (resp[18]<<8) + resp[19];
                dn->archId = resp[20];
                dn->numPorts = resp[21];
                dn->progId = (resp[22]<<8) + resp[23];
                dn->boardRev = resp[24];

                /*  Arch ID  = 0 (unknown) or Redboot */
                if ((dn->archId == 0) || (dn->progId == 0)) {
                        dn->majorVers = resp[25];
                        dn->minorVers = resp[26];
                }

                /*  SocketServer or NSLink */
                else if (resp[27]) {
                        dn->majorVers = resp[30];
                        dn->minorVers = resp[31];
                }

                /*  Error state */
                else {
                        dn->majorVers = 0;
                        dn->minorVers = 0;
                }
        } else
                dn->modelId = 0;

        if (memcmp(dn->mac,ComtrolMac,3))
                dn->displayString = strdup("Unknown, Non-Comtrol");
        else {
                char *ds;

                // kdr  We must remove this list, a model table and accessor functions
                //      should be used

                switch(dn->mac[3]) {
                case 0x02:
                        ds = "VS1000";
                        break;
                case 0x04:
                        ds = "VS2000";
                        break;
                case 0x05:
                        ds = "RPSH";
                        break;
                case 0x06:
                        ds = "RPSH-Si";
                        break;
                case 0x10:
                        ds = "RPSH-Si";
                        break;
                case 0x11:
                        ds = "RPSH-ia";
                        break;

                case 0x07:
                case 0x08:
                case 0x09:
                case 0x0a:
                case 0x0b:
                case 0x12:
                case 0x14:
                case 0x15:
                case 0x21:
                case 0x22:
                case 0x28:
                case 0x29:
                case 0x2A:
                case 0x2B:
                        ds = "DeviceMaster RTS";
                        break;

                case 0x17:
                case 0x19:
                        ds = "DeviceMaster PRO";
                        break;

                case 0x1C:
                case 0x1D:
                        ds = "DM Serial Hub";
                        break;

                case 0x1E:
                case 0x1F:
                case 0x20:
                case 0x23:
                case 0x24:
                default:
                        ds = "DeviceMaster";
                        break;
                }

                if (debugLevel > 2)
                        printf("ID Reply Received - %s  %x %x %x %x %x %x \n ",
                               ds, dn->mac[0],  dn->mac[1],  dn->mac[2],  dn->mac[3],  dn->mac[4],  dn->mac[5]);

                if (dn->modelId && dn->mac[3] > 6) {
                        cp  = sprintf(displayString,"%s %d port -", ds, dn->numPorts);

                        /*  If the extended data is present, add the firmware ID string (if present) */
                        if ((resp[13] & 0x01) && (resp[32] & 0x01)) {
                                cp += sprintf(&displayString[cp], "%s", &resp[34]);
                        } else {
                                switch (dn->progId) {
                                case 2:
                                        cp  += sprintf(&displayString[cp]," SocketServer");
                                        break;
                                case 1:
                                        cp  += sprintf(&displayString[cp]," NSLink");
                                        break;
                                case 0:
                                default:
                                        cp  += sprintf(&displayString[cp]," RedBoot");
                                        break;
                                }
                                cp  += sprintf(&displayString[cp]," v%d.%02d", dn->majorVers, dn->minorVers);
                        }

                        dn->displayString = strdup(displayString);
                } else
                        dn->displayString = strdup(ds);
        }
        dn->owned = !(resp[12]&0x02);
        dn->idle  = !(resp[12]&0x01);
}

void ListNode(DiscoverNode *dn)
{
        printf("(\"%02x:%02x:%02x:%02x:%02x:%02x\",",
               dn->mac[0],
               dn->mac[1],
               dn->mac[2],
               dn->mac[3],
               dn->mac[4],
               dn->mac[5]);
        printf("\"%s\",\"%s\",\"%s\")",
               dn->displayString,
               dn->idle  ? "idle" : "running",
               dn->owned ? "owned" : "free");
}

void ListNodes(DiscoverNode *dn)
{
        int i=1;
        while (dn) {
                ListNode(dn);
                printf(",");
                dn = dn->next;
                ++i;
        }
}

void ShowNode(DiscoverNode *dn)
{
        printf("%02x:%02x:%02x:%02x:%02x:%02x",
               dn->mac[0],
               dn->mac[1],
               dn->mac[2],
               dn->mac[3],
               dn->mac[4],
               dn->mac[5]);
        printf("  %s", dn->displayString);
}

void ShowNodes(DiscoverNode *dn)
{
        int i=1;
        while (dn) {
                printf(" %2d) ",i);
                ShowNode(dn);
                printf("\n");
                dn = dn->next;
                ++i;
        }
}

static int AutoLoadRemote(int sk, char *filename)
{
        IdInfoPkt *resp;
        unsigned char prodId = (RemoteType & SI_MASK) ? 0xaa : 0x55;

        SendIdQuery(sk,prodId);

        if (ReadRemote(sk,100,0,-1) < (sizeof *resp) + EthHdrLen) {
                printf("Didn't get reply to ID query\n");
                return 1;
        }

        resp = (IdInfoPkt*)ReadIt;

        if (resp->hdr.product_id != prodId ||
                        resp->hdr.packet_class != ADMIN_CLASS ||
                        resp->hdr.packet_type != ADMIN_ID_REPLY ||
                        memcmp(resp->id.MacAddr,RemoteMac,6)) {
                printf("Bad reply to ID query\n");
                return 1;
        }

        if (resp->id.state == 0x02)        /* idle and free */
        {
                if (LoadRemote(sk,filename) && LoadRemote(sk,filename))
                        return 1;
                if (StartRemote(sk) && StartRemote(sk))
                        return 1;
                return 0;
        }

        if (interactive) {
                printf("\n*** Remote is already");
                switch (resp->id.state) {
                case 0x00:
                        printf(" running and owned");
                        break;
                case 0x01:
                        printf(" running");
                        break;
                case 0x03:
                        printf(" owned");
                        break;
                }
                printf(", not attempting to load\n");
        }
        return 2;
}

//------------------------------------
//  Searches for attached Comtrol devices.  Puts the device info into
//  the global array of structs DiscoverTable.
//  Returns:  0 if OK, 1 if error
static int CheckNetwork(int sk)
{
        int rc, nodes, nodenum, pkts;
        DiscoverNode *dn, *dnlast;
        char *cp;
        struct timeval timeStart, timeNow;

        if (interactive)
                printf("\nChecking network for any possible remotes.\n\n");

        gettimeofday(&timeStart,NULL);

        //  Send out Comtrol broadcast queries for devices
        if (SendIdQuery(sk,0x55) || SendIdQuery(sk,0xaa))
                return 1;

        dnlast = NULL;
        nodes = 0;
        pkts = 0;

        //  Harvest the responses, put them in DiscoverTable
        do {
                rc = ReadRemote(sk, 1000, 0, -1);
                if ( rc > 0 ) {
                        IdInfoPkt *resp = (IdInfoPkt*)ReadIt;
                        pkts += 1;
                        if (((resp->hdr.product_id == 0x55) ||
                                        (resp->hdr.product_id == 0xaa)) &&
                                        (resp->hdr.packet_class == ADMIN_CLASS) &&
                                        (resp->hdr.packet_type == ADMIN_ID_REPLY) ) {

                                /* ID reply */
                                dn = (DiscoverNode *)malloc(sizeof(DiscoverNode));
                                if (!dn)
                                        break;
                                nodes += 1;
                                UpdateNode(dn,ReadIt,rc);
                                if (DiscoverTable)
                                        dnlast->next = dn;
                                else
                                        DiscoverTable = dn;
                                dnlast = dn;
                        }
                }
                gettimeofday(&timeNow,NULL);
        } while( (rc > 0) && (pkts < 1000) && tvDelta(&timeNow,&timeStart) < 1000);

        if (!interactive)
                return 0;

        if (nodes) {
                ShowNodes(DiscoverTable);
                printf("\n  q) quit\n\n");
                cp = readline("->");
                if (cp) {
                        nodenum = atoi(cp);
                        free(cp);
                        if ( (nodenum > 0) && (nodenum <= nodes) ) {
                                dn = DiscoverTable;
                                while( nodenum > 1 ) {
                                        nodenum -= 1;
                                        dn = dn->next;
                                }
                                memcpy( RemoteMac, dn->mac, 6 );
                                RemoteNode = dn;
                                return 0;
                        }
                }
        }
        return 1;
}

static void dumpIp(char *s1, unsigned int IpValue, char *s2)
{
        printf("%s%d.%d.%d.%d%s", s1,
               (IpValue>>24)&0xff,
               (IpValue>>16)&0xff,
               (IpValue>> 8)&0xff,
               (IpValue>> 0)&0xff,
               s2);
}

//------------------------------------
//  Gets the IP info from the device, using either ADMIN_IPINFO_GETCUR
//  or  ADMIN_IPINFO_GET (from flash) commands.  Prints info on screen.
//  Inputs:  sk (socket ptr), cmdCode (which command)
static int GetIPinfo(int sk, int cmdCode, int quiet)
{
        PktHdr *hdrp;
        int rc,len;
        struct {
                PktHdr hdr;
        }
        cmd;
        IpInfoPkt *resp;
        uint32_t ipAddr, ipMask, ipGate;

        if ( RemoteType & SI_MASK )
                cmd.hdr.product_id = PRODUCT_ID_AA;
        else
                cmd.hdr.product_id = PRODUCT_ID_55;

        cmd.hdr.concentrator_index = 0xff;
        cmd.hdr.packet_class = ADMIN_CLASS;
        cmd.hdr.length[0] = sizeof(cmd);
        cmd.hdr.length[1] = 0x00;
        cmd.hdr.type = cmdCode;
        cmd.hdr.crc = 0;

        rc = SendRemote( sk, (unsigned char *)&cmd, sizeof(cmd), 1 );
        if ( rc < 0 ) {
                if (!quiet)
                        printf("Send request failed.\n");
                return 1;
        }

        rc = ReadRemote(sk, 200, 0, 0xff);

        hdrp = (PktHdr *)ReadIt;

        if (rc < sizeof(AdminHdr)+EthHdrLen
                        || hdrp->type != cmdCode) {
                if (!quiet)
                        printf("Did not get ipinfo reply\n");
                return 2;
        }

        resp = (IpInfoPkt*)ReadIt;
        len  = (resp->hdr.length[1]<<8) + resp->hdr.length[0];

        ipAddr = ntohl(resp->ip.addr);
        ipMask = ntohl(resp->ip.mask);
        ipGate = ntohl(resp->ip.gate);

        if (debugLevel > 0) {
                if (len < sizeof(IpInfoPkt)+EthHdrLen)
                        printf("\nNo IP configuration present\n");
                else {
                        dumpIp("\n  IP Addr = ",ipAddr,"\n");
                        dumpIp("  IP Mask = ",ipMask,"\n");
                        dumpIp("  IP Gate = ",ipGate,"\n");
                }
        } else {
                if (len < sizeof(IpInfoPkt)+EthHdrLen)
                        printf( "None\n");
                else {
                        dumpIp("\"",ipAddr,"\",");
                        dumpIp("\"",ipMask,"\",");
                        dumpIp("\"",ipGate,"\"\n");
                }
        }
        return 0;
}

static int GetCurIPinfo(int sk, int quiet)
{
        return GetIPinfo(sk,ADMIN_IPINFO_GETCUR,quiet);
}

static int GetFlashIPinfo(int sk, int quiet)
{
        return GetIPinfo(sk,ADMIN_IPINFO_GET,quiet);
}


static int SendIPcmd(int sk,int quiet,int ipcmd)
{
        PktHdr *hdrp;
        int rc;
        struct {
                PktHdr hdr;
        }
        cmd;

        if (debugLevel > 0)
                printf("Erasing IP info\n");

        if ( RemoteType & SI_MASK )
                cmd.hdr.product_id = PRODUCT_ID_AA;
        else
                cmd.hdr.product_id = PRODUCT_ID_55;

        cmd.hdr.concentrator_index = 0xff;
        cmd.hdr.packet_class = ADMIN_CLASS;
        cmd.hdr.length[0] = sizeof(cmd);
        cmd.hdr.length[1] = 0x00;
        cmd.hdr.type = ipcmd;
        cmd.hdr.crc = 0;

        rc = SendRemote( sk, (unsigned char *)&cmd, sizeof(cmd), 1 );
        if ( rc < 0 ) {
                if (!quiet)
                        printf("Send request failed.\n");
                return 1;
        }

        rc = ReadRemote(sk, 5000, 0, 0xff);

        hdrp = (PktHdr *)ReadIt;

        if (rc < sizeof(AdminHdr)+EthHdrLen
                        || hdrp->type != ipcmd) {
                if (!quiet)
                        printf("Did not get correct reply\n");
                return 2;
        }
        return 0;
}

//------------------------------------
//  Sends the ADMIN_IPINFO_ERASE command to the device
static int EraseIPinfo(int sk,int quiet)
{
        return SendIPcmd(sk,quiet,ADMIN_IPINFO_ERASE);
}

//------------------------------------
//  Sends the ADMIN_IPINFO_DISABLE command to the device
static int DisableIPinfo(int sk,int quiet)
{
        return SendIPcmd(sk,quiet,ADMIN_IPINFO_DISABLE);
}

static int validByte(int v)
{
        if ((v<0) || (v>255)) {
                printf("Invalid byte value: %d.  Should be 0-255\n",v);
                return 0;
        }
        return 1;
}

static int parseIP(uint32_t *dest, char *str)
{
        int b1,b2,b3,b4;
        if (4 != sscanf(str,"%d.%d.%d.%d",&b1,&b2,&b3,&b4)) {
                printf("Invalid IP address '%s'. Should be nnn.nnn.nnn.nnn\n",str);
                return 0;
        }

        if (!(validByte(b1) && validByte(b2) && validByte(b3) && validByte(b4)))
                return 0;

        *dest = (b1<<24) + (b2<<16) + (b3<<8) + b4;
        return 1;
}


//------------------------------------
//  Sets the IP info in the device
static int SetIPinfo(int sk,
                     uint32_t ipAddr,
                     uint32_t ipMask,
                     uint32_t ipGate)
{
        IpInfoPkt cmd;
        IpInfoPkt *resp;
        int rc,len;
        uint32_t rIpAddr,rIpGate,rIpMask;

        if (debugLevel > 0) {
                printf("Setting %02x:%02x:%02x:%02x:%02x:%02x to ", RemoteMac[0], RemoteMac[1], RemoteMac[2], RemoteMac[3], RemoteMac[4], RemoteMac[5]);
                dumpIp("Addr=",ipAddr," ");
                dumpIp("Mask=",ipMask," ");
                dumpIp("Gateway=",ipGate,"\n");
        }

        if ( RemoteType & SI_MASK )
                cmd.hdr.product_id = PRODUCT_ID_AA;
        else
                cmd.hdr.product_id = PRODUCT_ID_55;

        cmd.hdr.concentrator_index = 0xff;
        cmd.hdr.packet_class = ADMIN_CLASS;
        cmd.hdr.length[0] = sizeof(cmd);
        cmd.hdr.length[1] = 0x00;
        cmd.hdr.packet_type = ADMIN_IPINFO_SET;


        if ((ipAddr & ipMask) != (ipGate & ipMask)) {
                dumpIp("Warning: Gateway ",ipGate," ");
                dumpIp("is not on local network ",ipAddr,"");
                dumpIp("/",ipMask,"\n");
        }

        cmd.ip.addr = htonl(ipAddr);
        cmd.ip.mask = htonl(ipMask);
        cmd.ip.gate = htonl(ipGate);

        rc = SendRemote( sk, (unsigned char *)&cmd, sizeof(cmd), 0);

        if ( rc < 0 ) {
                printf("Send request failed.\n");
                return 1;
        }

        rc = ReadRemote(sk, 1000, 0, 0xff);

        if (rc < (sizeof(IpInfoPkt)+EthHdrLen)) {
                printf("Did not get valid ipinfo reply\n");
                return 2;
        }

        resp = (IpInfoPkt*)ReadIt;
        len  = (resp->hdr.length[1]<<8) + resp->hdr.length[0];

        if (resp->hdr.packet_type != ADMIN_IPINFO_SET
                        || len != sizeof(IpInfoPkt)+EthHdrLen) {
                printf("Did not get valid ipinfo reply\n");
                return 3;
        }

        rIpAddr = ntohl(resp->ip.addr);
        rIpMask = ntohl(resp->ip.mask);
        rIpGate = ntohl(resp->ip.gate);

        if (debugLevel > 0) {
                printf("Reply info:\n");
                dumpIp("  IP Addr = ",rIpAddr,"\n");
                dumpIp("  IP Mask = ",rIpMask,"\n");
                dumpIp("  IP Gate = ",rIpGate,"\n");
        }


        if (rIpAddr != ipAddr ||
                        rIpMask != ipMask ||
                        rIpGate != ipGate) {
                printf("Reply data doesn't match request\n");
                return 4;
        }

        return 0;
}

static void DumpIt( unsigned int offset, unsigned char *buf, int len )
{
        int idx;
        unsigned int offs;
        char *hcp, *acp, hbuf[64], abuf[64];

        offs = offset;
        idx = 0;
        acp = abuf;
        hcp = hbuf;
        while( idx < len ) {
                sprintf(hcp,"%02x",buf[idx]);
                hcp += 2;
                *hcp = ' ';
                if ( isprint(buf[idx]) )
                        *acp++ = buf[idx];
                else
                        *acp++ = '.';
                idx += 1;
                if ( (idx&0x03) == 0 )
                        *hcp++ = ' ';
                if ( (idx&0x0f) == 0 ) {
                        *hcp = '\0';
                        *acp = '\0';
                        printf("%04x: %s %s\n",offs,hbuf,abuf);
                        hcp = hbuf;
                        acp = abuf;
                        *acp = '\0';
                        offs += 16;
                }
        }

        if ( abuf[0] ) {
                while( idx & 0x0f ) {
                        idx += 1;
                        *hcp++ = ' ';
                        *hcp++ = ' ';
                        if ( (idx&0x03) == 0 )
                                *hcp++ = ' ';
                }
                *hcp = '\0';
                *acp = '\0';
                printf("%04x: %s %s\n",offs,hbuf,abuf);
        }

        return;
}

static void DisplayHelp()
{
        printf("\n             IP Utility\n\n");
        printf(" ?                        Display help information.\n");
        printf(" ig                       Get/display flash IP config\n");
        printf(" ic                       Get/display current IP config\n");
        printf(" is <addr> <mask> <gate>  Set IP info\n");
        printf(" ie                       Erase IP info (use DHCP)\n");
        printf(" id                       Disable IP stack\n");
        printf(" l                        Load the remote.\n");
        printf(" s                        Start the remote.\n");
        printf(" z                        Shortcut for l, s\n");
        printf(" r                        Reset the remote.\n");
        printf(" L <filename>             Load the specified file.\n");
        printf(" x <filename>             Load and start the specified file.\n");
        printf(" q                        Quit the test.\n");
        printf("\n");

        return;
}

static void GetInput( int sk )
{
        char *cp, p1[128], p2[128], p3[128], p4[128], p5[128], p6[128];
        int rc, args;

        cp = readline("->");
        if (!cp)
                return;
        args = sscanf(cp, "%s %s %s %s %s %s\n", p1, p2, p3, p4, p5, p6 );
        free(cp);

        if ( args < 0 )
                return;

        switch( *p1 ) {
        case '?':
                DisplayHelp();
                break;

        case 'i':
                switch (p1[1]) {
                case 's':
                        if (args == 4) {
                                uint32_t ipAddr,ipGate,ipMask;
                                if (parseIP(&ipAddr,p2) &&
                                                parseIP(&ipMask,p3) &&
                                                parseIP(&ipGate,p4))
                                        SetIPinfo(sk,ipAddr,ipMask,ipGate);
                        } else
                                printf("Not enough parameters.  Usage: is ip-addr ip-mask ip-gate\n");
                        break;
                case 'g':
                        GetFlashIPinfo(sk,0);
                        break;
                case 'c':
                        GetCurIPinfo(sk,0);
                        break;
                case 'e':
                        EraseIPinfo(sk,0);
                        break;
                case 'd':
                        DisableIPinfo(sk,0);
                        break;
                default:
                        printf("Invalid command, please try again.\n");
                        break;
                }
                break;

        case 'l':
                rc = LoadRemote(sk,NULL);
                if ( rc )
                        printf("Cannot load remote\n");
                break;

        case 'L':
                if (args != 2) {
                        printf("Please specify filename.\n");
                        break;
                }
                rc = LoadRemote(sk,p2);
                if ( rc )
                        printf("Cannot load '%s'\n",p2);
                break;

        case 'x':
                if (args != 2) {
                        printf("Please specify filename.\n");
                        break;
                }
                rc = LoadRemote(sk,p2);
                if ( rc )
                        printf("Cannot load '%s'\n",p2);
                else {
                        rc = StartRemote(sk);
                        if ( rc )
                                printf("Cannot start remote\n");
                }
                break;

        case 'q':
                StopNow = 1;
                break;

        case 'r':
                rc = ResetRemote( sk );
                if ( rc )
                        printf("Cannot reset remote\n");
                break;

        case 's':
                rc = StartRemote(sk);
                if ( rc )
                        printf("Cannot start remote\n");
                break;

        case 'z':
                rc = LoadRemote(sk,NULL);
                printf("\n");
                if ( rc )
                        printf("Cannot load remote\n");
                else {
                        rc = StartRemote(sk);
                        if ( rc )
                                printf("Cannot start remote\n");
                }
                break;

        default:
                printf("Invalid command, please try again.\n");
                break;
        }

        return;
}

/* display usage message and exit */
void usage(void)
{
        printf("Usage: %s [-e <etherAddr> [-rqE] [-a <Addr> -m <Mask> -g <Gate> ]] [-x <dbgLvl>]\n",progName);
        exit(1);
}

int sk;
int resetOnExit;

void cleanUp(void)
{
        while (DiscoverTable) {
                DiscoverNode *tmp = DiscoverTable;
                DiscoverTable = DiscoverTable->next;
                if (tmp->displayString)
                        free(tmp->displayString);
                free(tmp);
        }

        if (resetOnExit)
                ResetRemote(sk);
        close(sk);
}


int main(int argc, char **argv)
{
        int rc, flags, argt;
        unsigned u;
        struct ifreq ifr;
        unsigned int mac0, mac1, mac2, mac3, mac4, mac5;
        uint32_t ipAddr=0,ipMask=0,ipGate=0;
        struct sockaddr_ll sockaddr;

        int valuesPresent = 0;
        int doQuery = 0;
        int doErase = 0;
        int doReset = 0;
        int doLoad  = 0;
        int noAutoload  = 0;
        int doList  = 0;
        int check_network = 1;
        char *filename  = NULL;

        progName = argv[0];

        do {
                argt = getopt( argc, argv, "qlrELnx:e:d:a:m:g:f:" );
                if ( argt > 0 ) {
                        if ( argt == 'e' ) {
                                if ( 6 != sscanf( optarg, "%x:%x:%x:%x:%x:%x",&mac0,&mac1,&mac2,&mac3,&mac4,&mac5)) {
                                        printf("MAC address must be in form: xx:xx:xx:xx:xx:xx\n");
                                        return 3;
                                }
                                RemoteMac[0] = mac0;
                                RemoteMac[1] = mac1;
                                RemoteMac[2] = mac2;
                                RemoteMac[3] = mac3;
                                RemoteMac[4] = mac4;
                                RemoteMac[5] = mac5;
                                check_network = 0;
                        } else if (argt == 'a') {
                                if (!parseIP(&ipAddr,optarg))
                                        return 1;
                                ++valuesPresent;
                        } else if (argt == 'm') {
                                if (!parseIP(&ipMask,optarg))
                                        return 1;
                                ++valuesPresent;
                        } else if (argt == 'g') {
                                if (!parseIP(&ipGate,optarg))
                                        return 1;
                                ++valuesPresent;
                        } else if (argt == 'f')
                                filename = strdup(optarg);
                        else if (argt == 'x')
                                debugLevel = atol(optarg);
                        else if ( argt == 'd' )
                                ethdevname = optarg;
                        else if ( argt == 'E') {
                                doErase = 1;
                                interactive = 0;
                        } else if (argt == 'n')
                                noAutoload = 1;
                        else if ( argt == 'r') {
                                doReset = 1;
                                interactive = 0;
                        } else if ( argt == 'l') {
                                doLoad = 1;
                                noAutoload = 1;
                                interactive = 0;
                        } else if ( argt == 'L') {
                                doList = 1;
                                interactive = 0;
                                check_network = 1;
                        } else if ( argt == 'q' ) {
                                doQuery = 1;
                                interactive = 0;
                        }
                }
        } while( argt > 0 );

        if (valuesPresent > 0 && valuesPresent < 3)
                usage();

        if (valuesPresent == 3)
                interactive = 0;

        if (!doList && check_network && !interactive) {
                printf("%s: You must specify -e <ether-addr> when using one of\n  -E\n  -q\n  -a addr> -m <mask> -g <gate>\n",progName);
                usage();
        }

        if (interactive && debugLevel == 0)
                debugLevel = 1;

        if (debugLevel > 0)
                printf("Using network device '%s'\n",ethdevname);

        sk = socket( AF_PACKET, SOCK_RAW, htons(0x11fe) );
        if ( sk < 0 ) {
                perror("socket");
                return 1;
        }

        atexit(cleanUp);

        strcpy( ifr.ifr_name, ethdevname );
        ifr.ifr_name[(sizeof(ifr.ifr_name))-1] = '\0';

        if (ioctl( sk, SIOCGIFINDEX, &ifr )) {
                perror("SIOCGIFINDEX");
                close( sk );
                return 1;
        }

        sockaddr.sll_family = AF_PACKET;
        sockaddr.sll_protocol = htons(0x11fe);
        sockaddr.sll_ifindex = ifr.ifr_ifindex;

        // only want to see packets from specified ethernet interface

        if (bind(sk,(struct sockaddr*) &sockaddr, sizeof sockaddr)) {
                perror("Couldn't bind packet socket");
                exit(1);
        }

        flags = fcntl( sk, F_GETFL );
        rc = fcntl( sk, F_SETFL, (flags|O_NONBLOCK) );
        if ( rc < 0 ) {
                perror("fcntl");
                close( sk );
                return 1;
        }

        u = sizeof sockaddr;
        getsockname(sk, (struct sockaddr*) &sockaddr, &u);

        memcpy(MyMac,sockaddr.sll_addr,6);

        if (check_network)
                if (CheckNetwork(sk))
                        return 1;

        if (doList) {
                ListNodes(DiscoverTable);
                return 0;
        }

        if (!SetRemoteType())
                return 4;

        // don't autoload devicemaster

        if (!noAutoload && (RemoteType != REMOTE_DEVICEMASTER))
                resetOnExit = (0 == AutoLoadRemote(sk,filename));

        if (!interactive) {
                if (doReset) {
                        if (ResetRemote(sk))
                                return 1;
                        sleep(3);
                }

                if (doLoad) {
                        if (LoadRemote(sk,filename))
                                return 1;
                        if (StartRemote(sk))
                                return 1;
                        sleep(2);
                }

                if (doQuery)
                        if (GetFlashIPinfo(sk,1) && GetFlashIPinfo(sk,0))
                                return 1;

                if (doErase)
                        if (EraseIPinfo(sk,1) && EraseIPinfo(sk,0))
                                return 1;

                if (valuesPresent==3)
                        return SetIPinfo(sk,ipAddr,ipMask,ipGate);

                return 0;
        } else {
                DisplayHelp();
                while (!StopNow)
                        GetInput(sk);
        }
        return 0;
}
