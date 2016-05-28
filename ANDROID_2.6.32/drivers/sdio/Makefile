#
# Makefile for the kernel mmc sdio device drivers.
#

ifeq ($(CONFIG_MMC_SDIO_DEBUG),y)
	EXTRA_CFLAGS		+= -DDEBUG
endif

obj-$(CONFIG_MMC_SDIO)		+= core/
obj-$(CONFIG_MMC_SDIO)		+= card/
obj-$(CONFIG_MMC_SDIO)		+= host/

