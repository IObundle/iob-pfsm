include $(PFSM_DIR)/hardware/hardware.mk

DEFINE+=$(defmacro)VCD

VSRC+=$(wildcard $(PFSM_HW_DIR)/testbench/*.v)
