################################################################################
# �Զ����ɵ��ļ�����Ҫ�༭��
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/iLLD/TC26B/Tricore/Cpu/Trap/IfxCpu_Trap.c 

OBJS += \
./Libraries/iLLD/TC26B/Tricore/Cpu/Trap/IfxCpu_Trap.o 

COMPILED_SRCS += \
./Libraries/iLLD/TC26B/Tricore/Cpu/Trap/IfxCpu_Trap.src 

C_DEPS += \
./Libraries/iLLD/TC26B/Tricore/Cpu/Trap/IfxCpu_Trap.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/iLLD/TC26B/Tricore/Cpu/Trap/%.src: ../Libraries/iLLD/TC26B/Tricore/Cpu/Trap/%.c Libraries/iLLD/TC26B/Tricore/Cpu/Trap/subdir.mk
	@echo '���ڹ����ļ��� $<'
	@echo '���ڵ��ã� TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fD:/software/AURIX-Studio-1.5.4/workspace/SmartCarTest/Debug/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo '�ѽ��������� $<'
	@echo ' '

Libraries/iLLD/TC26B/Tricore/Cpu/Trap/%.o: ./Libraries/iLLD/TC26B/Tricore/Cpu/Trap/%.src Libraries/iLLD/TC26B/Tricore/Cpu/Trap/subdir.mk
	@echo '���ڹ����ļ��� $<'
	@echo '���ڵ��ã� TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo '�ѽ��������� $<'
	@echo ' '


