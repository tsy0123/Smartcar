################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USER/Drive.c \
../USER/TSY_ADC.c \
../USER/TSY_WIFI.c \
../USER/image.c \
../USER/mymath.c 

OBJS += \
./USER/Drive.o \
./USER/TSY_ADC.o \
./USER/TSY_WIFI.o \
./USER/image.o \
./USER/mymath.o 

COMPILED_SRCS += \
./USER/Drive.src \
./USER/TSY_ADC.src \
./USER/TSY_WIFI.src \
./USER/image.src \
./USER/mymath.src 

C_DEPS += \
./USER/Drive.d \
./USER/TSY_ADC.d \
./USER/TSY_WIFI.d \
./USER/image.d \
./USER/mymath.d 


# Each subdirectory must supply rules for building sources it contributes
USER/%.src: ../USER/%.c
	@echo '正在构建文件： $<'
	@echo '正在调用： TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fD:/software/AURIX-Studio-1.5.2/workspace/SmartCarTest/Debug/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo '已结束构建： $<'
	@echo ' '

USER/%.o: ./USER/%.src
	@echo '正在构建文件： $<'
	@echo '正在调用： TASKING Assembler'
	astc -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo '已结束构建： $<'
	@echo ' '


