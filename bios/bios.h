#ifndef _XEMU_BIOS_H_
#define _XEMU_BIOS_H_

/*
 * ZERO-PAGE is the first page.
 * ROM starts from the second page.
 */
#define ROM_BASE    0x1000
#define FLASH_BASE  0x20000000

/*
 * Based on ROM
 * Head(0x0 ~ 0x100): Relocate dtb, sbi and payload and jump to fw_jump.
 * DTB(0x100 ~ 0x2000): FDT for this platform.
 * SBI(0x2000 ~ 0x20000): SBI fw_jump.
 */
#define DTB_LOAD_ADDR       (ROM_BASE + 0x100)
#define SBI_LOAD_ADDR       (ROM_BASE + 0x2000)

/*
 * Payload(0x100 ~ ): U-boot spl or kernel.
 * Based on FLASH
 */
#define PAYLOAD_LOAD_ADDR   (FLASH_BASE + 0x100)

#define SBI_LINK_ADDR       0x80000000
#define PAYLOAD_LINK_ADDR   0x80200000

#endif  /* _XEMU_BIOS_H_ */
