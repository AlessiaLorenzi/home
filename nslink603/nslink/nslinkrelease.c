#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <netpacket/packet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <errno.h>

void dumpHex(FILE *fp, unsigned char *p, int n)
{
        while (n) {
                fprintf(fp,"%02x ",*p);
                ++p;
                --n;
        }
}

void stripnl(char *p)
{
        char *s = strrchr(p,'\n');
        if (s)
                *s = '\0';
}

// using tempnam generate linker warnings under gcc 2.96, so we'll write our own :(
#if !defined(P_tmpdir)
# define P_tmpdir "/tmp"
#endif

static char *myTempName(char *prefix)
{
        int tries;
        char *tmpdir;
        static char path[256];
        struct stat statbuf;

        tmpdir = getenv("TMPDIR");
        if (!tmpdir)
                tmpdir = P_tmpdir;
        if (strlen(tmpdir)+strlen(prefix)+22 >= sizeof(path)) {
                fprintf(stderr,"Temporary directory path/prefix too long: %s/%s\n",tmpdir,prefix);
                return NULL;
        }
        srandom(time(NULL));
        for (tries=0; tries<10; ++tries) {
                int s;
                sprintf(path,"%s/%s.%08lx.%08lx",tmpdir,prefix,(long)time(NULL),(long)random());
                s = stat(path,&statbuf);
                if (s)
                        return path;
        }
        return NULL;
}


int lookupMajor(char *drvName)
{
        char buf[64];
        FILE *fp;
        char name[64];
        int major;
        fp = fopen("/proc/devices","r");
        while (!feof(fp)) {
                if (!fgets(buf,sizeof buf,fp))
                        break;
                if (sscanf(buf,"%d%s",&major,name) != 2)
                        continue;
                if (!strcmp(name,drvName))
                        return major;
        }
        return -1;
}

int readTimeout(int fd, void *buf, size_t count, int timeoutSeconds)
{
        static struct timeval tv = {
                                           0,0
                                   };
        fd_set rdset,wrset,erset;
        int n;

        tv.tv_sec = timeoutSeconds;

        FD_ZERO(&rdset);
        FD_ZERO(&wrset);
        FD_ZERO(&erset);
        FD_SET(fd,&rdset);
        FD_SET(fd,&erset);
        n = select(fd+1,&rdset,&wrset,&erset,&tv);
        if (n != 1) {
                fprintf(stderr,"No response from hub\n");
                exit(20);
        }
        if (FD_ISSET(fd,&erset)) {
                perror("select");
                exit(21);
        }
        if (!FD_ISSET(fd,&rdset)) {
                fprintf(stderr,"oops, shouldn't be here... select failed!\n");
                exit(22);
        }
        n = read(fd,buf,count);
        if (n<0) {
                perror("read");
                exit(23);
        }
        return n;
}

void verifyResponse(unsigned char *edata, int ecount, unsigned char *gdata, int gcount)
{
        if (ecount != gcount || memcmp(edata,gdata,ecount)) {
                fprintf(stderr, "invalid resp from hub:\n");
                fprintf(stderr, "  expected: [%2d] ",ecount);
                dumpHex(stderr,edata,ecount);
                fprintf(stderr, "\n");
                fprintf(stderr, "       got: [%2d] ",gcount);
                dumpHex(stderr,gdata,gcount);
                fprintf(stderr, "\n");
                exit(31);
        }
}

void readVerifyResponse(int fd, unsigned char *expected, int count, int timeout)
{
        int n;
        unsigned char buf[128];
        n = readTimeout(fd,buf,sizeof buf,timeout);
        verifyResponse(expected,n,buf,count);
}



