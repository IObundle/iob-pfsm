#!/usr/bin/env python3

import os
import sys

from iob_module import iob_module
from setup import setup

# Submodules
from iob_reg import iob_reg
from iob_reg_e import iob_reg_e


class iob_pfsm(iob_module):
    name = "iob_pfsm"
    version = "V0.10"
    flows = "sim emb"
    setup_dir = os.path.dirname(__file__)

    @classmethod
    def _run_setup(cls):
        # Hardware headers & modules
        iob_module.generate("iob_s_port")
        iob_module.generate("iob_s_portmap")
        iob_reg.setup()
        iob_reg_e.setup()

        cls._setup_confs()
        cls._setup_ios()
        cls._setup_regs()
        cls._setup_block_groups()

        # Verilog modules instances
        # TODO

        # Copy sources of this module to the build directory
        super()._run_setup()

        # Setup core using LIB function
        setup(cls)

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
                    "val": "NA",
                    "min": "NA",
                    "max": "NA",
                    "descr": "Width of the input bus.",
                },
                {
                    "name": "OUTPUT_W",
                    "type": "P",
                    "val": "NA",
                    "min": "NA",
                    "max": "NA",
                    "descr": "Width of the ouput bus.",
                },
                {
                    "name": "STATES_W",
                    "type": "P",
                    "val": "NA",
                    "min": "NA",
                    "max": "NA",
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
                        "name": "STATE_RESET",
                        "type": "W",
                        "n_bits": 1,
                        "rst_val": 0,
                        "addr": -1,
                        "log2n_items": 0,
                        "autologic": True,
                        "descr": "Reset PFSM to state 0.",
                    },
                    {
                        "name": "MEMORY",
                        "type": "W",
                        "n_bits": 32,
                        "rst_val": 0,
                        "addr": -1,
                        "log2n_items": "STATES_W+INPUT_W+clog((STATE_W+OUTPUT_W)/DATA_W)", # Memory size (State_W + Input_w) + bits to select data word in each memory address (clog((State_w + OUTPUT_W)/DATA_W))
                        "autologic": False,
                        "descr": 'Programmable memory (used as LUT) for PFSM.',
                    },
                ],
            }
        ]

    @classmethod
    def _setup_block_groups(cls):
        cls.block_groups += []
