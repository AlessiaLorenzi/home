################################################
#
#     Comtrol DeviceMaster Linux Driver Makefile
#
#     Works with Linux kernel version 2.4, 2.6, 3.0
#
#################################################

# FYI: to get 2.6 verbose building export this var to env: KBUILD_VERBOSE=1

# Retrieve the running kernel version
KVER=$(shell uname -r)

RM=rm -f
PWD:= $(shell pwd)
CURRENTDATE=$(shell date +%Y%m%d%H%M%S)

#  Find the kernel source.
ifeq ($(shell test -d /usr/src/linux && echo true),true)
  LINUX_SRC=/usr/src/linux
endif
ifeq ($(shell test -d /usr/src/kernel-headers-$(KVER) && echo true),true)
  LINUX_SRC=/usr/src/kernel-headers-$(KVER)
endif
ifeq ($(shell test -d /lib/modules/$(KVER)/build && echo true),true) 
  LINUX_SRC=/lib/modules/$(KVER)/build
endif

LINUX_VERSION=$(shell uname -r | awk '{ split($$0,a,"[.-]"); print a[1]"."a[2]}')

ifeq ($(LINUX_VERSION),2.4)
  OLD_BUILD=yes
else
  OLD_BUILD=no
endif

ifeq ($(MAKELEVEL),0)
$(warning LINUX_VERSION: $(LINUX_VERSION))
$(warning LINUX_SRC: $(LINUX_SRC))
endif

# Detect Linux distribution
ifeq ($(shell test -f /etc/SuSE-release && echo true),true)
  LINUX_DIST=suse
endif

ifeq ($(OLD_BUILD),yes)
KERNEL_CFLAGS = -mcpu=i486 -g -O2 -D__KERNEL__ -DLINUX -DMODULE -Wall -Wno-sign-compare -I$(LINUX_SRC)/include
DAEMON_CFLAGS = -mcpu=i486 -g -O2 -Wall
CFLAGS = $(DAEMON_CFLAGS)
LDFLAGS = -s -N
CC=gcc
OBJS=nslink.o
else
obj-m += nslink.o
EXTRA_FLAGS += -D__SMP__ -O2 -g
endif

ifeq ($(OLD_BUILD),yes)
# build targets for 2.4 kernels
all:: nslink.o nslinkd nslinkadmin nslinkrelease
nslink.o: nslink.c nslink.h nslink_int.h version.h
	$(CC) $(KERNEL_CFLAGS) -c -o nslink.o nslink.c
mod_nslink.o: nslink.c nslink.h nslink_int.h version.h
	$(CC) $(KERNEL_CFLAGS) -DMODVERSIONS -c -o mod_nslink.o nslink.c
nslinkd.o: nslinkd.c nslink.h version.h
else
# build targetsfor 2.6, 3.0 kernels
all:
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(PWD) modules
	make nslinkd
	make nslinkadmin
	make nslinkrelease

nslink.o: nslink.o nslink.c nslink_int.h nslink.h version.h
endif

nslinkd: nslinkd.o parse.o
	gcc -Wall -o nslinkd nslinkd.o parse.o

nslinkadmin: nslinkadmin.c
	gcc -Wall -o nslinkadmin nslinkadmin.c

nslinkrelease: nslinkrelease.c
	$(CC) -Wall -o nslinkrelease nslinkrelease.c

modules: nslink.o mod_nslink.o

clean::
	rm -f *.o 
	rm -f *.ko
	rm -f .nslink*
	rm -f nslinkd 
	rm -f *~ 
	rm -f lput 
	rm -f vtst 
	rm -f .kver 
	rm -f nslinkadmin 
	rm -f nslinkrelease
	rm -rf nslink
	rm -rf nslink.mod.c
	rm -rf .tmp*
	rm -rf *.symvers
	rm -rf Module.markers
	rm -rf modules.order

update-nslinkd:
	@test ! -f /var/run/nslinkd.pid || (echo -e "\n\n\n *** PLEASE STOP THE CURRENT NSLINK DRIVER BEFORE INSTALLING *** \n Commonly '/etc/init.d/nslink stop' to stop the currently runing driver'\n\n\n" && exit 1)
	@echo "--- Updating nslinkd..."
	cp nslinkd /sbin/nslinkd
	strip /sbin/nslinkd
	chmod 755 /sbin/nslinkd
	@echo "--- Issue '/etc/init.d/nslink start' to start the module without rebooting"
	@echo "--- nslinkd update complete"

update-driver:
	@test ! -f /var/run/nslinkd.pid || (echo -e "\n\n\n *** PLEASE STOP THE CURRENT NSLINK DRIVER BEFORE INSTALLING *** \n Commonly '/etc/init.d/nslink stop' to stop the currently runing driver'\n\n\n" && exit 1)
	@echo "--- Making sure the 'misc' module directory exists..."
	test -d /lib/modules/$(KVER)/misc || mkdir /lib/modules/$(KVER)/misc
	@echo "--- Copying module to proper directory..."
