#ifndef _PFSM_H_
#define _PFSM_H_

#include <stdint.h>

#include "iob_pfsm_swreg.h"

//PFSM functions

// Set PFSM base address and Verilog parameters
void pfsm_init(int base_address, uint32_t state_w, uint8_t input_w, uint32_t output_w);

// Writa a 32-bit word to the LUT memory
void pfsm_insert_word_lut(int addr, uint8_t word_select, uint32_t value);

// Soft reset
void pfsm_reset();

// Get current state of PFSM
uint32_t pfsm_get_state();

//Program PFSM memories, given a bitstream
uint32_t pfsm_bitstream_program(char* bitstream);

#endif //_PFSM_H_
