################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/main.c \
../src/stm32f3xx_it.c 

OBJS += \
./src/main.o \
./src/stm32f3xx_it.o 

C_DEPS += \
./src/main.d \
./src/stm32f3xx_it.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -flto -fno-move-loop-invariants -Wall -Wextra  -g3 -DSTM32F302x8 -I"C:\repos\canboot-slat\drivers\CMSIS\Include" -I"C:\repos\canboot-slat" -I"C:\repos\canboot-slat\drivers\CMSIS\Device\ST\STM32F3xx\Include" -I"C:\repos\canboot-slat\inc" -I"C:\repos\canboot-slat\ldscripts\STM32F302K8_FLASH.ld" -I"C:\repos\canboot-slat\drivers\STM32F3xx_HAL_Driver\Inc" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


