/*
 * Operation
 */

#include "operation.h"
#include "util.h"

const char *op_names[] = {
    "nop",

    "lui", "auipc",

    "jal", "jalr",

    "beq", "bne", "blt", "bge", "bltu", "bgeu",

    "lb", "lh", "lw", "ld", "lbu", "lhu", "lwu",
    "sb", "sh", "sw", "sd",

    "addi", "slli", "slti", "sltiu", "xori", "srli", "srai", "ori", "andi",
    "addiw", "slliw", "srliw", "sraiw",

    "add", "sub", "sll", "slt", "sltu", "xor", "srl", "sra", "or", "and",
    "addw", "subw", "sllw", "srlw", "sraw",

    "fence", "fence_i",

    "ecall", "ebreak", "uret", "sret", "mret",
    "wfi", "sfence_vma",

    "csrrw", "csrrs", "csrrc",
    "csrrwi", "csrrsi", "csrrci",

    "mul", "mulh", "mulhsu", "mulhu", "div", "divu", "rem", "remu",

    "mulw", "divw", "divuw", "remw", "remuw",

    "amo_add_d", "amo_swap_d", "lr_d", "sc_d",
    "amo_xor_d", "amo_or_d", "amo_and_d",
    "amo_min_d", "amo_max_d", "amo_minu_d", "amo_maxu_d",

    "amo_add_w", "amo_swap_w", "lr_w", "sc_w",
    "amo_xor_w", "amo_or_w", "amo_and_w",
    "amo_min_w", "amo_max_w", "amo_minu_w", "amo_maxu_w",

    "flw", "fsw", "fmv_w_x",

    "fld", "fsd",
};

const char *
op_name(op_t op)
{
    if (op >= OP_MAX_NUM)
        panic("%s: bad op %d\n", __func__, op);

    return op_names[op];
}
