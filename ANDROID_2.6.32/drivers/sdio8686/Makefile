# File: Makefile
# Copyright ?Marvell International Ltd. and/or its affiliates, 2003-2006

CC=		$(CROSS)gcc
LD=		$(CROSS)ld
BACKUP=		/root/backup
YMD=		`date +%Y%m%d%H%M`

#############################################################################
# Configuration Options
#############################################################################

# Debug Option
# DEBUG LEVEL n/1/2/3/4:
# n: NO DEBUG
# 1: Only PRINTM(MSG,...)
# 2: PRINTM(MSG,...) and PRINTM(FATAL,...)
# 3: PRINTM(MSG,...), PRINTM(FATAL,...) and PRINTM(WARN,...)
# 4: PRINTM(MSG,...), PRINTM(FATAL,...), PRINTM(WARN,...) and PRINTM(INFO,...)
CONFIG_DEBUG=n

# Proc in /proc/net/wlan
CONFIG_PROC_DEBUG=y

# Linux Power Management
CONFIG_ENABLE_PM=y

# Re-association in driver
CONFIG_REASSOCIATION=y

# Manufacturing firmware support
CONFIG_MFG_CMD_SUPPORT=y

#############################################################################
# Select Platform Tools
#############################################################################

MODEXT = ko
EXTRA_CFLAGS += -I$(PWD)/os/linux
EXTRA_CFLAGS += -I$(PWD)/wlan
#EXTRA_CFLAGS += -Wno-pointer-sign
#TOOLPATH_BULVERDE=	/usr/local/arm/3.3.2/bin
#BULVERDE_DIR= 		/tftpboot/bulverde/root
#TOOLPATH_WMT=	/opt/montavista/pro/devkit/arm/v5t_le_mvl5/bin
TOOLPATH_WMT= /opt/codesourcery/arm-2010q1/bin
WMT_DIR= 		../bin_sd8686/wmt

	INSTALLDIR=$(WMT_DIR)
#	PLATFORM=pxa270m
	PLATFORM=wmt
	TOOLPATH=$(TOOLPATH_WMT)
#	CROSS=$(TOOLPATH)/arm-linux-
	CROSS=$(TOOLPATH)/arm-none-linux-gnueabi-
	KERNEL_WE=16
	# Intel Bulverde Mainstone Platform
#	CONFIG_MAINSTONE=y
	CONFIG_MAINSTONE=n
	ifeq ($(CONFIG_MAINSTONE), y)
		EXTRA_CFLAGS += -D_MAINSTONE
		export CONFIG_MAINSTONE
	else
		# Default MVL kernel has WE14
		#KERNEL_WE=14
	endif
#	KERNELDIR=/usr/src/arm/linux-2.6.9
	KERNELDIR=../../


	EXTRA_CFLAGS += -I$(PWD)/if/if_sdio -I$(PWD)/io/sdio/$(PLATFORM)
	BINDIR = ../bin_sd8686

#############################################################################
# Compiler Flags
#############################################################################

EXTRA_CFLAGS += -I$(KERNELDIR)/include

	EXTRA_CFLAGS += -DFPNUM='"4"'

ifeq ($(CONFIG_DEBUG),1)
	EXTRA_CFLAGS += -DDEBUG_LEVEL1
endif

ifeq ($(CONFIG_DEBUG),2)
	EXTRA_CFLAGS += -DDEBUG_LEVEL1
	EXTRA_CFLAGS += -DDEBUG_LEVEL2
	DBG=	-dbg
endif

ifeq ($(CONFIG_DEBUG),3)
	EXTRA_CFLAGS += -DDEBUG_LEVEL1
	EXTRA_CFLAGS += -DDEBUG_LEVEL2
	EXTRA_CFLAGS += -DDEBUG_LEVEL3
	DBG=	-dbg
endif

ifeq ($(CONFIG_DEBUG),4)
	EXTRA_CFLAGS += -DDEBUG_LEVEL1
	EXTRA_CFLAGS += -DDEBUG_LEVEL2
	EXTRA_CFLAGS += -DDEBUG_LEVEL3
	EXTRA_CFLAGS += -DDEBUG_LEVEL4
	DBG=	-dbg
endif

ifeq ($(CONFIG_PROC_DEBUG),y)
	EXTRA_CFLAGS += -DPROC_DEBUG
	export CONFIG_PROC_DEBUG
