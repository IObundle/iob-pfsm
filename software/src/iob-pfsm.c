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


// Writa a 32-bit word to the LUT memory
// The LUT may have words larger than 32-bits. Use the word_select to select the which 32-bit word to write.
void pfsm_insert_word_lut(int addr, uint8_t word_select, uint32_t value){
  IOB_PFSM_SET_MEM_WORD_SELECT(word_select);
  IOB_PFSM_SET_MEMORY(value, addr);
}

//pulse soft reset to go back to state 0
void pfsm_reset(){
  IOB_PFSM_SET_SOFTRESET(1);
  IOB_PFSM_SET_SOFTRESET(0);
}

// Program PFSM given a bitstream with data to program the States LUT and every condition LUT.
// Returns number of bytes written (should be equal to the bitstream size)
uint32_t pfsm_bitstream_program(char* bitstream){
  uint8_t i;
  uint32_t j, byteCounter = 0;
  uint32_t data_words_per_lut_word = (g_state_w+g_output_w+32-1)/32; // Ceiling division of (state_w+output_w)/32
  // Program LUT memory
  for(j=0; j<(1<<(g_input_w+g_state_w)); j++){
    // Program each 32-bit word in this LUT address
    for(i=0; i<data_words_per_lut_word; i++){
      pfsm_insert_word_lut(j, i, 
                          bitstream[byteCounter]<<24 | bitstream[byteCounter+1]<<16 | bitstream[byteCounter+2]<<8 | bitstream[byteCounter+3]
                          );
      byteCounter+=4;
    }
  }

  // Reset PFSM back to state 0
  pfsm_reset();

  return byteCounter;
}
