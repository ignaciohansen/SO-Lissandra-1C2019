################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Biblioteca/src/Biblioteca.c \
../Biblioteca/src/BibliotecaMain.c 

OBJS += \
./Biblioteca/src/Biblioteca.o \
./Biblioteca/src/BibliotecaMain.o 

C_DEPS += \
./Biblioteca/src/Biblioteca.d \
./Biblioteca/src/BibliotecaMain.d 


# Each subdirectory must supply rules for building sources it contributes
Biblioteca/src/%.o: ../Biblioteca/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


