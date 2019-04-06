################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Kernel

dependents:
	-cd /home/utnso/TPSO/2019/Desarrollo/tp-2019-1c-mi_ultimo_segundo_tp/Biblioteca/Default && $(MAKE) all

# Tool invocations
Kernel: $(OBJS) $(USER_OBJS) /home/utnso/TPSO/2019/Desarrollo/tp-2019-1c-mi_ultimo_segundo_tp/Biblioteca/Default/Biblioteca
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "Kernel" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) Kernel
	-@echo ' '

.PHONY: all clean dependents
/home/utnso/TPSO/2019/Desarrollo/tp-2019-1c-mi_ultimo_segundo_tp/Biblioteca/Default/Biblioteca:

-include ../makefile.targets
