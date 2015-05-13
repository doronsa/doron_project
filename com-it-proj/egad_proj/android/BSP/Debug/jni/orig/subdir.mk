################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../jni/orig/LibBMP.c \
../jni/orig/PrinterBMP.c \
../jni/orig/hello-jni.c \
../jni/orig/sample.c \
../jni/orig/sii_api.c 

OBJS += \
./jni/orig/LibBMP.o \
./jni/orig/PrinterBMP.o \
./jni/orig/hello-jni.o \
./jni/orig/sample.o \
./jni/orig/sii_api.o 

C_DEPS += \
./jni/orig/LibBMP.d \
./jni/orig/PrinterBMP.d \
./jni/orig/hello-jni.d \
./jni/orig/sample.d \
./jni/orig/sii_api.d 


# Each subdirectory must supply rules for building sources it contributes
jni/orig/%.o: ../jni/orig/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


