#include "iob-fsm.h"

//FSM functions

//Set FSM base address
void fsm_init(int base_address){
  IOB_FSM_INIT_BASEADDR(base_address);
}

//Get values from inputs
uint32_t fsm_get(){
  return IOB_FSM_GET_FSM_INPUT();
}

//Set values on outputs
void fsm_set(uint32_t value){
  IOB_FSM_SET_FSM_OUTPUT(value);
}

//Set mask for outputs (bits 1 are driven outputs, bits 0 are tristate)
void fsm_set_output_enable(uint32_t value){
  IOB_FSM_SET_FSM_OUTPUT_ENABLE(value);
}
