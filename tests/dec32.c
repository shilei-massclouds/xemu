`include "isa.vh"

module dec32 (
    input wire [31:0]   inst,
    output wire [4:0]   rd,
    output wire [4:0]   rs1,
    output wire [4:0]   rs2,
    output wire [63:0]  imm,
    output wire         with_imm,
    alu_ops.src         alu_ops,
    io_ops.src          io_ops,
    bj_ops.src          bj_ops,
    sys_ops.src         sys_ops
);

    wire [6:0] op = inst[6:0];
    wire [2:0] funct3 = inst[14:12];
    assign rs1 = inst[19:15];
    assign rs2 = inst[24:20];
    wire [6:0] funct7 = inst[31:25];

    wire [63:0] i_imm = {{52{inst[31]}}, inst[31:20]};
    wire [63:0] s_imm = {{52{inst[31]}}, inst[31:25], inst[11:7]};
    wire [63:0] b_imm = {{51{inst[31]}}, inst[31], inst[7],
                         inst[30:25], inst[11:8], 1'b0};
    wire [63:0] u_imm = {{32{inst[31]}}, inst[31:12], 12'b0};
    wire [63:0] j_imm = {{43{inst[31]}}, inst[31], inst[19:12],
                         inst[20], inst[30:21], 1'b0};
    wire [63:0] c_imm = {59'b0, inst[19:15]};

    wire [63:0] m_imm6 = {58'b0, inst[25:20]};
    wire [63:0] m_imm5 = {59'b0, inst[24:20]};

    wire op_imm     = (op == `OP_IMM);
    wire op_reg     = (op == `OP_REG);
    wire op_br      = (op == `OP_BRANCH);
    wire op_load    = (op == `OP_LOAD); 
    wire op_store   = (op == `OP_STORE);
    wire op_jal     = (op == `OP_JAL);
    wire op_jalr    = (op == `OP_JALR);
    wire op_lui     = (op == `OP_LUI);
    wire op_auipc   = (op == `OP_AUIPC);
    wire op_system  = (op == `OP_SYSTEM);
    wire op_misc    = (op == `OP_MISC);
    wire op_amo     = (op == `OP_AMO);
    wire op_imm_w   = (op == `OP_IMM_W);
    wire op_reg_w   = (op == `OP_REG_W);

    wire rv_addi  = op_imm & (funct3 == 3'b000);
    wire rv_slti  = op_imm & (funct3 == 3'b010);
    wire rv_sltiu = op_imm & (funct3 == 3'b011);
    wire rv_xori  = op_imm & (funct3 == 3'b100);
    wire rv_ori   = op_imm & (funct3 == 3'b110);
    wire rv_andi  = op_imm & (funct3 == 3'b111);

    wire rv_slli = op_imm & (funct3 == 3'b001) & (funct7[6:1] == 6'b000000);
    wire rv_srli = op_imm & (funct3 == 3'b101) & (funct7[6:1] == 6'b000000);
    wire rv_srai = op_imm & (funct3 == 3'b101) & (funct7[6:1] == 6'b010000);

    wire rv_addiw = op_imm_w & (funct3 == 3'b000);
    wire rv_slliw = op_imm_w & (funct3 == 3'b001) & (funct7 == 7'b0000000);
    wire rv_srliw = op_imm_w & (funct3 == 3'b101) & (funct7 == 7'b0000000);
    wire rv_sraiw = op_imm_w & (funct3 == 3'b101) & (funct7 == 7'b0100000);

    wire rv_add  = op_reg & (funct3 == 3'b000) & (funct7 == 7'b0000000);
    wire rv_sub  = op_reg & (funct3 == 3'b000) & (funct7 == 7'b0100000);

    wire rv_addw = op_reg_w & (funct3 == 3'b000) & (funct7 == 7'b0000000);
    wire rv_subw = op_reg_w & (funct3 == 3'b000) & (funct7 == 7'b0100000);

    wire rv_xor  = op_reg & (funct3 == 3'b100) & (funct7 == 7'b0000000);
    wire rv_or   = op_reg & (funct3 == 3'b110) & (funct7 == 7'b0000000);
    wire rv_and  = op_reg & (funct3 == 3'b111) & (funct7 == 7'b0000000);

    wire rv_sll  = op_reg & (funct3 == 3'b001) & (funct7 == 7'b0000000);
    wire rv_srl  = op_reg & (funct3 == 3'b101) & (funct7 == 7'b0000000);
    wire rv_sra  = op_reg & (funct3 == 3'b101) & (funct7 == 7'b0100000);

    wire rv_sllw = op_reg_w & (funct3 == 3'b001) & (funct7 == 7'b0000000);
    wire rv_srlw = op_reg_w & (funct3 == 3'b101) & (funct7 == 7'b0000000);
    wire rv_sraw = op_reg_w & (funct3 == 3'b101) & (funct7 == 7'b0100000);

    wire rv_slt  = op_reg & (funct3 == 3'b010) & (funct7 == 7'b0000000);
    wire rv_sltu = op_reg & (funct3 == 3'b011) & (funct7 == 7'b0000000);

    wire rv_beq  = op_br & (funct3 == 3'b000);
    wire rv_bne  = op_br & (funct3 == 3'b001);
    wire rv_blt  = op_br & (funct3 == 3'b100);
    wire rv_bge  = op_br & (funct3 == 3'b101);
    wire rv_bltu = op_br & (funct3 == 3'b110);
    wire rv_bgeu = op_br & (funct3 == 3'b111);

    wire rv_ecall   = op_system & (funct3 == 3'b000) &
                      (inst[31:20] == 12'b0000000_00000);
    wire rv_ebreak  = op_system & (funct3 == 3'b000) &
                      (inst[31:20] == 12'b0000000_00001);
    wire rv_mret    = op_system & (funct3 == 3'b000) &
                      (inst[31:20] == 12'b0011000_00010);
    wire rv_wfi     = op_system & (funct3 == 3'b000) &
                      (inst[31:20] == 12'b0001000_00101);

    wire rv_csrrw   = op_system & (funct3 == 3'b001);
    wire rv_csrrs   = op_system & (funct3 == 3'b010);
    wire rv_csrrc   = op_system & (funct3 == 3'b011);
    wire rv_csrrwi  = op_system & (funct3 == 3'b101);
    wire rv_csrrsi  = op_system & (funct3 == 3'b110);
    wire rv_csrrci  = op_system & (funct3 == 3'b111);

    wire rv_fence   = op_misc & (funct3 == 3'b000);
    wire rv_fence_i = op_misc & (funct3 == 3'b001);

    wire ra_amo_add  = op_amo & (funct7[6:2] == 5'b00000);
    wire ra_amo_swap = op_amo & (funct7[6:2] == 5'b00001);
    wire ra_lr       = op_amo & (funct7[6:2] == 5'b00010);
    wire ra_sc       = op_amo & (funct7[6:2] == 5'b00011);
    wire ra_amo_xor  = op_amo & (funct7[6:2] == 5'b00100);
    wire ra_amo_or   = op_amo & (funct7[6:2] == 5'b01000);
    wire ra_amo_and  = op_amo & (funct7[6:2] == 5'b01100);
    wire ra_amo_min  = op_amo & (funct7[6:2] == 5'b10000);
    wire ra_amo_max  = op_amo & (funct7[6:2] == 5'b10100);
    wire ra_amo_minu = op_amo & (funct7[6:2] == 5'b11000);
    wire ra_amo_maxu = op_amo & (funct7[6:2] == 5'b11100);

    wire rm_mul     = op_reg & (funct7 == 7'b0000001) & (funct3 == 3'b000);
    wire rm_mulh    = op_reg & (funct7 == 7'b0000001) & (funct3 == 3'b001);
    wire rm_mulhsu  = op_reg & (funct7 == 7'b0000001) & (funct3 == 3'b010);
    wire rm_mulhu   = op_reg & (funct7 == 7'b0000001) & (funct3 == 3'b011);
    wire rm_div     = op_reg & (funct7 == 7'b0000001) & (funct3 == 3'b100);
    wire rm_divu    = op_reg & (funct7 == 7'b0000001) & (funct3 == 3'b101);
    wire rm_rem     = op_reg & (funct7 == 7'b0000001) & (funct3 == 3'b110);
    wire rm_remu    = op_reg & (funct7 == 7'b0000001) & (funct3 == 3'b111);

    wire rm_mulw    = op_reg_w & (funct7 == 7'b0000001) & (funct3 == 3'b000);
    wire rm_divw    = op_reg_w & (funct7 == 7'b0000001) & (funct3 == 3'b100);
    wire rm_divuw   = op_reg_w & (funct7 == 7'b0000001) & (funct3 == 3'b101);
    wire rm_remw    = op_reg_w & (funct7 == 7'b0000001) & (funct3 == 3'b110);
    wire rm_remuw   = op_reg_w & (funct7 == 7'b0000001) & (funct3 == 3'b111);

    assign rd = {5{~(op_br | op_store)}} & inst[11:7];

    assign imm = ({64{(op_imm | op_imm_w | op_jalr | op_load)}} & i_imm) |
                 ({64{op_store}} & s_imm) | ({64{op_br}} & b_imm) |
                 ({64{(op_lui | op_auipc)}} & u_imm) | ({64{op_jal}} & j_imm) |
                 ({64{op_system}} & c_imm) |
                 ({64{rv_slli|rv_srli|rv_srai}} & m_imm6) |
                 ({64{rv_slliw|rv_srliw|rv_sraiw}} & m_imm5);

    assign with_imm = ~(op_jal | op_br) & ~op_reg & ~op_reg_w & ~op_amo &
                      ~(rv_csrrw | rv_csrrs | rv_csrrc) &
                      ~(ra_lr | ra_sc);

    assign alu_ops.add_op    = rv_add | rv_addw | rv_addi | rv_addiw |
                               op_load | op_store;
    assign alu_ops.sub_op    = rv_sub | rv_subw;
    assign alu_ops.and_op    = rv_and | rv_andi;
    assign alu_ops.or_op     = rv_or | rv_ori;
    assign alu_ops.xor_op    = rv_xor | rv_xori;
    assign alu_ops.sll_op    = rv_sll | rv_slli | rv_sllw | rv_slliw;
    assign alu_ops.srl_op    = rv_srl | rv_srli | rv_srlw | rv_srliw;
    assign alu_ops.sra_op    = rv_sra | rv_srai | rv_sraw | rv_sraiw;
    assign alu_ops.slt_op    = rv_slt | rv_sltu | rv_slti | rv_sltiu;
    assign alu_ops.lui_op    = op_lui;
    assign alu_ops.auipc_op  = op_auipc;
    assign alu_ops.mul_op    = rm_mul | rm_mulw;
    assign alu_ops.mulh_op   = rm_mulh | rm_mulhu;
    assign alu_ops.mulhsu_op = rm_mulhsu;
    assign alu_ops.div_op    = rm_div | rm_divu | rm_divw | rm_divuw;
    assign alu_ops.rem_op    = rm_rem | rm_remu | rm_remw | rm_remuw;
    assign alu_ops.is_unsign = rv_sltu | rv_sltiu | rv_bltu | rv_bgeu |
                               rm_mulhu | rm_divu | rm_remu |
                               rm_divuw | rm_remuw;
    assign alu_ops.is_word   = op_imm_w | op_reg_w;

    assign io_ops.load_op = op_load;
    assign io_ops.store_op = op_store;
    assign io_ops.amo_add_op = ra_amo_add;
    assign io_ops.amo_swap_op = ra_amo_swap;
    assign io_ops.lr_op = ra_lr;
    assign io_ops.sc_op = ra_sc;
    assign io_ops.amo_xor_op = ra_amo_xor;
    assign io_ops.amo_or_op = ra_amo_or;
    assign io_ops.amo_and_op = ra_amo_and;
    assign io_ops.amo_min_op = ra_amo_min;
    assign io_ops.amo_max_op = ra_amo_max;
    assign io_ops.amo_minu_op = ra_amo_minu;
    assign io_ops.amo_maxu_op = ra_amo_maxu;
    assign io_ops.size = funct3;
    assign io_ops.mask = {{4{funct3[1]&funct3[0]}}, {2{funct3[1]}},
                          funct3[1]|funct3[0], 1'b1};

    assign bj_ops.beq_op    = rv_beq;
    assign bj_ops.bne_op    = rv_bne;
    assign bj_ops.blt_op    = rv_blt;
    assign bj_ops.bge_op    = rv_bge;
    assign bj_ops.bltu_op   = rv_bltu;
    assign bj_ops.bgeu_op   = rv_bgeu;
    assign bj_ops.jal_op    = op_jal;
    assign bj_ops.jalr_op   = op_jalr;

    assign sys_ops.ecall_op  = rv_ecall;
    assign sys_ops.ebreak_op = rv_ebreak;
    assign sys_ops.mret_op   = rv_mret;
    assign sys_ops.wfi_op    = rv_wfi;

    assign sys_ops.csrrw_op = rv_csrrw | rv_csrrwi;
    assign sys_ops.csrrs_op = rv_csrrs | rv_csrrsi;
    assign sys_ops.csrrc_op = rv_csrrc | rv_csrrci;

    assign sys_ops.csr_addr = inst[31:20];

endmodule
