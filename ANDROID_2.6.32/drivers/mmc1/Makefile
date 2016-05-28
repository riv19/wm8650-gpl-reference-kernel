#
# Makefile for the kernel mmc device drivers.
#

ifeq ($(CONFIG_MMC1_DEBUG),y)
	EXTRA_CFLAGS		+= -DDEBUG
endif

obj-$(CONFIG_MMC1)		+= core/
obj-$(CONFIG_MMC1)		+= card/
obj-$(CONFIG_MMC1)		+= host/