void macRelease(unsigned char* dstMac, char *intf, int port)
{
        int s;
        unsigned u;
        struct sockaddr_ll sockaddr;
        struct ifreq ifr;
        char myMac[6];
        unsigned char cmdBuffer[64];
        unsigned char rspBuffer[64];
        unsigned char buf[64];
        unsigned short proto = htons(0x11fe);
        static unsigned char comtrolHeader[] = {0xaa,0xfe,0x55,0x01,0x00,0x00};
        static unsigned char assignCommand[] = {0x73,0x00};
        static unsigned char assignResp[] = {0x74,0x00};
        static unsigned char releaseCommand[] = {0x62,0x00,0x78,0x00};
        static unsigned char releaseResp[] = {0x62,0x00,0x76,0x21,0x00};
        static unsigned char idReqCommand[11] = {0x01,0x08,0x00,0x02};

        //printf("macRelease %02x:%02x:%02x:%02x:%02x:%02x %s %d\n",dstMac[0],dstMac[1],dstMac[2],dstMac[3],dstMac[4],dstMac[5],intf,port);

        if ((s=socket(AF_PACKET, SOCK_RAW, proto)) < 0) {
                perror("socket");
                exit(2);
        }

        // look up interface index (needed for bind())
        strncpy(ifr.ifr_name, intf, sizeof(ifr.ifr_name));
        ifr.ifr_name[(sizeof(ifr.ifr_name))-1] = '\0';
        if (ioctl(s, SIOCGIFINDEX, &ifr)) {
                fprintf(stderr,"Couldn't find interface '%s'\n",intf);
                exit(3);
        }

        // only want to see packets from specified ethernet interface
        sockaddr.sll_family = AF_PACKET;
        sockaddr.sll_protocol = proto;
        sockaddr.sll_ifindex = ifr.ifr_ifindex;
        if (bind(s,(struct sockaddr*) &sockaddr, sizeof sockaddr)) {
                perror("bind");
                exit(4);
        }

        // lookup our MAC address
        u = sizeof sockaddr;
        if (getsockname(s, (struct sockaddr*) &sockaddr, &u)) {
                perror("getsockname");
                exit(5);
        }
        memcpy(myMac,sockaddr.sll_addr,6);

        // async command header
        memcpy(cmdBuffer,dstMac,6);
        memcpy(cmdBuffer+6,myMac,6);
        memcpy(cmdBuffer+12,&proto,2);
        memcpy(cmdBuffer+14,comtrolHeader,6);
        // async response header (for verification purposes)
        memcpy(rspBuffer,myMac,6);
        memcpy(rspBuffer+6,dstMac,6);
        memcpy(rspBuffer+12,&proto,2);
        memcpy(rspBuffer+14,comtrolHeader,6);

        // ID request
        memcpy(idReqCommand+4,myMac,6);
        memcpy(cmdBuffer+16,idReqCommand,sizeof idReqCommand);
        write(s,cmdBuffer,16+(sizeof idReqCommand));
        readTimeout(s,buf,sizeof buf, 3);

        // rest are async commands, so put header back the way it was
        memcpy(cmdBuffer+14,comtrolHeader,6);

        // assign command
        memcpy(cmdBuffer+20,assignCommand,2);
        write(s,cmdBuffer,22);
        memcpy(rspBuffer+20,assignResp,2);
        readTimeout(s,buf,sizeof buf, 3);
        verifyResponse(assignResp,2,buf+20,2);

        // release command
        releaseCommand[1] = port;
        memcpy(cmdBuffer+20,releaseCommand,4);
        write(s,cmdBuffer,24);
        releaseResp[1] = port;
        readTimeout(s,buf,sizeof buf, 3);
        verifyResponse(releaseResp,5,buf+20,5);

        // reset HDLC connection
        cmdBuffer[17] = 0x03;
        cmdBuffer[20] = 0x00;
        write(s,cmdBuffer,21);

        close(s);
}


void tcpRelease(char *host, int port)
{
        int s;
        struct hostent *h;
        struct sockaddr_in sin;
        static unsigned char relcmd[] = {0x62,0,0x78};
        static unsigned char relrsp[] = {0x62,0,0x76,0x21};

        //printf("tcpRelease %s %d\n",host,port);

        relcmd[1] = port;
        relrsp[1] = port;

        if ((s=socket(AF_INET,SOCK_STREAM,0)) < 0) {
                perror("socket");
                exit(2);
        }
        if ((h=gethostbyname(host)) == NULL) {
                perror("gethostbyname");
                exit(3);
        }
        sin.sin_family = AF_INET;
        sin.sin_port = htons(0x11fe);
        memcpy(&sin.sin_addr,h->h_addr_list[0],sizeof sin.sin_addr);
        if (connect(s,(struct sockaddr*)&sin,sizeof sin)) {
                perror("connect");
                exit(4);
        }
        if (write(s,relcmd,sizeof relcmd) != sizeof relcmd) {
                perror("write");
                exit(5);
        }
        readVerifyResponse(s,relrsp,sizeof relrsp,3);
        close(s);
}


