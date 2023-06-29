include $(FSM_DIR)/hardware/hardware.mk

DEFINE+=$(defmacro)VCD

VSRC+=$(wildcard $(FSM_HW_DIR)/testbench/*.v)
