################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/OEMLib/UART_Communication/uart_communication.c 

OBJS += \
./Libraries/OEMLib/UART_Communication/uart_communication.o 

C_DEPS += \
./Libraries/OEMLib/UART_Communication/uart_communication.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/OEMLib/UART_Communication/%.o: ../Libraries/OEMLib/UART_Communication/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM-GCC C Compiler'
	"$(TOOLCHAIN_ROOT)/bin/arm-none-eabi-gcc" -MMD -MT "$@" -DXMC4700_F144x2048 -I"$(PROJECT_LOC)/Libraries/OEMLib/Timer" -I"$(PROJECT_LOC)/Libraries/OEMLib/UART_Communication" -I"$(PROJECT_LOC)/Libraries/OEMLib/USB_Communication" -I"$(PROJECT_LOC)/Libraries/XMCLib/inc" -I"$(PROJECT_LOC)/Libraries/CMSIS/Include" -I"$(PROJECT_LOC)/Libraries/CMSIS/Infineon/XMC4700_series/Include" -I"$(PROJECT_LOC)" -I"$(PROJECT_LOC)/Dave/Generated" -I"$(PROJECT_LOC)/Libraries" -I"$(PROJECT_LOC)/Libraries/OEMLib/Base64" -I"$(PROJECT_LOC)/Libraries/OEMLib/DPS310PressureSensor" -I"$(PROJECT_LOC)/Libraries/OEMLib/Functions" -I"$(PROJECT_LOC)/Libraries/OEMLib/I2C" -O0 -ffunction-sections -fdata-sections -Wall -std=gnu99 -mfloat-abi=softfp -Wa,-adhlns="$@.lst" -pipe -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $@" -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mthumb -g -gdwarf-2 -o "$@" "$<" 
	@echo 'Finished building: $<'
	@echo.

