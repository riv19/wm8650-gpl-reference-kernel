CC= $(CROSS)gcc
TOOLPATH= /opt/montavista/pro/devkit/arm/v5t_le/bin
CROSS=$(TOOLPATH)/arm_v5t_le-
PLATFORM= vt8500
KERNELDIR=../../../
CFLAGS+= -I$(KERNELDIR)/include

obj-m := a.o
#a-objs := a.o
obj-m += b.o
#b-objs := b.o
all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	$(CROSS)strip a.ko
	$(CROSS)strip b.ko
clean:
	rm -rf a.ko b.ko
