################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
pedometer_task.obj: ../pedometer_task.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/sridh/workspace_v8/Pedometer_task" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS/Source/include" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS/Source/portable/Common" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS/Source/portable/MemMang" --include_path="C:/ti/TivaWare_C_Series-2.1.4.178" --include_path="C:/ti/TivaWare_C_Series-2.1.4.178/examples/boards/ek-tm4c1294xl" --include_path="C:/ti/TivaWare_C_Series-2.1.4.178/utils" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="C:/ti/TivaWare_C_Series-2.1.4.178/utils" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.1.LTS/include" --define=ccs="ccs" --define=TARGET_IS_TM4C123_RA0 --define=PART_TM4C1294NCPDT -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="pedometer_task.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

tm4c1294ncpdt_startup_ccs.obj: ../tm4c1294ncpdt_startup_ccs.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/sridh/workspace_v8/Pedometer_task" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS/Source/include" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS/Source/portable/Common" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS/Source/portable/MemMang" --include_path="C:/ti/TivaWare_C_Series-2.1.4.178" --include_path="C:/ti/TivaWare_C_Series-2.1.4.178/examples/boards/ek-tm4c1294xl" --include_path="C:/ti/TivaWare_C_Series-2.1.4.178/utils" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="C:/ti/TivaWare_C_Series-2.1.4.178/utils" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.1.LTS/include" --define=ccs="ccs" --define=TARGET_IS_TM4C123_RA0 --define=PART_TM4C1294NCPDT -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="tm4c1294ncpdt_startup_ccs.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

uartstdio.obj: C:/ti/TivaWare_C_Series-2.1.4.178/utils/uartstdio.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/sridh/workspace_v8/Pedometer_task" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS/Source/include" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS/Source/portable/Common" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS/Source/portable/MemMang" --include_path="C:/ti/TivaWare_C_Series-2.1.4.178" --include_path="C:/ti/TivaWare_C_Series-2.1.4.178/examples/boards/ek-tm4c1294xl" --include_path="C:/ti/TivaWare_C_Series-2.1.4.178/utils" --include_path="C:/Users/sridh/workspace_v8/Pedometer_task/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="C:/ti/TivaWare_C_Series-2.1.4.178/utils" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.1.LTS/include" --define=ccs="ccs" --define=TARGET_IS_TM4C123_RA0 --define=PART_TM4C1294NCPDT -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="uartstdio.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


