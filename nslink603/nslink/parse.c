/*
 * parse.c --- parse configuration file for rpshsid
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <ctype.h>
#include <arpa/inet.h>
#include "nslink.h"
#include "nslinkd.h"


/* macro used to write error message to both stderr and syslog */
#define eprintf(fmt, args...) \
  do {fprintf(stderr,fmt , ## args); syslog(LOG_WARNING, fmt , ## args);} while (0)

char *boot_file4_8 = 0;
char *boot_file2 = 0;
char *boot_fileDM = 0;

struct daemon_si_box_config box_config[MAX_NRP_SIS + 1];
int num_box_config = 0;

/*
 * Parse an ethernet address.  Return -1 on error, 0 for success.
 */
static int parse_ether_addr(const char *str, unsigned char addr[6])
{
        int a, b, c, d, e, f;

        if (sscanf(str, "%2x:%2x:%2x:%2x:%2x:%2x", &a, &b, &c, &d, &e, &f) != 6)
                return -1;
        addr[0] = a;
        addr[1] = b;
        addr[2] = c;
        addr[3] = d;
        addr[4] = e;
        addr[5] = f;
        return 0;
}

/*
 * Print an ethernet address
 */
char *sprint_ether_addr(unsigned char addr[6])
{
        static char buf[40];

        sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1],
                addr[2], addr[3], addr[4], addr[5]);
        return buf;
}

/*
 * Return a copy of a string using allocated memory.
 */
char *dup_string(const char *s)
{
        char *str;

        str = malloc(strlen(s) + 1);
        if (str)
                strcpy(str, s);
        return str;
}

