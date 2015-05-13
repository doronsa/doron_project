################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Calypso/7816_stub.c \
../Calypso/Bit2Byte.c \
../Calypso/ClyApp.c \
../Calypso/ClyAppApi.c \
../Calypso/ClyCrdOs.c \
../Calypso/ClyKey.c \
../Calypso/ClySamOs.c \
../Calypso/ClySessn.c \
../Calypso/ClyTktOs.c \
../Calypso/Core.c \
../Calypso/LibC.c \
../Calypso/LibCrypto.c \
../Calypso/LibTime.c 

OBJS += \
./Calypso/7816_stub.o \
./Calypso/Bit2Byte.o \
./Calypso/ClyApp.o \
./Calypso/ClyAppApi.o \
./Calypso/ClyCrdOs.o \
./Calypso/ClyKey.o \
./Calypso/ClySamOs.o \
./Calypso/ClySessn.o \
./Calypso/ClyTktOs.o \
./Calypso/Core.o \
./Calypso/LibC.o \
./Calypso/LibCrypto.o \
./Calypso/LibTime.o 

C_DEPS += \
./Calypso/7816_stub.d \
./Calypso/Bit2Byte.d \
./Calypso/ClyApp.d \
./Calypso/ClyAppApi.d \
./Calypso/ClyCrdOs.d \
./Calypso/ClyKey.d \
./Calypso/ClySamOs.d \
./Calypso/ClySessn.d \
./Calypso/ClyTktOs.d \
./Calypso/Core.d \
./Calypso/LibC.d \
./Calypso/LibCrypto.d \
./Calypso/LibTime.d 


# Each subdirectory must supply rules for building sources it contributes
Calypso/%.o: ../Calypso/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-eabi-gcc -I"/media/doronsa/457cd718-9daa-4ce7-ba1a-42d4d4842eb5/data/doron/workspace/project/arm-server/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


