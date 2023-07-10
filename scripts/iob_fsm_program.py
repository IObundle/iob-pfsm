#!/usr/bin/env python3
# Utilities to program and generate bitstream for iob-pfsm

import math
import struct

class iob_fsm_record:
    '''
    The fsm_record defines a state of the FSM.
    :param label: the state label. Used to reference the state when jumping from another one.
    :param input_cond: the input condition required to jump to the next state.
                       Otherwise will go to current state + 1.
                       Example: "--10" will jump to next_state if input bits [1:0] are 2b'10
    :param next_state: the next state label to jump to when input_cond is met.
    :param output_expr: the output expression for the current state. It may be one of the following types:
                        int: This value will define the constant output value bits
                        str: An expression to be evaluated by python to create the output value. May depend on input bits using the bit list `i`.
                             For example: Expession `2 | (i[1] & i[0])` will create binary output '1x' where `x` is the logical `and` between input bits 0 and 1.
    '''

    def __init__(self, label="", input_cond="", next_state="", output_expr=""):
        self.label = label
        self.input_cond = input_cond
        self.next_state = next_state
        self.output_expr = output_expr
        

class iob_fsm_program:
    '''
    The fsm_program defines the FSM and its states.
    It contains the list of the states and the list of the transitions.
    :param int state_w: number of bits for FSM states.
    :param int input_w: number of bits for FSM inputs.
    :param int output_w: number of bits for FSM outputs.
    :param int data_w: [optional] number of bits in each CPU data word.
    '''

    def __init__(self, state_w: int, input_w: int, output_w: int, data_w=32: int):
        self.state_w = state_w
        self.input_w = input_w
        self.output_w = output_w
        self.data_w = data_w

        self.records = []

    def add_record(self, record: iob_fsm_record):
        '''Add a record to the FSM program.

        :param iob_fsm_record record: the record to add.
        '''
        self.records.append(record)

    def set_records(self, records: list):
        '''Overwrite the list of the records.

        :param list records: the list of the records.
        '''
        self.records = records

    def get_records(self):
        '''Get the list of the records.

        :returns list: the list of the records.
        '''
        return self.records

    def generate_bitstream(self, output_file: str):
        '''Generate the bitstream for the FSM.

        :param str output_file: the output file path.
        '''

        with open(output_file, 'wb') as f:
            # Iterate through every record/state
            for record in range(len(records)):
                # Iterate through every input combination
                for input_comb in range(2**self.input_w):
                    next_state_bits = self.__get_next_state_bits(record, input_comb)
                    output_bits = self.__get_output_bits(record, input_comb)
                    # Pack bytes in big endian order
                    data_bits = struct.pack(">I", (next_state_bits << self.output_w) & output_bits)
                    f.write(data_bits)

            # No need to generate the rest `(2**self.state_w)-len(records)` states, since they are unused.

    def __get_output_bits(self, record: int, input_comb: int):
        '''Generate the output bits based on the current record and the input combination.

        :param int record: the record index.
        :param int input_comb: the current input combination.

        :returns int: output value
        '''
        current_record = self.records[record]
        output_expr = current_record.output_expr
        # Return 0 if output_expr is not set
        if output_expr == "":
            return 0

        # Return the output_expr value if its an int
        if type(output_expr) == int:
            return output_expr

        # If output_expr is string, evaluate that expression
        if type(output_expr) == str:
            return self.__eval_expression(output_expr, input_comb)

        raise Exception(f"Unknown expression {output_expr}")

    def print_truth_table(self, record: int):
        '''Print the truth table for the output_expr of the given record index.

        :param int record: the record index.
        '''
        current_record = self.records[record]
        output_expr = current_record.output_expr
        print(f"Truth table for record [{record}]{current_record.label} with output expression {output_expr}")
        # Iterate through every input combination
        for input_comb in range(2**self.input_w):
            print(f'{bin(input_comb)[2:]}: {self.__eval_expression(output_expr, input_comb)}')

    @staticmethod
    def __eval_expression(expr: str, input_comb: int):
        # Create bit list for inputs
        i = []
        while input_comb > 0:
            bit = input_comb & 1  # Extract the least significant bit
            i.insert(0, bit)  # Add the bit to the beginning of the list
            input_comb >>= 1  # Right shift the value by 1 bit

        eval("return "+expr)

    def __get_next_state_bits(self, record: int, input_comb: int):
        '''Generate the next state bits based on current record and the input combination.

        :param int record: the record index.
        :param int input_comb: the current input combination.

        :returns int: state number to jump to
        '''

        current_record = self.records[record]
        # Return current state + 1, if next_state is not configured
        if current_record.next_state == "":
            return record+1

        relevant_bits = current_record.input_cond.replace("0", "1").replace("-", "0")
        current_combination = input_comb & int(relevant_bits, 2)

        # Check if current input_combination matches the input_condition of this record
        if int(current_record.input_cond.replace("-","0"), 2) == current_combination:
            # Jump to the configured next_state of this record
            return self.__get_state_from_label(current_record.next_state)
        else:
            # Go to current_state + 1
            return record+1

    def __num_data_words_in_lut(self):
        return math.ceil((self.state_w+self.output_w)/self.data_w)

    def __num_bytes_in_lut_aligned(self):
        return self.__num_data_words_in_lut()*(self.data_w/8)

    def __get_state_from_label(self, label: str):
        '''Get the state number by its label.

        :param str label: the state label.
        '''
        for i in range(len(self.records)):
            if self.records[i].label == label:
                return i

        raise Exception(f"State '{label}' not found.")
