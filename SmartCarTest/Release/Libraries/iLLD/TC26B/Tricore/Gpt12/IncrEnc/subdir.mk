################################################################################
# �Զ����ɵ��ļ�����Ҫ�༭��
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/iLLD/TC26B/Tricore/Gpt12/IncrEnc/IfxGpt12_IncrEnc.c 

OBJS += \
./Libraries/iLLD/TC26B/Tricore/Gpt12/IncrEnc/IfxGpt12_IncrEnc.o 

COMPILED_SRCS += \
./Libraries/iLLD/TC26B/Tricore/Gpt12/IncrEnc/IfxGpt12_IncrEnc.src 

C_DEPS += \
./Libraries/iLLD/TC26B/Tricore/Gpt12/IncrEnc/IfxGpt12_IncrEnc.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/iLLD/TC26B/Tricore/Gpt12/IncrEnc/%.src: ../Libraries/iLLD/TC26B/Tricore/Gpt12/IncrEnc/%.c
	@echo '���ڹ����ļ��� $<'
	@echo '���ڵ��ã� TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fD:/software/AURIX-Studio-1.5.2/workspace/SmartCarTest/Release/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo '�ѽ��������� $<'
	@echo ' '

Libraries/iLLD/TC26B/Tricore/Gpt12/IncrEnc/%.o: ./Libraries/iLLD/TC26B/Tricore/Gpt12/IncrEnc/%.src
	@echo '���ڹ����ļ��� $<'
	@echo '���ڵ��ã� TASKING Assembler'
	astc -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo '�ѽ��������� $<'
	@echo ' '


