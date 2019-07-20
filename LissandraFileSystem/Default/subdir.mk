################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../InotifyLFS.c \
../LissandraFileSystem.c \
../lfsComunicacion.c \
../parser.c 

OBJS += \
./InotifyLFS.o \
./LissandraFileSystem.o \
./lfsComunicacion.o \
./parser.o 

C_DEPS += \
./InotifyLFS.d \
./LissandraFileSystem.d \
./lfsComunicacion.d \
./parser.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'

	gcc -Im -Ipthread -Icommons -Ireadline -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"



	@echo 'Finished building: $<'
	@echo ' '