void parse_config(const char *cfg_filename)
{
        FILE *f;
        char line[256], buf[256], *cp;
        char arg1[80], arg2[80], arg3[80], arg4[80], arg5[80], arg6[80], arg7[80];
        int argc, ifline=0, portnum;
        struct daemon_si_box_config *cfg=NULL;
        int line_num = 0;
        struct in_addr inaddr;
        int i,next_port;

        f = fopen(cfg_filename, "r");
        if (!f) {
                perror("fopen");
                exit(1);
        }

        while (!feof(f)) {
                if (!fgets(line, sizeof(line), f))
                        break;
                line_num++;
                line[sizeof(line) - 1] = 0;
                strcpy(buf, line);
                cp = strchr(buf, '\n');
                if (cp)
                        *cp = '\0';
                cp = strchr(buf, '#');
                if (cp)
                        *cp = '\0';
                arg1[0] = arg2[0] = arg3[0] = arg4[0] = arg5[0] = arg6[0] = arg7[0] = 0;
                argc = sscanf(buf, "%s %s %s %s %s %s %s", arg1, arg2, arg3, arg4, arg5, arg6, arg7);
                if (argc <= 0)
                        continue;

                cfg = box_config + num_box_config;
                ifline = 0;

                if (strcmp(arg1, "bootfile") == 0 || strcmp(arg1, "bootfile-4_8p") == 0) {
                        /* bootfile */
                        if (*arg2 == '\0')
                                eprintf("Missing bootfile in line #%d, using default.\n", line_num);
                        else
                                boot_file4_8 = dup_string(arg2);
                } else if (strcmp(arg1, "bootfile-2p") == 0) {
                        /* bootfile */
                        if (*arg2 == '\0')
                                eprintf("Missing bootfile in line #%d, using default.\n", line_num);
                        else
                                boot_file2 = dup_string(arg2);
                } else if (strcmp(arg1, "bootfile-DM") == 0) {
                        /* bootfile */
                        if (*arg2 == '\0')
                                eprintf("Missing bootfile in line #%d, using default.\n", line_num);
                        else
                                boot_fileDM = dup_string(arg2);
                } else if (strchr(arg1,'.') && ((argc>=2)&&(argc<=5))) {
                        /* IP address */
                        int n = -1;		//number of ports
                        long m = 120;		//link timeout seconds
                        long s = 0;		//scan period ms
                        char *ep;
                        long t = 0;		//retransmit timer

                        n = atoi(arg2);

                        if (n!=1 && n!=2 && n!=4 && n!=8 && n!=16 && n!= 32)
                                eprintf("Invalid number of ports at line #%d, line ignored.\n", line_num);
                        else if ((argc>=3) && ((m=strtol(arg3,&ep,10)),*ep))
                                eprintf("Invalid link timeout at line $%d, line ignored.\n",line_num);
                        else if ((argc>=4) && ((s=strtol(arg4,&ep,10)),*ep))
                                eprintf("Invalid scan period at line $%d, line ignored.\n",line_num);
                        else if ((argc>=5) && ((t=strtol(arg5,&ep,10)),*ep))
                                eprintf("Invalid Retransmit time at line $%d, line ignored.\n",line_num);
                        else if (!inet_aton(arg1, &inaddr))
                                eprintf("Syntax error in IP address at line #%d, line ignored.\n", line_num);
                        else {
                                cfg->ip_addr = inaddr.s_addr;
                                cfg->interface[0] = '\0';
                                cfg->num_lines = n;
                                cfg->link_timeout = m;
                                cfg->scan_period = s;
                                if(t==0)			//default retransmit timer 
                                        t=200;
                                if( (t>=40 && t<=200) && (t%20==0) )	//legitimate retransmit timer value
                                        cfg->rexmit_timer = t;
                                ifline = 1;
                        }
                } else if (strchr(arg1,':') && ((argc>=3)&&(argc<=6))) {
                        /* Ethernet address */
                        int n = -1;		//number of ports
                        long m = 120;		//link timeout seconds
                        long s = 10;		//scanperiod ms
                        char *ep;
                        long t = 0;		//retransmit timer

                        n = atoi(arg3);

                        if (parse_ether_addr(arg1, cfg->ether_addr))
                                eprintf("Syntax error in Ethernet address at line #%d, line ignored.\n", line_num);
                        else if (n!=1 && n!=2 && n!=4 && n!=8 && n!=16 && n!=32)
                                eprintf("Invalid number of ports at line #%d, line ignored.\n", line_num);
                        else if ((argc>=4) && ((m=strtol(arg4,&ep,10)),*ep))
                                eprintf("Invalid link timeout at line $%d, line ignored.\n",line_num);
                        else if ((argc>=5) && ((s=strtol(arg5,&ep,10)),*ep))
                                eprintf("Invalid scan period at line $%d, line ignored.\n",line_num);
                        else if ((argc>=6) && ((t=strtol(arg6,&ep,10)),*ep))
                                eprintf("Invalid Retransmit time at line $%d, line ignored.\n",line_num);
                        else {
                                strncpy(cfg->interface, arg2, sizeof(cfg->interface));
                                cfg->interface[sizeof(cfg->interface - 1)] = '\0';
                                cfg->num_lines = n;
                                cfg->link_timeout = m;
                                cfg->scan_period = s;
    
				if(t==0)			//default retransmit timer
  					t=200;
				if( (t>=40 && t<=200) && (t%20==0) && (t!=0))		//legitimate retransmit timer value
                                	cfg->rexmit_timer = t;
                            ifline = 1;
                        }
                } else if (isdigit(arg1[0]) && ((argc>=2)&&(argc<=4))) {
                        /* RsMode */
                        portnum = atoi(arg1);
                        if (!num_box_config || (portnum < 1) || (portnum > MAX_NRP_PORTS))
                                eprintf("Invalid port mode line at line #%d, line ignored.\n",line_num);
                        else {
                                portnum -= 1;
                                cfg = box_config + (num_box_config - 1);
                                if (strcmp(arg2, "232") == 0)
                                        cfg->rsmode[portnum] = SI_RSMODE_232;
                                else if (strcmp(arg2, "422") == 0)
                                        cfg->rsmode[portnum] = SI_RSMODE_422;
                                else if (strcmp(arg2, "485") == 0)
                                        cfg->rsmode[portnum] = SI_RSMODE_485;
                                else if (strcasecmp(arg2, "485_2-wire") == 0)
                                        cfg->rsmode[portnum] = SI_RSMODE_485;
                                else if (strcasecmp(arg2, "485_4-wire_Master") == 0)
                                        cfg->rsmode[portnum] = SI_RSMODE_485FDM;
                                else if (strcasecmp(arg2, "485_4-wire_Slave") == 0)
                                        cfg->rsmode[portnum] = SI_RSMODE_485FDS;
                                else if (strcasecmp(arg2, "Off") == 0)
                                        cfg->rsmode[portnum] = SI_RSMODE_OFF;
                                else {
                                        eprintf("Invalid RsMode in line #%d, disabling port", line_num);
                                        cfg->rsmode[portnum] = SI_RSMODE_OFF;
                                }

                                if (argc >= 3) {
                                        char *ep;
                                        long int ict = 0;
                                        ict = strtol(arg3,&ep,10);
                                        if ((ict<0) || *ep) {
                                                eprintf("Invalid inter-char timeout at line %d, ignoring",line_num);
                                                ict = 0;
                                        }
                                        cfg->inter_char_timeout[portnum] = ict;
                                }

                                cfg->low_latency[portnum] = 0;
                                cfg->rx_fifo_disable[portnum] = 0;

                                if (argc >= 4) {
                                        // parse options
                                        char *p = strtok(arg4,",");
                                        while (p) {
                                                if (!strcasecmp(p,"low_latency"))
                                                        cfg->low_latency[portnum] = 1;
                                                else if (!strcasecmp(p,"rx_fifo_disable"))
                                                        cfg->rx_fifo_disable[portnum] = 1;
                                                else
                                                        eprintf("Invalid option %s at line %d",p,line_num);
                                                p = strtok(NULL,",");
                                        }
                                } else
                                        cfg->low_latency[portnum] = 0;
                        }
                } else {
                        eprintf("Unrecognized config command in line #%d, line ignored.\n", line_num);
                        break;
                }

                if (ifline) {
                        if ((cfg->num_lines > MAX_NRP_PORTS) || (cfg->num_lines <= 0))
                                ifline = 0;
                }

                if (ifline) {
                        num_box_config++;
                        if (num_box_config > MAX_NRP_SIS) {
                                eprintf("Too many NS-Link listed in the configuration file. NS-Link device at %s ignored.\n", arg1);
                                num_box_config--;
                        }
                }
        }
        fclose(f);

        // fill in base port values

        next_port = 0;
        for (i=0; i<num_box_config; ++i) {
                box_config[i].base_port = next_port;
                next_port += box_config[i].num_lines;
        }

}
