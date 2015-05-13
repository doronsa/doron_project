################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/DemonTest.c \
../src/P_TwMtrBase.c \
../src/ProtoLink.c \
../src/gpio_util.c 

OBJS += \
./src/DemonTest.o \
./src/P_TwMtrBase.o \
./src/ProtoLink.o \
./src/gpio_util.o 

C_DEPS += \
./src/DemonTest.d \
./src/P_TwMtrBase.d \
./src/ProtoLink.d \
./src/gpio_util.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include -I/usr/include/x86-linux-gnu -I/usr/include/x86-linux-gnu/4.8/include -I/usr/include/x86-linux-gnu/4.8/include-fixed -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


