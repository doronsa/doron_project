################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../7816_stub.c \
../BSPutil.c \
../Bit2Byte.c \
../ClyApp.c \
../ClyAppApi.c \
../ClyCrdOs.c \
../ClyKey.c \
../ClySamOs.c \
../ClySessn.c \
../ClyTktOs.c \
../Core.c \
../LibC.c \
../LibCrypto.c \
../LibTime.c \
../os_porting.c \
../server_demo.c 

OBJS += \
./7816_stub.o \
./BSPutil.o \
./Bit2Byte.o \
./ClyApp.o \
./ClyAppApi.o \
./ClyCrdOs.o \
./ClyKey.o \
./ClySamOs.o \
./ClySessn.o \
./ClyTktOs.o \
./Core.o \
./LibC.o \
./LibCrypto.o \
./LibTime.o \
./os_porting.o \
./server_demo.o 

C_DEPS += \
./7816_stub.d \
./BSPutil.d \
./Bit2Byte.d \
./ClyApp.d \
./ClyAppApi.d \
./ClyCrdOs.d \
./ClyKey.d \
./ClySamOs.d \
./ClySessn.d \
./ClyTktOs.d \
./Core.d \
./LibC.d \
./LibCrypto.d \
./LibTime.d \
./os_porting.d \
./server_demo.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DENABLE_COMM -I"/home/doron-linux/workspace/project/CalypsoLibLinux2/include" -I"/home/doron-linux/workspace/project/CalypsoLibLinux2" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


