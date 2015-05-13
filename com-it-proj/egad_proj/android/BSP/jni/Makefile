CC=/media/doronsa/457cd718-9daa-4ce7-ba1a-42d4d4842eb5/data/doron/var_jb_422_110/jb_422_110_build/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.6/bin/arm-linux-androideabi-gcc --sysroot=${SYSROOT}
PROGRAMS = bsp_api.c sii_api.c lib_bmp.c  printer_bmp.c 

LIBDIR = $(PREFIXDIR)/lib
INCDIR = -I. -I/home/user/sample/14/include
LIBS = /home/user/sample/14/usr/lib
AIO_LIBS = $(LIBS)/librt.so

LDFLAGS	= $(PROGRAMS)   $(INCDIR)  -o printer 

CFLAGS = -g 

RM	= rm -f

all:: 
	$(CC) $(CFLAGS) $(LDFLAGS)


clean::
	$(RM)   printer  *.o


