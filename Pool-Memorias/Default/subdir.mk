################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Inotify.c \
../gestionMemoria.c \
../main_memoria.c \
../memoria.c \
../parser.c \
../retardos.c 

OBJS += \
./Inotify.o \
./gestionMemoria.o \
./main_memoria.o \
./memoria.o \
./parser.o \
./retardos.o 

C_DEPS += \
./Inotify.d \
./gestionMemoria.d \
./main_memoria.d \
./memoria.d \
./parser.d \
./retardos.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


