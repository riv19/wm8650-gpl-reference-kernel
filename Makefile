
all: ubin

menuconfig:
	make -C ANDROID_2.6.32 menuconfig
	
ubin:
	make -C ANDROID_2.6.32 ubin

Android_defconfig:
	make -C ANDROID_2.6.32 Android_defconfig
	
clean:	
	make -C ANDROID_2.6.32 clean
	cp ANDROID_2.6.32_Driver_Obj/* ANDROID_2.6.32/. -arf
	find ANDROID_2.6.32 -name "built-in.o" -exec rm -rf {} \;
	find ANDROID_2.6.32 -name ".*.o.cmd" -exec rm -rf {} \;


