# README #

# iob-pfsm

## What is this repository for? ##

The IObundle Programmable Finite State Machine (PFSM) is a RISC-V-based Peripheral written in Verilog, which users can download, modify, simulate and implement in FPGA or ASIC.
This core can be programmed to act as a Mealy or Moore FSM.
Its hardware mostly consists of a memory, used as a Lookup table (LUT), and a register to keep track of the state.
<img
  src="/document/figures/PFSM_BD.jpg"
  alt="PFSM_BD.jpg"
  title="PFSM Block Diagram"
  style="display: inline-block; margin: 0 auto; max-width: 300px">

This core provides drivers to load a bitstream at run time to reprogram the PFSM.
It also provides the `scripts/iob_fsm_program.py` python module to generate bitstreams based on given state instructions.

## Integrate in SoC ##

* Check out [IOb-SoC-SUT](https://github.com/IObundle/iob-soc-sut)

## Usage

The main class that describes this core is located in the `iob_pfsm.py` Python modules. It contains a set of methods useful to set up and instantiate this core.

The following steps describe the process of creating a PFSM peripheral in an IOb-SoC-based system:
1) Import the `iob_pfsm` class
2) Run the `iob_pfsm.setup()` method to copy the required sources of this module to the build directory.
3) Run the `iob_pfsm.instance(...)` method to create a Verilog instance of the PFSM peripheral.
To use this core as a peripheral of an IOb-SoC-based system:
  4) Add the created instance to the peripherals list of the IOb-SoC-based system.
  5) Write the firmware to run in the system, including the `iob-pfsm.h` C header and use its driver functions to control this core.

## Example configuration

The `iob_soc_tester.py` script of the [IOb-SoC-SUT](https://github.com/IObundle/iob-soc-sut) system, uses the following lines of code to instantiate a PFSM peripheral with the instance name `PFSM0`:
```Python
# Import the iob_pfsm class
from iob_pfsm import iob_pfsm

# Class of the Tester system
class iob_soc_tester(iob_soc):
  ...
  # Method that runs the setup process of the Tester system
  @classmethod
  def _run_setup(cls):
    ...
    # Setup the PFSM module (Copies every file and dependency required to the build directory)
    iob_pfsm.setup()
    ...
    # Create a Verilog instance of this module, named 'PFSM0', and add it to the peripherals list of the system.
    cls.peripherals.append(
        iob_pfsm.instance(
            "PFSM0",
            "PFSM interface",
            parameters={"STATE_W": "2", "INPUT_W": "1", "OUTPUT_W": "1"},
        )
    )
```

## Generate a bitstream ##

To generate a bitstream to program this PFSM, you can use the `scripts/iob_fsm_program.py` python module.
This scripts allows you to describe each state of PFSM machine and their behaviour.

The following commented examples show how to use this module:
```Python
# Import the classes from that python module
from iob_fsm_program import iob_fsm_program, iob_fsm_record

# The `iob_fsm_program` object describes a program, including its states, for a specific PFSM.

# Start by creating an `iob_fsm_program` object and initialize it with the same parameters as the PFSM used.
# In this example, we use a PFSM with 2^2 states, 1 input and 1 output.
fsm_prog = iob_fsm_program(2,1,1) # State_w = 2, Input_w = 1, Output_w = 1

# Add a list of records to the program, each of them describing a state.
fsm_prog.add_record([
    # Format: iob_fsm_record("label", "input_cond", "next_state", "output_expr")
    iob_fsm_record("state_0", "", "", "0"), # Label: state_0, Output: 0
    iob_fsm_record("", "", "", "1"), # No label; Output: 1
    iob_fsm_record("state_1", "1", "state_0", "0"), # Label: state_1; Jump to `state_0` if input is 1; Output 0
    iob_fsm_record("", "", "", "i[0]"), # No label; Output: equal to bit 0 of input
    ])

# Optionally, you can print the truth table of the output of a specific record.
# This can be useful to debug output expressions.
fsm_prog.print_truth_table(3) # State with index 3 (the one with `i[0]` output_expr)

# Generate the bitstream for the program
fsm_prog.generate_bitstream('path/to/bitstream.bin')
```

## Brief description of C interface ##

The PFSM has an internal register based memory that is used as a LUT to determine the outputs of each state and the corresponding next state.

An example of some C code is given, with explanations:

```C
// Set PFSM base address and Verilog parameters
pfsm_init(int base_address, uint32_t state_w, uint8_t input_w, uint32_t output_w);

// Writa a 32-bit word to the LUT memory
pfsm_insert_word_lut(int addr, uint8_t word_select, uint32_t value);

// Soft reset
pfsm_reset();

// Get current state of PFSM
uint32_t current_state = pfsm_get_state();

//Program PFSM memories, given a bitstream
uint32_t num_bytes_programmed = pfsm_bitstream_program(char* bitstream);
```
