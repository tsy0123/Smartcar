################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/Service/CpuGeneric/If/SpiIf.c 

OBJS += \
./Libraries/Service/CpuGeneric/If/SpiIf.o 

COMPILED_SRCS += \
./Libraries/Service/CpuGeneric/If/SpiIf.src 

C_DEPS += \
./Libraries/Service/CpuGeneric/If/SpiIf.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/Service/CpuGeneric/If/%.src: ../Libraries/Service/CpuGeneric/If/%.c Libraries/Service/CpuGeneric/If/subdir.mk
	@echo '正在构建文件： $<'
	@echo '正在调用： TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fD:/software/AURIX-Studio-1.5.4/workspace/SmartCarTest/Debug/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo '已结束构建： $<'
	@echo ' '

Libraries/Service/CpuGeneric/If/%.o: ./Libraries/Service/CpuGeneric/If/%.src Libraries/Service/CpuGeneric/If/subdir.mk
	@echo '正在构建文件： $<'
	@echo '正在调用： TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo '已结束构建： $<'
	@echo ' '


