################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/AppSw/Tricore/Driver/LQ_ADC.c \
../src/AppSw/Tricore/Driver/LQ_CCU6.c \
../src/AppSw/Tricore/Driver/LQ_DMA.c \
../src/AppSw/Tricore/Driver/LQ_EEPROM.c \
../src/AppSw/Tricore/Driver/LQ_EMEM.c \
../src/AppSw/Tricore/Driver/LQ_FFT.c \
../src/AppSw/Tricore/Driver/LQ_GPIO.c \
../src/AppSw/Tricore/Driver/LQ_GPSR.c \
../src/AppSw/Tricore/Driver/LQ_GPT12_ENC.c \
../src/AppSw/Tricore/Driver/LQ_GTM.c \
../src/AppSw/Tricore/Driver/LQ_QSPI.c \
../src/AppSw/Tricore/Driver/LQ_SOFTI2C.c \
../src/AppSw/Tricore/Driver/LQ_SPI.c \
../src/AppSw/Tricore/Driver/LQ_STM.c \
../src/AppSw/Tricore/Driver/LQ_UART.c 

OBJS += \
./src/AppSw/Tricore/Driver/LQ_ADC.o \
./src/AppSw/Tricore/Driver/LQ_CCU6.o \
./src/AppSw/Tricore/Driver/LQ_DMA.o \
./src/AppSw/Tricore/Driver/LQ_EEPROM.o \
./src/AppSw/Tricore/Driver/LQ_EMEM.o \
./src/AppSw/Tricore/Driver/LQ_FFT.o \
./src/AppSw/Tricore/Driver/LQ_GPIO.o \
./src/AppSw/Tricore/Driver/LQ_GPSR.o \
./src/AppSw/Tricore/Driver/LQ_GPT12_ENC.o \
./src/AppSw/Tricore/Driver/LQ_GTM.o \
./src/AppSw/Tricore/Driver/LQ_QSPI.o \
./src/AppSw/Tricore/Driver/LQ_SOFTI2C.o \
./src/AppSw/Tricore/Driver/LQ_SPI.o \
./src/AppSw/Tricore/Driver/LQ_STM.o \
./src/AppSw/Tricore/Driver/LQ_UART.o 

COMPILED_SRCS += \
./src/AppSw/Tricore/Driver/LQ_ADC.src \
./src/AppSw/Tricore/Driver/LQ_CCU6.src \
./src/AppSw/Tricore/Driver/LQ_DMA.src \
./src/AppSw/Tricore/Driver/LQ_EEPROM.src \
./src/AppSw/Tricore/Driver/LQ_EMEM.src \
./src/AppSw/Tricore/Driver/LQ_FFT.src \
./src/AppSw/Tricore/Driver/LQ_GPIO.src \
./src/AppSw/Tricore/Driver/LQ_GPSR.src \
./src/AppSw/Tricore/Driver/LQ_GPT12_ENC.src \
./src/AppSw/Tricore/Driver/LQ_GTM.src \
./src/AppSw/Tricore/Driver/LQ_QSPI.src \
./src/AppSw/Tricore/Driver/LQ_SOFTI2C.src \
./src/AppSw/Tricore/Driver/LQ_SPI.src \
./src/AppSw/Tricore/Driver/LQ_STM.src \
./src/AppSw/Tricore/Driver/LQ_UART.src 

C_DEPS += \
./src/AppSw/Tricore/Driver/LQ_ADC.d \
./src/AppSw/Tricore/Driver/LQ_CCU6.d \
./src/AppSw/Tricore/Driver/LQ_DMA.d \
./src/AppSw/Tricore/Driver/LQ_EEPROM.d \
./src/AppSw/Tricore/Driver/LQ_EMEM.d \
./src/AppSw/Tricore/Driver/LQ_FFT.d \
./src/AppSw/Tricore/Driver/LQ_GPIO.d \
./src/AppSw/Tricore/Driver/LQ_GPSR.d \
./src/AppSw/Tricore/Driver/LQ_GPT12_ENC.d \
./src/AppSw/Tricore/Driver/LQ_GTM.d \
./src/AppSw/Tricore/Driver/LQ_QSPI.d \
./src/AppSw/Tricore/Driver/LQ_SOFTI2C.d \
./src/AppSw/Tricore/Driver/LQ_SPI.d \
./src/AppSw/Tricore/Driver/LQ_STM.d \
./src/AppSw/Tricore/Driver/LQ_UART.d 


# Each subdirectory must supply rules for building sources it contributes
src/AppSw/Tricore/Driver/%.src: ../src/AppSw/Tricore/Driver/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fD:/software/AURIX-Studio-1.5.2/workspace/SmartCarTest/Release/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo 'Finished building: $<'
	@echo ' '

src/AppSw/Tricore/Driver/%.o: ./src/AppSw/Tricore/Driver/%.src
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo 'Finished building: $<'
	@echo ' '


