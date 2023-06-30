#include <stdbool.h>

#include "iob_pfsm_swreg.h"

//PFSM functions

//Set PFSM base address
void pfsm_init(int base_address);

//Get values from inputs
uint32_t pfsm_get();

//Set values on outputs
void pfsm_set(uint32_t outputs);

//Set mask for outputs (bits 1 are driven outputs, bits 0 are tristate)
void pfsm_set_output_enable(uint32_t value);
