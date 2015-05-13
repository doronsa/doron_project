################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../jni/7816_stub.c \
../jni/BSPutil.c \
../jni/Bit2Byte.c \
../jni/ClyApp.c \
../jni/ClyAppApi.c \
../jni/ClyCrdOs.c \
../jni/ClyKey.c \
../jni/ClySamOs.c \
../jni/ClySessn.c \
../jni/ClyTktOs.c \
../jni/Core.c \
../jni/LibC.c \
../jni/LibCrypto.c \
../jni/LibTime.c \
../jni/P_TwMtrBase.c \
../jni/ProtoLink.c \
../jni/bsp_api.c \
../jni/bsp_jni.c \
../jni/lib_bmp.c \
../jni/os_porting.c \
../jni/printer_bmp.c \
../jni/sii_api.c 

OBJS += \
./jni/7816_stub.o \
./jni/BSPutil.o \
./jni/Bit2Byte.o \
./jni/ClyApp.o \
./jni/ClyAppApi.o \
./jni/ClyCrdOs.o \
./jni/ClyKey.o \
./jni/ClySamOs.o \
./jni/ClySessn.o \
./jni/ClyTktOs.o \
./jni/Core.o \
./jni/LibC.o \
./jni/LibCrypto.o \
./jni/LibTime.o \
./jni/P_TwMtrBase.o \
./jni/ProtoLink.o \
./jni/bsp_api.o \
./jni/bsp_jni.o \
./jni/lib_bmp.o \
./jni/os_porting.o \
./jni/printer_bmp.o \
./jni/sii_api.o 

C_DEPS += \
./jni/7816_stub.d \
./jni/BSPutil.d \
./jni/Bit2Byte.d \
./jni/ClyApp.d \
./jni/ClyAppApi.d \
./jni/ClyCrdOs.d \
./jni/ClyKey.d \
./jni/ClySamOs.d \
./jni/ClySessn.d \
./jni/ClyTktOs.d \
./jni/Core.d \
./jni/LibC.d \
./jni/LibCrypto.d \
./jni/LibTime.d \
./jni/P_TwMtrBase.d \
./jni/ProtoLink.d \
./jni/bsp_api.d \
./jni/bsp_jni.d \
./jni/lib_bmp.d \
./jni/os_porting.d \
./jni/printer_bmp.d \
./jni/sii_api.d 


# Each subdirectory must supply rules for building sources it contributes
jni/%.o: ../jni/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


