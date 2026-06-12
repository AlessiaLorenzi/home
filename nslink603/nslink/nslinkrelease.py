#!/usr/local/bin/python
# needs Python 2.0 or newer
import sys
import os,stat,fcntl
import socket,select
import struct
import re
import commands

def toHex(s):
    return " ".join([("%02x" % ord(c)) for c in s])

def lookupMajor(driverName):
    for line in open("/proc/devices","r").readlines():
        word = line.split()
        if len(word) == 2 and word[1] == driverName:
            return int(word[0])
    msg = "driver named '%s' not found" % driverName
    raise msg

def readTimeout(s):
    r,w,e = select.select([s],[],[s],3.0)
    if len(r) == 0 and len(w) == 0 and len(e) == 0:
        raise "timeout -- no response from hub"
    return s.recv(1500)

def verify(g,e):
    if e != g:
        msg = "bad response from hub. expected [%s], got [%s]\n" % (toHex(e),toHex(g))
        raise msg

def verifymac(g,e):
    if (len(g) < len(e)) or (e != g[:len(e)]):
        msg = "bad response from hub. expected [%s], got [%s]\n" % (toHex(e),toHex(g))
        raise msg
    
def tcpRelease(host,port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host,0x11fe))
    s.send(struct.pack("!BBB",0x62,port,0x78))
    verify(readTimeout(s),struct.pack("!BBBB",0x62,port,0x76,0x21))
    s.close()

def macRelease(macStr,intf,port):
    proto = 0x11fe
    dstAddr = "".join(map(chr,[int(x,16) for x in macStr.split(":")]))
    s = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, proto)
    s.bind((intf,proto))
    ifName,ifProto,pktType,hwType,hwAddr = s.getsockname()
    srcAddr = hwAddr
    cmdhdr = struct.pack("!6s6shBBBBBB",dstAddr,srcAddr,proto,0xaa,0xfe,0x55,0x01,0x00,0x00)
    rsphdr = struct.pack("!6s6shBBBBBB",srcAddr,dstAddr,proto,0xaa,0xfe,0x55,0x01,0x00,0x00)
    # ID request
    s.send(cmdhdr[:16]+struct.pack("!BhB6sB",0x01,0x0800,0x02,srcAddr,0x00))
    readTimeout(s) # ignore ID response
    # Assign request
    s.send(cmdhdr + struct.pack("!BB",0x73,0))
    verifymac(readTimeout(s),rsphdr+struct.pack("!BB",0x74,0))
    # Release command
    s.send(cmdhdr + struct.pack("!BBBB",0x62,port,0x78,0))
    verifymac(readTimeout(s),rsphdr+struct.pack("!BBBBB",0x62,port,0x76,0x21,0))
    # Resync HDLC connection
    s.send(cmdhdr[:-3] + struct.pack("!BBBB",3,0,0,0))
    s.close()

def devRelease(devName):
    s = os.stat(devName)
    if not stat.S_ISCHR(s[stat.ST_MODE]):
        msg = "devRelease: '%s' is not char device" % devName
        raise msg
    # os.stat() dosn't return the dev type, so use /usr/sbin/stat
    s,o = commands.getstatusoutput("stat "+devName)
    if s:
        msg = "devRelease: error running /usr/bin/stat: %d" % s
        raise msg
    r = re.compile("Device type: ([0-9]{1,3}),([0-9]{1,3})")
    m = r.search(o)
    if not m:
        raise "devRelease: error parsing stat output"
    major = int(m.group(1))
    minor = int(m.group(2))
    if major != lookupMajor("ttySI"):
        raise "not nslink device"
    ctlmajor = lookupMajor("NSLinkctl")
    t = os.tempnam(None,".nslnkctl")
    s,o = commands.getstatusoutput("mknod --mode=600 %s c %d %d" % (t,ctlmajor,0))
    if s:
        msg = "error running mknod : %s" % o
        raise msg
    fd = os.open(t,os.O_RDONLY)
    os.unlink(t)
    fcntl.ioctl(fd,0x0056530E,minor)
    os.close(fd)

    
def main(argv):
    ethAddressRE = re.compile(r"^[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}$")
    portNumberRE = re.compile(r"^[0-9]{1,2}$")

    def usage(prog):
        sys.stderr.write("usage: %s IPhost port\n" % prog)
        sys.stderr.write("       %s ethAddr [ethX] port\n"% prog)
        sys.stderr.write("       %s /dev/ttySIx\n" % prog)
    
    if len(argv) < 2:
        usage(argv[0])
        sys.exit(1)
        
    if len(argv) == 3 and ethAddressRE.match(argv[1]) and portNumberRE.match(argv[2]):
        macRelease(argv[1],'eth0',int(argv[2]))
    elif len(argv) == 4 and ethAddressRE.match(argv[1]) and argv[2].startswith("eth") and portNumberRE.match(argv[3]):
        macRelease(argv[1],argv[2],int(argv[3]))
    elif len(argv) == 2:
        devRelease(argv[1])
    elif len(argv) == 3 and portNumberRE.match(argv[2]):
        tcpRelease(argv[1],int(argv[2]))
    else:
        usage(argv[0])
        sys.exit(2)
        
    sys.exit(0)

main(sys.argv)
