################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/AppSw/Tricore/User/ANO_DT.c \
../src/AppSw/Tricore/User/LQ_ImageProcess.c \
../src/AppSw/Tricore/User/LQ_Inductor.c \
../src/AppSw/Tricore/User/LQ_MotorServo.c \
../src/AppSw/Tricore/User/LQ_PID.c 

OBJS += \
./src/AppSw/Tricore/User/ANO_DT.o \
./src/AppSw/Tricore/User/LQ_ImageProcess.o \
./src/AppSw/Tricore/User/LQ_Inductor.o \
./src/AppSw/Tricore/User/LQ_MotorServo.o \
./src/AppSw/Tricore/User/LQ_PID.o 

COMPILED_SRCS += \
./src/AppSw/Tricore/User/ANO_DT.src \
./src/AppSw/Tricore/User/LQ_ImageProcess.src \
./src/AppSw/Tricore/User/LQ_Inductor.src \
./src/AppSw/Tricore/User/LQ_MotorServo.src \
./src/AppSw/Tricore/User/LQ_PID.src 

C_DEPS += \
./src/AppSw/Tricore/User/ANO_DT.d \
./src/AppSw/Tricore/User/LQ_ImageProcess.d \
./src/AppSw/Tricore/User/LQ_Inductor.d \
./src/AppSw/Tricore/User/LQ_MotorServo.d \
./src/AppSw/Tricore/User/LQ_PID.d 


# Each subdirectory must supply rules for building sources it contributes
src/AppSw/Tricore/User/%.src: ../src/AppSw/Tricore/User/%.c
	@echo '正在构建文件： $<'
	@echo '正在调用： TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fD:/software/AURIX-Studio-1.5.2/workspace/SmartCarTest/Release/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo '已结束构建： $<'
	@echo ' '

src/AppSw/Tricore/User/%.o: ./src/AppSw/Tricore/User/%.src
	@echo '正在构建文件： $<'
	@echo '正在调用： TASKING Assembler'
	astc -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo '已结束构建： $<'
	@echo ' '


