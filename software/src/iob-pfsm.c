#include "iob-pfsm.h"

// Global variables to store parameters of the current PFSM
static uint32_t g_state_w;
static uint8_t g_input_w;
static uint32_t g_output_w;

//PFSM functions

//Set PFSM base address
void pfsm_init(int base_address, uint32_t state_w, uint8_t input_w, uint32_t output_w){
  IOB_PFSM_INIT_BASEADDR(base_address);
  g_state_w=state_w;
  g_input_w=input_w;
  g_output_w=output_w;
}


// Writa a 32-bit word to the states LUT memory
// The states LUT may have words larger than 32-bits. Use the word_select to select the which 32-bit word to write.
void pfsm_insert_word_states_lut(int addr, uint8_t word_select, uint32_t value){
  IOB_PFSM_SET_STATE_MEM_WORD_SELECT(word_select);
  IOB_PFSM_SET_STATES_MEMORY(value, addr);
}

// Writa a 32-bit word. Each bit goes to a state condition of a different state.
// Each word in the condtions LUT of a state is 1-bit wide. Writing bits to 32 LUTs at a time allows us to save on the number of instructions to fill all the memories.
//
// The range_num value selects which 32 states will be modified. 
//     If any range_num is 0, then first 32 states will be modified.
//     If the range_num is 1, then the states 32 to 63 will be modified.
//     ...
// All the states will be modified for the same input_ports condition, given in the condition_comb argument.
//
// Since you have to fill the conditions one by one for each input_ports combination, then you should call this function as many times as those combinations.
void pfsm_insert_word_conditions_lut(int range_num, uint8_t condition_comb, uint32_t value){
  IOB_PFSM_SET_CONDITION_COMB(condition_comb);
  IOB_PFSM_SET_CONDITION_MEMORY(value, range_num);
}

//pulse soft reset to go back to state 0
void pfsm_reset(){
  IOB_PFSM_SET_SOFTRESET(1);
  IOB_PFSM_SET_SOFTRESET(0);
}

// Program PFSM given a bitstream with data to program the States LUT and every condition LUT.
// Returns number of bytes written (should be equal to the bitstream size)
uint32_t pfsm_bitstream_program(char* bitstream){
  uint32_t i, byteCounter = 0;
  uint8_t j;
  uint32_t data_words_per_lut_word = (g_state_w+g_output_w+32-1)/32; // Ceiling division of (state_w+output_w)/32
  // Program states memory
  for(j=0; j<(1<<g_state_w); j++){
    // Program each 32-bit word in this LUT address
    for(i=0; i<data_words_per_lut_word; i++){
      pfsm_insert_word_states_lut(j, i, 
                                  bitstream[byteCounter]<<24 | bitstream[byteCounter+1]<<16 | bitstream[byteCounter+2]<<8 | bitstream[byteCounter+3]
                                  );
      byteCounter+=4;
    }
  }
  
  uint32_t div_states_by_data_w = ((1<<g_state_w)+32-1)/32; // Ceiling division of (2^state_w)/32
  // For each input_ports combination
  for(j=0; j<(1<<g_input_w); j++){
    // Program evey state for this combination, 32 states at each time
    for(i=0; i<div_states_by_data_w; i++){
      pfsm_insert_word_conditions_lut(i, j, bitstream[byteCounter]<<24 | bitstream[byteCounter+1]<<16 | bitstream[byteCounter+2]<<8 | bitstream[byteCounter+3]);
    }
    byteCounter+=4;
  }

  // Reset PFSM back to state 0
  pfsm_reset();

  return byteCounter;
}
