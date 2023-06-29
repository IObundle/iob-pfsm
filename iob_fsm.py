#!/usr/bin/env python3

import os
import sys

from iob_module import iob_module
from setup import setup

# Submodules
from iob_reg import iob_reg
from iob_reg_e import iob_reg_e

class iob_fsm(iob_module):
    name='iob_fsm'
    version="V0.10"
    flows="sim emb"
    setup_dir=os.path.dirname(__file__)

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
        super()._setup_confs([
            # Macros

            # Parameters
            {'name':'DATA_W',      'type':'P', 'val':'32', 'min':'NA', 'max':'32', 'descr':"Data bus width"},
            {'name':'ADDR_W',      'type':'P', 'val':'`IOB_FSM_SWREG_ADDR_W', 'min':'NA', 'max':'NA', 'descr':"Address bus width"},
            {'name':'FSM_W',      'type':'P', 'val':'32', 'min':'NA', 'max':'DATA_W', 'descr':"Number of FSM (can be up to DATA_W)"},
        ])

    @classmethod
    def _setup_ios(cls):
        cls.ios += [
            {'name': 'iob_s_port', 'descr':'CPU native interface', 'ports': [
            ]},
            {'name': 'general', 'descr':'GENERAL INTERFACE SIGNALS', 'ports': [
                {'name':"clk_i" , 'type':"I", 'n_bits':'1', 'descr':"System clock input"},
                {'name':"arst_i", 'type':"I", 'n_bits':'1', 'descr':"System reset, asynchronous and active high"},
                {'name':"cke_i", 'type':"I", 'n_bits':'1', 'descr':"System clock enable signal."},
            ]},
            {'name': 'fsm', 'descr':'', 'ports': [
                {'name':'input_ports', 'type':'I', 'n_bits':'FSM_W', 'descr':'Input interface'},
                {'name':'output_ports', 'type':'O', 'n_bits':'FSM_W', 'descr':'Output interface'},
                {'name':'output_enable', 'type':'O', 'n_bits':'FSM_W', 'descr':'Output Enable interface can be used to tristate outputs on external module'},
            ]}
        ]

    @classmethod
    def _setup_regs(cls):
        cls.regs += [
            {'name': 'fsm', 'descr':'FSM software accessible registers.', 'regs': [
                {'name':"FSM_INPUT", 'type':"R", 'n_bits':32, 'rst_val':0, 'addr':-1, 'log2n_items':0, 'autologic':True, 'descr':"32 bits: 1 bit for value of each FSM input."},
                {'name':"FSM_OUTPUT", 'type':"W", 'n_bits':32, 'rst_val':0, 'addr':-1, 'log2n_items':0, 'autologic':True, 'descr':"32 bits: 1 bit for value of each FSM output."},
                {'name':"FSM_OUTPUT_ENABLE", 'type':"W", 'n_bits':32, 'rst_val':0, 'addr':-1, 'log2n_items':0, 'autologic':True, 'descr':'32 bits: 1 bit for each FSM. Bits with "1" are driven with output value, bits with "0" are in tristate.'},
            ]}
        ]

    @classmethod
    def _setup_block_groups(cls):
        cls.block_groups += []
