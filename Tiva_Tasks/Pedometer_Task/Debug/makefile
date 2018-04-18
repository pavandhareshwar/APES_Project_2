################################################################################
# Automatically-generated file. Do not edit!
################################################################################

CG_TOOL_ROOT := /Users/pavandhareshwar/CCS/ccsv8/tools/compiler/ti-cgt-arm_18.1.1.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./pedometer_task.obj" \
"./tm4c1294ncpdt_startup_ccs.obj" \
"./uartstdio.obj" \
"./FreeRTOS/Source/croutine.obj" \
"./FreeRTOS/Source/event_groups.obj" \
"./FreeRTOS/Source/list.obj" \
"./FreeRTOS/Source/queue.obj" \
"./FreeRTOS/Source/stream_buffer.obj" \
"./FreeRTOS/Source/tasks.obj" \
"./FreeRTOS/Source/timers.obj" \
"./FreeRTOS/Source/portable/CCS/ARM_CM4F/port.obj" \
"./FreeRTOS/Source/portable/CCS/ARM_CM4F/portasm.obj" \
"./FreeRTOS/Source/portable/Common/mpu_wrappers.obj" \
"./FreeRTOS/Source/portable/MemMang/heap_4.obj" \
"../tm4c1294ncpdt.cmd" \
$(GEN_CMDS__FLAG) \
-llibc.a \
-l"/Users/pavandhareshwar/Downloads/TivaWare/SW-TM4C-2.1.4.178/driverlib/ccs/Debug/driverlib.lib" \

-include ../makefile.init

RM := rm -rf
RMDIR := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include FreeRTOS/Source/subdir_vars.mk
-include FreeRTOS/Source/portable/CCS/ARM_CM4F/subdir_vars.mk
-include FreeRTOS/Source/portable/Common/subdir_vars.mk
-include FreeRTOS/Source/portable/MemMang/subdir_vars.mk
-include subdir_rules.mk
-include FreeRTOS/Source/subdir_rules.mk
-include FreeRTOS/Source/portable/CCS/ARM_CM4F/subdir_rules.mk
-include FreeRTOS/Source/portable/Common/subdir_rules.mk
-include FreeRTOS/Source/portable/MemMang/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
tiva_freertos_proj2.out \

EXE_OUTPUTS__QUOTED += \
"tiva_freertos_proj2.out" \

BIN_OUTPUTS += \
tiva_freertos_proj2.hex \

BIN_OUTPUTS__QUOTED += \
"tiva_freertos_proj2.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "tiva_freertos_proj2.out"

# Tool invocations
tiva_freertos_proj2.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: ARM Linker'
	"/Users/pavandhareshwar/CCS/ccsv8/tools/compiler/ti-cgt-arm_18.1.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --define=ccs="ccs" --define=TARGET_IS_TM4C129_RA0 --define=PART_TM4C1294NCPDT -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi -z -m"tiva_freertos_proj2.map" --heap_size=0 --stack_size=512 -i"/Users/pavandhareshwar/CCS/ccsv8/tools/compiler/ti-cgt-arm_18.1.1.LTS/lib" -i"/Users/pavandhareshwar/CCS/ccsv8/tools/compiler/ti-cgt-arm_18.1.1.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="tiva_freertos_proj2_linkInfo.xml" --rom_model -o "tiva_freertos_proj2.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

tiva_freertos_proj2.hex: $(EXE_OUTPUTS)
	@echo 'Building files: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo 'Invoking: ARM Hex Utility'
	"/Users/pavandhareshwar/CCS/ccsv8/tools/compiler/ti-cgt-arm_18.1.1.LTS/bin/armhex"  -o "tiva_freertos_proj2.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "pedometer_task.obj" "tm4c1294ncpdt_startup_ccs.obj" "uartstdio.obj" "FreeRTOS/Source/croutine.obj" "FreeRTOS/Source/event_groups.obj" "FreeRTOS/Source/list.obj" "FreeRTOS/Source/queue.obj" "FreeRTOS/Source/stream_buffer.obj" "FreeRTOS/Source/tasks.obj" "FreeRTOS/Source/timers.obj" "FreeRTOS/Source/portable/CCS/ARM_CM4F/port.obj" "FreeRTOS/Source/portable/CCS/ARM_CM4F/portasm.obj" "FreeRTOS/Source/portable/Common/mpu_wrappers.obj" "FreeRTOS/Source/portable/MemMang/heap_4.obj" 
	-$(RM) "pedometer_task.d" "tm4c1294ncpdt_startup_ccs.d" "uartstdio.d" "FreeRTOS/Source/croutine.d" "FreeRTOS/Source/event_groups.d" "FreeRTOS/Source/list.d" "FreeRTOS/Source/queue.d" "FreeRTOS/Source/stream_buffer.d" "FreeRTOS/Source/tasks.d" "FreeRTOS/Source/timers.d" "FreeRTOS/Source/portable/CCS/ARM_CM4F/port.d" "FreeRTOS/Source/portable/Common/mpu_wrappers.d" "FreeRTOS/Source/portable/MemMang/heap_4.d" 
	-$(RM) "FreeRTOS/Source/portable/CCS/ARM_CM4F/portasm.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