void devRelease(char *devName)
{
        int fd;
        struct stat s;
        int major,minor;
        int nslinkMajor;
        int nslinkCtlMajor;
        char *name;
        //printf("devRelease %s\n",devName);

        if (stat(devName,&s)) {
                perror("stat");
                exit(2);
        }
        major = s.st_rdev >> 8;
        minor = s.st_rdev & 0xff;

        nslinkMajor = lookupMajor("ttySI");
        if (nslinkMajor < 0 ) {
                fprintf(stderr,"nslink driver not running\n");
                exit(3);
        }
        if (major != nslinkMajor) {
                fprintf(stderr,"%s is not nslink device\n",devName);
                exit(4);
        }
        nslinkCtlMajor = lookupMajor("NSLinkctl");
        if (nslinkCtlMajor < 0) {
                fprintf(stderr,"nslink control device not found\n");
                exit(5);
        }
        name = myTempName(".nslnkrelease");
        if (!name) {
                fprintf(stderr,"error generating temp filename\n");
                exit(6);
        }
        if (mknod(name,S_IFCHR | 0600,nslinkCtlMajor << 8)) {
                perror("mknod");
                exit(7);
        }
        if ((fd=open(name,O_RDONLY)) < 0) {
                perror("open");
                exit(8);
        }
        if (ioctl(fd,0x0056530E,minor)) {
                if (errno == ETIME)
                        fprintf(stderr,"no response from hub\n");
                else
                        perror("ioctl");
                exit(9);
        }
}

#define SI_RELEASE_MODULE       0x0056530F

static void moduleRelease(void)
{
        char resp;
        char *name;
        int nslinkCtlMajor, fd;

        printf("This will allow the module to be unloaded if the system \n");
        printf("incorrectly believes it to be in use.\n\n");
        printf("WARNING:  Be sure to close all serial ports. \n\n");
        printf("Do you wish to continue? >>> ");
        scanf("%c", &resp);

        nslinkCtlMajor = lookupMajor("NSLinkctl");
        if (nslinkCtlMajor < 0)  {
                fprintf(stderr,"nslink control device not found\n");
                exit(5);
        }
        if ((resp == 'y') || (resp == 'Y')) {
                printf("\nReleasing NSLINK module.... ");

                name = myTempName(".nslnkrelease");
                if (!name) {
                        fprintf(stderr,"error generating temp filename\n");
                        exit(6);
                }
                if (mknod(name,S_IFCHR | 0600, nslinkCtlMajor << 8))   {
                        perror("mknod");
                        exit(7);
                }
                if ((fd = open(name,O_RDONLY)) < 0)   {
                        perror("open");
                        exit(8);
                }
                if (ioctl(fd, SI_RELEASE_MODULE, 0))   {
                        if (errno == ETIME)
                                fprintf(stderr,"no response from hub\n");
                        else
                                perror("ioctl");
                        exit(9);
                }
                printf("DONE.  Retry the module unload. \n");
        } else {
                printf("\nNo action taken \n");
        }
}

int parseEthAddr(unsigned char *mac, char *s)
{
        unsigned b0,b1,b2,b3,b4,b5;
        if (6 != sscanf(s,"%x:%x:%x:%x:%x:%x",&b0,&b1,&b2,&b3,&b4,&b5))
                return 0;
        if (mac) {
                mac[0] = b0;
                mac[1] = b1;
                mac[2] = b2;
                mac[3] = b3;
                mac[4] = b4;
                mac[5] = b5;
        }
        return 1;
}

void usage(char *prog)
{
        fprintf(stderr,"usage: %s /dev/ttySIxx\n",prog);
        fprintf(stderr,"       %s IPhost portnumber\n",prog);
        fprintf(stderr,"       %s EthAddr [ethN] portnumber\n",prog);
        //  fprintf(stderr,"       %s module - Release the module if it won't unload \n",prog);
}


int main(int argc, char *argv[])
{
        unsigned char mac[6];

        if (argc == 3 && parseEthAddr(mac,argv[1]) && isdigit(argv[2][0]))
                macRelease(mac,"eth0",atoi(argv[2]));
        else if (argc == 4 && parseEthAddr(mac,argv[1]) && !strncmp(argv[2],"eth",3) && isdigit(argv[3][0]))
                macRelease(mac,argv[2],atoi(argv[3]));
        else if (argc == 3 && isdigit(argv[2][0]))
                tcpRelease(argv[1],atoi(argv[2]));
        else if (argc == 2) {
                if (strcmp(argv[1], "module"))
                        devRelease(argv[1]);
                else
                        moduleRelease();
        } else {
                usage(argv[0]);
                return 1;
        }
        return 0;
}
