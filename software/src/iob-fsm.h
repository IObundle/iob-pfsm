#include <stdbool.h>

#include "iob_fsm_swreg.h"

//FSM functions

//Set FSM base address
void fsm_init(int base_address);

//Get values from inputs
uint32_t fsm_get();

//Set values on outputs
void fsm_set(uint32_t outputs);

//Set mask for outputs (bits 1 are driven outputs, bits 0 are tristate)
void fsm_set_output_enable(uint32_t value);
