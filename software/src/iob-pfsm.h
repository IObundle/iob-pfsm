#ifndef _PFSM_H_
#define _PFSM_H_

#include <stdint.h>

#include "iob_pfsm_swreg.h"

//PFSM functions

//Set PFSM base address and Verilog parameters
void pfsm_init(int base_address, uint32_t state_w, uint8_t input_w, uint32_t output_w);

// Writa a 32-bit word to the states LUT memory
void pfsm_insert_word_states_lut(int addr, uint8_t word_select, uint32_t value);

// Writa a 32-bit word. Each bit goes to a state condition of a different state.
void pfsm_insert_word_conditions_lut(int range_num, uint8_t condition_comb, uint32_t value);

//Soft reset
void pfsm_reset();

//Program PFSM memories, given a bitstream
uint32_t pfsm_bitstream_program(char* bitstream);

#endif //_PFSM_H_