endif

ifeq ($(CONFIG_ENABLE_PM),y)
	EXTRA_CFLAGS += -DENABLE_PM
endif

ifeq ($(CONFIG_REASSOCIATION),y)
	EXTRA_CFLAGS += -DREASSOCIATION
endif

ifeq ($(CONFIG_MFG_CMD_SUPPORT),y)
	EXTRA_CFLAGS += -DMFG_CMD_SUPPORT
endif

#############################################################################
# Make Targets
#############################################################################

ifneq ($(KERNELRELEASE),)

#WLANOBJS =	wlan/wlan_main.o wlan/wlan_fw.o wlan/wlan_wext.o \
#		wlan/wlan_rx.o wlan/wlan_tx.o \
#		wlan/wlan_cmd.o wlan/wlan_cmdresp.o \
#		wlan/wlan_proc.o wlan/wlan_scan.o wlan/wlan_join.o

#WLANOBJS += wlan/wlan_wmm.o
#WLANOBJS += wlan/wlan_11d.o
#WLANOBJS += wlan/wlan_fops.o

ifeq ($(CONFIG_PROC_DEBUG), y)
#WLANOBJS += wlan/wlan_debug.o
endif

#IFOBJS := if/if_sdio/if_sdio.o
#obj-m := sd8xxx.o
#sd8xxx-objs := $(WLANOBJS) $(IFOBJS)
IOOBJS := io/sdio/$(PLATFORM)/sdio.o
obj-m += sdio.o
sdio-objs := $(IOOBJS)

# Otherwise we were called directly from the command line; invoke the kernel build system.
else

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	$(CROSS)strip --strip-debug sdio.$(MODEXT)
	#$(CROSS)strip --strip-debug sd8xxx.$(MODEXT)
endif

###############################################################

export		CC LD CFLAGS KERNELDIR

.PHONY: app/wlanconfig wlan clean distclean

	@echo "Finished Making Marvell Wlan Linux Driver"

wlan app/wlanconfig:
	$(MAKE) -C $@

echo:
	@echo "Please make sure the following are done before building:"
	@echo "	1. Modify release_version.h to set correct version number"
	@echo "	2. Updates the README files"

strip:
	$(CROSS)strip --strip-debug sdio.$(MODEXT)
	$(CROSS)strip --strip-debug sd8xxx.$(MODEXT)

build:		echo default

	@if [ ! -d $(BINDIR) ]; then \
		mkdir $(BINDIR); \
	fi 
	cp -f sd8xxx.$(MODEXT) $(BINDIR)/sd8686$(DBG).$(MODEXT)
	cp -f sdio.$(MODEXT) $(BINDIR)

	cp -f README $(BINDIR)
	$(MAKE) -C app/wlanconfig $@ INSTALLDIR=$(BINDIR)
	cp -r config $(BINDIR)

clean:
	-find . -name "*.o" -exec rm {} \;
	-find . -name "*.ko" -exec rm {} \;
	-find . -name ".*.cmd" -exec rm {} \;
	-find . -name "*.mod.c" -exec rm {} \;
	-rm -rf .tmp_versions
	$(MAKE) -C app/wlanconfig $@

install: default

	cp -f sdio.$(MODEXT) $(INSTALLDIR)
	$(MAKE) -C app/wlanconfig $@ INSTALLDIR=$(INSTALLDIR)
	cp -f sd8xxx.$(MODEXT) $(INSTALLDIR)/sd8686$(DBG).$(MODEXT)
	echo "sd8686 Driver Installed"

distclean:
	-find . -name "*.o" -exec rm {} \;
	-find . -name "*.orig" -exec rm {} \;
	-find . -name "*.swp" -exec rm {} \;
	-find . -name "*.*~" -exec rm {} \;
	-find . -name "*~" -exec rm {} \;
	-find . -name "*.d" -exec rm {} \;
	-find . -name "*.a" -exec rm {} \;
	-find . -name "tags" -exec rm {} \;
	-find . -name ".*" -exec rm -rf 2> /dev/null \;
	-find . -name "*.ko" -exec rm {} \;
	-find . -name ".*.cmd" -exec rm {} \;
	-find . -name "*.mod.c" -exec rm {} \;
	-rm -rf .tmp_versions
	$(MAKE) -C app/wlanconfig $@

# End of file
