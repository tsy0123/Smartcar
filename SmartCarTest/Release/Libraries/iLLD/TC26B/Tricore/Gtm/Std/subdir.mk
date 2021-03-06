################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm.c \
../Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Atom.c \
../Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Cmu.c \
../Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Dpll.c \
../Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Tbu.c \
../Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Tim.c \
../Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Tom.c 

OBJS += \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm.o \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Atom.o \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Cmu.o \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Dpll.o \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Tbu.o \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Tim.o \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Tom.o 

COMPILED_SRCS += \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm.src \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Atom.src \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Cmu.src \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Dpll.src \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Tbu.src \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Tim.src \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Tom.src 

C_DEPS += \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm.d \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Atom.d \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Cmu.d \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Dpll.d \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Tbu.d \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Tim.d \
./Libraries/iLLD/TC26B/Tricore/Gtm/Std/IfxGtm_Tom.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/iLLD/TC26B/Tricore/Gtm/Std/%.src: ../Libraries/iLLD/TC26B/Tricore/Gtm/Std/%.c
	@echo '正在构建文件： $<'
	@echo '正在调用： TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fD:/software/AURIX-Studio-1.5.2/workspace/SmartCarTest/Release/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo '已结束构建： $<'
	@echo ' '

Libraries/iLLD/TC26B/Tricore/Gtm/Std/%.o: ./Libraries/iLLD/TC26B/Tricore/Gtm/Std/%.src
	@echo '正在构建文件： $<'
	@echo '正在调用： TASKING Assembler'
	astc -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo '已结束构建： $<'
	@echo ' '


