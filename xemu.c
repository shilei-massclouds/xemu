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
#include "system_map.h"
#include "virtio.h"
#include "trace.h"

#define VIRTIO_MMIO_AS_START_0  0x0000000010001000UL
#define VIRTIO_MMIO_AS_END_0    0x0000000010001FFFUL

extern address_space root_as;

uint64_t _pc = 0x1000;
const char *vda_filename = "./image/test.raw";

uint64_t
fetch(address_space *as, uint32_t *inst)
{
    uint32_t lo;
    uint32_t hi;

    bool has_except = false;

    if ((_pc + 2) & (PAGE_SIZE - 1UL)) {
        *inst = (uint32_t)as_read(as, _pc, 4, 0, &has_except);
        if (has_except)
            return raise_except(_pc, CAUSE_INST_PAGE_FAULT, _pc);

        return 0;
    }

    lo = (uint32_t)as_read(as, _pc, 2, 0, &has_except);
    if (has_except)
        return raise_except(_pc, CAUSE_INST_PAGE_FAULT, _pc);

    hi = (uint32_t)as_read(as, _pc + 2, 2, 0, &has_except);
    if (has_except)
        return raise_except(_pc, CAUSE_INST_PAGE_FAULT, _pc);

    *inst = ((hi << 16) | lo);
    return 0;
}

int
main(void)
{
    uint64_t i;
    device_t *rom;
    device_t *flash;

    printf("[XEMU startup ...]\n");

    setup_system_map();
    setup_trace_table();

    /* Init root address space */
    init_address_space(&root_as,
                       ROOT_ADDRESS_SPACE_START,
                       ROOT_ADDRESS_SPACE_END);

    /* Init CSR */
    csr_init();

    cpu_enable_clock();

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
        uint32_t  opcode;

        uint64_t next_pc = 0;
        uint32_t inst = 0;

        next_pc = handle_interrupt(_pc);
        if (next_pc) {
            /* An interrupt occurs */
            _pc = next_pc;
            continue;
        }

        /* Fetch */
        next_pc = fetch(&root_as, &inst);
        if (next_pc) {
            /* An except occurs during fetch */
            _pc = next_pc;
            continue;
        }

        /* Decode */
        next_pc = decode(_pc, inst, &op, &rd, &rs1, &rs2, &imm,
                         &csr_addr, &opcode);

        /* Execute */
        _pc = execute(&root_as, _pc, next_pc,
                      op, rd, rs1, rs2, imm,
                      csr_addr, opcode);
    }

    return 0;
}
