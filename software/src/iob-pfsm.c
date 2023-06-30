#include "iob-pfsm.h"

//PFSM functions

//Set PFSM base address
void pfsm_init(int base_address){
  IOB_PFSM_INIT_BASEADDR(base_address);
}

//Get values from inputs
uint32_t pfsm_get(){
  return IOB_PFSM_GET_PFSM_INPUT();
}

//Set values on outputs
void pfsm_set(uint32_t value){
  IOB_PFSM_SET_PFSM_OUTPUT(value);
}

//Set mask for outputs (bits 1 are driven outputs, bits 0 are tristate)
void pfsm_set_output_enable(uint32_t value){
  IOB_PFSM_SET_PFSM_OUTPUT_ENABLE(value);
}
