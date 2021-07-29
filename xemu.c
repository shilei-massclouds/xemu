/*
 * xemu entry
 */

#include <stdio.h>

#include "address_space.h"
#include "isa.h"
#include "operation.h"
#include "decode.h"
#include "util.h"
#include "device.h"
#include "execute.h"
#include "csr.h"
#include "mmu.h"
#include "trap.h"
#include "trace.h"
#include "virtio.h"

#define VIRTIO_MMIO_AS_START_0  0x0000000010001000
#define VIRTIO_MMIO_AS_END_0    0x0000000010001FFF

extern address_space root_as;

const char *vda_filename = "./image/test.raw";

uint64_t
fetch(address_space *as, uint64_t pc, uint32_t *inst)
{
    uint32_t lo;
    uint32_t hi;

    bool has_except = false;

    if ((pc + 2) & (PAGE_SIZE - 1UL)) {
        *inst = read(as, pc, 4, 0, &has_except);
        if (has_except)
            return trap_enter(pc, CAUSE_INST_PAGE_FAULT, pc);

        return 0;
    }

    lo = read(as, pc, 2, 0, &has_except);
    if (has_except)
        return trap_enter(pc, CAUSE_INST_PAGE_FAULT, pc);

    hi = read(as, pc + 2, 2, 0, &has_except);
    if (has_except)
        return trap_enter(pc, CAUSE_INST_PAGE_FAULT, pc);

    *inst = ((hi << 16) | lo);
    return 0;
}

int
main()
{
    int i;
    device_t *rom;
    device_t *flash;
    uint64_t pc = 0x1000;

    printf("[XEMU startup ...]\n");

    /* Init root address space */
    init_address_space(&root_as,
                       ROOT_ADDRESS_SPACE_START,
                       ROOT_ADDRESS_SPACE_END);

    /* Init CSR */
    csr_init();

    rtc_init(&root_as);
    pci_host_init(&root_as);

    plic_init(&root_as);
    clint_init(&root_as);

    rom = rom_init(&root_as);
    rom_add_file(rom, "image/bios.bin", 0);
    rom_add_file(rom, "image/virt.dtb", 0x100);
    rom_add_file(rom, "image/fw_jump.bin", 0x2000);

    flash = flash_init(&root_as);
    flash_add_file(flash, "image/payload.bin", 0x100);

    ram_init(&root_as);

    uart_init(&root_as, 0xa);

    for (i = 0; i < 8; i++) {
        device_t *vdev;
        vdev = virtio_mmio_init(&root_as,
                                VIRTIO_MMIO_AS_START_0 + i * 0x1000,
                                VIRTIO_MMIO_AS_END_0 + i * 0x1000);

        if (i == 0) {
            virtio_dev_t *blk = virtio_blk_init(vda_filename, i + 1);
            virtio_set_backend(vdev, blk);
        }
    }

    while (1) {
        op_t      op;
        uint32_t  rd;
        uint32_t  rs1;
        uint32_t  rs2;
        uint64_t  imm;
        uint32_t  csr_addr;

        uint64_t next_pc = 0;
        uint64_t new_pc = 0;
        uint32_t inst = 0;

        /* Fetch */
        next_pc = fetch(&root_as, pc, &inst);
        if (next_pc) {
            /* An except occurs during fetch */
            pc = next_pc;
            continue;
        }

        /* Decode */
        next_pc = decode(pc, inst, &op, &rd, &rs1, &rs2, &imm, &csr_addr);

        trace_decode(pc, op, rd, rs1, rs2, imm, csr_addr);

        /* Execute */
        new_pc = execute(&root_as, pc, next_pc,
                         op, rd, rs1, rs2, imm, csr_addr);

        trace_execute(pc, op, rd, rs1, rs2, imm, csr_addr);

        if (new_pc) {
            pc = new_pc;
            continue;
        }

        pc = next_pc;

        if (check_interrupt()) {
            uint64_t cause;
            if (priv == S_MODE)
                cause = CAUSE_S_EXTERNAL_INTR;
            else if (priv == M_MODE)
                cause = CAUSE_M_EXTERNAL_INTR;
            else
                cause = CAUSE_U_EXTERNAL_INTR;

            pc = trap_enter(pc, cause, 0);
        }
    }

    return 0;
}
