################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/iLLD/TC26B/Tricore/Gtm/Atom/Pwm/IfxGtm_Atom_Pwm.c 

OBJS += \
./Libraries/iLLD/TC26B/Tricore/Gtm/Atom/Pwm/IfxGtm_Atom_Pwm.o 

COMPILED_SRCS += \
./Libraries/iLLD/TC26B/Tricore/Gtm/Atom/Pwm/IfxGtm_Atom_Pwm.src 

C_DEPS += \
./Libraries/iLLD/TC26B/Tricore/Gtm/Atom/Pwm/IfxGtm_Atom_Pwm.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/iLLD/TC26B/Tricore/Gtm/Atom/Pwm/%.src: ../Libraries/iLLD/TC26B/Tricore/Gtm/Atom/Pwm/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fD:/software/AURIX-Studio-1.5.2/workspace/SmartCarTest/Release/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo 'Finished building: $<'
	@echo ' '

Libraries/iLLD/TC26B/Tricore/Gtm/Atom/Pwm/%.o: ./Libraries/iLLD/TC26B/Tricore/Gtm/Atom/Pwm/%.src
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo 'Finished building: $<'
	@echo ' '


