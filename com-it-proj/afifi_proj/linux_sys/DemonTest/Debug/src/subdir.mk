################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/DemonTest.c \
../src/KeyBoard.c \
../src/P_TwMtrBase.c \
../src/ProtoLink.c \
../src/dcu_util.c \
../src/demon_util.c \
../src/gpio_util.c \
../src/i2cbusses.c 

OBJS += \
./src/DemonTest.o \
./src/KeyBoard.o \
./src/P_TwMtrBase.o \
./src/ProtoLink.o \
./src/dcu_util.o \
./src/demon_util.o \
./src/gpio_util.o \
./src/i2cbusses.o 

C_DEPS += \
./src/DemonTest.d \
./src/KeyBoard.d \
./src/P_TwMtrBase.d \
./src/ProtoLink.d \
./src/dcu_util.d \
./src/demon_util.d \
./src/gpio_util.d \
./src/i2cbusses.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include -I/usr/include/x86-linux-gnu -I/usr/include/x86-linux-gnu/4.8/include -I/usr/include/x86-linux-gnu/4.8/include-fixed -O0 -g3 -Wall -c -fmessage-length=0 -m32 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


