`timescale 1ns/1ps

`include "iob_lib.vh"
`include "iob_pfsm_conf.vh"
`include "iob_pfsm_swreg_def.vh"

`define IOB_PFSM_CEIL_DIV(A, B) (A % B == 0 ? (A / B) : ((A/B) + 1))

module iob_pfsm # (
     `include "iob_pfsm_params.vs"
   ) (
     `include "iob_pfsm_io.vs"
    );
    // This mapping is required because "iob_pfsm_swreg_inst.vs" uses "iob_s_portmap.vs" (This would not be needed if mkregs used "iob_s_s_portmap.vs" instead)
    wire [1-1:0] iob_avalid = iob_avalid_i; //Request valid.
    wire [ADDR_W-1:0] iob_addr = iob_addr_i; //Address.
    wire [DATA_W-1:0] iob_wdata = iob_wdata_i; //Write data.
    wire [(DATA_W/8)-1:0] iob_wstrb = iob_wstrb_i; //Write strobe.
    wire [1-1:0] iob_rvalid; assign iob_rvalid_o = iob_rvalid; //Read data valid.
    wire [DATA_W-1:0] iob_rdata; assign iob_rdata_o = iob_rdata; //Read data.
    wire [1-1:0] iob_ready; assign iob_ready_o = iob_ready; //Interface ready.

    //BLOCK Register File & Configuration control and status register file.
    `include "iob_pfsm_swreg_inst.vs"

    // Keep track of current PFSM state
    wire [STATE_W-1:0] next_state, state_reg_o, jump_state;
    iob_reg_r #(
      .DATA_W (STATE_W),
      .RST_VAL(1'b0)
    ) state_reg (
      .clk_i(clk_i),
      .arst_i(arst_i),
      .rst_i(SOFTRESET),
      .cke_i(cke_i),
      .data_i(next_state),
      .data_o(state_reg_o)
    );

    // If this wire has any bits set, then should jump to jump_state
    wire [2**STATE_W-1:0] jump_enabled;
    // Go to either jump_state of state+1, based on value of jump_enabled
    assign next_state = |jump_enabled ? jump_state : state_reg_o + 1'b1;

    // Number of bytes in IOb-Native data bus
    localparam N_BYTES_DATA_WORD = `IOB_PFSM_CEIL_DIV(DATA_W,8);
    // Number of bytes in each LUT memory word
    localparam N_BYTES_LUT_WORD = `IOB_PFSM_CEIL_DIV((STATE_W+OUTPUT_W),8);

    // LUT data input signal
    wire [N_BYTES_LUT_WORD*8-1:0] lut_data_input = {`IOB_PFSM_CEIL_DIV(STATE_W+OUTPUT_W,DATA_W){iob_wdata}};

    // LUT byte write enable (based on selected word via STATE_MEM_WORD_SELECT)
    wire [N_BYTES_LUT_WORD-1:0] lut_w_en = {N_BYTES_DATA_WORD{1'b1}}<<(N_BYTES_DATA_WORD*STATE_MEM_WORD_SELECT);

    wire [N_BYTES_LUT_WORD*8-1:0] states_lut_o;
   // Memory used as LUT for PFSM states
   iob_ram_2p_be #(
      .DATA_W(N_BYTES_LUT_WORD*8),
      .ADDR_W(STATE_W)
   ) states_lut (
      .clk_i(clk_i),
      .w_en_i  ({N_BYTES_LUT_WORD{STATES_MEMORY_wen}} & lut_w_en),
      .w_data_i(lut_data_input),
      .w_addr_i((iob_addr-`IOB_PFSM_STATES_MEMORY_ADDR)>>$clog2(N_BYTES_DATA_WORD)),
      .r_addr_i(next_state),
      .r_en_i  (1'b1),
      .r_data_o(states_lut_o)
   );

   // Extract bits from word of `states_lut_o`
   assign output_ports = states_lut_o[0+:OUTPUT_W]; // Least significant bits connected to output
   assign jump_state = states_lut_o[OUTPUT_W+:STATE_W]; // Followed by jump_state bits
   // Most significant bits of states_lut_o (if any) are ignored

    wire [2**STATE_W-1:0] condition_lut_o;

   // Generate a condition LUT for each state
   generate
      genvar i;
      for (i = 0; i < 2**STATE_W; i = i + 1) begin: gen_condition_luts
         // Memory used as LUT for condition of PFSM state
         iob_ram_2p #(
            .DATA_W(1),
            .ADDR_W(INPUT_W)
         ) condition_lut (
            .clk_i(clk_i),
            .w_en_i  (CONDITION_MEMORY_wen && i/DATA_W == (iob_addr-`IOB_PFSM_CONDITION_MEMORY_ADDR)), // Enable if this lut belongs to the range of states selected via the addr
            .w_data_i(iob_wdata[i%DATA_W]), // Write bit corresponding to this state condition
            .w_addr_i(CONDITION_COMB), // Select which combination of inputs we are writing to
            .r_addr_i(input_ports), // Select which input combination is active
            .r_en_i  (1'b1),
            .r_data_o(condition_lut_o[i])
         );
         // Enable jump if this LUT output is set and its corresponding state is selected
         assign jump_enabled[i] = condition_lut_o[i] && state_reg_o==i;
      end
   endgenerate


endmodule
