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
<<<<<<< HEAD
	gcc -Im -Ipthread -Icommons -Ireadline -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
=======
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
>>>>>>> 10c7dc442d0451c54d3d19e119d0bbde224bbdee
	@echo 'Finished building: $<'
	@echo ' '


