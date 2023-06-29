/* PC Emulation of FSM peripheral */

#include <stdint.h>
#include <stdio.h>

#include "iob_fsm_swreg.h"

static uint32_t base;
void IOB_FSM_INIT_BASEADDR(uint32_t addr) {
	base = addr;
}

// Core Setters and Getters
uint32_t IOB_FSM_GET_FSM_INPUT() {
    return 0xaaaaaaaa;
}

void IOB_FSM_SET_FSM_OUTPUT(uint32_t value) {
}

void IOB_FSM_SET_FSM_OUTPUT_ENABLE(uint32_t value) {
}

uint16_t IOB_FSM_GET_VERSION() {
    return 0xaaaa;
}