ifeq ($(OLD_BUILD),yes)
	cp nslink.o /lib/modules/$(KVER)/misc
else
	cp *.ko /lib/modules/$(KVER)/misc
endif
	@echo "--- Rebuilding module dependencies..."
	@/sbin/depmod -a -q
	@echo "--- Issue '/etc/init.d/nslink start' to start the module without rebooting"
	@echo "--- nslink driver module update complete"

install:
	@test ! -f /var/run/nslinkd.pid || (echo -e "\n\n\n *** PLEASE STOP THE CURRENT NSLINK DRIVER BEFORE INSTALLING *** \n Commonly '/etc/init.d/nslink stop' to stop the currently runing driver'\n\n\n" && exit 1)
	@echo "--- Making sure the 'misc' module directory exists..."
	test -d /lib/modules/$(KVER)/misc || mkdir /lib/modules/$(KVER)/misc
	@echo "--- Copying module to proper directory..."
ifeq ($(OLD_BUILD),yes)
	cp nslink.o /lib/modules/$(KVER)/misc
else
	cp *.ko /lib/modules/$(KVER)/misc
endif
	@echo "--- Copying the .conf configuration file to /etc..."
	test ! -f /etc/rpshsi.conf || mv /etc/rpshsi.conf /etc/nslink.conf
	test -f /etc/nslink.conf || cp nslink.conf /etc
	@echo "--- Setting up the 'nslink' startup script..."
	rm -f /etc/init.d/rpshsi
ifeq ($(LINUX_DIST),suse)
	# Suse Linux
	INITD=`find /etc/ -type d -name "init.d"`; \
	cp rc.nslink $${INITD}/nslink; \
	chmod 755 $${INITD}/nslink; \
	/sbin/insserv nslink
else
	# Non-Suse Linux distribution
	INITD=`find /etc/ -type d -name "init.d"`; \
	cp rc.nslink $${INITD}/nslink; \
	chmod 755 $${INITD}/nslink; \
	find /etc/ -type d -name "rc2.d" -exec ln -sf $${INITD}/nslink {}/S83nslink \;; \
	find /etc/ -type d -name "rc3.d" -exec ln -sf $${INITD}/nslink {}/S83nslink \;; \
	find /etc/ -type d -name "rc4.d" -exec ln -sf $${INITD}/nslink {}/S83nslink \;; \
	find /etc/ -type d -name "rc5.d" -exec ln -sf $${INITD}/nslink {}/S83nslink \;; \
	find /etc/ -type d -name "rc0.d" -exec ln -sf $${INITD}/nslink {}/K20nslink \;; \
	find /etc/ -type d -name "rc1.d" -exec ln -sf $${INITD}/nslink {}/K20nslink \;; \
	find /etc/ -type d -name "rc6.d" -exec ln -sf $${INITD}/nslink {}/K20nslink \;
endif
	@echo "--- Copying firmware to /etc..."
	find . -type f -name "rpshsi.bin" -exec cp -p rpshsi.bin /etc/rpshsi.bin \;; \
	find . -type f -name "rpshsi2p.bin" -exec cp -p rpshsi2p.bin /etc/rpshsi2p.bin \;; \
	find . -type f -name "devmast.bin" -exec cp -p devmast.bin /etc/devmast.bin \;
	@echo "--- Installing nslinkd..."
	cp nslinkd /sbin/nslinkd
	strip /sbin/nslinkd
	chmod 755 /sbin/nslinkd
	@echo "--- Installing nslink tools..."
	install --mode=700 nslinkadmin      /usr/sbin
	install --mode=700 nslinktool       /usr/sbin
	install --mode=700 nslinkrelease    /usr/sbin
	install --mode=700 nslinkrelease.py /usr/sbin
	@echo "--- Installing man pages..."
	mkdir -p /usr/local/man/man8
	install --mode=644 *8 /usr/local/man/man8/
	mkdir -p /usr/local/man/man5
	install --mode=644 *5 /usr/local/man/man5/
	@echo "--- Rebuilding module dependencies..."
	@/sbin/depmod -a -q
	@echo "--- Issue '/etc/init.d/nslink start' to start the module without rebooting"
	@echo "--- nslink module installation complete"


uninstall:
	find /lib/modules/$(KVER) -name nslink.ko -exec rm {} \;
	INITD=`find /etc/ -name "init.d"`; \
	find $${INITD}/ -name nslink -exec rm {} \;
	find /etc/ -type l -name "*nslink" -exec rm {} \;
	find /usr/bin/ -name "nslink*" -exec rm {} \;
	find /etc/ -name "rpshsi*.bin" -exec rm {} \;
	find /etc/ -name "devmast*.bin" -exec rm {} \;
	find /usr/local/man/ -name "nslink*" -exec rm {} \;

dist: clean
	tar zcvhf ../devicemaster-linux-$(CURRENTDATE).tar.gz -C .. nslink
