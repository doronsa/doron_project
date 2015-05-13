export SYSROOT=/media/doronsa/457cd718-9daa-4ce7-ba1a-42d4d4842eb5/data/doron/var_jb_422_110/jb_422_110_build/prebuilts/ndk/8/platforms/android-14/arch-arm/
SYSROOT=/media/doronsa/457cd718-9daa-4ce7-ba1a-42d4d4842eb5/data/doron/var_jb_422_110/jb_422_110_build/prebuilts/ndk/8/platforms/android-14/arch-arm/

CC=/media/doronsa/457cd718-9daa-4ce7-ba1a-42d4d4842eb5/data/doron/var_jb_422_110/jb_422_110_build/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.6/bin/arm-linux-androideabi-gcc --sysroot=${SYSROOT}

LIBS = /media/doronsa/457cd718-9daa-4ce7-ba1a-42d4d4842eb5/data/doron/var_jb_422_110/jb_422_110_build/prebuilts/ndk/8/platforms/android-14/arch-arm/usr/lib
INCDIR = -I. -I/media/doronsa/457cd718-9daa-4ce7-ba1a-42d4d4842eb5/data/doron/var_jb_422_110/jb_422_110_build/prebuilts/ndk/8/platforms/android-14/arch-arm/usr/include
#KERNEL_INC = -I/media/doronsa/457cd718-9daa-4ce7-ba1a-42d4d4842eb5/data/doron/var_jb_422_110/jb_422_110_build/kernel_imx/include/
KERNEL_INC = -I/media/doronsa/457cd718-9daa-4ce7-ba1a-42d4d4842eb5/data/doron/var_jb_422_110/jb_422_110_build/prebuilts/gcc/linux-x86/host/i686-linux-glibc2.7-4.6/sysroot/usr/include/
#./prebuilts/gcc/linux-x86/host/x86_64-linux-glibc2.7-4.6/sysroot/usr/include/linux/i2c-dev.h
#./prebuilts/gcc/linux-x86/host/i686-linux-glibc2.7-4.6/sysroot/usr/include/linux/i2c-dev.h
#./prebuilts/gcc/linux-x86/host/i686-linux-glibc2.7-4.4.3/sysroot/usr/include/linux/i2c-dev.h


PROGRAMS = src/*.c 

LIBDIR = $(PREFIXDIR)/lib
INCDIR = #$(KERNEL_INC)#-I/home/user/sample/14/include

LDFLAGS	= $(PROGRAMS)   $(INCDIR)  -o DemonTest 

CFLAGS = -g 

RM	= rm -f

all:: 
	$(CC) $(CFLAGS) $(LDFLAGS)


clean::
	$(RM)   DemonTest  obj/*.o




