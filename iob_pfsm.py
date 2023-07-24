#!/usr/bin/env python3

import os
import shutil

from iob_module import iob_module

# Submodules
from iob_reg import iob_reg
from iob_reg_e import iob_reg_e


class iob_pfsm(iob_module):
    name = "iob_pfsm"
    version = "V0.10"
    flows = "sim emb"
    setup_dir = os.path.dirname(__file__)

    @classmethod
    def _create_submodules_list(cls):
        ''' Create submodules list with dependencies of this module
        '''
        super()._create_submodules_list([
            {"interface": "iob_s_port"},
            {"interface": "iob_s_portmap"},
            iob_reg,
            iob_reg_e,
        ])

    @classmethod
    def _specific_setup(cls):
        # Verilog modules instances
        # TODO

        # Copy iob_pfsm_program.py script to the build directory
        os.makedirs(os.path.join(cls.build_dir, "scripts"), exist_ok=True)
        shutil.copy(
            os.path.join(cls.setup_dir, "scripts/iob_pfsm_program.py"),
            os.path.join(cls.build_dir, "scripts/"),
        )

    @classmethod
    def _setup_confs(cls):
        super()._setup_confs(
            [
                # Macros
                # Parameters
                {
                    "name": "DATA_W",
                    "type": "P",
                    "val": "32",
                    "min": "NA",
                    "max": "32",
                    "descr": "Data bus width",
                },
                {
                    "name": "ADDR_W",
                    "type": "P",
                    "val": "`IOB_PFSM_SWREG_ADDR_W",
                    "min": "NA",
                    "max": "NA",
                    "descr": "Address bus width",
                },
                {
                    "name": "INPUT_W",
                    "type": "P",
                    "val": "1",
                    "min": "NA",
                    "max": "4",
                    "descr": "Width of the input bus.",
                },
                {
                    "name": "OUTPUT_W",
                    "type": "P",
                    "val": "16",
                    "min": "NA",
                    "max": "32",
                    "descr": "Width of the ouput bus.",
                },
                {
                    "name": "STATE_W",
                    "type": "P",
                    "val": "4",
                    "min": "NA",
                    "max": "10",
                    "descr": "Number of PFSM states (log2).",
                },
            ]
        )

    @classmethod
    def _setup_ios(cls):
        cls.ios += [
            {"name": "iob_s_port", "descr": "CPU native interface", "ports": []},
            {
                "name": "general",
                "descr": "General interface signals",
                "ports": [
                    {
                        "name": "clk_i",
                        "type": "I",
                        "n_bits": "1",
                        "descr": "System clock input",
                    },
                    {
                        "name": "arst_i",
                        "type": "I",
                        "n_bits": "1",
                        "descr": "System reset, asynchronous and active high",
                    },
                    {
                        "name": "cke_i",
                        "type": "I",
                        "n_bits": "1",
                        "descr": "System clock enable signal.",
                    },
                ],
            },
            {
                "name": "pfsm",
                "descr": "",
                "ports": [
                    {
                        "name": "input_ports",
                        "type": "I",
                        "n_bits": "INPUT_W",
                        "descr": "PFSM input interface",
                    },
                    {
                        "name": "output_ports",
                        "type": "O",
                        "n_bits": "OUTPUT_W",
                        "descr": "PFSM output interface",
                    },
                ],
            },
        ]

    @classmethod
    def _setup_regs(cls):
        cls.regs += [
            {
                "name": "PFSM",
                "descr": "Programmable FSM software accessible registers.",
                "regs": [
                    {
                        "name": "MEMORY",
                        "type": "W",
                        "n_bits": "DATA_W",
                        "rst_val": 0,
                        "addr": -1,
                        "log2n_items": "INPUT_W+STATE_W",
                        "autologic": False,
                        "descr": "Write word to the PFSM programmable memory (used as LUT). If LUT word is greater than DATA_W, use MEM_WORD_SELECT to select which DATA_W word we are writing to the LUT memory.",
                    },
                    {
                        "name": "MEM_WORD_SELECT",
                        "type": "W",
                        # Number of `DATA_W` words in each state_mem word = Ceiling division of state_mem_word_w by data_w
                        # Number of bits required to represent these words = $clog2( number_of_words )
                        # Register must have at least 1 bit = `IOB_MAX( number_of_bits, 1 )
                        "n_bits": "`IOB_MAX($clog2((STATE_W+OUTPUT_W+DATA_W-1)/DATA_W),1)",
                        "rst_val": 0,
                        "addr": -1,
                        "log2n_items": 0,
                        "autologic": True,
                        "descr": "If the LUT memory word is greater than DATA_W, use this to select which DATA_W word we are writing to the address of the LUT memory.",
                    },
                    {
                        "name": "SOFTRESET",
                        "type": "W",
                        "n_bits": 1,
                        "rst_val": 0,
                        "addr": -1,
                        "log2n_items": 0,
                        "autologic": True,
                        "descr": "Reset PFSM to state 0.",
                    },
                    {
                        "name": "CURRENT_STATE",
                        "type": "R",
                        "n_bits": "STATE_W",
                        "rst_val": 0,
                        "addr": -1,
                        "log2n_items": 0,
                        "autologic": True,
                        "descr": "Read current state of PFSM.",
                    },
                ],
            }
        ]

    @classmethod
    def _setup_block_groups(cls):
        cls.block_groups += []
