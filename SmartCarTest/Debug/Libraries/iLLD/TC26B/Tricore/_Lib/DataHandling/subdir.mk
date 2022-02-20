################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/Ifx_CircularBuffer.asm.c \
../Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/Ifx_CircularBuffer.c \
../Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/Ifx_Fifo.c 

OBJS += \
./Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/Ifx_CircularBuffer.asm.o \
./Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/Ifx_CircularBuffer.o \
./Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/Ifx_Fifo.o 

COMPILED_SRCS += \
./Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/Ifx_CircularBuffer.asm.src \
./Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/Ifx_CircularBuffer.src \
./Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/Ifx_Fifo.src 

C_DEPS += \
./Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/Ifx_CircularBuffer.asm.d \
./Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/Ifx_CircularBuffer.d \
./Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/Ifx_Fifo.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/%.src: ../Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/%.c
	@echo '正在构建文件： $<'
	@echo '正在调用： TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fD:/software/AURIX-Studio-1.5.2/workspace/SmartCarTest/Debug/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo '已结束构建： $<'
	@echo ' '

Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/%.o: ./Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/%.src
	@echo '正在构建文件： $<'
	@echo '正在调用： TASKING Assembler'
	astc -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo '已结束构建： $<'
	@echo ' '


