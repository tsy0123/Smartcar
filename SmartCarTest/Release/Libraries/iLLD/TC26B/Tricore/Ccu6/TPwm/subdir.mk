################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/iLLD/TC26B/Tricore/Ccu6/TPwm/IfxCcu6_TPwm.c 

OBJS += \
./Libraries/iLLD/TC26B/Tricore/Ccu6/TPwm/IfxCcu6_TPwm.o 

COMPILED_SRCS += \
./Libraries/iLLD/TC26B/Tricore/Ccu6/TPwm/IfxCcu6_TPwm.src 

C_DEPS += \
./Libraries/iLLD/TC26B/Tricore/Ccu6/TPwm/IfxCcu6_TPwm.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/iLLD/TC26B/Tricore/Ccu6/TPwm/%.src: ../Libraries/iLLD/TC26B/Tricore/Ccu6/TPwm/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fD:/software/AURIX-Studio-1.5.2/workspace/SmartCarTest/Release/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo 'Finished building: $<'
	@echo ' '

Libraries/iLLD/TC26B/Tricore/Ccu6/TPwm/%.o: ./Libraries/iLLD/TC26B/Tricore/Ccu6/TPwm/%.src
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo 'Finished building: $<'
	@echo ' '


