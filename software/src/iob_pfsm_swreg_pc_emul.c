/* PC Emulation of PFSM peripheral */

#include <stdint.h>
#include <stdio.h>

#include "iob_pfsm_swreg.h"

static uint32_t base;
void IOB_PFSM_INIT_BASEADDR(uint32_t addr) {
	base = addr;
}

// Core Setters and Getters
uint32_t IOB_PFSM_GET_PFSM_INPUT() {
    return 0xaaaaaaaa;
}

void IOB_PFSM_SET_PFSM_OUTPUT(uint32_t value) {
}

void IOB_PFSM_SET_PFSM_OUTPUT_ENABLE(uint32_t value) {
}

uint16_t IOB_PFSM_GET_VERSION() {
    return 0xaaaa;
}


