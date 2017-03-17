################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/system_stm32f3xx.c 

S_UPPER_SRCS += \
../drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/startup_stm32f302x8.S 

OBJS += \
./drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/startup_stm32f302x8.o \
./drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/system_stm32f3xx.o 

S_UPPER_DEPS += \
./drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/startup_stm32f302x8.d 

C_DEPS += \
./drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/system_stm32f3xx.d 


# Each subdirectory must supply rules for building sources it contributes
drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/%.o: ../drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -flto -fno-move-loop-invariants -Wall -Wextra  -g3 -x assembler-with-cpp -DSTM32F302x8 -I"C:\repos\canboot-slat\drivers\CMSIS\Include" -I"C:\repos\canboot-slat" -I"C:\repos\canboot-slat\drivers\CMSIS\Device\ST\STM32F3xx\Include" -I"C:\repos\canboot-slat\inc" -I"C:\repos\canboot-slat\ldscripts\STM32F302K8_FLASH.ld" -I"C:\repos\canboot-slat\drivers\STM32F3xx_HAL_Driver\Inc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/%.o: ../drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -flto -fno-move-loop-invariants -Wall -Wextra  -g3 -DSTM32F302x8 -I"C:\repos\canboot-slat\drivers\CMSIS\Include" -I"C:\repos\canboot-slat" -I"C:\repos\canboot-slat\drivers\CMSIS\Device\ST\STM32F3xx\Include" -I"C:\repos\canboot-slat\inc" -I"C:\repos\canboot-slat\ldscripts\STM32F302K8_FLASH.ld" -I"C:\repos\canboot-slat\drivers\STM32F3xx_HAL_Driver\Inc" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


