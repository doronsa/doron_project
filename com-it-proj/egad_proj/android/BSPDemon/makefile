#export SYSROOT=/home/doron-linux/workspace/android-ndk-r9b/platforms/android-14/arch-arm
SYSROOT=/home/doron-linux/workspace/android-ndk-r9b/platforms/android-14/arch-arm
INCDIR = -I. -I/home/doron-linux/workspace/var_jb_422_110/jb_422_110_build/prebuilts/ndk/8/platforms/android-14/arch-arm/usr/include 

CC=/home/doron-linux/workspace/var_jb_422_110/jb_422_110_build/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.6/bin/arm-linux-androideabi-gcc --sysroot=${SYSROOT}

LIBS = /home/doron-linux/workspace/var_jb_422_110/jb_422_110_build/prebuilts/ndk/8/platforms/android-14/arch-arm/usr/lib

PROGRAMS = src/*.c 
INCLUDE_DIR	:= include 

#LIBDIR = $(PREFIXDIR)/lib
LIBDIR = -L$(LIBS) -llog


LDFLAGS	= $(PROGRAMS)   $(INCDIR)  -o bspDaemon

CFLAGS = -g 

RM	= rm -f

all:: 
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBDIR)


clean::
	$(RM)   DemonTest  obj/*.o




