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
#include "regfile.h"
#include "mmu.h"
#include "trap.h"
#include "trace.h"

#define VIRTIO_MMIO_AS_START_0  0x0000000010001000
#define VIRTIO_MMIO_AS_END_0    0x0000000010001FFF


uint32_t
fetch(address_space *as, uint64_t pc, int *except)
{
    uint32_t lo;
    uint32_t hi;

    if ((pc + 2) & (PAGE_SIZE - 1UL))
        return read(as, pc, 4, 0, except);

    lo = read(as, pc, 2, 0, except);
    if (*except)
        return 0;

    hi = read(as, pc + 2, 2, 0, except);
    if (*except)
        return 0;

    return ((hi << 16) | lo);
}

int
main()
{
    int i;
    device_t *rom;
    device_t *flash;
    address_space root_as;
    uint32_t inst;
    uint64_t pc = 0x1000;

    fprintf(stderr, "XEMU startup ...\n");

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

    uart_init(&root_as);

    for (i = 0; i < 8; i++) {
        virtio_mmio_init(&root_as,
                         VIRTIO_MMIO_AS_START_0 + i * 0x1000,
                         VIRTIO_MMIO_AS_END_0 + i * 0x1000);
    }

    while (1) {
        uint64_t next_pc;
        uint64_t new_pc;

        op_t      op;
        uint32_t  rd;
        uint32_t  rs1;
        uint32_t  rs2;
        uint64_t  imm;
        uint32_t  csr_addr;
        int except = 0;

        /* Fetch */
        inst = fetch(&root_as, pc, &except);

        if (except) {
            pc = trap_enter(pc, CAUSE_INST_PAGE_FAULT, pc);
            continue;
        }

        /* Decode */
        next_pc = pc + decode(inst, &op, &rd, &rs1, &rs2, &imm, &csr_addr);

        trace_decode(pc, op, rd, rs1, rs2, imm, csr_addr);

        /* Execute */
        new_pc = execute(&root_as, pc, next_pc,
                         op, rd, rs1, rs2, imm, csr_addr);

        trace_execute(pc, op, rd, rs1, rs2, imm, csr_addr);

        pc = new_pc ? new_pc : next_pc;
    }

    return 0;
}
