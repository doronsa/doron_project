################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/BSPutil.c \
../src/P_TwMtrBase.c \
../src/ProtoLink.c \
../src/os_porting.c \
../src/server_demo.c 

OBJS += \
./src/BSPutil.o \
./src/P_TwMtrBase.o \
./src/ProtoLink.o \
./src/os_porting.o \
./src/server_demo.o 

C_DEPS += \
./src/BSPutil.d \
./src/P_TwMtrBase.d \
./src/ProtoLink.d \
./src/os_porting.d \
./src/server_demo.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-eabi-gcc -I"/media/doronsa/457cd718-9daa-4ce7-ba1a-42d4d4842eb5/data/doron/workspace/project/arm-server/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


